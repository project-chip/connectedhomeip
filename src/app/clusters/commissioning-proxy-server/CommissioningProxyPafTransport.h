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
#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyScanCache.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>
#include <app/data-model-provider/OperationTypes.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>
#include <wifipaf/WiFiPAFLayerDelegate.h>
#include <wifipaf/WiFiPAFRole.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Wi-Fi PAF (PAFTP) transport driver for the Commissioning Proxy cluster.
 *
 * Implements the connect / relay / disconnect flow over chip::WiFiPAF: it drives the
 * NAN subscribe and PAFTP session against a commissionee, keeps the
 * sessionId -> WiFiPAFSession mapping, and relays ProxyMessage payloads. All
 * transport-agnostic bookkeeping — session allocation, ProxyMessage routing, the scan
 * cache and multi-transport scan aggregation — belongs to the host cluster and is
 * reached through its subsystem accessors.
 *
 * Discovery has no portable form, so scanning and the platform's subscribe id come from
 * the injected CommissioningProxyPafAdapter; nothing here includes a platform header.
 *
 * Not copyable and not relocatable once started: it hands @c this to the WiFiPAF layer,
 * to the adapter, and to the timer delegate as an opaque context.
 */
class CommissioningProxyPafTransport : public CommissioningProxyTransport
{
public:
    /**
     * @param adapter        platform PAF discovery hooks; must outlive this transport.
     * @param timerDelegate  drives the connect timeout, and is handed to the
     *                       background-scan registry.
     */
    /**
     * @p fabricTable is watched so the proxy's own NAN publish receive handler can be
     * dropped once the proxy is commissioned; a later subscribe would otherwise leave
     * the platform with two handlers for the same traffic. May be nullptr where no
     * FabricTable exists (unit tests), in which case the handler is left alone.
     */
    CommissioningProxyPafTransport(CommissioningProxyPafAdapter & adapter, TimerDelegate & timerDelegate,
                                   FabricTable * fabricTable = nullptr);
    ~CommissioningProxyPafTransport() override;

    CommissioningProxyPafTransport(const CommissioningProxyPafTransport &)             = delete;
    CommissioningProxyPafTransport & operator=(const CommissioningProxyPafTransport &) = delete;

    CapabilitiesBitmap GetTransportType() const override { return CapabilitiesBitmap::kWiFiPAF; }
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

    /// Taken from the cache rather than restated, so a result the transport accepts is by
    /// construction one the cache can also hold.
    static constexpr size_t kMaxAddressBytes      = CommissioningProxyScanCache::kMaxAddressBytes;
    static constexpr size_t kMaxExtendedDataBytes = CommissioningProxyScanCache::kMaxExtendedDataBytes;

    /// One live proxy session and the PAF session carrying it. `inUse` false marks a free
    /// slot, matching the fixed-slot idiom the cluster's own subsystems use.
    struct SessionSlot
    {
        bool inUse                         = false;
        uint16_t sessionId                 = 0;
        WiFiPAF::WiFiPAFSession pafSession = {};
    };

    /// State of the single in-flight ProxyConnectRequest. Held inline (MaxSessions == 1
    /// makes it unique) rather than heap-allocated.
    struct ConnectCtx
    {
        app::CommandHandler::Handle handle;
        app::ConcreteCommandPath path;
        uint16_t discriminator              = 0;
        uint32_t subscribeId                = 0; ///< NAN subscribe id, from the adapter
        CommissioningProxyCluster * cluster = nullptr;
        FabricIndex fabricIndex             = kUndefinedFabricIndex;
    };

    /// Compact record for foreground-scan dedup, carrying its own copy of the variable
    /// length fields so nothing points at adapter memory once the callback returns.
    struct ScanRecord
    {
        uint8_t address[kMaxAddressBytes]           = {};
        uint8_t addressLen                          = 0;
        uint16_t discriminator                      = 0;
        uint16_t vendorId                           = 0;
        uint16_t productId                          = 0;
        uint8_t extendedData[kMaxExtendedDataBytes] = {};
        uint8_t extendedDataLen                     = 0;
        uint16_t wiFiBand                           = 0;
    };

    /**
     * Wraps WiFiPAFLayer::mWiFiPAFTransport so PAFTP traffic for proxy sessions is
     * relayed back to the commissioner, while everything else falls through to the
     * delegate that was installed before (the application's own PAF commissioning).
     */
    class ProxyPafDelegate : public WiFiPAF::WiFiPAFLayerDelegate
    {
    public:
        explicit ProxyPafDelegate(CommissioningProxyPafTransport & owner) : mOwner(owner) {}

        void Install();
        void Uninstall();

