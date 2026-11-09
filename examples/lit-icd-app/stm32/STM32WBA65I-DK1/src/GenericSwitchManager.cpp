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

#include "GenericSwitchManager.h"
#include "log_module.h"

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif // defined(ENABLE_CHIP_SHELL)

#include "AppEvent.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

GenericSwitchManager GenericSwitchManager::sSwitch;

/**
 * @brief Function that triggers a generic switch OnInitialPress event
 */
void GenericSwitchManager::GenericSwitchOnInitialPress()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::InitialPress::Id;

    err = DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void GenericSwitchManager::GenericSwitchOnLongPress()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::LongPress::Id;

    err = DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

/**
 * @brief Function that triggers a generic switch OnShortRelease event
 */
void GenericSwitchManager::GenericSwitchOnShortRelease()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::ShortRelease::Id;

    err = DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

/**
 * @brief Function that triggers a generic switch OnLongRelease event
 */
void GenericSwitchManager::GenericSwitchOnLongRelease()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::LongRelease::Id;

    err = DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void GenericSwitchManager::GenericSwitchWorkerFunction(intptr_t context)
{

    CHIP_ERROR err                = CHIP_NO_ERROR;
    GenericSwitchEventData * data = reinterpret_cast<GenericSwitchEventData *>(context);
    LOG_ERROR_APP("P5 Worker: event=%lu endpoint=%u\n", (unsigned long) data->event, (unsigned int) data->endpoint);

    switch (data->event)
    {
    case Switch::Events::InitialPress::Id: {
        uint8_t currentPosition = 1;

        if (auto * switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint))
        {
            err = switchCluster->SetCurrentPosition(currentPosition);
            switchCluster->OnInitialPress(currentPosition);
        }
        break;
    }
    case Switch::Events::LongPress::Id: {
        uint8_t currentPosition = 1;

        if (auto * switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint))
        {
            switchCluster->OnLongPress(currentPosition);
        }
        break;
    }
    case Switch::Events::ShortRelease::Id: {
        uint8_t previousPosition = 1;
        uint8_t currentPosition  = 0;

        if (auto * switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint))
        {
            err = switchCluster->SetCurrentPosition(currentPosition);
            switchCluster->OnShortRelease(previousPosition);
        }
        break;
    }
    case Switch::Events::LongRelease::Id: {
        uint8_t previousPosition = 1;
        uint8_t currentPosition  = 0;

        if (auto * switchCluster = Clusters::Switch::FindClusterOnEndpoint(data->endpoint))
        {
            err = switchCluster->SetCurrentPosition(currentPosition);
            switchCluster->OnLongRelease(previousPosition);
        }
        break;
    }
    default:
        break;
    }

    Platform::Delete(data);
}
