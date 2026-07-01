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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyMockDelegate : public Delegate
{
public:
    CommissioningProxyMockDelegate();
    ~CommissioningProxyMockDelegate() override; // = default;

    Protocols::InteractionModel::Status
    ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan> address,
                        chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap> transport,
                        uint16_t discriminator, chip::VendorId vendorid, uint16_t productid, uint16_t timeout,
                        chip::BitMask<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap> wiFiBand,
                        app::CommandHandler * commandObj, const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyScanRequest(chip::BitMask<CapabilitiesBitmap> transport,
                                                         chip::BitMask<WiFiBandBitmap> wiFiBands, app::CommandHandler * commandObj,
                                                         const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyMessageRequest(uint16_t sessionId, chip::Optional<chip::ByteSpan> message,
                                                            uint8_t responseTimeout, app::CommandHandler * commandObj,
                                                            const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyDisconnectRequest(uint16_t sessionId, chip::FabricIndex fabricIndex) override;

    Protocols::InteractionModel::Status ProxyBackgroundScanStartRequest(
        chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap> transport, uint16_t timeout,
        chip::BitMask<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap> wiFiBands, chip::FabricIndex fabricIndex,
        chip::NodeId nodeId, app::CommandHandler * commandObj, const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status
    ProxyBackgroundScanStopRequest(chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap> transport,
                                   chip::BitMask<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap> wiFiBands,
                                   chip::FabricIndex fabricIndex, chip::NodeId nodeId) override;

    uint8_t GetMaxSessions() override;
    void SetMaxSessions(uint8_t v) { mMaxSessions = v; } // test helper
    uint8_t GetActiveSessionCount() override { return mActiveSessionCount; }
    void SetActiveSessionCount(uint8_t v) { mActiveSessionCount = v; } // test helper
    uint8_t GetMaxCachedResults() override { return 10; }
    uint8_t GetNumCachedResults() override { return 0; }
    uint16_t GetCacheTimeout() override { return mCacheTimeout; }
    void SetCacheTimeout(uint16_t v) override { mCacheTimeout = v; }
    CHIP_ERROR EncodeCachedResults(app::AttributeValueEncoder &) override { return CHIP_NO_ERROR; }

    void SetServer(CommissioningProxyCluster * server) override { mServer = server; }
    CommissioningProxyCluster * GetServer() const override { return mServer; }

    uint8_t GetScanMaxTime() override;
    void SetScanMaxTime(uint8_t seconds) override;
    chip::BitMask<WiFiBandBitmap> GetSupportedWiFiBands() override { return mSupportedWiFiBands; }
    void SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap> bands) { mSupportedWiFiBands = bands; }

private:
    CommissioningProxyCluster * mServer = nullptr;
    uint8_t mScanMaxTime                = 120;
    chip::BitMask<WiFiBandBitmap> mSupportedWiFiBands;
    uint8_t mMaxSessions        = 1;
    uint8_t mActiveSessionCount = 0;   // settable via SetActiveSessionCount() for MaxSessions tests
    uint16_t mCacheTimeout      = 120; // spec default
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
