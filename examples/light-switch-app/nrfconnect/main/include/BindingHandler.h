/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <controller/InvokeInteraction.h>
#include <platform/CHIPDeviceLayer.h>

class BindingHandler
{
public:
    static void Init();
    static void SwitchWorkerHandler(intptr_t);
    static void PrintBindingTable();
    static bool IsGroupBound();

    struct BindingData
    {
        chip::EndpointId EndpointId;
        chip::CommandId CommandId;
        chip::ClusterId ClusterId;
        uint8_t Value;
        bool IsGroup{false};
    };

private:
    static void OnOffProcessCommand(chip::CommandId, const EmberBindingTableEntry &, chip::DeviceProxy *, void *);
    static void LevelControlProcessCommand(chip::CommandId, const EmberBindingTableEntry &, chip::DeviceProxy *, void *);
    static void LightSwitchChangedHandler(const EmberBindingTableEntry &, chip::DeviceProxy *, void *);
    static void InitInternal(intptr_t);
};
