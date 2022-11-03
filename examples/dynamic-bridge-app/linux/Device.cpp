/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

Device::Device(chip::Span<chip::DataVersion> dataVersions, chip::Span<EmberAfCluster> clusters,
               chip::Span<ClusterInterface *> clusterImpl, const chip::Span<const EmberAfDeviceType> & deviceTypeList,
               chip::EndpointId parentId) :
    mParentEndpointId(parentId),
    mDataVersions(dataVersions), mClusters(clusters), mClusterImpl(clusterImpl), mDeviceTypeList(deviceTypeList)
{
    mEndpointType.cluster      = clusters.data();
    mEndpointType.clusterCount = (uint8_t) clusters.size();
    mEndpointType.endpointSize = 0;
}

void Device::SetEndpointId(chip::EndpointId id)
{
    mEndpointId = id;
    for (auto * c : mClusterImpl)
        c->SetEndpointId(id);
}

const char * Device::GetName()
{
    return mDeviceName.c_str();
}

void Device::SetName(const char * name)
{
    mDeviceName = name;
}
