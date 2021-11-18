/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/app-common/zap-generated/attribute-id.h>
#include <app-common/app-common/zap-generated/cluster-id.h>
#include <app/binding/BindingTable.h>
#include <zap-generated/CHIPClusters.h>

using chip::OperationalDeviceProxy;
using chip::app::BindingTableEntry;
using chip::app::ConcreteAttributePath;
using chip::Controller::OnOffCluster;

extern void MatterBoundAttributeChangedCallback(const BindingTableEntry & binding, OperationalDeviceProxy * peerDevice,
                                                const ConcreteAttributePath & attributePath, uint16_t size, uint8_t * value)
{
    if (attributePath.mClusterId != ZCL_ON_OFF_CLUSTER_ID || attributePath.mAttributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        return;
    }
    if (size == 0 || value == nullptr)
    {
        return;
    }
    OnOffCluster clusterClient;
    clusterClient.Associate(peerDevice, binding.mPeerEndpointId);
    if (*value)
    {
        clusterClient.On(nullptr, nullptr);
    }
    else
    {
        clusterClient.Off(nullptr, nullptr);
    }
}
