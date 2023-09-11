/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

class LightSwitchMgr
{
public:
    enum class LightSwitchAction : uint8_t
    {
        Toggle, // Triggers an toggle command
        On,     // Triggers an On command
        Off     // Trigger an Off command
    };

    struct GenericSwitchEventData
    {
        chip::EndpointId endpoint;
        chip::EventId event;
    };

    CHIP_ERROR Init(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint);

    void GenericSwitchOnInitialPress();
    void GenericSwitchOnShortRelease();

    void TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand = false);

    static LightSwitchMgr & GetInstance() { return sSwitch; }

private:
    static LightSwitchMgr sSwitch;

    static void GenericSwitchWorkerFunction(intptr_t context);

    chip::EndpointId mLightSwitchEndpoint   = chip::kInvalidEndpointId;
    chip::EndpointId mGenericSwitchEndpoint = chip::kInvalidEndpointId;
};
