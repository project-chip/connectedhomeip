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

// Covers the Commissioning Proxy PAF *driver*: scan result handling, the response cap,
// and the single-subscribe-slot arbitration between the foreground scan, the background
// scan and connect.
//
// This is distinct from src/wifipaf/tests, which covers the PAF layer and the PAFTP
// protocol underneath (fragmentation, acks, session pool, endpoint lifecycle) and knows
// nothing about the cluster. Nothing here re-tests those.
//
// Cases that differ from the BLE driver's suite, rather than merely repeating it, are
// marked "PAF-specific" below.

#include "CommissioningProxyMockTimer.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafTransport.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/Span.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>

#include <cstdint>
#include <cstring>

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

/// Stands in for ConnectivityManagerImpl's NAN discovery surface. Records what the
/// transport asked for and lets a test play peers back through the registered callbacks.
class FakePafProxyAdapter : public CommissioningProxyPafAdapter
{
public:
    CHIP_ERROR StartForegroundScan(System::Clock::Seconds16 window, DiscoveryCallback onDevice, ScanCompleteCallback onDone,
                                   void * context) override
    {
        foregroundStartCalls++;
        lastWindow = window;
        if (mStartForegroundResult != CHIP_NO_ERROR)
        {
            return mStartForegroundResult;
        }
        mForegroundCallback = onDevice;
        mForegroundDone     = onDone;
        mForegroundContext  = context;
        return CHIP_NO_ERROR;
    }

    void StopForegroundScan() override
    {
        foregroundStopCalls++;
        mForegroundCallback = nullptr;
        mForegroundDone     = nullptr;
        mForegroundContext  = nullptr;
    }

    CHIP_ERROR StartBackgroundScan(DiscoveryCallback cb, void * context) override
    {
        backgroundStartCalls++;
        if (mStartBackgroundResult != CHIP_NO_ERROR)
        {
            return mStartBackgroundResult;
        }
        mBackgroundCallback = cb;
        mBackgroundContext  = context;
        backgroundScanning  = true;
        return CHIP_NO_ERROR;
    }

    void StopBackgroundScan() override
    {
        backgroundStopCalls++;
        backgroundScanning  = false;
        mBackgroundCallback = nullptr;
        mBackgroundContext  = nullptr;
    }

    uint32_t PendingConnectSubscribeId() const override { return mSubscribeId; }

    /// Report a peer to the in-flight foreground scan.
    void ReportForeground(uint16_t discriminator, uint16_t vendorId, uint16_t productId, uint8_t addressTag,
                          ByteSpan extendedData = ByteSpan(), uint16_t wiFiBand = 0)
    {
        if (mForegroundCallback == nullptr)
        {
            return;
        }
        const uint8_t mac[6] = { 0x02, 0x11, 0x22, 0x33, 0x44, addressTag };
        mForegroundCallback(mForegroundContext, ByteSpan(mac, sizeof(mac)), discriminator, vendorId, productId, extendedData,
                            wiFiBand);
    }

    /// Close the foreground scan out, as the platform does when its window expires.
    void CompleteForegroundScan()
    {
        if (mForegroundDone == nullptr)
        {
            return;
        }
        ScanCompleteCallback onDone = mForegroundDone;
        void * context              = mForegroundContext;
        mForegroundCallback         = nullptr;
        mForegroundDone             = nullptr;
        mForegroundContext          = nullptr;
        onDone(context);
    }

    void ReportBackground(uint16_t discriminator, uint16_t vendorId, uint16_t productId, uint8_t addressTag, uint16_t wiFiBand = 0)
    {
        if (mBackgroundCallback == nullptr)
        {
            return;
        }
        const uint8_t mac[6] = { 0x02, 0x11, 0x22, 0x33, 0x44, addressTag };
        mBackgroundCallback(mBackgroundContext, ByteSpan(mac, sizeof(mac)), discriminator, vendorId, productId, ByteSpan(),
                            wiFiBand);
    }

    void SetStartForegroundResult(CHIP_ERROR err) { mStartForegroundResult = err; }
    void SetStartBackgroundResult(CHIP_ERROR err) { mStartBackgroundResult = err; }

    unsigned foregroundStartCalls = 0;
    unsigned foregroundStopCalls  = 0;
    unsigned backgroundStartCalls = 0;
    unsigned backgroundStopCalls  = 0;
    bool backgroundScanning       = false;
    System::Clock::Seconds16 lastWindow{ 0 };

private:
    DiscoveryCallback mForegroundCallback = nullptr;
    ScanCompleteCallback mForegroundDone  = nullptr;
    void * mForegroundContext             = nullptr;
    DiscoveryCallback mBackgroundCallback = nullptr;
    void * mBackgroundContext             = nullptr;
    CHIP_ERROR mStartForegroundResult     = CHIP_NO_ERROR;
    CHIP_ERROR mStartBackgroundResult     = CHIP_NO_ERROR;
    uint32_t mSubscribeId                 = 0;
};

