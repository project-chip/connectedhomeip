/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

    enum class LightSwitchLevelControl : uint8_t
    {
        MoveToLevel
    };

    enum class LightSwitchColorControl : uint8_t
    {
        MoveToColor
    };

    struct GenericSwitchEventData
    {
        chip::EndpointId endpoint;
        chip::EventId event;
    };

    CHIP_ERROR Init(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint);


    void TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand, chip::EndpointId endpoint, chip::ClusterId clusterId);
    void TriggerLightSwitchLevelControl(LightSwitchLevelControl action, bool isGroupCommand, chip::EndpointId endpoint, chip::ClusterId clusterId, uint8_t value);
    void TriggerLightSwitchColorControl(LightSwitchColorControl action, bool isGroupCommand, chip::EndpointId endpoint, chip::ClusterId clusterId, uint8_t value);

    static LightSwitchMgr & GetInstance() { return sSwitch; }

private:
    static LightSwitchMgr sSwitch;

    chip::EndpointId mLightSwitchEndpoint   = chip::kInvalidEndpointId;
    chip::EndpointId mGenericSwitchEndpoint = chip::kInvalidEndpointId;
};
