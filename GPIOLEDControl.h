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

#ifndef GPIOLEDCONTROL_H
#define GPIOLEDCONTROL_H

#include <Mutex.h>

extern "C"
{
    #include <driver/uart.h>

    #ifdef CONFIG_IDF_TARGET_ESP32

        #include <freertos/timers.h>

    #endif
}

namespace IDFix
{
	namespace IODevices
	{
        /**
         * @brief The GPIOLEDControl class allows to control different LED signaling states
         */
        class GPIOLEDControl
        {
            public:

                /**
                 * @brief Constructs a GPIOLEDControl object
                 *
                 * @param pin       the GPIO pin connected to the LED
                 * @param inverted  set to true if low signal lights up the LED
                 */
                GPIOLEDControl(uint8_t pin, bool inverted = false);
                ~GPIOLEDControl();

                /**
                 * @brief Set the LED into a evenly blinking mode
                 *
                 * @param interval  the blink interval in milliseconds
                 */
                void            blink(uint16_t interval);

                /**
                 * @brief Set the LED into a blinking mode with different on/off intervals
                 *
                 * @param on    the on-interval in milliseconds
                 * @param off   the off-interval in milliseconds
                 */
                void            blinkSequence(uint16_t on, uint16_t off);

                /**
                 * @brief       Set the LED on or off
                 *
                 * @param on    the on/off state of the LED
                 */
                void            setOn(bool on = true);

                /**
                 * @brief       Set the LED off
                 */
                void            setOff();

                /**
                 * @brief       Returns if the LED is currently in a blinking mode
                 * @return      true if the LED is in blinking mode
                 */
                bool            isBlinking();

                /**
                 * @brief       Returns if the LED is manually set to ON (not if it's on in blinking mode)
                 * @return      true if the LED is set to ON
                 */
                bool            isOn();

            private:

                /**
                 * @brief   Callback on timer timeout
                 */
                void            blinkTime();

                /**
                 * @brief   Start the timer for blinking mode
                 */
                void            startBlinking();

                uint16_t        _onInterval = { 1000 };
                uint16_t        _offInterval = { 1000 };
                uint8_t         _pin;
                bool            _on = { false };
                bool            _blink = { false };
                bool            _inverted = { false };

                #ifdef CONFIG_IDF_TARGET_ESP32
                    static void     blinkTimeWrapper(TimerHandle_t xTimer);
                    TimerHandle_t   _blinkTimer = { nullptr };

                #else

                    static void     blinkTimeWrapper(void* instance);

                #endif

                Mutex           _mutex;
        };
    }
}

#endif