struct TestCommissioningProxyPafTransport : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    /// Declaration order matters: the transport is destroyed before the adapter and the
    /// timer it holds references to.
    CommissioningProxyMockTimer mockTimer;
    FakePafProxyAdapter adapter;
    CommissioningProxyPafTransport transport{ adapter, mockTimer };
};

/// A cluster with the PAF transport registered, so command-driven paths run end to end.
struct HostedTransport
{
    HostedTransport(CommissioningProxyMockTimer & timer, CommissioningProxyPafTransport & transport) :
        cluster(kTestEndpointId,
                CommissioningProxyCluster::Config(BitMask<Feature>(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface),
                                                  BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)),
                timer)
    {
        cluster.RegisterTransport(transport);
    }

    CommissioningProxyCluster cluster;
};

} // namespace

// ---------------------------------------------------------------------------
// Foreground scan: the platform owns the window (PAF-specific)
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyPafTransport, ScanPassesTheWindowToThePlatformAndArmsNoTimer)
{
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(20)), Status::Success);
    EXPECT_EQ(adapter.foregroundStartCalls, 1u);
    // PAF-specific: the platform times the window, so unlike the BLE driver this one must
    // arm no scan timer of its own — and it must hand the duration across intact.
    EXPECT_EQ(adapter.lastWindow.count(), 20u);
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);
}

TEST_F(TestCommissioningProxyPafTransport, SecondScanWhileRunningIsBusy)
{
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Busy);
    EXPECT_EQ(adapter.foregroundStartCalls, 1u);
}

TEST_F(TestCommissioningProxyPafTransport, ScanReportsFailureWhenThePlatformCannotStart)
{
    adapter.SetStartForegroundResult(CHIP_ERROR_BUSY);
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Failure);
    // A failed start must leave no scan in progress, or every later scan returns Busy.
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Failure);
    EXPECT_EQ(adapter.foregroundStartCalls, 2u);
}

TEST_F(TestCommissioningProxyPafTransport, ScanCompletionReleasesTheSubscribeSlot)
{
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    // PAF-specific: completion arrives from the platform, not from a local timer.
    adapter.CompleteForegroundScan();
    EXPECT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
}

// ---------------------------------------------------------------------------
// Foreground scan results, driven through the cluster so the real aggregator
// builds the ProxyScanResponse
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyPafTransport, ScanResponseDedupesOnDiscriminatorVendorProduct)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport             = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF);
    [[maybe_unused]] auto pending = tester.Invoke(request);
    ASSERT_FALSE(tester.GetCommandHandler().HasResponse());

    adapter.ReportForeground(0x123, 0xFFF1, 0x8000, 1);
    adapter.ReportForeground(0x123, 0xFFF1, 0x8000, 2); // duplicate
    adapter.ReportForeground(0x123, 0xFFF1, 0x8001, 3); // differs by productID
    adapter.CompleteForegroundScan();

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.numberOfResults, 2);
}

TEST_F(TestCommissioningProxyPafTransport, ScanResponseIsCappedAtMaxCachedResults)
{
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport             = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF);
    [[maybe_unused]] auto pending = tester.Invoke(request);

    for (size_t i = 0; i < kMaxCachedResults + 2; i++)
    {
        adapter.ReportForeground(static_cast<uint16_t>(0x200 + i), 0xFFF1, 0x8000, static_cast<uint8_t>(i));
    }
    adapter.CompleteForegroundScan();

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.numberOfResults, static_cast<uint8_t>(kMaxCachedResults));
}

TEST_F(TestCommissioningProxyPafTransport, ScanResultCarriesExtendedDataAndBand)
{
    // PAF-specific: BLE advertisements carry neither, so this copy-into-fixed-storage path
    // exists only here.
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport             = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF);
    [[maybe_unused]] auto pending = tester.Invoke(request);

    const uint8_t extended[] = { 0xde, 0xad, 0xbe, 0xef };
    adapter.ReportForeground(0x321, 0xFFF1, 0x8000, 1, ByteSpan(extended, sizeof(extended)),
                             static_cast<uint16_t>(WiFiBandBitmap::k2g4));
    adapter.CompleteForegroundScan();

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.numberOfResults, 1);

    auto iter = response.proxyScanResult.begin();
    ASSERT_TRUE(iter.Next());
    const auto & result = iter.GetValue();
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    ASSERT_FALSE(result.extendedData.IsNull());
    ASSERT_TRUE(result.extendedData.Value().data_equal(ByteSpan(extended, sizeof(extended))));
    ASSERT_TRUE(result.wiFiBand.HasValue());
    EXPECT_EQ(result.wiFiBand.Value(), WiFiBandBitmap::k2g4);
}

