/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/codegen-data-model-provider/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace {

using namespace chip::Access;

#if !CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
bool CanAccessEvent(const Access::SubjectDescriptor & subjectDescriptor, const ConcreteClusterPath & path,
                    Access::Privilege aNeededPrivilege)
{
    Access::RequestPath requestPath{ .cluster     = path.mClusterId,
                                     .endpoint    = path.mEndpointId,
                                     .requestType = Access::RequestType::kEventReadRequest };
    // leave requestPath.entityId optional value unset to indicate wildcard
    CHIP_ERROR err = Access::GetAccessControl().Check(subjectDescriptor, requestPath, aNeededPrivilege);
    return (err == CHIP_NO_ERROR);
}
#endif

bool CanAccessEvent(const Access::SubjectDescriptor & subjectDescriptor, const ConcreteEventPath & path)
{
    Access::RequestPath requestPath{ .cluster     = path.mClusterId,
                                     .endpoint    = path.mEndpointId,
                                     .requestType = Access::RequestType::kEventReadRequest,
                                     .entityId    = path.mEventId };
    CHIP_ERROR err = Access::GetAccessControl().Check(subjectDescriptor, requestPath, RequiredPrivilege::ForReadEvent(path));
    return (err == CHIP_NO_ERROR);
}

bool EventSupported(const EmberAfCluster * cluster, EventId eventId)
{
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
    for (size_t i = 0; i < cluster->eventCount; ++i)
    {
        if (cluster->eventList[i] == eventId)
        {
            return true;
        }
    }

    return false;
#else
    // No way to tell. Just claim supported.
    return true;
#endif // CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
}

bool HasValidEventPathForEndpointAndCluster(EndpointId endpoint, const EmberAfCluster * cluster, const EventPathParams & path,
                                            const Access::SubjectDescriptor & subjectDescriptor)
{
    if (path.HasWildcardEventId())
    {
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        for (decltype(cluster->eventCount) idx = 0; idx < cluster->eventCount; ++idx)
        {
            ConcreteEventPath path(endpoint, cluster->clusterId, cluster->eventList[idx]);
            // If we get here, the path exists.  We just have to do an ACL check for it.
            bool isValid = CanAccessEvent(subjectDescriptor, path);
            if (isValid)
            {
                return true;
            }
        }

        return false;
#else
        // We have no way to expand wildcards.  Just assume that we would need
        // View permissions for whatever events are involved.
        ConcreteClusterPath clusterPath(endpoint, cluster->clusterId);
        return CanAccessEvent(subjectDescriptor, clusterPath, Access::Privilege::kView);
#endif
    }

    if (!EventSupported(cluster, path.mEventId))
    {
        // Not an existing event path.
        return false;
    }
    return CanAccessEvent(subjectDescriptor, ConcreteEventPath(endpoint, cluster->clusterId, path.mEventId));
}

bool HasValidEventPathForEndpoint(EndpointId endpoint, const EventPathParams & path,
                                  const Access::SubjectDescriptor & subjectDescriptor)
{
    if (path.HasWildcardClusterId())
    {
        auto * endpointType = emberAfFindEndpointType(endpoint);
        if (endpointType == nullptr)
        {
            // Not going to have any valid paths in here.
            return false;
        }

        for (decltype(endpointType->clusterCount) idx = 0; idx < endpointType->clusterCount; ++idx)
        {
            bool hasValidPath =
                HasValidEventPathForEndpointAndCluster(endpoint, &endpointType->cluster[idx], path, subjectDescriptor);
            if (hasValidPath)
            {
                return true;
            }
        }

        return false;
    }

    auto * cluster = emberAfFindServerCluster(endpoint, path.mClusterId);
    if (cluster == nullptr)
    {
        // Nothing valid here.
        return false;
    }
    return HasValidEventPathForEndpointAndCluster(endpoint, cluster, path, subjectDescriptor);
}

} // namespace

bool CodegenDataModelProvider::EventPathReadable(const EventPathParams & path, const Access::SubjectDescriptor & descriptor)
{

    if (path.HasWildcardEndpointId())
    {
        for (uint16_t endpointIndex = 0; endpointIndex < emberAfEndpointCount(); ++endpointIndex)
        {
            if (!emberAfEndpointIndexIsEnabled(endpointIndex))
            {
                continue;
            }
            if (HasValidEventPathForEndpoint(emberAfEndpointFromIndex(endpointIndex), path, descriptor))
            {
                return true;
            }
        }
        return false;
    }

    // No need to check whether the endpoint is enabled, because
    // emberAfFindEndpointType returns null for disabled endpoints.
    return HasValidEventPathForEndpoint(path.mEndpointId, path, descriptor);
}

} // namespace app
} // namespace chip
