/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

// This is intended as a TOP LEVEL INCLUDE inside code that uses it

namespace chip {
namespace app {
namespace {

static bool CanAccessEvent(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteClusterPath & aPath,
                           Access::Privilege aNeededPrivilege)
{
    Access::RequestPath requestPath{ .cluster     = aPath.mClusterId,
                                     .endpoint    = aPath.mEndpointId,
                                     .requestType = Access::RequestType::kEventReadRequest };
    // leave requestPath.entityId optional value unset to indicate wildcard
    CHIP_ERROR err = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath, aNeededPrivilege);
    return (err == CHIP_NO_ERROR);
}

static bool CanAccessEvent(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteEventPath & aPath)
{
    Access::RequestPath requestPath{ .cluster     = aPath.mClusterId,
                                     .endpoint    = aPath.mEndpointId,
                                     .requestType = Access::RequestType::kEventReadRequest,
                                     .entityId    = aPath.mEventId };
    CHIP_ERROR err = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath, RequiredPrivilege::ForReadEvent(aPath));
    return (err == CHIP_NO_ERROR);
}

/**
 * Helper to handle wildcard events in the event path.
 */
static bool HasValidEventPathForEndpointAndCluster(EndpointId aEndpoint, const EmberAfCluster * aCluster,
                                                   const EventPathParams & aEventPath,
                                                   const Access::SubjectDescriptor & aSubjectDescriptor)
{
    if (aEventPath.HasWildcardEventId())
    {
        // We have no way to expand wildcards.  Just assume that we would need
        // View permissions for whatever events are involved.
        ConcreteClusterPath clusterPath(aEndpoint, aCluster->clusterId);
        return CanAccessEvent(aSubjectDescriptor, clusterPath, Access::Privilege::kView);
    }

    ConcreteEventPath path(aEndpoint, aCluster->clusterId, aEventPath.mEventId);
    return CanAccessEvent(aSubjectDescriptor, path);
}

/**
 * Helper to handle wildcard clusters in the event path.
 */
static bool HasValidEventPathForEndpoint(EndpointId aEndpoint, const EventPathParams & aEventPath,
                                         const Access::SubjectDescriptor & aSubjectDescriptor)
{
    if (aEventPath.HasWildcardClusterId())
    {
        auto * endpointType = emberAfFindEndpointType(aEndpoint);
        if (endpointType == nullptr)
        {
            // Not going to have any valid paths in here.
            return false;
        }

        for (decltype(endpointType->clusterCount) idx = 0; idx < endpointType->clusterCount; ++idx)
        {
            bool hasValidPath =
                HasValidEventPathForEndpointAndCluster(aEndpoint, &endpointType->cluster[idx], aEventPath, aSubjectDescriptor);
            if (hasValidPath)
            {
                return true;
            }
        }

        return false;
    }

    auto * cluster = emberAfFindServerCluster(aEndpoint, aEventPath.mClusterId);
    if (cluster == nullptr)
    {
        // Nothing valid here.
        return false;
    }
    return HasValidEventPathForEndpointAndCluster(aEndpoint, cluster, aEventPath, aSubjectDescriptor);
}

} // namespace
} // namespace app
} // namespace chip
