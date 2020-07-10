/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file contains the Cluster Server class. The Cluster Server maintains all the cluster
 *      servers that this devices exposes.
 *
 */

#ifndef CHIPCLUSTERSERVER_H_
#define CHIPCLUSTERSERVER_H_

#include <datamodel/Deque.h>
#include <datamodel/Endpoint.h>

namespace chip {
namespace DataModel {

/**
 * @brief
 *   This class implements the Cluster Server that maintains all the server endpoints of this
 *   device.
 */
class ClusterServer
{
public:
    Endpoint mEndpoints;

    ClusterServer(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion) :
        mEndpoints(ZCLVersion, applicationVersion, stackVersion, HWVersion)
    {}

    virtual ~ClusterServer() {}

    /**
     * @brief
     *   Add clusters to endpointId 1 of this Cluster Server
     *
     * @param cluster Pointer to the cluster object being added
     */
    CHIP_ERROR AddCluster(Cluster * cluster) { return mEndpoints.AddCluster(cluster); }

    /**
     * @brief
     *   Add a new endpoint to this Cluster Server
     *
     * @param cluster Pointer to the endpoint object being added
     */
    CHIP_ERROR AddEndpoint(Endpoint * endpoint)
    {
        mEndpoints.Insert(endpoint);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Retrieve a pointer to the corresponding Endpoint, if any
     *
     * @param endpointId the index of the desired Endpoint
     * @return pointer to the Endpoint or null if not found
     */
    Endpoint * GetEndpoint(uint8_t endpointId)
    {
        /* Endpoint #0 is reserved, until we handle that properly, we will offset everything by 1 */
        endpointId--;
        auto * p = mEndpoints.Find([&endpointId](Endpoint * item) -> bool { return (endpointId-- == 0); });
        return p;
    }

    /**
     * @brief
     *   Retrieve a pointer to the corresponding Cluster, if any
     *
     * @param endpointId the index of the Endpoint with the Cluster
     * @param clusterId the id of desired Cluster
     *
     * @return pointer to the Cluster or null if not found
     */
    Cluster * GetCluster(uint8_t endPointId, uint16_t clusterId)
    {
        Cluster * cluster = nullptr;

        auto endpoint = GetEndpoint(endPointId);
        if (endpoint != nullptr)
        {
            cluster = endpoint->GetCluster(clusterId);
        }
        return cluster;
    }

    /**
     * @brief
     *   Retrieve a pointer to the corresponding Attribute, if any
     *
     * @param endpointId the index of the desired Endpoint
     * @param clusterId the id of desired Cluster
     * @param attrId the id of desired Attribute
     *
     * @return pointer to the Attribute or null if not found
     */
    Attribute * GetAttribute(uint8_t endPointId, uint16_t clusterId, uint16_t attrId)
    {
        Attribute * attr = nullptr;

        auto cluster = GetCluster(endPointId, clusterId);
        if (cluster != nullptr)
        {
            attr = cluster->GetAttribute(attrId);
        }
        return attr;
    }

    /**
     * @brief
     *   Find and set an Attribute's Value
     *
     * @param endpointId the index of the desired Endpoint
     * @param clusterId the id of desired Cluster
     * @param attrId the id of desired Attribute
     * @param value the Value to set
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    CHIP_ERROR SetValue(uint8_t endPointId, uint16_t clusterId, uint16_t attrId, Value & value)
    {
        auto endpoint = GetEndpoint(endPointId);

        if (endpoint != nullptr)
        {
            auto cluster = endpoint->GetCluster(clusterId);

            if (cluster != nullptr)
            {
                return cluster->Set(attrId, value);
            }
        }
        return CHIP_ERROR_INTERNAL;
    }

    /**
     * @brief
     *   Find and read an Attribute's Value
     *
     * @param endpointId the index of the desired Endpoint
     * @param clusterId the id of desired Cluster
     * @param attrId the id of desired Attribute
     * @param value the Value to populate with the Attribute's value
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    CHIP_ERROR GetValue(uint8_t endPointId, uint16_t clusterId, uint16_t attrId, Value & value)
    {
        auto endpoint = GetEndpoint(endPointId);

        if (endpoint != nullptr)
        {
            auto cluster = endpoint->GetCluster(clusterId);

            if (cluster != nullptr)
            {
                auto attr = cluster->GetAttribute(attrId);

                if (attr != nullptr)
                {
                    value = attr->mValue;
                    return CHIP_NO_ERROR;
                }
            }
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERSERVER_H_ */
