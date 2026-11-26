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

#include "CommissioningProxyMockTimer.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <ble/Ble.h>
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissioningProxy;
using namespace chip::app::Clusters::CommissioningProxy::Commands;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr size_t kMaxCachedResults   = CHIP_CONFIG_COMMISSIONING_PROXY_MAX_CACHED_RESULTS;

/// Stands in for BLEManagerImpl. Records what the transport asked the platform to do and
/// lets a test play discovery results back through the callback the transport registered.
class FakeBleProxyAdapter : public CommissioningProxyBleAdapter
{
public:
    CHIP_ERROR EnableCentralRole() override
    {
        enableCentralRoleCalls++;
        return enableCentralRoleResult;
    }

    CHIP_ERROR StartScan(DiscoveryCallback cb, void * context) override
    {
        startScanCalls++;
        if (mStartScanResult != CHIP_NO_ERROR)
        {
            return mStartScanResult;
        }
        mCallback = cb;
        mContext  = context;
        scanning  = true;
        return CHIP_NO_ERROR;
    }

    void StopScan() override
    {
        stopScanCalls++;
        scanning  = false;
        mCallback = nullptr;
        mContext  = nullptr;
        // Retires the scan, so anything it queued is stale even if a later scan
        // re-registers a callback. Mirrors ReleaseProxyScanForwarder() on Linux.
        mScanGeneration++;
    }

    /// A discovery result the platform has queued but not yet delivered.
    ///
    /// Models the Linux path: ProxyScanForwarder::OnDeviceScanned copies the result on the
    /// GLib scan thread and posts it with PlatformMgr().ScheduleWork(), so the work item
    /// outlives the scan that produced it. It is stamped with the scan's generation and
    /// dropped at dispatch if that scan has since been stopped, which is the guarantee
    /// CommissioningProxyBleAdapter::StopScan() documents and DispatchProxyScanResult
    /// enforces on Linux.
    class PendingResult
    {
    public:
        PendingResult(FakeBleProxyAdapter & adapter, uint16_t discriminator, uint16_t vendorId, uint16_t productId,
                      uint8_t addressTag) :
            mAdapter(adapter),
            mDiscriminator(discriminator), mVendorId(vendorId), mProductId(productId), mAddressTag(addressTag),
            mGeneration(adapter.mScanGeneration)
        {}

        void Dispatch()
        {
            // Order mirrors Linux: DispatchProxyScanResult compares the generation, and
            // only a surviving result reaches OnPlatformScanResult's callback check.
            if (mAdapter.mScanGeneration != mGeneration || mAdapter.mCallback == nullptr)
            {
                return;
            }
            const uint8_t mac[6] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, mAddressTag };
            mAdapter.mCallback(mAdapter.mContext, ByteSpan(mac, sizeof(mac)), mDiscriminator, mVendorId, mProductId);
        }

    private:
        FakeBleProxyAdapter & mAdapter;
        uint16_t mDiscriminator;
        uint16_t mVendorId;
        uint16_t mProductId;
        uint8_t mAddressTag;
        uint32_t mGeneration;
    };

    /// Capture a discovery result without delivering it, so a test can choose when the
    /// queued work item runs relative to StopScan()/StartScan().
    PendingResult CapturePendingResult(uint16_t discriminator, uint16_t vendorId, uint16_t productId, uint8_t addressTag)
    {
        return PendingResult(*this, discriminator, vendorId, productId, addressTag);
    }

    /// Play one discovery result back to whoever is scanning. The address varies with
    /// @p addressTag so distinct devices get distinct addresses. Goes through
    /// PendingResult so the immediate and the deferred path cannot drift.
    void ReportDevice(uint16_t discriminator, uint16_t vendorId, uint16_t productId, uint8_t addressTag)
    {
        CapturePendingResult(discriminator, vendorId, productId, addressTag).Dispatch();
    }

    bool IsScanning() const { return scanning; }
    void SetStartScanResult(CHIP_ERROR err) { mStartScanResult = err; }

    /// CHIP_ERROR_BUSY means peripheral activity has not ended yet (see
    /// CommissioningProxyBleAdapter::EnableCentralRole()).
    CHIP_ERROR enableCentralRoleResult = CHIP_NO_ERROR;
    unsigned enableCentralRoleCalls    = 0;
    unsigned startScanCalls            = 0;
    unsigned stopScanCalls             = 0;
    bool scanning                      = false;

