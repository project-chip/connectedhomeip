/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

Device::Device(chip::Span<chip::DataVersion> dataVersions, chip::Span<EmberAfCluster> clusters,
               chip::Span<ClusterImpl *> clusterImpl, const chip::Span<const EmberAfDeviceType> & deviceTypeList,
               chip::EndpointId parentId) :
    mParentEndpointId(parentId),
    mDataVersions(dataVersions), mClusters(clusters), mClusterImpl(clusterImpl), mDeviceTypeList(deviceTypeList)
{
    mEndpointType.cluster      = clusters.data();
    mEndpointType.clusterCount = (uint8_t) clusters.size();
    mEndpointType.endpointSize = 0;
}

const char * Device::GetName()
{
    return mDeviceName;
}

void Device::SetName(const char * name)
{
    mDeviceName = name;
}

void Device::SetEndpointId(chip::EndpointId id)
{
    mEndpointId = id;
    for (auto * c : mClusterImpl)
        c->SetEndpointId(id);
}

Action::Action(uint16_t actionId, std::string name, ActionTypeEnum type, uint16_t endpointListId, uint16_t supportedCommands,
               ActionStateEnum status, bool isVisible)
{
    mActionId          = actionId;
    mName              = name;
    mType              = type;
    mEndpointListId    = endpointListId;
    mSupportedCommands = supportedCommands;
    mStatus            = status;
    mIsVisible         = isVisible;
}
