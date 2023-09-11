/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

class LightSwitch
{
public:
    enum class Action : uint8_t
    {
        Toggle, /// Switch state on lighting-app device
        On,     /// Turn on light on lighting-app device
        Off     /// Turn off light on lighting-app device
    };

    void Init(chip::EndpointId aLightDimmerSwitchEndpoint, chip::EndpointId aLightGenericSwitchEndpointId);
    void InitiateActionSwitch(Action);
    void DimmerChangeBrightness();
    void GenericSwitchInitialPress();
    void GenericSwitchReleasePress();
    chip::EndpointId GetLightSwitchEndpointId() { return mLightSwitchEndpoint; }

    static LightSwitch & GetInstance()
    {
        static LightSwitch sLightSwitch;
        return sLightSwitch;
    }

private:
    constexpr static auto kOnePercentBrightnessApproximation = 3;
    constexpr static auto kMaximumBrightness                 = 254;

    chip::EndpointId mLightSwitchEndpoint;
    chip::EndpointId mLightGenericSwitchEndpointId;
};
