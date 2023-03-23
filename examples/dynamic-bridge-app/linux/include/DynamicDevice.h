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

#include "Device.h"
#include "GeneratedClusters.h"

#include <memory>
#include <vector>

// This file provides implementations that use dynamic storage
struct DynamicCluster : public CommonCluster
{
    DynamicCluster(std::unique_ptr<GeneratedCluster> impl, const chip::CommandId * incomingCommands = nullptr,
                   const chip::CommandId * outgoingCommands = nullptr) :
        CommonCluster(std::move(impl)),
        mIncomingCommands(incomingCommands), mOutgoingCommands(outgoingCommands)
    {
        for (auto & attr : mAttributes)
            mAttributeDecls.push_back(attr->GetMetadata());
    }

    std::vector<EmberAfAttributeMetadata> mAttributeDecls;
    const chip::CommandId * mIncomingCommands;
    const chip::CommandId * mOutgoingCommands;
};

class DynamicDevice
{
public:
    DynamicDevice();

    template <typename... Types>
    DynamicDevice(Types &... clusters)
    {}

    virtual ~DynamicDevice() = default;

    void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; }

    void AddDeviceType(EmberAfDeviceType type);

    // Add each cluster that the device will ever use.
    DynamicDevice & AddCluster(std::unique_ptr<CommonCluster> cluster, const chip::Span<EmberAfAttributeMetadata> & attribs,
                               const chip::CommandId * incomingCommands, const chip::CommandId * outgoingCommands);
    DynamicDevice & AddCluster(CommonCluster * cluster, const chip::Span<EmberAfAttributeMetadata> & attribs,
                               const chip::CommandId * incomingCommands, const chip::CommandId * outgoingCommands);

    DynamicDevice & AddCluster(std::unique_ptr<DynamicCluster> cluster);

    // After setting all options, create the device.
    Device CreateDevice();

    const std::vector<CommonCluster *> & clusters() const { return mClusterRawPtrs; }

private:
    std::vector<std::unique_ptr<CommonCluster>> mClusters;
    std::vector<CommonCluster *> mClusterRawPtrs;
    std::vector<ClusterInterface *> mClusterBaseRawPtrs;

    std::vector<EmberAfCluster> mClusterDecls;

    std::vector<chip::DataVersion> mVersions;

    std::vector<EmberAfDeviceType> mDeviceTypes;

    chip::EndpointId mParentEndpointId = 1;
};