private:
    DiscoveryCallback mCallback = nullptr;
    void * mContext             = nullptr;
    CHIP_ERROR mStartScanResult = CHIP_NO_ERROR;
    uint32_t mScanGeneration    = 0;
};

/// Stands in for the delegates BLEManagerImpl gives BleLayer, so a connect gets past
/// BleLayer's own checks. Records the connect the transport starts.
class FakeBlePlatform : public Ble::BlePlatformDelegate, public Ble::BleApplicationDelegate, public Ble::BleConnectionDelegate
{
public:
    CHIP_ERROR Install(Ble::BleLayer & layer) { return layer.Init(this, this, this, &DeviceLayer::SystemLayer()); }

    void NewConnection(Ble::BleLayer *, void *, const SetupDiscriminator & discriminator) override
    {
        newConnectionCalls++;
        requestedDiscriminator = discriminator.GetLongValue();
    }
    void NewConnection(Ble::BleLayer *, void *, BLE_CONNECTION_OBJECT) override {}
    CHIP_ERROR CancelConnection() override { return CHIP_NO_ERROR; }

    unsigned newConnectionCalls     = 0;
    uint16_t requestedDiscriminator = 0;

private:
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 0; }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                              System::PacketBufferHandle) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                                System::PacketBufferHandle) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

struct TestCommissioningProxyBleTransport : public ::testing::Test
{
    // Every timer the transport and the cluster arm goes through mockTimer, so nothing
    // under test reaches the system layer or needs a running event loop.
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_SUCCESS(blePlatform.Install(bleLayer)); }

    /// Declaration order matters: the transport is destroyed before the adapter, the timer
    /// and the BleLayer it holds references to.
    CommissioningProxyMockTimer mockTimer;
    FakeBleProxyAdapter adapter;
    FakeBlePlatform blePlatform;
    Ble::BleLayer bleLayer;
    CommissioningProxyBleTransport transport{ adapter, mockTimer, &bleLayer };
};

/// A cluster with the BLE transport registered, so command-driven paths run end to end.
struct HostedTransport
{
    explicit HostedTransport(CommissioningProxyMockTimer & timer, CommissioningProxyBleTransport & transport) :
        cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>(Feature::kBackgroundScan)), timer)
    {
        cluster.RegisterTransport(transport);
    }

    CommissioningProxyCluster cluster;
};

ProxyConnectRequest::Type MakeBleConnectRequest(uint16_t timeoutSeconds)
{
    ProxyConnectRequest::Type request;
    request.address.SetNull();
    request.transport     = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    request.discriminator = 3841;
    request.vendorID      = VendorId::Common;
    request.productID     = 0;
    request.timeout       = timeoutSeconds;
    return request;
}

} // namespace

// ---------------------------------------------------------------------------
// Foreground scan: adapter interaction and the scan window
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, ScanStartsAdapterScanAndArmsWindow)
{
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    EXPECT_EQ(adapter.startScanCalls, 1u);
    EXPECT_TRUE(adapter.IsScanning());
    // The window is what ends the scan, so it must be armed.
    EXPECT_EQ(mockTimer.ActiveCount(), 1u);
}

TEST_F(TestCommissioningProxyBleTransport, SecondScanWhileRunningIsBusy)
{
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Busy);
    // The in-flight scan must not have been disturbed by the rejected one.
    EXPECT_EQ(adapter.startScanCalls, 1u);
    EXPECT_EQ(adapter.stopScanCalls, 0u);
}

TEST_F(TestCommissioningProxyBleTransport, ScanReportsFailureWhenAdapterCannotStart)
{
    adapter.SetStartScanResult(CHIP_ERROR_BUSY);
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Failure);
    EXPECT_FALSE(adapter.IsScanning());
    // No window may be left armed, or it would later stop a scan that never started.
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

