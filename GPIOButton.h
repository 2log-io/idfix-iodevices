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

#ifndef GPIOBUTTON_H
#define GPIOBUTTON_H

#include <string>
#include <stdlib.h>
#include <functional>
#include "IDFixTask.h"
#include "Mutex.h"

extern "C"
{
    #include "freertos/FreeRTOS.h"
    #include <driver/uart.h>
}

namespace IDFix
{
    namespace IODevices
    {
        /**
         * @brief The GPIOButton class allows to handle button events from GPIO connected buttons.
         */
        class GPIOButton : public IDFix::Task
        {
            public:

                /**
                 * @brief Construct a GPIOButton
                 *
                 * @param pin           the GPIO pin connected to the button
                 * @param inverted      set to true if button press sets the GPIO to low
                 */
                GPIOButton(uint8_t pin, bool inverted = false);

                typedef         std::function<void()> CallbackFunction;

                /**
                 * @brief Start the button polling
                 */
                void            start(void);

                /**
                 * @brief Set the callback function to handle button press event
                 *
                 * @param callback  the function to be called on button press
                 */
                void            setButtonPressedCallback(CallbackFunction callback);

                /**
                 * @brief Set the duration interval until the button is perceived as pressed
                 * @param interval  the duration in milliseconds
                 */
                void            setPressDuration(uint16_t interval);

                /**
                 * @brief Get the current button state
                 *
                 * @return  \c true if button is currently pressed
                 * @return  \c false if button is currently not pressed
                 */
                bool            isPressed() const;

            private:

                virtual void        run() override;

                bool                _inverted = false;
                int                 _currentTriggerCounter = { 0 };
                int                 _neededTriggerCounter = { 0 };
                uint8_t             _gpioPin;
                CallbackFunction    _buttonPressedCallback;
        };
    }
}

#endif
