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
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBgScanRegistry.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app::Clusters::CommissioningProxy;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr FabricIndex kFabric1 = 1;
constexpr FabricIndex kFabric2 = 2;
constexpr NodeId kNode1        = 0x1111;
constexpr NodeId kNode2        = 0x2222;
constexpr NodeId kNode3        = 0x3333;
constexpr NodeId kNode4        = 0x4444;
constexpr NodeId kNode5        = 0x5555;

const BitMask<CapabilitiesBitmap> kBle{ CapabilitiesBitmap::kBle };
const BitMask<CapabilitiesBitmap> kPaf{ CapabilitiesBitmap::kWiFiPAF };
const BitMask<CapabilitiesBitmap> kNoTransport{};
const BitMask<WiFiBandBitmap> k2g4{ WiFiBandBitmap::k2g4 };
const BitMask<WiFiBandBitmap> k5g{ WiFiBandBitmap::k5g };
const BitMask<WiFiBandBitmap> kBoth{ WiFiBandBitmap::k2g4, WiFiBandBitmap::k5g };
const BitMask<WiFiBandBitmap> kNoBands{};

// Records the hardware calls the registry makes and lets a test program the
// StartHardwareScan() result (to model the BUSY-defer and hard-failure paths).
class MockHardwareControl : public CommissioningProxyBgScanRegistry::HardwareControl
{
public:
    CHIP_ERROR StartHardwareScan() override
    {
        startCount++;
        return startResult;
    }
    void StopHardwareScan() override { stopCount++; }
    void ClearCachedResults(BitMask<WiFiBandBitmap> bands) override
    {
        clearCount++;
        lastClearedBands = bands;
    }

    int startCount = 0;
    int stopCount  = 0;
    int clearCount = 0;
    BitMask<WiFiBandBitmap> lastClearedBands{};
    CHIP_ERROR startResult = CHIP_NO_ERROR;
};

// Most tests pass timeoutSecs == 0 so no lifetime timer is armed and the registry
// logic is exercised synchronously. The lifetime tests at the end instead arm a timer
// and advance CommissioningProxyMockTimer's virtual clock.
constexpr uint16_t kNoTimeout = 0;

} // namespace

TEST(TestCommissioningProxyBgScanRegistry, FirstFabricStartsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_FALSE(reg.IsPaused());
}

TEST(TestCommissioningProxyBgScanRegistry, SecondFabricDoesNotRestartHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1); // already running; not restarted
}

TEST(TestCommissioningProxyBgScanRegistry, StartBusyStaysPausedAndKeepsFabric)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    hw.startResult = CHIP_ERROR_BUSY; // radio held (BLE scanner / PAF connect slot)
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1);
    EXPECT_TRUE(reg.IsPaused());
    EXPECT_FALSE(reg.IsEmpty()); // fabric kept, deferred

    // Radio frees up: resume restarts the hardware scan.
    hw.startResult = CHIP_NO_ERROR;
    reg.ResumeIfNeeded();
    EXPECT_EQ(hw.startCount, 2);
    EXPECT_FALSE(reg.IsPaused());
}

TEST(TestCommissioningProxyBgScanRegistry, StartHardFailureRejectsAndDropsFabric)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    hw.startResult = CHIP_ERROR_INTERNAL;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Failure);
    EXPECT_TRUE(reg.IsEmpty());
    // Nothing was started, so the radio is not stopped and the cache is not cleared.
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 0);
}

// A failed Start SHALL be a no-op: a hard hardware failure while refreshing an
// existing fabric must not destroy the registration that was already working.
TEST(TestCommissioningProxyBgScanRegistry, StartHardFailureOnRefreshKeepsExistingRegistration)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);

    // The radio is taken for a connect: the registry pauses (one stop so far).
    reg.Pause();
    EXPECT_TRUE(reg.IsPaused());
    EXPECT_EQ(hw.stopCount, 1);

    // Refreshing that same fabric now fails hard.
    hw.startResult = CHIP_ERROR_INTERNAL;
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k5g, kNoTimeout), Status::Failure);

    // The previous registration survives, still paused and awaiting a resume; nothing
    // extra was stopped and no cached results were thrown away.
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_TRUE(reg.IsPaused());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 0);

    // The old bands are intact: a stop for 2G4 still overlaps.
    hw.startResult = CHIP_NO_ERROR;
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
}

