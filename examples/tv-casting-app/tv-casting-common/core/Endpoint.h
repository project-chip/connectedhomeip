/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "BaseCluster.h"
#include "CastingPlayer.h"
#include "Types.h"

#include "lib/support/logging/CHIPLogging.h"
#include <app-common/zap-generated/cluster-objects.h>

#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

namespace matter {
namespace casting {
namespace core {

class EndpointAttributes
{
public:
    // value of 0 means the attribute could not be read for the corresponding Endpoint
    chip::EndpointId mId = 0;
    uint16_t mVendorId   = 0;
    uint16_t mProductId  = 0;
    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> mDeviceTypeList;
};

class CastingPlayer;
class BaseCluster;

/**
 * @brief An Endpoint on a CastingPlayer e.g. a Speaker or a Matter Content App
 */
class Endpoint : public std::enable_shared_from_this<Endpoint>
{

private:
    CastingPlayer * mCastingPlayer;

    EndpointAttributes mAttributes;
    std::map<chip::ClusterId, memory::Strong<BaseCluster>> mClusters;

public:
    Endpoint(CastingPlayer * castingPlayer, const EndpointAttributes & attributes)
    {
        this->mCastingPlayer = castingPlayer;
        this->mAttributes    = attributes;
    }

    ~Endpoint() {}

    Endpoint()                       = delete;
    Endpoint(Endpoint & other)       = delete;
    void operator=(const Endpoint &) = delete;

    CastingPlayer * GetCastingPlayer() const { return mCastingPlayer; }

    /**
     * @brief Compares based on the Id
     */
    bool operator==(const Endpoint & other) const { return this->mAttributes.mId == other.mAttributes.mId; }

    chip::EndpointId GetId() const { return mAttributes.mId; }

    /**
     * @return uint16_t - value 0 indicates no ProductId was returned for this Endpoint
     */
    uint16_t GetProductId() const { return mAttributes.mProductId; }

    /**
     * @return uint16_t - value 0 indicates no VendorId was returned for this Endpoint
     */
    uint16_t GetVendorId() const { return mAttributes.mVendorId; }

    /**
     * @return uint16_t - empty vector indicates no DeviceTypeList was returned for this Endpoint
     */
    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> GetDeviceTypeList() const
    {
        return mAttributes.mDeviceTypeList;
    }

    /**
     * @return uint16_t - empty vector indicates no ServerList was returned for this Endpoint
     */
    std::vector<chip::ClusterId> GetServerList()
    {
        ChipLogProgress(AppServer, "Endpoint::GetServerList() mClusters.size(): %d", static_cast<int>(mClusters.size()));
        std::vector<chip::ClusterId> serverList;
        for (auto const & cluster : mClusters)
        {
            serverList.push_back(cluster.first);
        }
        return serverList;
    }

    void RegisterClusters(std::vector<chip::ClusterId> clusters);

    /**
     * @brief Registers a cluster of type T against the passed in clusterId
     * for this Endpoint
     */
    template <typename T>
    void RegisterCluster(const chip::ClusterId clusterId)
    {
        ChipLogProgress(AppServer, "Endpoint::RegisterCluster() Registering clusterId %d for endpointId %d", clusterId, GetId());
        static_assert(std::is_base_of<BaseCluster, T>::value, "T must be derived from BaseCluster");
        auto cluster = std::make_shared<T>(shared_from_this());
        cluster->SetUp();
        mClusters[clusterId] = std::static_pointer_cast<BaseCluster>(cluster);
    }

    /**
     * @brief Returns a cluster of type T, if applicable. Returns nullptr otherwise
     */
    template <typename T>
    memory::Strong<T> GetCluster()
    {
        static_assert(std::is_base_of<BaseCluster, T>::value, "T must be derived from BaseCluster");
        ChipLogProgress(AppServer, "Endpoint::GetCluster() mClusters.size(): %d", static_cast<int>(mClusters.size()));
        for (const auto & pair : mClusters)
        {
            auto cluster = std::dynamic_pointer_cast<T>(pair.second);
            if (cluster)
            {
                return cluster;
            }
        }
        return nullptr;
    }

    void LogDetail() const
    {
        ChipLogProgress(AppServer, "Endpoint::LogDetail() Endpoint ID: %d, Vendor ID: %d, Product ID: %d", mAttributes.mId,
                        mAttributes.mVendorId, mAttributes.mProductId);
    }
};

}; // namespace core
}; // namespace casting
}; // namespace matter
