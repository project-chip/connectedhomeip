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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Test double for a platform transport driver.
 *
 * Simulates a transport that completes synchronously so unit tests can drive the
 * cluster's dispatch/orchestration without a real BLE/PAF stack or event loop.
 * Each behaviour has a knob so a test can force a specific status or error path.
 */
class CommissioningProxyMockTransport : public CommissioningProxyTransport
{
public:
    explicit CommissioningProxyMockTransport(CapabilitiesBitmap type = CapabilitiesBitmap::kBle) : mType(type) {}

    CapabilitiesBitmap GetTransportType() const override { return mType; }
    void SetHost(CommissioningProxyCluster * host) override { mHost = host; }

    Protocols::InteractionModel::Status Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                                uint16_t discriminator, uint16_t timeout) override;
    Protocols::InteractionModel::Status CancelPendingConnect(FabricIndex fabricIndex) override
    {
        mCancelCalled     = true;
        mLastCancelFabric = fabricIndex;
        // Model a real driver: InvalidInState if no connect is pending; Success if
        // the pending connect is owned by this fabric; NotFound otherwise.
        if (mPendingConnectFabric == kUndefinedFabricIndex)
            return Protocols::InteractionModel::Status::InvalidInState;
        return (fabricIndex == mPendingConnectFabric) ? Protocols::InteractionModel::Status::Success
                                                      : Protocols::InteractionModel::Status::NotFound;
    }
    Protocols::InteractionModel::Status Disconnect(uint16_t sessionId) override { return mDisconnectStatus; }
    CHIP_ERROR SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf) override;
    Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime) override;
    Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands, FabricIndex fabricIndex,
                                                    NodeId nodeId) override
    {
        return mBgScanStartStatus;
    }
    Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId) override
    {
        return mBgScanStopStatus;
    }
    void OnAllSessionsClosed() override { mOnAllSessionsClosedCount++; }
    bool IsConnectPending() const override { return mConnectPending; }
    void Shutdown() override {}

    // --- Test knobs ---------------------------------------------------------
    void SetConnectStatus(Protocols::InteractionModel::Status s) { mConnectStatus = s; }
    void SetConnectPending(bool p) { mConnectPending = p; }
    // Simulate a pending ProxyConnectRequest owned by @p f (kUndefinedFabricIndex =
    // none), for the null-SessionID CancelPendingConnect paths.
    void SetPendingConnectFabric(FabricIndex f) { mPendingConnectFabric = f; }
    void SetDisconnectStatus(Protocols::InteractionModel::Status s) { mDisconnectStatus = s; }
    void SetScanStatus(Protocols::InteractionModel::Status s) { mScanStatus = s; }
    void SetBgScanStartStatus(Protocols::InteractionModel::Status s) { mBgScanStartStatus = s; }
    void SetBgScanStopStatus(Protocols::InteractionModel::Status s) { mBgScanStopStatus = s; }
    void SetSendMessageError(CHIP_ERROR e) { mSendMessageError = e; }
    // When true (default), a successful SendMessage synchronously delivers a null
    // ProxyMessageResponse back through the session manager. Set false to leave the
    // request pending (e.g. to exercise the BUSY path on a second request).
    void SetAutoRespond(bool a) { mAutoRespond = a; }
    // When true (default), a successful Scan synchronously contributes results to the
    // aggregator. Set false to leave the foreground scan in-flight (e.g. to exercise
    // the concurrent-scan BUSY path on a second ProxyScanRequest).
    void SetAutoContribute(bool a) { mAutoContribute = a; }
    // When true, a successful SendMessage synchronously delivers a TIMEOUT failure for
    // the pending ProxyMessageRequest (models the commissionee never replying).
    void SetSendMessageTimeout(bool t) { mSendMessageTimeout = t; }

    uint16_t LastSessionId() const { return mLastSessionId; }
    uint8_t OnAllSessionsClosedCount() const { return mOnAllSessionsClosedCount; }
    bool CancelCalled() const { return mCancelCalled; }
    FabricIndex LastCancelFabric() const { return mLastCancelFabric; }

private:
    CapabilitiesBitmap mType;
    CommissioningProxyCluster * mHost = nullptr;

    Protocols::InteractionModel::Status mConnectStatus     = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mDisconnectStatus  = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mScanStatus        = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mBgScanStartStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mBgScanStopStatus  = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR mSendMessageError                           = CHIP_NO_ERROR;
    bool mAutoRespond                                      = true;
    bool mAutoContribute                                   = true;
    bool mSendMessageTimeout                               = false;
    bool mConnectPending                                   = false;
    uint16_t mLastSessionId                                = 0;
    uint8_t mOnAllSessionsClosedCount                      = 0;
    bool mCancelCalled                                     = false;
    FabricIndex mLastCancelFabric                          = kUndefinedFabricIndex;
    FabricIndex mPendingConnectFabric                      = kUndefinedFabricIndex;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