TEST_F(TestCommissioningProxyBleTransport, ScanStopsAdapterWhenWindowCannotBeArmed)
{
    mockTimer.FailNextStart();
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Failure);
    // Without this the radio would scan forever with nothing left to stop it.
    EXPECT_EQ(adapter.startScanCalls, 1u);
    EXPECT_EQ(adapter.stopScanCalls, 1u);
    EXPECT_FALSE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, ScanWindowExpiryReleasesTheScanner)
{
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    mockTimer.AdvanceClock(System::Clock::Seconds16(10));

    EXPECT_EQ(adapter.stopScanCalls, 1u);
    EXPECT_FALSE(adapter.IsScanning());
    // Scanner released, so a fresh scan is accepted rather than reported Busy.
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
}

// ---------------------------------------------------------------------------
// Foreground scan: result handling, driven through the cluster so the real
// aggregator builds the ProxyScanResponse
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, ScanResponseDedupesOnDiscriminatorVendorProduct)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    // Async command: this starts the scan and the aggregator answers only when the window
    // closes, so the InvokeResult carries a synthesised status and is deliberately
    // ignored — the assertions are on the handler, as in TestCommissioningProxyCluster.
    [[maybe_unused]] auto pending = tester.Invoke(request);
    ASSERT_FALSE(tester.GetCommandHandler().HasResponse());

    // Two identical devices plus one that differs only in productID.
    adapter.ReportDevice(0x123, 0xFFF1, 0x8000, 1);
    adapter.ReportDevice(0x123, 0xFFF1, 0x8000, 2);
    adapter.ReportDevice(0x123, 0xFFF1, 0x8001, 3);

    // Advance exactly to the scan window, not past it. The aggregator's watchdog sits at
    // scanMaxTime + 5s, and CommissioningProxyMockTimer fires due timers in array order
    // rather than deadline order — so overshooting can fire the watchdog first, which
    // emits a 0-result response and leaves the transport with nowhere to contribute.
    mockTimer.AdvanceClock(System::Clock::Seconds16(host.cluster.GetScanMaxTime()));

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.numberOfResults, 2);
}

TEST_F(TestCommissioningProxyBleTransport, ScanResponseIsCappedAtMaxCachedResults)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    // Async command: this starts the scan and the aggregator answers only when the window
    // closes, so the InvokeResult carries a synthesised status and is deliberately
    // ignored — the assertions are on the handler, as in TestCommissioningProxyCluster.
    [[maybe_unused]] auto pending = tester.Invoke(request);
    ASSERT_FALSE(tester.GetCommandHandler().HasResponse());

    // Two more distinct devices than the store can hold. numberOfResults is a uint8_t
    // and the spec caps the response at MaxCachedResults, so the surplus must be dropped
    // rather than overflowing either.
    for (size_t i = 0; i < kMaxCachedResults + 2; i++)
    {
        adapter.ReportDevice(static_cast<uint16_t>(0x200 + i), 0xFFF1, 0x8000, static_cast<uint8_t>(i));
    }

    // Advance exactly to the scan window, not past it. The aggregator's watchdog sits at
    // scanMaxTime + 5s, and CommissioningProxyMockTimer fires due timers in array order
    // rather than deadline order — so overshooting can fire the watchdog first, which
    // emits a 0-result response and leaves the transport with nowhere to contribute.
    mockTimer.AdvanceClock(System::Clock::Seconds16(host.cluster.GetScanMaxTime()));

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.numberOfResults, static_cast<uint8_t>(kMaxCachedResults));
}

