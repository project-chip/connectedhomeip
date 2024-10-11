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

#include "core/Endpoint.h"
#include "core/Types.h"

#include <app/server/Server.h>
#include <controller/CHIPCluster.h>

namespace matter {
namespace casting {

namespace core {
class EndpointAttributes;
};

namespace support {
/**
 * @brief EndpointListLoader builds Endpoints corresponding to the CastingPlayer::GetTargetCastingPlayer by reading Bindings and
 * fetching Endpoint attributes (like VendorID, ProductID, DeviceTypeList, ServerList, etc). It then loads all of these Endpoints
 * into the CastingPlayer::GetTargetCastingPlayer. Finally, it calls mOnCompleted() on the CastingPlayer::GetTargetCastingPlayer to
 * report the status of the Connection process to the client.
 */
class EndpointListLoader
{
public:
    static EndpointListLoader * GetInstance();

    /**
     * @brief Initializes EndpointListLoader with session info of the session the CastingApp just connected on.
     */
    void Initialize(chip::Messaging::ExchangeManager * exchangeMgr, const chip::SessionHandle * sessionHandle);

    /**
     * @brief Reads Bindings and fetches attributes (like VendorID, ProductID, DeviceTypeList, ServerList, etc) for each Endpoint
     * discovered in them.
     */
    CHIP_ERROR Load();

private:
    EndpointListLoader();
    static EndpointListLoader * _endpointListLoader;

    /**
     * @brief When all DesiredAttributes reads are completed, this function creates the list of Endpoints and loads/registers them
     * on the CastingPlayer::GetTargetCastingPlayer
     */
    void Complete();

    /**
     * @brief Fetches the VendorId for the endpoint with passed in endpointAttributes->mID
     */
    CHIP_ERROR ReadVendorId(core::EndpointAttributes * endpointAttributes);

    /**
     * @brief Fetches the ProductId for the endpoint with passed in endpointAttributes->mID
     */
    CHIP_ERROR ReadProductId(core::EndpointAttributes * endpointAttributes);

    /**
     * @brief Fetches the DeviceTypeList for the endpoint with passed in endpointAttributes->mID
     */
    CHIP_ERROR ReadDeviceTypeList(core::EndpointAttributes * endpointAttributes);

    /**
     * @brief Fetches the ServerList for the endpoint with passed in endpointAttributes->mID
     */
    CHIP_ERROR ReadServerList(std::vector<chip::ClusterId> * endpointServerList, chip::EndpointId endpointId);

    chip::Messaging::ExchangeManager * mExchangeMgr     = nullptr;
    const chip::SessionHandle * mSessionHandle          = nullptr;
    unsigned long mNewEndpointsToLoad                   = 0;
    unsigned long mPendingAttributeReads                = 0;
    core::EndpointAttributes * mEndpointAttributesList  = nullptr;
    std::vector<chip::ClusterId> * mEndpointServerLists = nullptr;
};

/**
 * @brief Enumerated list of all Endpoint Attributes that need to be fetched
 */
enum DesiredAttributes
{
    kVendorId = 0,
    kProductId,
    kDeviceTypeList,
    kServerList,

    kTotalDesiredAttributes
};

}; // namespace support
}; // namespace casting
}; // namespace matter
