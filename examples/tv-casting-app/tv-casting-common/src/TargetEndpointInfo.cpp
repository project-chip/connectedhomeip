/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