// ---------------------------------------------------------------------------
// Background scan: the registry's hardware hooks land on the adapter, and the
// foreground scan takes the single scanner off it
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, BackgroundScanStartDrivesTheAdapter)
{
    EXPECT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    EXPECT_EQ(adapter.startScanCalls, 1u);
    EXPECT_TRUE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, BackgroundScanStartSucceedsWhileScannerIsBusy)
{
    // BUSY from the platform means "someone else holds the radio"; the registry keeps the
    // request and retries later, so the command still succeeds.
    adapter.SetStartScanResult(CHIP_ERROR_BUSY);
    EXPECT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    EXPECT_FALSE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, BackgroundScanReportsIntoTheClusterCache)
{
    HostedTransport host{ mockTimer, transport };

    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);

    adapter.ReportDevice(0x321, 0xFFF1, 0x8000, 1);
    adapter.ReportDevice(0x322, 0xFFF1, 0x8000, 2);
    // Re-discovery of an already-cached device refreshes it rather than adding a second
    // entry.
    adapter.ReportDevice(0x321, 0xFFF1, 0x8000, 1);

    EXPECT_EQ(host.cluster.ScanCache().Count(), 2);
}

TEST_F(TestCommissioningProxyBleTransport, ForegroundScanTakesTheScannerFromTheBackgroundScan)
{
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    ASSERT_EQ(adapter.startScanCalls, 1u);

    // BLE has one scanner: the foreground scan must stop the background one before
    // claiming it, or StartScan would report BUSY.
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    EXPECT_EQ(adapter.stopScanCalls, 1u);
    EXPECT_EQ(adapter.startScanCalls, 2u);

    // When the window closes the background scan gets the scanner back.
    mockTimer.AdvanceClock(System::Clock::Seconds16(10));
    EXPECT_EQ(adapter.startScanCalls, 3u);
    EXPECT_TRUE(adapter.IsScanning());
}

// ---------------------------------------------------------------------------
// Late discovery results: a result the platform queued before a scan stopped
// must not be attributed to whatever runs after it.
//
// The transport cannot tell a stale result from a live one - it sees only the
// callback it registered - so what these pin is the StopScan() guarantee in
// CommissioningProxyBleAdapter.h, modelled by FakeBleProxyAdapter. They also pin
// that the transport hands the scanner back through the adapter at every
// transition, which is what the generation keys off: dropping PauseBgScan() from
// Scan(), or StopHardwareScan() from the registry's OnBecameEmpty(), turns them
// red.
//
// Only the two restart cases discriminate the generation. After a plain stop the
// adapter holds no callback either, so the first test is guarded twice over and
// stays green even with the generation removed - it pins the stop reaching the
// adapter, not the staleness check.
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, LateBackgroundResultAfterStopIsIgnored)
{
    HostedTransport host{ mockTimer, transport };

    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);

    // BlueZ has discovered a device and queued it onto the Matter event loop, but the work
    // item has not run yet.
    auto pending = adapter.CapturePendingResult(0x123, 0xFFF1, 0x8000, 1);

    ASSERT_EQ(transport.BgScanStop(BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle), BitMask<WiFiBandBitmap>{},
                                   /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    // Stopping the last request clears every cached result on the transport.
    ASSERT_EQ(host.cluster.ScanCache().Count(), 0);

    // Equivalent to an already-scheduled Linux work item finally executing after StopScan().
    pending.Dispatch();

    EXPECT_EQ(host.cluster.ScanCache().Count(), 0);
}

TEST_F(TestCommissioningProxyBleTransport, LateBackgroundResultDoesNotLandInARestartedScan)
{
    HostedTransport host{ mockTimer, transport };

    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    auto pending = adapter.CapturePendingResult(0x123, 0xFFF1, 0x8000, 1);

    ASSERT_EQ(transport.BgScanStop(BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle), BitMask<WiFiBandBitmap>{},
                                   /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    ASSERT_EQ(host.cluster.ScanCache().Count(), 0);

    // A second client starts a fresh background scan before the queued work item runs, so
    // the adapter holds a live callback again.
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x22),
              Status::Success);

    pending.Dispatch();

    // The result belongs to a scan that has stopped and had its results cleared, so it must
    // not reappear as a result of the new one.
    EXPECT_EQ(host.cluster.ScanCache().Count(), 0);
}