TEST_F(TestCommissioningProxyPafTransport, OversizedExtendedDataIsDroppedNotTruncated)
{
    // PAF-specific: the record's extended-data buffer is fixed, so an advertisement
    // claiming more than it holds must not be copied in partially — a truncated blob
    // would be reported to the commissioner as if it were complete.
    HostedTransport host{ mockTimer, transport };
    ClusterTester tester(host.cluster);

    ProxyScanRequest::Type request;
    request.transport             = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF);
    [[maybe_unused]] auto pending = tester.Invoke(request);

    uint8_t oversized[200] = {};
    memset(oversized, 0xa5, sizeof(oversized));
    adapter.ReportForeground(0x400, 0xFFF1, 0x8000, 1, ByteSpan(oversized, sizeof(oversized)));
    adapter.CompleteForegroundScan();

    ASSERT_TRUE(tester.GetCommandHandler().HasResponse());
    ProxyScanResponse::DecodableType response;
    ASSERT_EQ(tester.GetCommandHandler().DecodeResponse(response), CHIP_NO_ERROR);
    // The device is still reported; only the extended data it could not hold is absent.
    EXPECT_EQ(response.numberOfResults, 1);
    auto iter = response.proxyScanResult.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_TRUE(iter.GetValue().extendedData.IsNull());
}

// ---------------------------------------------------------------------------
// Background scan and the single subscribe slot
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyPafTransport, BackgroundScanStartDrivesTheAdapter)
{
    EXPECT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4),
                                    /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    EXPECT_EQ(adapter.backgroundStartCalls, 1u);
    EXPECT_TRUE(adapter.backgroundScanning);
}

TEST_F(TestCommissioningProxyPafTransport, BackgroundScanStartSucceedsWhileTheSlotIsBusy)
{
    adapter.SetStartBackgroundResult(CHIP_ERROR_BUSY);
    EXPECT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4),
                                    /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    EXPECT_FALSE(adapter.backgroundScanning);
}

TEST_F(TestCommissioningProxyPafTransport, BackgroundScanReportsIntoTheClusterCache)
{
    HostedTransport host{ mockTimer, transport };

    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4),
                                    /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);

    adapter.ReportBackground(0x321, 0xFFF1, 0x8000, 1, static_cast<uint16_t>(WiFiBandBitmap::k2g4));
    adapter.ReportBackground(0x322, 0xFFF1, 0x8000, 2, static_cast<uint16_t>(WiFiBandBitmap::k2g4));
    // Re-discovery refreshes the cached entry rather than adding a second one.
    adapter.ReportBackground(0x321, 0xFFF1, 0x8000, 1, static_cast<uint16_t>(WiFiBandBitmap::k2g4));

    EXPECT_EQ(host.cluster.ScanCache().Count(), 2);
}

TEST_F(TestCommissioningProxyPafTransport, ForegroundScanTakesTheSubscribeSlotFromTheBackgroundScan)
{
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4),
                                    /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    ASSERT_EQ(adapter.backgroundStartCalls, 1u);

    // One NAN subscribe slot: the foreground scan must stop the background one before
    // claiming it.
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);
    EXPECT_EQ(adapter.backgroundStopCalls, 1u);
    EXPECT_FALSE(adapter.backgroundScanning);
}

// ---------------------------------------------------------------------------
// Session-keyed operations with no session, and teardown
// ---------------------------------------------------------------------------

TEST_F(TestCommissioningProxyPafTransport, DisconnectUnknownSessionIsNotFound)
{
    EXPECT_EQ(transport.Disconnect(/*sessionId=*/7), Status::NotFound);
}

TEST_F(TestCommissioningProxyPafTransport, SendMessageOnUnknownSessionFails)
{
    EXPECT_EQ(transport.SendMessage(/*sessionId=*/7, System::PacketBufferHandle::New(16)), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestCommissioningProxyPafTransport, CancelPendingConnectWithNothingPendingIsInvalidInState)
{
    EXPECT_EQ(transport.CancelPendingConnect(/*fabricIndex=*/1), Status::InvalidInState);
    EXPECT_FALSE(transport.IsConnectPending());
}

TEST_F(TestCommissioningProxyPafTransport, ShutdownDetachesAnInFlightForegroundScan)
{
    // The platform owns the scan window and cannot always abort it, so Shutdown must
    // detach the callbacks: a completion arriving afterwards would otherwise reach a
    // destroyed transport.
    ASSERT_EQ(transport.Scan(System::Clock::Seconds16(10)), Status::Success);

    transport.Shutdown();
    EXPECT_EQ(adapter.foregroundStopCalls, 1u);

    // Nothing is delivered even if the platform reports late.
    adapter.ReportForeground(0x555, 0xFFF1, 0x8000, 1);
    adapter.CompleteForegroundScan();
}

TEST_F(TestCommissioningProxyPafTransport, ShutdownStopsTheBackgroundScanAndIsIdempotent)
{
    ASSERT_EQ(transport.BgScanStart(System::Clock::Seconds16(30), BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4),
                                    /*fabricIndex=*/1, /*nodeId=*/0x11),
              Status::Success);
    ASSERT_TRUE(adapter.backgroundScanning);

    transport.Shutdown();
    EXPECT_FALSE(adapter.backgroundScanning);
    EXPECT_EQ(mockTimer.ActiveCount(), 0u);

    const unsigned stopsAfterFirstShutdown = adapter.backgroundStopCalls;
    transport.Shutdown();
    EXPECT_EQ(adapter.backgroundStopCalls, stopsAfterFirstShutdown);
}
