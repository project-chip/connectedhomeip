/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/CHIPDeviceLayer.h>

constexpr size_t kMaxNumberOfClustersPerEndpoint = 10;

class TargetEndpointInfo
{
public:
    void Initialize(chip::EndpointId endpointId);
    void Reset() { mInitialized = false; }
    bool IsInitialized() { return mInitialized; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

    bool HasCluster(chip::ClusterId clusterId);
    chip::ClusterId * GetClusters();
    bool AddCluster(chip::ClusterId clusterId);
    void PrintInfo();

private:
    chip::ClusterId mClusters[kMaxNumberOfClustersPerEndpoint] = {};
    chip::EndpointId mEndpointId;
    bool mInitialized = false;
};
