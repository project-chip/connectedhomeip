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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBgScanRegistry.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>
#include <app/data-model-provider/OperationTypes.h>
#include <ble/Ble.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief BLE (BTP) transport driver for the Commissioning Proxy cluster.
 *
 * Implements the connect / relay / disconnect flow over chip::Ble: it drives the BTP
 * handshake against a commissionee, keeps the sessionId -> BLEEndPoint mapping, and
 * relays ProxyMessage payloads. All transport-agnostic bookkeeping — session
 * allocation, ProxyMessage routing, the scan cache and multi-transport scan
 * aggregation — belongs to the host cluster and is reached through its subsystem
 * accessors.
 *
 * The two things BLE cannot express portably (switching the local role to central,
 * and driving a scan for commissionable devices) come from the injected
 * CommissioningProxyBleAdapter, so nothing here includes a platform header.
 *
 * Not copyable and not relocatable once started: it hands @c this to BleLayer, to the
 * adapter, and to the timer delegate as an opaque context.
 */
class CommissioningProxyBleTransport : public CommissioningProxyTransport
{
public:
    /**
     * @param adapter        platform BLE hooks; must outlive this transport.
     * @param timerDelegate  drives the connect timeout and the foreground-scan window,
     *                       and is handed to the background-scan registry.
     */
    CommissioningProxyBleTransport(CommissioningProxyBleAdapter & adapter, TimerDelegate & timerDelegate);
    ~CommissioningProxyBleTransport() override;

    CommissioningProxyBleTransport(const CommissioningProxyBleTransport &)             = delete;
    CommissioningProxyBleTransport & operator=(const CommissioningProxyBleTransport &) = delete;

    CapabilitiesBitmap GetTransportType() const override { return CapabilitiesBitmap::kBle; }
    void SetHost(CommissioningProxyCluster * host) override;

    Protocols::InteractionModel::Status Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                                uint16_t discriminator, System::Clock::Seconds16 timeout) override;
    Protocols::InteractionModel::Status CancelPendingConnect(FabricIndex fabricIndex) override;
    Protocols::InteractionModel::Status Disconnect(uint16_t sessionId) override;
    CHIP_ERROR SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf) override;
    Protocols::InteractionModel::Status Scan(System::Clock::Seconds16 scanMaxTime) override;
    Protocols::InteractionModel::Status BgScanStart(System::Clock::Seconds16 timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                    FabricIndex fabricIndex, NodeId nodeId) override;
    Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId) override;
    void OnFabricRemoved(FabricIndex fabricIndex) override;
    void OnAllSessionsClosed() override;
    bool IsConnectPending() const override;
    void Shutdown() override;

