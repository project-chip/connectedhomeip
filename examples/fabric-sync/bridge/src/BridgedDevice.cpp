/*
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

using namespace chip;
using namespace chip::app::Clusters::Actions;

namespace bridge {

BridgedDevice::BridgedDevice(ScopedNodeId scopedNodeId)
{
    mReachable    = false;
    mScopedNodeId = scopedNodeId;
    mEndpointId   = kInvalidEndpointId;
}

void BridgedDevice::LogActiveChangeEvent(uint32_t promisedActiveDurationMs)
{
    EndpointId endpointId = mEndpointId;

    DeviceLayer::SystemLayer().ScheduleLambda([endpointId, promisedActiveDurationMs]() {
        app::Clusters::BridgedDeviceBasicInformation::Events::ActiveChanged::Type event{};
        event.promisedActiveDuration = promisedActiveDurationMs;
        EventNumber eventNumber      = 0;

        CHIP_ERROR err = app::LogEvent(event, endpointId, eventNumber);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogProgress(NotSpecified, "LogEvent for ActiveChanged failed %s", err.AsString());
        }
    });
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

void BridgedDevice::ReachableChanged(bool reachable)
{
    EndpointId endpointId = mEndpointId;
    bool reachableChanged = (mReachable != reachable);
    if (reachableChanged)
    {
        SetReachable(reachable);
        DeviceLayer::SystemLayer().ScheduleLambda([endpointId]() {
            MatterReportingAttributeChangeCallback(endpointId, app::Clusters::BridgedDeviceBasicInformation::Id,
                                                   app::Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id);

            app::Clusters::BridgedDeviceBasicInformation::Events::ReachableChanged::Type event{};
            EventNumber eventNumber = 0;

            CHIP_ERROR err = app::LogEvent(event, endpointId, eventNumber);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(NotSpecified, "LogEvent for ActiveChanged failed %s", err.AsString());
            }
        });
    }
}

void BridgedDevice::SetAdminCommissioningAttributes(const AdminCommissioningAttributes & aAdminCommissioningAttributes)
{
    EndpointId endpointId = mEndpointId;
    bool windowChanged =
        (aAdminCommissioningAttributes.commissioningWindowStatus != mAdminCommissioningAttributes.commissioningWindowStatus);
    bool fabricIndexChanged = (aAdminCommissioningAttributes.openerFabricIndex != mAdminCommissioningAttributes.openerFabricIndex);
    bool vendorChanged      = (aAdminCommissioningAttributes.openerVendorId != mAdminCommissioningAttributes.openerVendorId);

    mAdminCommissioningAttributes = aAdminCommissioningAttributes;

    DeviceLayer::SystemLayer().ScheduleLambda([endpointId, windowChanged, fabricIndexChanged, vendorChanged]() {
        if (windowChanged)
        {
            MatterReportingAttributeChangeCallback(endpointId, app::Clusters::AdministratorCommissioning::Id,
                                                   app::Clusters::AdministratorCommissioning::Attributes::WindowStatus::Id);
        }
        if (fabricIndexChanged)
        {
            MatterReportingAttributeChangeCallback(endpointId, app::Clusters::AdministratorCommissioning::Id,
                                                   app::Clusters::AdministratorCommissioning::Attributes::AdminFabricIndex::Id);
        }
        if (vendorChanged)
        {
            MatterReportingAttributeChangeCallback(endpointId, app::Clusters::AdministratorCommissioning::Id,
                                                   app::Clusters::AdministratorCommissioning::Attributes::AdminVendorId::Id);
        }
    });
}

} // namespace bridge