TEST(TestCommissioningProxyBgScanRegistry, StopLastFabricStopsHardwareAndClears)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
}

TEST(TestCommissioningProxyBgScanRegistry, StopUnknownFabricNotFound)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::NotFound);
}

TEST(TestCommissioningProxyBgScanRegistry, StopNoOverlapSucceedsAndKeepsFabric)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    // Stop targets a different transport (BLE); nothing overlaps this PAF fabric.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::Success);
    EXPECT_FALSE(reg.IsEmpty()); // fabric untouched
    EXPECT_EQ(hw.stopCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, StopBandSubsetKeepsFabricThenRemoves)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, kBoth, kNoTimeout), Status::Success);

    // transport==0 means "stop only the given bands": drop 2G4, keep 5G. Nobody scans
    // 2G4 any more, so its cached results go while the radio keeps running for 5G.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k2g4.Raw());

    // Stopping the remaining band empties the fabric and tears the whole transport down.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k5g), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 2);
    EXPECT_EQ(hw.lastClearedBands.Raw(), 0u);
}

TEST(TestCommissioningProxyBgScanRegistry, StopWithOtherFabricCoveringKeepsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);

    // Removing fabric 1 leaves fabric 2 still scanning PAF/2G4 — hardware stays up.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kPaf, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, PauseStopsAndResumeRestarts)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1);

    reg.Pause();
    EXPECT_TRUE(reg.IsPaused());
    EXPECT_EQ(hw.stopCount, 1);

    reg.ResumeIfNeeded();
    EXPECT_FALSE(reg.IsPaused());
    EXPECT_EQ(hw.startCount, 2);
}

TEST(TestCommissioningProxyBgScanRegistry, PauseWhenEmptyIsNoop)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    reg.Pause();
    EXPECT_FALSE(reg.IsPaused());
    EXPECT_EQ(hw.stopCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, ResumeWhenNotPausedIsNoop)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
    reg.ResumeIfNeeded();
    EXPECT_EQ(hw.startCount, 1); // was already running
}

TEST(TestCommissioningProxyBgScanRegistry, ResumeStillBusyStaysPaused)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    reg.Pause();
    EXPECT_TRUE(reg.IsPaused());

    hw.startResult = CHIP_ERROR_BUSY; // radio still held on the resume attempt
    reg.ResumeIfNeeded();
    EXPECT_TRUE(reg.IsPaused()); // stays paused, will retry later
}

TEST(TestCommissioningProxyBgScanRegistry, ShutdownStopsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    {
        CommissioningProxyBgScanRegistry reg(hw, timers);
        EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
        reg.Shutdown();
        EXPECT_TRUE(reg.IsEmpty());
        EXPECT_EQ(hw.stopCount, 1);
    }
    // Destructor runs Shutdown() again on the now-empty registry: no extra stop.
    EXPECT_EQ(hw.stopCount, 1);
}

TEST(TestCommissioningProxyBgScanRegistry, RefreshExistingFabricUpdatesBands)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    // Re-register the same fabric with a different band; still one fabric, no restart.
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k5g, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1);

    // A stop for the OLD band (2G4) no longer overlaps — proves the bands were updated.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());

    // Stopping the current band (5G) removes it.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k5g), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
}

// Spec § ProxyBackGroundScanStartRequest Timeout: "The background scan is
// automatically stopped when this duration elapses."
TEST(TestCommissioningProxyBgScanRegistry, LifetimeExpiryRemovesFabricAndStopsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(hw.startCount, 1);

    // Still inside the lifetime: nothing changes.
    timers.AdvanceClock(System::Clock::Seconds16(29));
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);

    // Spec: "any cached results for that fabric's transports are cleared".
    timers.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
}

