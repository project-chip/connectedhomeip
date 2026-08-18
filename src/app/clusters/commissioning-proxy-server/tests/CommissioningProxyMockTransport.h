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
        {
            return Protocols::InteractionModel::Status::InvalidInState;
        }
        if (fabricIndex != mPendingConnectFabric)
        {
            return Protocols::InteractionModel::Status::NotFound;
        }

        // The connect is no longer pending once it has been cancelled.
        mPendingConnectFabric = kUndefinedFabricIndex;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status Disconnect(uint16_t sessionId) override { return mDisconnectStatus; }
    CHIP_ERROR SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf) override;
    Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime) override;
    Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands, FabricIndex fabricIndex,
                                                    NodeId nodeId) override
    {
        if (mBgScanStartStatus == Protocols::InteractionModel::Status::Success)
        {
            mBgScanRunning = true;
        }
        return mBgScanStartStatus;
    }
    Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId) override
    {
        mBgScanStopCount++;
        mBgScanRunning = false;
        return mBgScanStopStatus;
    }
    void OnAllSessionsClosed() override { mOnAllSessionsClosedCount++; }
    void OnFabricRemoved(FabricIndex fabricIndex) override
    {
        mFabricRemovedCount++;
        mLastRemovedFabric = fabricIndex;
    }
    bool IsConnectPending() const override { return mConnectPending; }
    void Shutdown() override {}

    // --- Test Control  ---------------------------------------------------------
    void SetConnectStatus(Protocols::InteractionModel::Status s) { mConnectStatus = s; }
    void SetConnectPending(bool p) { mConnectPending = p; }
    // Simulate a pending ProxyConnectRequest owned by @p f (kUndefinedFabricIndex =
    // none), for the null-SessionID CancelPendingConnect paths.
    void SetPendingConnectFabric(FabricIndex f) { mPendingConnectFabric = f; }
    void SetDisconnectStatus(Protocols::InteractionModel::Status s) { mDisconnectStatus = s; }
    void SetScanStatus(Protocols::InteractionModel::Status s) { mScanStatus = s; }
    void SetBgScanStartStatus(Protocols::InteractionModel::Status s) { mBgScanStartStatus = s; }
    /// True once BgScanStart() succeeded and no BgScanStop() has followed.
    bool BgScanRunning() const { return mBgScanRunning; }
    uint32_t BgScanStopCount() const { return mBgScanStopCount; }
    void SetBgScanStopStatus(Protocols::InteractionModel::Status s) { mBgScanStopStatus = s; }
    void SetSendMessageError(CHIP_ERROR e) { mSendMessageError = e; }
    // When true (default), a successful SendMessage synchronously delivers a null
    // ProxyMessageResponse back through the session manager. Set false to leave the
    // request pending, as a commissionee that never replies would (e.g. to exercise the
    // BUSY path on a second request, or the session's response timeout).
    void SetAutoRespond(bool a) { mAutoRespond = a; }
    // When true (default), a successful Scan synchronously contributes results to the
    // aggregator. Set false to leave the foreground scan in-flight (e.g. to exercise
    // the concurrent-scan BUSY path on a second ProxyScanRequest).
    void SetAutoContribute(bool a) { mAutoContribute = a; }
    /// Deliver this transport's two scan results to the aggregator now, standing in for
    /// the driver's own scan-completion callback. Pairs with SetAutoContribute(false).
    void ContributeScanResults();
    /// Report an asynchronous failure for @p sessionId's pending ProxyMessageRequest, as a
    /// driver does when its transport connection drops mid-exchange.
    void FailPendingMessage(uint16_t sessionId, Protocols::InteractionModel::Status status);

    uint16_t LastSessionId() const { return mLastSessionId; }
    uint8_t OnAllSessionsClosedCount() const { return mOnAllSessionsClosedCount; }
    bool CancelCalled() const { return mCancelCalled; }
    uint32_t FabricRemovedCount() const { return mFabricRemovedCount; }
    FabricIndex LastRemovedFabric() const { return mLastRemovedFabric; }
    FabricIndex LastCancelFabric() const { return mLastCancelFabric; }

private:
    CapabilitiesBitmap mType;
    CommissioningProxyCluster * mHost = nullptr;

    Protocols::InteractionModel::Status mConnectStatus     = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mDisconnectStatus  = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mScanStatus        = Protocols::InteractionModel::Status::Success;
    bool mBgScanRunning                                    = false;
    uint32_t mBgScanStopCount                              = 0;
    Protocols::InteractionModel::Status mBgScanStartStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mBgScanStopStatus  = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR mSendMessageError                           = CHIP_NO_ERROR;
    bool mAutoRespond                                      = true;
    bool mAutoContribute                                   = true;
    bool mConnectPending                                   = false;
    uint16_t mLastSessionId                                = 0;
    uint8_t mOnAllSessionsClosedCount                      = 0;
    bool mCancelCalled                                     = false;
    FabricIndex mLastCancelFabric                          = kUndefinedFabricIndex;
    FabricIndex mPendingConnectFabric                      = kUndefinedFabricIndex;
    uint32_t mFabricRemovedCount                           = 0;
    FabricIndex mLastRemovedFabric                         = kUndefinedFabricIndex;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