TEST_F(TestCommissioningProxyBleTransport, LateBackgroundResultDoesNotEnterTheForegroundScan)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    auto pending = adapter.CapturePendingResult(0x123, 0xFFF1, 0x8000, 1);

    // Scan() pauses the background scan and claims the single scanner for the foreground
    // scan in one synchronous turn, so the queued item lands with the foreground handler
    // registered.
    ProxyScanRequest::Type request;
    request.transport = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    // Async command: the aggregator answers only when the window closes, so the InvokeResult
    // carries a synthesised status and is deliberately ignored.
    [[maybe_unused]] auto invoke = tester.Invoke(request);

    pending.Dispatch();

    // Advance exactly to the scan window, not past it - see the note in
    // ScanResponseDedupesOnDiscriminatorVendorProduct about the aggregator's watchdog.
    mockTimer.AdvanceClock(System::Clock::Seconds16(host.cluster.GetScanMaxTime()));

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    // A device the background scan discovered, before the foreground scan started, is not a
    // foreground scan result.
    EXPECT_EQ(response.numberOfResults, 0);
}

// ---------------------------------------------------------------------------
// ProxyConnectRequest while the BLE stack cannot switch to central role yet
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, ConnectWaitsWhileTheCentralRoleIsUnavailable)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/30));
    EXPECT_FALSE(tester.GetCommandHandler().HasStatus());
    EXPECT_TRUE(transport.IsConnectPending());

    unsigned calls = adapter.enableCentralRoleCalls;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_GT(adapter.enableCentralRoleCalls, calls);
    EXPECT_FALSE(tester.GetCommandHandler().HasStatus());

    // Once the switch succeeds the connect starts and the retry stops.
    adapter.enableCentralRoleResult = CHIP_NO_ERROR;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_EQ(blePlatform.newConnectionCalls, 1u);
    EXPECT_EQ(blePlatform.requestedDiscriminator, 3841u);
    EXPECT_TRUE(transport.IsConnectPending());
    EXPECT_FALSE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(mockTimer.ActiveCount(), 1u); // the connect timeout only

    calls = adapter.enableCentralRoleCalls;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_EQ(adapter.enableCentralRoleCalls, calls);

    ASSERT_EQ(transport.CancelPendingConnect(tester.GetCommandHandler().GetAccessingFabricIndex()), Status::Success);
    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Failure));
}

TEST_F(TestCommissioningProxyBleTransport, ConnectFailsAtOnceWhenTheCentralRoleSwitchFails)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_INTERNAL;

    auto result = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/30));
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Failure));
    EXPECT_FALSE(transport.IsConnectPending());
    EXPECT_EQ(mockTimer.ActiveCount(), 0u); // the connect timeout is cancelled too
}

TEST_F(TestCommissioningProxyBleTransport, ConnectFailsAtOnceWhenTheCentralRoleRetryTimerCannotBeStarted)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    // With no Timeout, the retry timer is the first timer the request starts.
    mockTimer.FailNextStart();
    auto result = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/0));
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Failure));
    EXPECT_FALSE(transport.IsConnectPending());
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

TEST_F(TestCommissioningProxyBleTransport, ConnectTimesOutWhileTheCentralRoleStaysUnavailable)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/5));
    // The retry restarts at 4.75 s, so it is still due after the timeout unless the timeout
    // cancels it.
    mockTimer.AdvanceClock(System::Clock::Milliseconds32(4750));
    mockTimer.AdvanceClock(System::Clock::Milliseconds32(250));

    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Timeout));
    EXPECT_FALSE(transport.IsConnectPending());
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

TEST_F(TestCommissioningProxyBleTransport, ConnectFailsWhenTheCentralRoleSwitchFailsWhileWaiting)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending   = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/30));
    adapter.enableCentralRoleResult = CHIP_ERROR_INTERNAL;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));

    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Failure));
    EXPECT_FALSE(transport.IsConnectPending());
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

// A Timeout of 0 means no timeout, so only a cancel ends the wait.
TEST_F(TestCommissioningProxyBleTransport, ConnectWithoutTimeoutWaitsForTheCentralRoleUntilCancelled)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/0));
    mockTimer.AdvanceClock(System::Clock::Seconds16(600));

    EXPECT_FALSE(tester.GetCommandHandler().HasStatus());
    EXPECT_TRUE(transport.IsConnectPending());
    EXPECT_EQ(mockTimer.ActiveCount(), 1u); // the retry only

    ASSERT_EQ(transport.CancelPendingConnect(tester.GetCommandHandler().GetAccessingFabricIndex()), Status::Success);
    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Failure));
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);

    const unsigned callsAtCancel = adapter.enableCentralRoleCalls;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_EQ(adapter.enableCentralRoleCalls, callsAtCancel);
}