// Without a lifetime timer the hardware scan would run unbounded, so a StartTimer
// failure SHALL reject the request and tear down the scan it had already started.
TEST(TestCommissioningProxyBgScanRegistry, LifetimeTimerArmFailureRejectsAndCleansUp)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    timers.FailNextStart();
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Failure);

    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_FALSE(reg.IsPaused());
    EXPECT_EQ(hw.startCount, 1); // hardware was started before the timer failed
    EXPECT_EQ(hw.stopCount, 1);  // ... so it must be stopped again
    EXPECT_EQ(hw.clearCount, 1);
}

// Refreshing a node's request cancels the fabric's lifetime timer so it can be re-armed
// at the new deadline. If the re-arm fails the old timer is already gone, so keeping the
// fabric registered would scan unbounded — the fabric SHALL be released instead.
TEST(TestCommissioningProxyBgScanRegistry, RefreshTimerArmFailureReleasesTheFabric)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, 60), Status::Success);
    EXPECT_EQ(hw.startCount, 1);

    // Node 1 refreshes its own request with a new timeout; re-arming the fabric's
    // lifetime fails, so the fabric goes — node 2's request included.
    timers.FailNextStart();
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 45), Status::Failure);

    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1); // nothing is left to scan for
    EXPECT_EQ(hw.clearCount, 1);

    // No timer survived the release: running past every deadline changes nothing.
    timers.AdvanceClock(System::Clock::Seconds16(120));
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);

    // The freed slot carries no request into the next fabric to claim it.
    EXPECT_EQ(reg.Start(kFabric2, kNode3, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Stop(kFabric2, kNode2, kPaf, k5g), Status::NotFound);
}

// Releasing a fabric on a re-arm failure must not disturb another fabric's scan; only the
// bands the released requests were the last to want stop being cached.
TEST(TestCommissioningProxyBgScanRegistry, RefreshTimerArmFailureKeepsOtherFabricScanning)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k5g, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);

    timers.FailNextStart();
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k5g, 45), Status::Failure);

    EXPECT_FALSE(reg.IsEmpty()); // fabric 2 still wants the radio
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k5g.Raw()); // fabric 2 still scans 2G4
}

// A fabric released because its lifetime could not be re-armed takes its surviving
// requests' bands with it, so their cached results are dropped too — even when another
// fabric keeps the radio running.
TEST(TestCommissioningProxyBgScanRegistry, ReArmFailureOnStopClearsTheReleasedFabricsBands)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, 60), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode3, kPaf, k2g4, kNoTimeout), Status::Success);

    // Node 1 stopping shortens fabric 1 to node 2's deadline; the re-arm fails, so
    // fabric 1 is released and node 2's 5G results are no longer wanted by anyone.
    timers.FailNextStart();
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kPaf, k2g4), Status::Success);

    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k5g.Raw());
}

// Dropping a fabric because its lifetime could not be re-armed must take the fabric's
// surviving requests with it: the next fabric to occupy the freed slot cannot inherit
// requests it never made.
TEST(TestCommissioningProxyBgScanRegistry, LifetimeReArmFailureDropsTheFabricsRequests)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, 60), Status::Success);

    // Node 1 stopping shortens the fabric to node 2's deadline, so the lifetime is
    // re-armed — and that failing drops the fabric, node 2's request included.
    timers.FailNextStart();
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kPaf, k2g4), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);

    // Another fabric takes the freed slot; node 2's request is not part of it.
    EXPECT_EQ(reg.Start(kFabric2, kNode3, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Stop(kFabric2, kNode2, kPaf, k5g), Status::NotFound);
    EXPECT_EQ(reg.Stop(kFabric2, kNode3, kPaf, k2g4), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
}

// Spec: the proxy keeps per-fabric records and scans "until the timeout fires or
// ProxyBackGroundScanStopRequest is received for that fabric". Each fabric's lifetime
// is therefore independent: expiring one must not disturb another.
TEST(TestCommissioningProxyBgScanRegistry, IndependentLifetimesExpireSeparately)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 10), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, 60), Status::Success);
    EXPECT_EQ(hw.startCount, 1);

    // Fabric 1 expires; fabric 2 still wants the scan, so the radio keeps running.
    timers.AdvanceClock(System::Clock::Seconds16(10));
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 0);

    // Fabric 2 expires: last fabric gone, scan torn down.
    timers.AdvanceClock(System::Clock::Seconds16(50));
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
}

