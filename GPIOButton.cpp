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

#include "GPIOButton.h"
#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>

extern "C"
{
    #include <freertos/FreeRTOS.h>
    #include <esp_log.h>
}

namespace
{
    const char* LOG_TAG     = "IDFix::GPIOButton";
    const int   LOOP_DELAY  = 200;
}

namespace IDFix
{
    namespace IODevices
    {
        GPIOButton::GPIOButton(uint8_t pin, bool inverted) : IDFix::Task("GPIO_Button", 3072, 10),
            _inverted(inverted),
            _gpioPin(pin)
        {
            gpio_config_t resetPin;
            resetPin.intr_type      = GPIO_INTR_DISABLE;

            if ( pin <= 31 )
            {
                resetPin.pin_bit_mask   = (BIT(pin));
            }
            else
            {
                resetPin.pin_bit_mask   = ((uint64_t)(((uint64_t)1)<<pin));
            }
            resetPin.mode           = GPIO_MODE_INPUT;
            resetPin.pull_down_en   = GPIO_PULLDOWN_DISABLE;
            resetPin.pull_up_en     = GPIO_PULLUP_ENABLE;
            if( gpio_config(&resetPin) != ESP_OK)
            {
                ESP_LOGE(LOG_TAG, "Failed to set GPIO config for button");
            }
        }

        void GPIOButton::start()
        {
            startTask();
        }

        void GPIOButton::run()
        {
            while(1)
            {
                if(isPressed())
                {
                    if( ++_currentTriggerCounter >= _neededTriggerCounter)
                    {
                        if(_buttonPressedCallback)
                        {
                            _buttonPressedCallback();
                        }
                        _currentTriggerCounter = 0;
                    }
                }
                else
                {
                    _currentTriggerCounter = 0;
                }
                Task::delay(LOOP_DELAY);
            }
        }

        void GPIOButton::setButtonPressedCallback(CallbackFunction callback)
        {
            _buttonPressedCallback = callback;
        }

        void GPIOButton::setPressDuration(uint16_t interval)
        {
            _neededTriggerCounter = interval / LOOP_DELAY;
        }

        bool GPIOButton::isPressed() const
        {
            bool pressed = gpio_get_level( static_cast<gpio_num_t>(_gpioPin) );

            if(_inverted)
                pressed = !pressed;

            return pressed;
        }
    }
}
