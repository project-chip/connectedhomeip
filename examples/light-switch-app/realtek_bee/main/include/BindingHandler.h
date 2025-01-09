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

//  #include "app-common/zap-generated/ids/Clusters.h"
//  #include "app-common/zap-generated/ids/Commands.h"
//  #include "lib/core/CHIPError.h"

// CHIP_ERROR InitBindingHandler();
// void SwitchWorkerFunction(intptr_t context);
// void SwitchWorkerFunction2(int localEndpointId);
// void SwitchWorkerFunction3(int remoteNodeId);
// void BindingWorkerFunction(intptr_t context);

// struct BindingCommandData
// {
//     chip::EndpointId localEndpointId;
//     chip::CommandId commandId;
//     chip::ClusterId clusterId;
//     bool isGroup = false;
// };

// struct SubscribeCommandData
// {
//     chip::EndpointId localEndpointId;
//     chip::NodeId nodeId;//remote nodeid
//     chip::FabricIndex fabricIndex;
// };

#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "lib/core/CHIPError.h"
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <controller/InvokeInteraction.h>
#include <platform/CHIPDeviceLayer.h>

class BindingHandler
{
public:
    struct BindingData
    {
        chip::EndpointId EndpointId;
        chip::CommandId CommandId;
        chip::ClusterId ClusterId;
        bool IsGroup{ false };
    };

    struct SubscribeCommandData
    {
        chip::EndpointId localEndpointId;
        chip::NodeId nodeId; // remote nodeid
        chip::FabricIndex fabricIndex;
    };

    void Init();
    bool IsGroupBound();

    static void SwitchWorkerFunction(intptr_t);
    static void SwitchWorkerFunction2(int localEndpointId);
    static void SwitchWorkerFunction3(int remoteNodeId);
    static void OnInvokeCommandFailure(BindingData & aBindingData, CHIP_ERROR aError);

    static BindingHandler & GetInstance()
    {
        static BindingHandler sBindingHandler;
        return sBindingHandler;
    }

private:
    static void OnOffProcessCommand(chip::CommandId, const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void LightSwitchChangedHandler(const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void LightSwitchContextReleaseHandler(void * context);
    static void InitInternal(intptr_t);
    bool mCaseSessionRecovered = false;

#ifdef CONFIG_ENABLE_CHIP_SHELL
    static void BindingWorkerFunction(intptr_t context);

    static CHIP_ERROR SwitchHelpHandler(int argc, char ** argv);
    static CHIP_ERROR SwitchCommandHandler(int argc, char ** argv);
    // OnOff switch shell functions
    static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv);
    static CHIP_ERROR OnOffSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR OnSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR OffSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR ToggleSwitchCommandHandler(int argc, char ** argv);
    // bind switch shell functions
    static CHIP_ERROR BindingHelpHandler(int argc, char ** argv);
    static CHIP_ERROR BindingSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR BindingGroupBindCommandHandler(int argc, char ** argv);
    static CHIP_ERROR BindingUnicastBindCommandHandler(int argc, char ** argv);
    // Groups switch shell functions
    static CHIP_ERROR GroupsHelpHandler(int argc, char ** argv);
    static CHIP_ERROR GroupsSwitchCommandHandler(int argc, char ** argv);
    // Groups OnOff switch shell functions
    static CHIP_ERROR GroupsOnOffHelpHandler(int argc, char ** argv);
    static CHIP_ERROR GroupsOnOffSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR GroupOnSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR GroupOffSwitchCommandHandler(int argc, char ** argv);
    static CHIP_ERROR GroupToggleSwitchCommandHandler(int argc, char ** argv);
    //
    static CHIP_ERROR TableCommandHelper(int argc, char ** argv);
    //
    static void RegisterSwitchCommands();
#endif // ENABLE_CHIP_SHELL
};
