/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app/CommandHandler.h>
#include <app/data-model-provider/OperationTypes.h>

// Forward-declare to avoid include cycles
// (Cluster includes Delegate, so Delegate must not include Cluster)
namespace chip { namespace app { namespace Clusters { namespace CommissioningProxy { class CommissioningProxyCluster; }}}}

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() const { return mEndpointId; }

    /**
     * Optional back-pointer to the server cluster instance.
     * Default implementation is a no-op so existing delegates don't have to care.
     */
    virtual void SetServer(CommissioningProxyCluster * ) {}
    virtual CommissioningProxyCluster * GetServer() const { return nullptr; }

    /**
     * @brief Handles starting a Scan Request of required Transport and Bands
     *
     * @param transport  Which transports to use
     * @param wiFiBands  The frequency of the transports (if applicable)
     * @param commandObj The command handler object from the command
     * @param request    Invokde path
     * @return Success if successful
     *
     * The delegate handles all the transport specific options.
     */
    virtual Protocols::InteractionModel::Status
    ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan> address,
                        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBand,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request) = 0;

    /**
     * @brief Handles starting a Scan Request of required Transport and Bands
     *
     * @param transport  Which transports to use
     * @param wiFiBands  The frequency of the transports (if applicable)
     * @param commandObj The command handler object from the command
     * @param request    Invokde path
     * @return Success if successful
     *
     * The delegate handles all the transport specific options.
     */
    virtual Protocols::InteractionModel::Status
    ProxyScanRequest(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                     chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
                     chip::app::CommandHandler * commandObj,
                     const DataModel::InvokeRequest & request) = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual uint8_t GetScanMaxTime()         = 0;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual void SetScanMaxTime(uint8_t seconds) = 0;

protected:
    EndpointId mEndpointId = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
