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

#include "DynamicDevice.h"

void DynamicAttributeList::Add(chip::AttributeId id, EmberAfAttributeType type, uint16_t maxSize, EmberAfClusterMask mask)
{
    // Do what DECLARE_DYNAMIC_ATTRIBUTE does.
    mAttributeDecls.emplace_back(EmberAfAttributeMetadata{
        id, type, maxSize, (uint8_t) (mask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)), ZAP_EMPTY_DEFAULT() });
}

void DynamicAttributeList::Add(EmberAfAttributeMetadata metadata)
{
    mAttributeDecls.emplace_back(metadata);
}

DynamicDeviceImpl::DynamicDeviceImpl() {}

void DynamicDeviceImpl::AddDeviceType(EmberAfDeviceType type)
{
    mDeviceTypes.push_back(type);
}

Device DynamicDeviceImpl::CreateDevice()
{
    // All nodes are bridged devices.
    mDeviceTypes.push_back(EmberAfDeviceType{ DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT });
    mVersions.resize(mClusterRawPtrs.size());
    for (auto * c : mClusterRawPtrs)
        mClusterBaseRawPtrs.push_back(c);

    return Device(chip::Span<chip::DataVersion>(mVersions.data(), mVersions.size()),
                  chip::Span<EmberAfCluster>(mClusterDecls.data(), mClusterDecls.size()),
                  chip::Span<ClusterImpl *>(mClusterBaseRawPtrs.data(), mClusterBaseRawPtrs.size()),
                  chip::Span<EmberAfDeviceType>(mDeviceTypes.data(), mDeviceTypes.size()));
}

DynamicDeviceImpl & DynamicDeviceImpl::AddCluster(std::unique_ptr<CommonCluster> cluster, const DynamicAttributeList & attribs,
                                                  const chip::CommandId * incomingCommands,
                                                  const chip::CommandId * outgoingCommands)
{
    AddCluster(cluster.get(), attribs, incomingCommands, outgoingCommands);
    mClusters.emplace_back(std::move(cluster));
    return *this;
}

DynamicDeviceImpl & DynamicDeviceImpl::AddCluster(CommonCluster * cluster, const DynamicAttributeList & attribs,
                                                  const chip::CommandId * incomingCommands,
                                                  const chip::CommandId * outgoingCommands)
{
    // Do what DECLARE_DYNAMIC_CLUSTER does.
    mClusterDecls.emplace_back(EmberAfCluster{ cluster->GetClusterId(), attribs.mAttributeDecls.data(),
                                               (uint16_t) attribs.mAttributeDecls.size(), 0, ZAP_CLUSTER_MASK(SERVER), nullptr,
                                               incomingCommands, outgoingCommands });
    mClusterRawPtrs.emplace_back(cluster);
    return *this;
}
