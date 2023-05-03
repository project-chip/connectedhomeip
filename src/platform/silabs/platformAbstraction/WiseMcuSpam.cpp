/*
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include "init_ccpPlatform.h"
#include <FreeRTOS.h>
#include <task.h>

// TODO add includes ?
extern "C" void RSI_Board_LED_Set(int, bool);
extern "C" void RSI_Board_LED_Toggle(int);

namespace chip {
namespace DeviceLayer {
namespace Silabs {

SilabsPlatform SilabsPlatform::sSilabsPlatformAbstractionManager;
SilabsPlatform::SilabsButtonCb SilabsPlatform::mButtonCallback = nullptr;

CHIP_ERROR SilabsPlatform::Init(void)
{
    mButtonCallback = nullptr;
    init_ccpPlatform();
    return CHIP_NO_ERROR;
}

#ifdef ENABLE_WSTK_LEDS
void SilabsPlatform::InitLed(void)
{
    // TODO
    SilabsPlatformAbstractionBase::InitLed();
}

CHIP_ERROR SilabsPlatform::SetLed(bool state, uint8_t led)
{
    // TODO add range check
    RSI_Board_LED_Set(led, state);
    return CHIP_NO_ERROR;
}

bool SilabsPlatform::GetLedState(uint8_t led)
{
    // TODO
    return SilabsPlatformAbstractionBase::GetLedState(led);
}

CHIP_ERROR SilabsPlatform::ToggleLed(uint8_t led)
{
    // TODO add range check
    RSI_Board_LED_Toggle(led);
    return CHIP_NO_ERROR;
}
#endif // ENABLE_WSTK_LEDS

void SilabsPlatform::StartScheduler()
{
    vTaskStartScheduler();
}

extern "C" {
void sl_button_on_change(uint8_t btn, uint8_t btnAction)
{
    if (Silabs::GetPlatform().mButtonCallback == nullptr)
    {
        return;
    }

    Silabs::GetPlatform().mButtonCallback(btn, btnAction);
}
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