// Spec § ProxyBackGroundScanStopRequest: "If the NodeID and FabricID of the client do
// not match those recorded when background scanning was started for this fabric, the
// proxy SHALL take no action and the command SHALL be rejected with a status of
// NOT_FOUND."
TEST(TestCommissioningProxyBgScanRegistry, StopFromNodeWithNoRequestNotFound)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);

    // Node 2 never started a scan, so it has no request to stop.
    EXPECT_EQ(reg.Stop(kFabric1, kNode2, kPaf, k2g4), Status::NotFound);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);

    // The owner still can.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kPaf, k2g4), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
}

// Spec: a Stop is identified by NodeID + FabricID, so each node on a fabric keeps its
// own request and the fabric scans the union of them. One node's scan is untouched by
// another's.
TEST(TestCommissioningProxyBgScanRegistry, SecondNodeOnSameFabricAddsItsOwnRequest)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1); // already running; not restarted for the second

    // Node 1 stopping its own 2G4 leaves node 2's 5G request scanning: the radio stays
    // on, but 2G4's cached results go since no request covers that band now.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k2g4.Raw());

    // Only once the last request goes does the radio stop and the transport clear.
    EXPECT_EQ(reg.Stop(kFabric1, kNode2, kNoTransport, k5g), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 2);
}

// Spec § ProxyBackGroundScanStopRequest: stop the requested transports and bands
// "unless another active background scan overlaps with them".
TEST(TestCommissioningProxyBgScanRegistry, StopLeavesBandsAnotherRequestStillCovers)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, kBoth, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    // Node 1 drops 5G; node 2 still wants it, so scanning continues.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k5g), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);

    // Node 1 keeps 2G4: dropping that too removes only node 1's request.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(reg.Stop(kFabric1, kNode2, kNoTransport, k5g), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
}

// The fabric holds one timer, at the latest deadline of its requests, so a shorter
// request does not end the fabric's scan early.
TEST(TestCommissioningProxyBgScanRegistry, FabricTimerUsesLatestDeadline)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 10), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, 60), Status::Success);

    timers.AdvanceClock(System::Clock::Seconds16(11));
    EXPECT_FALSE(reg.IsEmpty()); // node 2 still has 49s to run

    timers.AdvanceClock(System::Clock::Seconds16(49));
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
}

// Dropping the request that set the fabric's deadline SHALL pull the timer back to the
// longest survivor, rather than leave the fabric scanning until the removed deadline.
TEST(TestCommissioningProxyBgScanRegistry, StopShortensFabricTimerToSurvivingRequest)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 300), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, 30), Status::Success);

    // The 300s request goes; only the 30s one is left.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());

    timers.AdvanceClock(System::Clock::Seconds16(31));
    EXPECT_TRUE(reg.IsEmpty());
}

// A request with Timeout 0 never expires, so the fabric holds no timer at all while it
// is registered.
TEST(TestCommissioningProxyBgScanRegistry, NoTimeoutRequestSuppressesFabricTimer)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, kNoTimeout), Status::Success);
    EXPECT_EQ(timers.ActiveCount(), 0u);

    timers.AdvanceClock(System::Clock::Seconds16(60));
    EXPECT_FALSE(reg.IsEmpty());
}