TEST_F(TestCommissioningProxyBleTransport, ShutdownAnswersAConnectWaitingForTheCentralRole)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/30));
    transport.Shutdown();

    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Failure));
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

TEST_F(TestCommissioningProxyBleTransport, ScanIsBusyWhileAConnectIsPending)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    adapter.enableCentralRoleResult = CHIP_ERROR_BUSY;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/30));
    ASSERT_TRUE(transport.IsConnectPending());
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Busy);

    adapter.enableCentralRoleResult = CHIP_NO_ERROR;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    ASSERT_EQ(blePlatform.newConnectionCalls, 1u);
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Busy);

    EXPECT_EQ(adapter.startScanCalls, 0u);

    ASSERT_EQ(transport.CancelPendingConnect(tester.GetCommandHandler().GetAccessingFabricIndex()), Status::Success);
    EXPECT_FALSE(transport.IsConnectPending());
}

// The background scan resumes when the connect times out, not when the foreground scan ends.
TEST_F(TestCommissioningProxyBleTransport, ConnectEndsAForegroundScanAndKeepsTheBackgroundScanPaused)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(60), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);

    ProxyScanRequest::Type scanRequest;
    scanRequest.transport             = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    [[maybe_unused]] auto pendingScan = tester.Invoke(scanRequest);
    ASSERT_TRUE(adapter.IsScanning());
    adapter.ReportDevice(0x123, 0xFFF1, 0x8000, 1);
    const unsigned startsBeforeConnect = adapter.startScanCalls;
    const unsigned stopsBeforeConnect  = adapter.stopScanCalls;

    adapter.enableCentralRoleResult      = CHIP_ERROR_BUSY;
    [[maybe_unused]] auto pendingConnect = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/20));
    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    EXPECT_EQ(tester.GetCommandHandler().GetResponseCommandId(), ProxyScanResponse::Id);
    ProxyScanResponse::DecodableType response;
    ASSERT_SUCCESS(tester.GetCommandHandler().DecodeResponse(response));
    EXPECT_EQ(response.numberOfResults, 1);
    EXPECT_FALSE(tester.GetCommandHandler().HasStatus());
    EXPECT_TRUE(transport.IsConnectPending());
    EXPECT_EQ(adapter.stopScanCalls, stopsBeforeConnect + 1);

    // Neither the role switch nor the connect starting touches the scanner again, and the
    // scan window timer was cancelled with the scan.
    adapter.enableCentralRoleResult = CHIP_NO_ERROR;
    mockTimer.AdvanceClock(System::Clock::Seconds16(1));
    ASSERT_EQ(blePlatform.newConnectionCalls, 1u);
    mockTimer.AdvanceClock(System::Clock::Seconds16(host.cluster.GetScanMaxTime()));
    EXPECT_EQ(adapter.startScanCalls, startsBeforeConnect);
    EXPECT_EQ(adapter.stopScanCalls, stopsBeforeConnect + 1);
    EXPECT_FALSE(adapter.IsScanning());

    // The connect times out and hands the scanner back to the background scan.
    mockTimer.AdvanceClock(System::Clock::Seconds16(20));
    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Timeout));
    EXPECT_FALSE(transport.IsConnectPending());
    EXPECT_EQ(adapter.startScanCalls, startsBeforeConnect + 1);
    EXPECT_TRUE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, ConnectPausesTheBackgroundScanUntilItTimesOut)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(60), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    ASSERT_TRUE(adapter.IsScanning());
    const unsigned startsBeforeConnect = adapter.startScanCalls;
    const unsigned stopsBeforeConnect  = adapter.stopScanCalls;

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/20));
    ASSERT_EQ(blePlatform.newConnectionCalls, 1u);
    EXPECT_TRUE(transport.IsConnectPending());
    EXPECT_EQ(adapter.stopScanCalls, stopsBeforeConnect + 1);
    EXPECT_FALSE(adapter.IsScanning());

    mockTimer.AdvanceClock(System::Clock::Seconds16(20));
    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Timeout));
    EXPECT_EQ(adapter.startScanCalls, startsBeforeConnect + 1);
    EXPECT_TRUE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, ConnectRefusedByTheBleLayerFailsAndResumesTheBackgroundScan)
{
    // An uninitialised BleLayer refuses every connect.
    Ble::BleLayer refusingLayer;
    CommissioningProxyBleTransport refusingTransport{ adapter, mockTimer, &refusingLayer };
    HostedTransport host{ mockTimer, refusingTransport };
    ClusterTester tester(host.cluster);
    ASSERT_EQ(refusingTransport.BgScanStart(System::Clock::Seconds16(60), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1,
                                            /*nodeId=*/0x11),
              Status::Success);
    const unsigned startsBeforeConnect = adapter.startScanCalls;
    const unsigned stopsBeforeConnect  = adapter.stopScanCalls;

    auto result = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/20));
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Failure));
    EXPECT_FALSE(refusingTransport.IsConnectPending());
    EXPECT_EQ(adapter.stopScanCalls, stopsBeforeConnect + 1);
    EXPECT_EQ(adapter.startScanCalls, startsBeforeConnect + 1);
    EXPECT_TRUE(adapter.IsScanning());
}

