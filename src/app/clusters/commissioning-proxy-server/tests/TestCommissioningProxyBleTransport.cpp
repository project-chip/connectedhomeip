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
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/Span.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissioningProxy;
using namespace chip::app::Clusters::CommissioningProxy::Commands;
using namespace chip::Testing;
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
        return CHIP_NO_ERROR;
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
    }

    /// Play one discovery result back to whoever is scanning. The address varies with
    /// @p addressTag so distinct devices get distinct addresses.
    void ReportDevice(uint16_t discriminator, uint16_t vendorId, uint16_t productId, uint8_t addressTag)
    {
        if (mCallback == nullptr)
        {
            return;
        }
        const uint8_t mac[6] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, addressTag };
        mCallback(mContext, ByteSpan(mac, sizeof(mac)), discriminator, vendorId, productId);
    }

    bool IsScanning() const { return scanning; }
    void SetStartScanResult(CHIP_ERROR err) { mStartScanResult = err; }

    unsigned enableCentralRoleCalls = 0;
    unsigned startScanCalls         = 0;
    unsigned stopScanCalls          = 0;
    bool scanning                   = false;

private:
    DiscoveryCallback mCallback = nullptr;
    void * mContext             = nullptr;
    CHIP_ERROR mStartScanResult = CHIP_NO_ERROR;
};

struct TestCommissioningProxyBleTransport : public ::testing::Test
{
    // Every timer the transport and the cluster arm goes through mockTimer, so nothing
    // under test reaches the system layer or needs a running event loop.
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    /// Declaration order matters: the transport is destroyed before the adapter and the
    /// timer it holds references to.
    CommissioningProxyMockTimer mockTimer;
    FakeBleProxyAdapter adapter;
    CommissioningProxyBleTransport transport{ adapter, mockTimer };
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
