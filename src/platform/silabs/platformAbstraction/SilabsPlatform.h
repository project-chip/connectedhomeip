/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    enum class ButtonAction : uint8_t
    {
        ButtonReleased = 0,
        ButtonPressed,
        ButtonDisabled,
        InvalidAction
    };

    // Generic Peripherical methods
    CHIP_ERROR Init(void) override;

    // LEDS
#ifdef ENABLE_WSTK_LEDS
    void InitLed(void) override;
    CHIP_ERROR SetLed(bool state, uint8_t led) override;
    bool GetLedState(uint8_t led) override;
    CHIP_ERROR ToggleLed(uint8_t led) override;
#endif

    inline void SetButtonsCb(SilabsButtonCb callback) override { mButtonCallback = callback; }

    static SilabsButtonCb mButtonCallback;

    void StartScheduler(void) override;

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