// A fabric may only hold so many concurrent requests; the cap does not stop a node that
// already has one from refreshing it.
TEST(TestCommissioningProxyBgScanRegistry, RequestLimitPerFabricRejectsExtraNodes)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    static_assert(CHIP_CONFIG_COMMISSIONING_PROXY_MAX_BGSCAN_REQUESTS_PER_FABRIC == 4, "test assumes a cap of 4");
    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode3, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode4, kPaf, k2g4, kNoTimeout), Status::Success);

    EXPECT_EQ(reg.Start(kFabric1, kNode5, kPaf, k2g4, kNoTimeout), Status::ResourceExhausted);

    // An existing node refreshing is not a new request.
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    // The cap is per fabric, so another fabric is unaffected.
    EXPECT_EQ(reg.Start(kFabric2, kNode5, kPaf, k2g4, kNoTimeout), Status::Success);
}

// Spec § ProxyBackGroundScanStopRequest: "clear all cached results for the transports
// and bands on which it has stopped scanning" — so a band another fabric still scans
// keeps its results.
TEST(TestCommissioningProxyBgScanRegistry, StopClearsOnlyBandsNoFabricStillScans)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, kBoth, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    // Fabric 1 drops both bands. 5G survives on fabric 2, so only 2G4's results go.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, kBoth), Status::Success);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k2g4.Raw());
    EXPECT_EQ(hw.stopCount, 0); // fabric 2 still wants the radio
}

// A band that another fabric still scans SHALL not have its results cleared at all.
TEST(TestCommissioningProxyBgScanRegistry, StopOfSharedBandClearsNothing)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);

    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_EQ(hw.clearCount, 0); // fabric 2 still scans 2G4
    EXPECT_FALSE(reg.IsEmpty());
}

// Spec § ProxyBackGroundScanStartRequest: "When the per-fabric Timeout elapses ... any
// cached results for that fabric's transports are cleared" — again only for bands no
// surviving fabric covers.
TEST(TestCommissioningProxyBgScanRegistry, LifetimeExpiryClearsOnlyItsOwnBands)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, 30), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    timers.AdvanceClock(System::Clock::Seconds16(31));

    EXPECT_FALSE(reg.IsEmpty()); // fabric 2 has no timeout
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k2g4.Raw());
    EXPECT_EQ(hw.stopCount, 0);
}

// When the last request goes the transport itself stops, which clears everything on it
// (bands == 0), not just one band.
TEST(TestCommissioningProxyBgScanRegistry, LastStopClearsWholeTransport)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);

    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), 0u);
    EXPECT_EQ(hw.stopCount, 1);
}

// FabricIndex values are reused after a fabric is removed, so a Timeout-0 request —
// which has no timer and would otherwise never end — must not outlive its fabric.
TEST(TestCommissioningProxyBgScanRegistry, RemoveFabricDropsItsRequestsAndStopsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric1, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    reg.RemoveFabric(kFabric1);

    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), 0u); // whole transport, nothing left scanning

    // The reused index starts clean: a stop from the old node finds nothing.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::NotFound);
}

// Removing one fabric SHALL leave another fabric's scan running, clearing only the
// bands that nobody covers any more.
TEST(TestCommissioningProxyBgScanRegistry, RemoveFabricLeavesOtherFabricsScanning)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k5g, kNoTimeout), Status::Success);

    reg.RemoveFabric(kFabric1);

    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 1);
    EXPECT_EQ(hw.lastClearedBands.Raw(), k2g4.Raw());
}

// Removing a fabric with no requests is a no-op.
TEST(TestCommissioningProxyBgScanRegistry, RemoveUnknownFabricIsNoop)
{
    MockHardwareControl hw;
    CommissioningProxyMockTimer timers;
    CommissioningProxyBgScanRegistry reg(hw, timers);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    reg.RemoveFabric(kFabric2);

    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 0);
}
