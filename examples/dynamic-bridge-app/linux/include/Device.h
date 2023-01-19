/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <lib/support/Span.h>

#include <stdbool.h>
#include <stdint.h>

#include "Clusters.h"

// This represents a single logical device occupying one endpoint. A composed device consists of multiple
// Device objects that reference a tree.
class Device
{
public:
    Device(chip::Span<chip::DataVersion> dataVersions, chip::Span<EmberAfCluster> clusters,
           chip::Span<ClusterInterface *> clusterImpl, const chip::Span<const EmberAfDeviceType> & deviceTypeList,
           chip::EndpointId parentId);
    ~Device() = default;

    const chip::Span<chip::DataVersion> & versions() { return mDataVersions; }
    const chip::Span<const EmberAfDeviceType> & deviceTypes() { return mDeviceTypeList; }
    const EmberAfEndpointType * endpointType() { return &mEndpointType; }
    const chip::Span<ClusterInterface *> & clusters() { return mClusterImpl; }

    void SetEndpointId(chip::EndpointId id);
    chip::EndpointId GetEndpointId() { return mEndpointId; }
    chip::EndpointId GetParentEndpointId() { return mParentEndpointId; }

    const char * GetName();
    void SetName(const char * name);

private:
    chip::EndpointId mParentEndpointId;
    chip::EndpointId mEndpointId;
    chip::Span<chip::DataVersion> mDataVersions;
    chip::Span<EmberAfCluster> mClusters;
    chip::Span<ClusterInterface *> mClusterImpl;
    chip::Span<const EmberAfDeviceType> mDeviceTypeList;
    EmberAfEndpointType mEndpointType;
    std::string mDeviceName;
};

int AddDeviceEndpoint(Device * dev);
int RemoveDeviceEndpoint(Device * dev);
Device * FindDeviceEndpoint(chip::EndpointId id);
