/*   2log.io
 *   Copyright (C) 2021 - 2log.io | mail@2log.io,  sascha@2log.io
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GPIOLEDControl.h"

#include <driver/gpio.h>
#include <MutexLocker.h>

extern "C"
{
    #include <freertos/FreeRTOS.h>
    #include <esp_log.h>

    #ifdef CONFIG_IDF_TARGET_ESP8266
        #include <driver/hw_timer.h>
    #endif
}

namespace
{
    const char* LOG_TAG = "IDFix::GPIOLEDControl";
}

namespace IDFix
{
	namespace IODevices
	{
        GPIOLEDControl::GPIOLEDControl(uint8_t pin, bool inverted) : _pin(pin), _inverted(inverted)
        {
            gpio_config_t ledPin;

            ledPin.intr_type		= GPIO_INTR_DISABLE;
            ledPin.pin_bit_mask     = (BIT(pin));
            ledPin.mode             = GPIO_MODE_OUTPUT;
            ledPin.pull_down_en     = GPIO_PULLDOWN_DISABLE;
            ledPin.pull_up_en       = GPIO_PULLUP_DISABLE;

            if( gpio_config(&ledPin) != ESP_OK)
            {
                ESP_LOGE(LOG_TAG, "Failed to set GPIO config for LED");
            }

            #ifdef CONFIG_IDF_TARGET_ESP32

                _blinkTimer = xTimerCreate("blink_timer", pdMS_TO_TICKS(portMAX_DELAY), pdFALSE, static_cast<void*>(this), &GPIOLEDControl::blinkTimeWrapper);

                if ( _blinkTimer == nullptr )
                {
                    ESP_LOGE(LOG_TAG, "Failed to create blink timer");
                }

            #else

                hw_timer_init(&GPIOLEDControl::blinkTimeWrapper, static_cast<void*>(this) );

            #endif
        }

        GPIOLEDControl::~GPIOLEDControl()
        {
            #ifdef CONFIG_IDF_TARGET_ESP32

                xTimerStop(_blinkTimer, 100);
                xTimerDelete(_blinkTimer, 100);

            #else

                hw_timer_disarm();
                hw_timer_deinit();

            #endif
        }

        void GPIOLEDControl::blink(uint16_t interval)
        {
            blinkSequence(interval, interval);
        }

        void GPIOLEDControl::blinkSequence(uint16_t on, uint16_t off)
        {
            volatile MutexLocker locker(_mutex);

            #ifdef CONFIG_IDF_TARGET_ESP32
                xTimerStop(_blinkTimer, 100);
            #else
                hw_timer_disarm();
            #endif

            _blink          = true;
            _on             = true;
            _onInterval     = on;
            _offInterval    = off;

            startBlinking();
        }

        void GPIOLEDControl::setOn(bool on)
        {
            volatile MutexLocker locker(_mutex);

            _blink = false;

            #ifdef CONFIG_IDF_TARGET_ESP32
                xTimerStop(_blinkTimer, 100);
            #else
                hw_timer_disarm();
            #endif

            gpio_set_level( static_cast<gpio_num_t>(_pin), _inverted ? ! on : on);
            _on = on;
        }

        void GPIOLEDControl::setOff()
        {
            setOn(false);
        }

        bool GPIOLEDControl::isBlinking()
        {
            volatile MutexLocker locker(_mutex);

            return _blink;
        }

        bool GPIOLEDControl::isOn()
        {
            volatile MutexLocker locker(_mutex);

            return _on && (! _blink);
        }

        #ifdef CONFIG_IDF_TARGET_ESP32

            void GPIOLEDControl::blinkTimeWrapper(TimerHandle_t xTimer)
            {
                GPIOLEDControl *objectInstance = static_cast<GPIOLEDControl*>( pvTimerGetTimerID(xTimer) );
                objectInstance->blinkTime();
            }

        #else

            void GPIOLEDControl::blinkTimeWrapper(void *instance)
            {
                GPIOLEDControl *objectInstance = static_cast<GPIOLEDControl*>(instance);
                objectInstance->blinkTime();
            }

        #endif

        void GPIOLEDControl::blinkTime()
        {
            _on = ! _on;
            gpio_set_level( static_cast<gpio_num_t>(_pin), _inverted ? ! _on : _on);

            uint16_t nextInterval;

            if ( _on )
            {
                nextInterval = _onInterval;
            }
            else
            {
                nextInterval = _offInterval;
            }

            #ifdef CONFIG_IDF_TARGET_ESP32

                if( xTimerChangePeriod( _blinkTimer, pdMS_TO_TICKS(nextInterval) , 100 ) != pdPASS )
                {
                    ESP_LOGE(LOG_TAG, "Failed to start blink timer!");
                    _blink = false;
                }

            #else

                if( hw_timer_alarm_us(nextInterval * 1000, false) != ESP_OK )
                {
                    ESP_LOGE(LOG_TAG, "Failed to start blink timer!");
                    _blink = false;
                }

            #endif
        }

        void GPIOLEDControl::startBlinking()
        {
            gpio_set_level( static_cast<gpio_num_t>(_pin), _inverted ? ! _on : _on);

            #ifdef CONFIG_IDF_TARGET_ESP32

                if( xTimerChangePeriod( _blinkTimer, pdMS_TO_TICKS(_onInterval) , 100 ) != pdPASS )
                {
                    ESP_LOGE(LOG_TAG, "Failed to start blink timer!");
                    _blink = false;
                }

            #else

               if( hw_timer_alarm_us(_onInterval * 1000, false) != ESP_OK )
                {
                    ESP_LOGE(LOG_TAG, "Failed to start blink timer!");
                    _blink = false;
                }

            #endif
        }


    }
}