TEST_F(TestCommissioningProxyBleTransport, BackgroundScanRefreshWaitsForAPendingConnect)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(60), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);

    [[maybe_unused]] auto pending = tester.Invoke(MakeBleConnectRequest(/*timeoutSeconds=*/20));
    ASSERT_TRUE(transport.IsConnectPending());
    ASSERT_FALSE(adapter.IsScanning());
    const unsigned startsDuringConnect = adapter.startScanCalls;

    EXPECT_EQ(transport.BgScanStart(System::Clock::Seconds16(60), BitMask<WiFiBandBitmap>{}, /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    EXPECT_EQ(adapter.startScanCalls, startsDuringConnect);
    EXPECT_FALSE(adapter.IsScanning());

    mockTimer.AdvanceClock(System::Clock::Seconds16(20));
    ASSERT_TRUE(tester.GetCommandHandler().HasStatus());
    EXPECT_EQ(tester.GetCommandHandler().GetLastStatus().status, ClusterStatusCode(Status::Timeout));
    EXPECT_EQ(adapter.startScanCalls, startsDuringConnect + 1);
    EXPECT_TRUE(adapter.IsScanning());
}

// ---------------------------------------------------------------------------
// Session-keyed operations with no session, and teardown
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyBleTransport, DisconnectUnknownSessionIsNotFound)
{
    EXPECT_EQ(transport.Disconnect(/*sessionId=*/7), Status::NotFound);
}

TEST_F(TestCommissioningProxyBleTransport, SendMessageOnUnknownSessionFails)
{
    EXPECT_EQ(transport.SendMessage(/*sessionId=*/7, System::PacketBufferHandle::New(16)), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestCommissioningProxyBleTransport, CancelPendingConnectWithNothingPendingIsInvalidInState)
{
    EXPECT_EQ(transport.CancelPendingConnect(/*fabricIndex=*/1), Status::InvalidInState);
    EXPECT_FALSE(transport.IsConnectPending());
}

TEST_F(TestCommissioningProxyBleTransport, ShutdownStopsAnInFlightScanAndCanRunTwice)
{
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    ASSERT_TRUE(adapter.IsScanning());

    transport.Shutdown();
    EXPECT_FALSE(adapter.IsScanning());
    // The window must be disarmed too: firing after teardown would report results to a
    // cluster that no longer exists.
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);

    const unsigned stopsAfterFirstShutdown = adapter.stopScanCalls;
    transport.Shutdown();
    EXPECT_EQ(adapter.stopScanCalls, stopsAfterFirstShutdown);
}