        CHIP_ERROR WiFiPAFMessageReceived(WiFiPAF::WiFiPAFSession & rxInfo, System::PacketBufferHandle && msg) override;
        CHIP_ERROR WiFiPAFMessageSend(WiFiPAF::WiFiPAFSession & txInfo, System::PacketBufferHandle && msg) override;
        CHIP_ERROR WiFiPAFCloseSession(WiFiPAF::WiFiPAFSession & sessionInfo) override;
        bool WiFiPAFResourceAvailable() override;

    private:
        CommissioningProxyPafTransport & mOwner;
        WiFiPAF::WiFiPAFLayerDelegate * mOriginalTransport = nullptr;
    };

    /// Supplies the shared background-scan registry with the PAF start/stop/clear hooks.
    class BgScanHardware : public CommissioningProxyBgScanRegistry::HardwareControl
    {
    public:
        explicit BgScanHardware(CommissioningProxyPafTransport & owner) : mOwner(owner) {}

        CHIP_ERROR StartHardwareScan() override;
        void StopHardwareScan() override;
        void ClearCachedResults(BitMask<WiFiBandBitmap> bands) override;

    private:
        CommissioningProxyPafTransport & mOwner;
    };

    class ConnectTimeoutTimer : public TimerContext
    {
    public:
        explicit ConnectTimeoutTimer(CommissioningProxyPafTransport & owner) : mOwner(owner) {}
        void TimerFired() override { mOwner.OnConnectTimeout(); }

    private:
        CommissioningProxyPafTransport & mOwner;
    };

    /// Resuming the background scan has to be deferred: it can be reached from inside a
    /// PAFTP endpoint-close callstack, and starting a scan makes a blocking D-Bus call, so
    /// running it inline would re-enter the WiFiPAF layer mid-teardown. A zero-delay timer
    /// rather than PlatformMgr().ScheduleWork() because a timer can be cancelled - queued
    /// work cannot, and would outlive this object.
    class ResumeBgScanTimer : public TimerContext
    {
    public:
        explicit ResumeBgScanTimer(CommissioningProxyPafTransport & owner) : mOwner(owner) {}
        void TimerFired() override { mOwner.mBgScan.ResumeIfNeeded(); }

    private:
        CommissioningProxyPafTransport & mOwner;
    };

    // --- WiFiPAF connect callbacks. context is always `this`. ---
    static void HandleConnectSuccess(void * context);
    static void HandleConnectError(void * context, CHIP_ERROR err);

    // --- Adapter discovery callbacks. context is always `this`. ---
    static void HandleForegroundScanResult(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId, ByteSpan extendedData, uint16_t wiFiBand);
    static void HandleForegroundScanDone(void * context);
    static void HandleBackgroundScanResult(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId, ByteSpan extendedData, uint16_t wiFiBand);

    void OnConnectTimeout();

    /// Build the kWiFiPAF ScanResultStruct the cluster stores. Shared by the foreground
    /// and background paths so the field set cannot drift between them.
    static ScanResultT MakeScanResult(ByteSpan address, uint16_t discriminator, uint16_t vendorId, uint16_t productId,
                                      ByteSpan extendedData, uint16_t wiFiBand);

    SessionSlot * FindSlot(uint16_t sessionId);
    SessionSlot * FindSlotByPeer(uint32_t peerId);
    SessionSlot * ClaimSlot();
    bool AnySessionOpen() const;

    /// Tear down an in-flight connect that did not succeed: cancel the subscribe, close
    /// any PAFTP endpoint the handshake created, drop the PAF session, answer the
    /// originating ProxyConnectRequest with @p status, and release the subscribe slot.
    /// @p cancelTimer is false only when called from the timeout handler itself.
    void FailPendingConnect(Protocols::InteractionModel::Status status, bool cancelTimer);

    CommissioningProxyPafAdapter & mAdapter;
    TimerDelegate & mTimerDelegate;

    ProxyPafDelegate mProxyDelegate{ *this };
    BgScanHardware mBgScanHardware{ *this };
    ConnectTimeoutTimer mConnectTimer{ *this };
    ResumeBgScanTimer mResumeBgScanTimer{ *this };

    /// Declared after mBgScanHardware so the registry is destroyed first: its destructor
    /// may call back into the hardware hooks.
    CommissioningProxyBgScanRegistry mBgScan;

    CommissioningProxyCluster * mHost = nullptr;

    /// Drops the publish receive handler as soon as the proxy's own commissioning ends.
    static void OnDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    FabricTable * mFabricTable = nullptr;
    /// Whether OnDeviceEvent is currently registered with the platform manager.
    bool mPublishHandlerArmed = false;

    SessionSlot mSessions[kMaxSessions];
    std::optional<ConnectCtx> mPendingConnect;

    ScanRecord mScanResults[kMaxScanResults];
    size_t mScanResultCount = 0;
    bool mScanInProgress    = false;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
