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

#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

struct ActiveChangeEventWorkData
{
    chip::EndpointId mEndpointId;
    uint32_t mPromisedActiveDuration;
};

struct ReportAttributeChangedWorkData
{
    chip::EndpointId mEndpointId;
    bool mWindowChanged      = false;
    bool mFabricIndexChanged = false;
    bool mVendorChanged      = false;
};

void ActiveChangeEventWork(intptr_t arg)
{
    ActiveChangeEventWorkData * data = reinterpret_cast<ActiveChangeEventWorkData *>(arg);

    chip::app::Clusters::BridgedDeviceBasicInformation::Events::ActiveChanged::Type event{};
    event.promisedActiveDuration  = data->mPromisedActiveDuration;
    chip::EventNumber eventNumber = 0;

    CHIP_ERROR err = chip::app::LogEvent(event, data->mEndpointId, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "LogEvent for ActiveChanged failed %s", err.AsString());
    }
    chip::Platform::Delete(data);
}

void ReportAttributeChangedWork(intptr_t arg)
{
    ReportAttributeChangedWorkData * data = reinterpret_cast<ReportAttributeChangedWorkData *>(arg);

    if (data->mWindowChanged)
    {
        MatterReportingAttributeChangeCallback(data->mEndpointId, chip::app::Clusters::AdministratorCommissioning::Id,
                                               chip::app::Clusters::AdministratorCommissioning::Attributes::WindowStatus::Id);
    }
    if (data->mFabricIndexChanged)
    {
        MatterReportingAttributeChangeCallback(data->mEndpointId, chip::app::Clusters::AdministratorCommissioning::Id,
                                               chip::app::Clusters::AdministratorCommissioning::Attributes::AdminFabricIndex::Id);
    }
    if (data->mVendorChanged)
    {
        MatterReportingAttributeChangeCallback(data->mEndpointId, chip::app::Clusters::AdministratorCommissioning::Id,
                                               chip::app::Clusters::AdministratorCommissioning::Attributes::AdminVendorId::Id);
    }
    chip::Platform::Delete(data);
}

} // namespace

using namespace chip::app::Clusters::Actions;

BridgedDevice::BridgedDevice(chip::ScopedNodeId scopedNodeId)
{
    mReachable    = false;
    mScopedNodeId = scopedNodeId;
    mEndpointId   = chip::kInvalidEndpointId;
}

void BridgedDevice::LogActiveChangeEvent(uint32_t promisedActiveDurationMs)
{
    ActiveChangeEventWorkData * workdata = chip::Platform::New<ActiveChangeEventWorkData>();
    workdata->mEndpointId                = mEndpointId;
    workdata->mPromisedActiveDuration    = promisedActiveDurationMs;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(ActiveChangeEventWork, reinterpret_cast<intptr_t>(workdata));
}

void BridgedDevice::SetReachable(bool reachable)
{
    mReachable = reachable;

    if (reachable)
    {
        ChipLogProgress(NotSpecified, "BridgedDevice[%s]: ONLINE", mAttributes.uniqueId.c_str());
    }
    else
    {
        ChipLogProgress(NotSpecified, "BridgedDevice[%s]: OFFLINE", mAttributes.uniqueId.c_str());
    }
}

void BridgedDevice::SetAdminCommissioningAttributes(const AdminCommissioningAttributes & aAdminCommissioningAttributes)
{
    ReportAttributeChangedWorkData * workdata = chip::Platform::New<ReportAttributeChangedWorkData>();

    workdata->mEndpointId = mEndpointId;
    workdata->mWindowChanged =
        (aAdminCommissioningAttributes.commissioningWindowStatus != mAdminCommissioningAttributes.commissioningWindowStatus);
    workdata->mFabricIndexChanged =
        (aAdminCommissioningAttributes.openerFabricIndex != mAdminCommissioningAttributes.openerFabricIndex);
    workdata->mVendorChanged = (aAdminCommissioningAttributes.openerVendorId != mAdminCommissioningAttributes.openerVendorId);

    mAdminCommissioningAttributes = aAdminCommissioningAttributes;
    chip::DeviceLayer::PlatformMgr().ScheduleWork(ReportAttributeChangedWork, reinterpret_cast<intptr_t>(workdata));
}
