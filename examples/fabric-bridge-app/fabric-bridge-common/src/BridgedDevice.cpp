/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BridgedDevice.h"

#include <cstdio>
#include <string>

#include <app/EventLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

struct ActiveChangeEventWorkData {
    chip::EndpointId mEndpointId;
    uint32_t mPromisedActiveDuration;
};

static void ActiveChangeEventWork(intptr_t arg)
{
    ActiveChangeEventWorkData* data = reinterpret_cast<ActiveChangeEventWorkData *>(arg);

    chip::app::Clusters::BridgedDeviceBasicInformation::Events::ActiveChanged::Type event{};
    event.promisedActiveDuration  = data->mPromisedActiveDuration;
    chip::EventNumber eventNumber = 0;

    // TODO DNS this is not called from Matter event loop so it will crash
    CHIP_ERROR err = chip::app::LogEvent(event, data->mEndpointId, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "LogEvent for ActiveChanged failed %s", err.AsString());
    }
    chip::Platform::Delete(data);
}

} // namespace

using namespace chip::app::Clusters::Actions;

BridgedDevice::BridgedDevice(chip::NodeId nodeId)
{
    mReachable  = false;
    mNodeId     = nodeId;
    mEndpointId = chip::kInvalidEndpointId;
}

void BridgedDevice::LogActiveChangeEvent(uint32_t promisedActiveDuration)
{
    ActiveChangeEventWorkData* workdata = chip::Platform::New<ActiveChangeEventWorkData>();
    workdata->mEndpointId = mEndpointId;
    workdata->mPromisedActiveDuration = promisedActiveDuration;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(ActiveChangeEventWork, reinterpret_cast<intptr_t>(workdata));
}

bool BridgedDevice::IsReachable()
{
    return mReachable;
}

bool BridgedDevice::IsIcd()
{
    return mIsIcd;
}

void BridgedDevice::SetReachable(bool reachable)
{
    mReachable = reachable;

    if (reachable)
    {
        ChipLogProgress(NotSpecified, "BridgedDevice[%s]: ONLINE", mName);
    }
    else
    {
        ChipLogProgress(NotSpecified, "BridgedDevice[%s]: OFFLINE", mName);
    }
}

void BridgedDevice::SetName(const char * name)
{
    ChipLogProgress(NotSpecified, "BridgedDevice[%s]: New Name=\"%s\"", mName, name);

    chip::Platform::CopyString(mName, name);
}
