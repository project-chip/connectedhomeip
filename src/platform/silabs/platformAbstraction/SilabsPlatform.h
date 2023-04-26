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

#pragma once

#include <platform/silabs/platformAbstraction/SilabsPlatformBase.h>
#include <stdint.h>
#include <stdio.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

class SilabsPlatform : virtual public SilabsPlatformAbstractionBase
{

public:
    // Generic Peripherical methods
    CHIP_ERROR Init(void) override;

    // LEDS
#ifdef ENABLE_WSTK_LEDS
    void InitLed(void) override;
    CHIP_ERROR SetLed(bool state, uint8_t led) override;
    bool GetLedState(uint8_t led) override;
    CHIP_ERROR ToggleLed(uint8_t led) override;
#endif

private:
    friend SilabsPlatform & GetPlatform(void);

    // To make underlying SDK thread safe
    void SilabsPlatformLock(void);
    void SilabsPlatformUnlock(void);

    SilabsPlatform(){};
    virtual ~SilabsPlatform() = default;

    static SilabsPlatform sSilabsPlatformAbstractionManager;
};

inline SilabsPlatform & GetPlatform(void)
{
    return SilabsPlatform::sSilabsPlatformAbstractionManager;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
