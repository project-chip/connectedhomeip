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

#include <platform/CHIPDeviceLayer.h>

class TargetEndpointInfo
{
public:
    void Initialize(chip::EndpointId endpointId);
    void Reset() { mInitialized = false; }
    bool IsInitialized() { return mInitialized; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

    bool HasCluster(chip::ClusterId clusterId);
    bool AddCluster(chip::ClusterId clusterId);
    void PrintInfo();

private:
    static constexpr size_t kMaxNumberOfClustersPerEndpoint    = 10;
    chip::ClusterId mClusters[kMaxNumberOfClustersPerEndpoint] = {};
    chip::EndpointId mEndpointId;
    bool mInitialized = false;
};
