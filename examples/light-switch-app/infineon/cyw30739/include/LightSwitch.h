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
        Toggle, // Switch state on lighting-app device
        On,     // Turn on light on lighting-app device
        Off     // Turn off light on lighting-app device
    };

    enum class Status : uint8_t
    {
        Toggle = 0,
        On,
        Off,
        Unknown
    };

    void Init(chip::EndpointId aLightDimmerSwitchEndpoint);
    void InitiateActionSwitch(Action);
    Status GetSwitchStatus();
    void DimmerChangeBrightness(uint16_t kValue);

    chip::EndpointId GetLightSwitchEndpointId() { return mLightSwitchEndpoint; }

    static LightSwitch & GetInstance()
    {
        static LightSwitch sLightSwitch;
        return sLightSwitch;
    }

private:
    Status mStatus = Status::Unknown;
    chip::EndpointId mLightSwitchEndpoint;
};
