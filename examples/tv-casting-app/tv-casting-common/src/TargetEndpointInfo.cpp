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
#include "TargetEndpointInfo.h"

using namespace chip;

void TargetEndpointInfo::Initialize(EndpointId endpointId)
{
    mEndpointId = endpointId;
    for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
    {
        mClusters[i] = kInvalidClusterId;
    }
    mInitialized = true;
}

bool TargetEndpointInfo::HasCluster(ClusterId clusterId)
{
    for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
    {
        if (mClusters[i] == clusterId)
        {
            return true;
        }
    }
    return false;
}

chip::ClusterId * TargetEndpointInfo::GetClusters()
{
    return mClusters;
}

bool TargetEndpointInfo::AddCluster(ClusterId clusterId)
{
    for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
    {
        if (mClusters[i] == clusterId)
        {
            return true;
        }
        if (mClusters[i] == kInvalidClusterId)
        {
            mClusters[i] = clusterId;
            return true;
        }
    }
    return false;
}

void TargetEndpointInfo::PrintInfo()
{
    ChipLogProgress(NotSpecified, "   endpoint=%d", mEndpointId);
    for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
    {
        if (mClusters[i] != kInvalidClusterId)
        {

            ChipLogProgress(NotSpecified, "      cluster=" ChipLogFormatMEI, ChipLogValueMEI(mClusters[i]));
        }
    }
}