private:
    using ScanResultT = Structs::ScanResultStruct::Type;

    static constexpr size_t kMaxSessions    = CHIP_CONFIG_COMMISSIONING_PROXY_MAX_SESSIONS;
    static constexpr size_t kMaxScanResults = CHIP_CONFIG_COMMISSIONING_PROXY_MAX_CACHED_RESULTS;

    /// One promoted proxy session and the endpoint carrying it. `inUse` false marks a
    /// free slot, matching the fixed-slot idiom the cluster's own subsystems use.
    struct EndpointSlot
    {
        bool inUse                  = false;
        uint16_t sessionId          = 0;
        Ble::BLEEndPoint * endpoint = nullptr;
    };

    /// State of the single in-flight ProxyConnectRequest. Held inline (MaxSessions == 1
    /// makes it unique) rather than heap-allocated.
    struct ConnectCtx
    {
        app::CommandHandler::Handle handle;
        app::ConcreteCommandPath path;
        uint16_t discriminator              = 0;
        CommissioningProxyCluster * cluster = nullptr;
        FabricIndex fabricIndex             = kUndefinedFabricIndex;
        Ble::BLEEndPoint * endpoint         = nullptr; ///< set when the L2CAP wrap completes
    };

    /// Compact record for foreground-scan dedup. Deliberately smaller than a
    /// ScanResultStruct: only what the spec dedups on, plus the address to report.
    struct ScanRecord
    {
        uint8_t mac[6]         = {};
        uint16_t discriminator = 0;
        uint16_t vendorId      = 0;
        uint16_t productId     = 0;
    };

    /**
     * Wraps BleLayer::mBleTransport so central-role events for proxy-owned endpoints
     * come back here, while everything else falls through to the delegate that was
     * installed before (the application's own peripheral commissioning).
     */
    class ProxyBleDelegate : public Ble::BleLayerDelegate
    {
    public:
        explicit ProxyBleDelegate(CommissioningProxyBleTransport & owner) : mOwner(owner) {}

        void Install();
        void Uninstall();

        void OnBleConnectionComplete(Ble::BLEEndPoint * endpoint) override;
        void OnBleConnectionError(CHIP_ERROR err) override;
        void OnEndPointConnectComplete(Ble::BLEEndPoint * endpoint, CHIP_ERROR err) override;
        void OnEndPointMessageReceived(Ble::BLEEndPoint * endpoint, System::PacketBufferHandle && msg) override;
        void OnEndPointConnectionClosed(Ble::BLEEndPoint * endpoint, CHIP_ERROR err) override;
        CHIP_ERROR SetEndPoint(Ble::BLEEndPoint * endpoint) override;

    private:
        CommissioningProxyBleTransport & mOwner;
        Ble::BleLayerDelegate * mOriginalTransport = nullptr;
    };

    /// Supplies the shared background-scan registry with the BLE start/stop/clear hooks.
    /// The registry owns the per-fabric records, lifetime timers and paused state.
    class BgScanHardware : public CommissioningProxyBgScanRegistry::HardwareControl
    {
    public:
        explicit BgScanHardware(CommissioningProxyBleTransport & owner) : mOwner(owner) {}

        CHIP_ERROR StartHardwareScan() override;
        void StopHardwareScan() override;
        void ClearCachedResults(BitMask<WiFiBandBitmap> bands) override;

    private:
        CommissioningProxyBleTransport & mOwner;
    };

    /// Separate TimerContexts because TimerDelegate keys a timer on its context, and the
    /// connect timeout and the scan window can be armed at the same time.
    class ConnectTimeoutTimer : public TimerContext
    {
    public:
        explicit ConnectTimeoutTimer(CommissioningProxyBleTransport & owner) : mOwner(owner) {}
        void TimerFired() override { mOwner.OnConnectTimeout(); }

    private:
        CommissioningProxyBleTransport & mOwner;
    };

    class ScanWindowTimer : public TimerContext
    {
    public:
        explicit ScanWindowTimer(CommissioningProxyBleTransport & owner) : mOwner(owner) {}
        void TimerFired() override { mOwner.OnScanWindowExpired(); }

    private:
        CommissioningProxyBleTransport & mOwner;
    };

    // --- BleLayer connect callbacks. appState is always `this`. ---
    static void HandleConnectFound(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void HandleConnectError(void * appState, CHIP_ERROR err);

    // --- Adapter discovery callbacks. context is always `this`. ---
    static void HandleForegroundScanResult(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId);
    static void HandleBackgroundScanResult(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId);

    void OnConnectTimeout();
    void OnScanWindowExpired();

    /// Build the kBle ScanResultStruct the cluster stores. Shared by the foreground and
    /// background paths so the field set cannot drift between them.
    static ScanResultT MakeScanResult(ByteSpan address, uint16_t discriminator, uint16_t vendorId, uint16_t productId);

    Ble::BleLayer * GetBleLayer() const;

    EndpointSlot * FindSlot(uint16_t sessionId);
    EndpointSlot * FindSlot(const Ble::BLEEndPoint * endpoint);
    EndpointSlot * ClaimSlot();
    bool AnySessionOpen() const;

    /// Complete the pending connect's IM exchange with @p status and release it, then
    /// hand the scanner back to a background scan that was paused for it.
    void FailPendingConnect(Protocols::InteractionModel::Status status);

    void PauseBgScan() { mBgScan.Pause(); }
    void ResumeBgScanIfNeeded() { mBgScan.ResumeIfNeeded(); }

    CommissioningProxyBleAdapter & mAdapter;
    TimerDelegate & mTimerDelegate;

    ProxyBleDelegate mProxyDelegate{ *this };
    BgScanHardware mBgScanHardware{ *this };
    ConnectTimeoutTimer mConnectTimer{ *this };
    ScanWindowTimer mScanTimer{ *this };

    /// Declared after mBgScanHardware so the registry is destroyed first: its destructor
    /// may call back into the hardware hooks.
    CommissioningProxyBgScanRegistry mBgScan;

    CommissioningProxyCluster * mHost = nullptr;

    EndpointSlot mEndpoints[kMaxSessions];
    std::optional<ConnectCtx> mPendingConnect;

    /// The endpoint this transport owns that carries no session: during the BTP handshake,
    /// before a SessionID exists, and inside Disconnect() after the slot is freed but
    /// before Close() returns. Endpoints carrying a session live in mEndpoints instead.
    /// OnEndPointConnectionClosed consults both; one matching neither belongs to the
    /// application, and its close is forwarded to the original delegate.
    Ble::BLEEndPoint * mSessionlessEndpoint = nullptr;

    ScanRecord mScanResults[kMaxScanResults];
    size_t mScanResultCount = 0;
    bool mScanInProgress    = false;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
