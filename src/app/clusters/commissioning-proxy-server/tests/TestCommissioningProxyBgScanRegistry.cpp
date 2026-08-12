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
    void ClearCachedResults() override { clearCount++; }

    int startCount         = 0;
    int stopCount          = 0;
    int clearCount         = 0;
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

    // transport==0 means "stop only the given bands": drop 2G4, keep 5G.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k2g4), Status::Success);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 0);

    // Stopping the remaining band empties the fabric and tears the scan down.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kNoTransport, k5g), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
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
