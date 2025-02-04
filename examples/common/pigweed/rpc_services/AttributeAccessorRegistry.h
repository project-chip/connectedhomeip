/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "pw_status/status.h"
#include <app/AttributeReportBuilder.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <map>
#include <pigweed/rpc_services/AttributeAccessor.h>
#include <vector>

namespace chip {
namespace rpc {

/** @brief Custom write interception handler registry.
 *
 *    This class is specifically meant for registering custom Attribute Accessors that
 *    allow mini-AAI-handlers to process PigweedRPC read/writes separately from the cluster
 *    code. It is meant to be used by samples using this PigweedRPC Attributes service to
 *    allow the RPC interface to be used in more ways than simply simulating writes from a Matter
 *    client at the IM level. Handlers registered here by applications will be attempted before any
 *    standard processing of read/write would take place.
 */
class AttributeAccessorRegistry
{
public:
    /**
     * Register an attribute access override. If the accessor objects EndpointId
     * id NULL, the registration happens at cluster level, i.e. all endpoints
     * can use this accessor. Registrations can be unregistered via one of the
     * Unregister* calls.
     * Registration will fail if there is an already-registered override for the
     * same set of attributes.
     *
     * @return false if there is an existing override that the new one would
     *               conflict with.  In this case the override is not registered.
     * @return true if registration was successful.
     */
    bool Register(AttributeAccessor * attrOverride)
    {
        Optional<EndpointId> endpointId = attrOverride->GetEndpointId();
        ClusterId clusterId             = attrOverride->GetClusterId();
        if (endpointId.HasValue())
        {
            std::pair<EndpointId, ClusterId> endpointClusterPair = std::make_pair(endpointId.Value(), clusterId);
            if (perEndpointClusterAccessors.find(endpointClusterPair) != perEndpointClusterAccessors.end())
            {
                ChipLogError(Support, "Duplicate attribute override registration failed. endpointId: %u , clusterId: %u",
                             endpointId.Value(), clusterId);
                return false;
            }
            ChipLogProgress(Support, "Registering attribute accessor for endpointId: %u , clusterId: %u", endpointId.Value(),
                            clusterId);
            perEndpointClusterAccessors.insert(std::make_pair(endpointClusterPair, attrOverride));
        }
        else
        {
            if (perClusterAccessors.find(clusterId) != perClusterAccessors.end())
            {
                ChipLogError(Support, "Duplicate attribute override registration failed. clusterId: %u", clusterId);
                return false;
            }
            ChipLogProgress(Support, "Registering attribute accessor for clusterId: %u", clusterId);
            perClusterAccessors.insert(std::make_pair(clusterId, attrOverride));
        }
        return true;
    }

    /**
     * Unregister an attribute access override (for example if the object
     * implementing AttributeAccessor is being destroyed).
     */
    void Unregister(AttributeAccessor * attrOverride)
    {
        Optional<EndpointId> endpointId = attrOverride->GetEndpointId();
        ClusterId clusterId             = attrOverride->GetClusterId();
        if (endpointId.HasValue())
        {
            std::pair<EndpointId, ClusterId> endpointClusterPair = std::make_pair(endpointId.Value(), clusterId);
            if (perEndpointClusterAccessors.find(endpointClusterPair) == perEndpointClusterAccessors.end())
            {
                ChipLogError(Support, "Unregistration failed. No registration found for endpointId: %u , clusterId: %u",
                             endpointId.Value(), clusterId);
                return;
            }
            perEndpointClusterAccessors.erase(endpointClusterPair);
        }
        else
        {
            if (perClusterAccessors.find(clusterId) == perClusterAccessors.end())
            {
                ChipLogError(Support, "Unregistration failed. No registration found for clusterId: %u", clusterId);
                return;
            }
            perClusterAccessors.erase(clusterId);
        }
    }

    /**
     * Unregister all attribute accessors that match this given endpoint.
     * Cluster level attribute accessors do not get unregistered.
     */
    void UnregisterAllForEndpoint(EndpointId endpointId)
    {
        std::vector<std::pair<EndpointId, ClusterId>> removeRegistrations;
        for (std::map<std::pair<EndpointId, ClusterId>, AttributeAccessor *>::iterator it = perEndpointClusterAccessors.begin();
             it != perEndpointClusterAccessors.end(); ++it)
        {
            if ((it->first).first == endpointId)
            {
                removeRegistrations.push_back(it->first);
            }
        }
        for (std::pair<EndpointId, ClusterId> endpointClusterPair : removeRegistrations)
        {
            ChipLogProgress(Support, "Removing attribute accessor registration for endpointId: %u , clusterId: %u",
                            endpointClusterPair.first, endpointClusterPair.second);
            perEndpointClusterAccessors.erase(endpointClusterPair);
        }
    }

    /**
     * Unregisters all attribute accessors.
     */
    void UnregisterAll()
    {
        perClusterAccessors.clear();
        perEndpointClusterAccessors.clear();
    }

    /**
     *  Get the registered attribute access override that is cluster-endpoint specific.
     */
    AttributeAccessor * Get(EndpointId aEndpointId, ClusterId aClusterId)
    {
        std::pair<EndpointId, ClusterId> endpointClusterPair = std::make_pair(aEndpointId, aClusterId);
        if (perEndpointClusterAccessors.find(endpointClusterPair) != perEndpointClusterAccessors.end())
        {
            return perEndpointClusterAccessors.at(endpointClusterPair);
        }
        return nullptr;
    }

    /**
     * Get the registered attribute access override that is cluster specific (Global to all endpoints)
     */
    AttributeAccessor * Get(ClusterId aClusterId)
    {
        if (perClusterAccessors.find(aClusterId) != perClusterAccessors.end())
        {
            return perClusterAccessors.at(aClusterId);
        }
        return nullptr;
    }

    /**
     * Returns the singleton instance of the attribute accessor registory.
     */
    static AttributeAccessorRegistry & Instance()
    {
        static AttributeAccessorRegistry instance;
        return instance;
    }

private:
    std::map<ClusterId, AttributeAccessor *> perClusterAccessors;
    std::map<std::pair<EndpointId, ClusterId>, AttributeAccessor *> perEndpointClusterAccessors;
};

} // namespace rpc
} // namespace chip
