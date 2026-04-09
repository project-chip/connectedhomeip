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
#include <app/AttributeValueEncoder.h>
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
     * @brief Handles connecting to a commissionable device discovered by the proxy
     *
     * @param address       Transport address of the device (null if not available)
     * @param transport     Which transport to use (exactly one bit set)
     * @param discriminator Discriminator of the device to connect to
     * @param vendorid      Vendor ID of the device (0 if not used)
     * @param productid     Product ID of the device (0 if not used)
     * @param timeout       Maximum time in seconds to wait for connection
     * @param wiFiBand      Wi-Fi band to use (only valid when WI feature enabled)
     * @param commandObj    The command handler object; delegate SHALL call AddResponse with ProxyConnectResponse
     * @param request       Invoke path
     * @return Success if connection was established and ProxyConnectResponse has been sent
     *
     * The delegate handles all transport-specific connection logic. On success, the delegate
     * SHALL call commandObj->AddResponse() with a ProxyConnectResponse containing the sessionId.
     * If the connection cannot be established, an appropriate error Status is returned.
     */
    virtual Protocols::InteractionModel::Status
    ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan> address,
                        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        uint16_t timeout,
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

    /**
     * @brief Forward a raw Matter packet to the commissionee via the proxy's
     *        transport (e.g. WiFi-PAF) and return the response in ProxyMessageResponse.
     *
     * @param sessionId      The proxy session ID from ProxyConnectResponse.
     * @param message        The raw Matter packet bytes to forward (nullable).
     * @param responseTimeout Maximum time in seconds to wait for the commissionee response.
     * @param commandObj     The command handler; delegate SHALL call AddResponse with
     *                       ProxyMessageResponse once the commissionee replies.
     * @param request        The invoke path.
     */
    virtual Protocols::InteractionModel::Status
    ProxyMessageRequest(uint16_t sessionId,
                        chip::Optional<chip::ByteSpan> message,
                        uint8_t responseTimeout,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request) = 0;

    /**
     * @brief Disconnect and clean up a proxy session established by ProxyConnectRequest.
     *
     * The delegate SHALL close the transport connection to the commissionee and
     * release all resources associated with the session.
     *
     * @param sessionId  The proxy session ID from ProxyConnectResponse to disconnect.
     * @return Success if the session was found and cleaned up; NotFound if unknown.
     */
    virtual Protocols::InteractionModel::Status
    ProxyDisconnectRequest(uint16_t sessionId) { return Protocols::InteractionModel::Status::UnsupportedCommand; }

    /**
     * @brief Start a continuous background scan for commissionable devices.
     *
     * Discovery results are cached in the CachedResults attribute; each result
     * has a per-entry TTL (CacheTimeout seconds) that resets on rediscovery and
     * removes the entry on expiry.
     *
     * @param transport     Transport(s) to scan on.
     * @param timeout       Scan lifetime in seconds; 0 = infinite (spec §10.5.7.6).
     * @param wiFiBands     Wi-Fi bands to scan (WI feature only).
     * @param fabricIndex   Fabric index of the requesting commissioner.
     * @param nodeId        Node ID of the requesting commissioner.
     * @param commandObj    Command handler (synchronous response — IM status only).
     * @param request       Invoke path.
     * @return Success, Busy (another caller's scan is running), or InvalidCommand.
     */
    virtual Protocols::InteractionModel::Status
    ProxyBackgroundScanStartRequest(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                                    uint16_t timeout,
                                    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
                                    chip::FabricIndex fabricIndex,
                                    chip::NodeId nodeId,
                                    app::CommandHandler * commandObj,
                                    const DataModel::InvokeRequest & request) = 0;

    /**
     * @brief Stop the background scan started by ProxyBackgroundScanStartRequest.
     *
     * Per spec §10.5.7.7: silently ignore if the requesting NodeId+FabricIndex
     * do not match the original requester — return Success in that case.
     *
     * @param transport     Transport(s) to stop.
     * @param wiFiBands     Wi-Fi bands (WI feature only).
     * @param fabricIndex   Fabric index of the requesting commissioner.
     * @param nodeId        Node ID of the requesting commissioner.
     * @return Success always (mismatch is silently ignored per spec).
     */
    virtual Protocols::InteractionModel::Status
    ProxyBackgroundScanStopRequest(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                                   chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
                                   chip::FabricIndex fabricIndex,
                                   chip::NodeId nodeId) = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual uint8_t GetScanMaxTime()        = 0;
    virtual uint8_t GetMaxCachedResults()   = 0;
    virtual uint8_t GetNumCachedResults()   = 0;
    virtual uint16_t GetCacheTimeout()      = 0;
    virtual chip::BitMask<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap> GetSupportedWiFiBands() = 0;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual void SetScanMaxTime(uint8_t seconds)    = 0;
    virtual void SetCacheTimeout(uint16_t seconds)  = 0;

    /**
     * @brief Encode the CachedResults attribute.
     *
     * Encodes DataModel::NullNullable when there are no cached results or no
     * active background scan; otherwise encodes the list of ScanResultStruct.
     */
    virtual CHIP_ERROR EncodeCachedResults(app::AttributeValueEncoder & encoder) = 0;

protected:
    EndpointId mEndpointId = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
