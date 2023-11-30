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

#include "CastingPlayer.h"
#include "Cluster.h"
#include "Types.h"

#include "lib/support/logging/CHIPLogging.h"
#include <app-common/zap-generated/cluster-objects.h>

#include <iostream>
#include <map>
#include <memory>
#include <type_traits>

namespace matter {
namespace casting {
namespace core {

class EndpointAttributes
{
public:
    chip::EndpointId mId = 0;
    uint16_t mVendorId   = 0;
    uint16_t mProductId  = 0;
    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> mDeviceTypeList;
};

class CastingPlayer;

class Endpoint : public std::enable_shared_from_this<Endpoint>
{

private:
    CastingPlayer * mCastingPlayer;

    EndpointAttributes mAttributes;
    std::map<chip::ClusterId, memory::Strong<BaseCluster>> mClusters;

protected:
    CastingPlayer * GetCastingPlayer() const { return mCastingPlayer; }

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

    /**
     * @brief Compares based on the Id
     */
    bool operator==(const Endpoint & other) const { return this->mAttributes.mId == other.mAttributes.mId; }

    chip::EndpointId GetId() const { return mAttributes.mId; }

    uint16_t GetProductId() const { return mAttributes.mProductId; }

    uint16_t GetVendorId() const { return mAttributes.mVendorId; }

    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> GetDeviceTypeList() const
    {
        return mAttributes.mDeviceTypeList;
    }

    std::vector<chip::ClusterId> GetServerList()
    {
        std::vector<chip::ClusterId> serverList;
        for (auto const & cluster : mClusters)
        {
            serverList.push_back(cluster.first);
        }
        return serverList;
    }

    template <typename T>
    void RegisterCluster(const chip::ClusterId clusterId)
    {
        static_assert(std::is_base_of<BaseCluster, T>::value, "T must be derived from BaseCluster");
        auto cluster         = std::make_shared<T>(shared_from_this());
        mClusters[clusterId] = std::static_pointer_cast<BaseCluster>(cluster);
    }

    template <typename T>
    memory::Strong<T> GetCluster()
    {
        static_assert(std::is_base_of<BaseCluster, T>::value, "T must be derived from BaseCluster");
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
};

}; // namespace core
}; // namespace casting
}; // namespace matter
