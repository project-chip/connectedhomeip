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

// All tests use timeoutSecs == 0 so no lifetime timer is armed: the registry logic
// is exercised synchronously with no dependency on the system layer / event loop.
// The lifetime-timer expiry path reuses the same removal + OnBecameEmpty() logic
// that Stop() exercises directly here.
constexpr uint16_t kNoTimeout = 0;

} // namespace

TEST(TestCommissioningProxyBgScanRegistry, FirstFabricStartsHardware)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1);
    EXPECT_FALSE(reg.IsEmpty());
    EXPECT_FALSE(reg.IsPaused());
}

TEST(TestCommissioningProxyBgScanRegistry, SecondFabricDoesNotRestartHardware)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Start(kFabric2, kNode2, kPaf, k2g4, kNoTimeout), Status::Success);
    EXPECT_EQ(hw.startCount, 1); // already running; not restarted
}

TEST(TestCommissioningProxyBgScanRegistry, StartBusyStaysPausedAndKeepsFabric)
{
    MockHardwareControl hw;
    hw.startResult = CHIP_ERROR_BUSY; // radio held (BLE scanner / PAF connect slot)
    CommissioningProxyBgScanRegistry reg(hw);

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
    hw.startResult = CHIP_ERROR_INTERNAL;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Failure);
    EXPECT_TRUE(reg.IsEmpty());
    // Nothing was started, so the radio is not stopped and the cache is not cleared.
    EXPECT_EQ(hw.stopCount, 0);
    EXPECT_EQ(hw.clearCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, StopLastFabricStopsHardwareAndClears)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::Success);
    EXPECT_TRUE(reg.IsEmpty());
    EXPECT_EQ(hw.stopCount, 1);
    EXPECT_EQ(hw.clearCount, 1);
}

TEST(TestCommissioningProxyBgScanRegistry, StopUnknownFabricNotFound)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::NotFound);
}

TEST(TestCommissioningProxyBgScanRegistry, StopNoOverlapSucceedsAndKeepsFabric)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kPaf, k2g4, kNoTimeout), Status::Success);
    // Stop targets a different transport (BLE); nothing overlaps this PAF fabric.
    EXPECT_EQ(reg.Stop(kFabric1, kNode1, kBle, kNoBands), Status::Success);
    EXPECT_FALSE(reg.IsEmpty()); // fabric untouched
    EXPECT_EQ(hw.stopCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, StopBandSubsetKeepsFabricThenRemoves)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

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
    CommissioningProxyBgScanRegistry reg(hw);

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
    CommissioningProxyBgScanRegistry reg(hw);

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
    CommissioningProxyBgScanRegistry reg(hw);

    reg.Pause();
    EXPECT_FALSE(reg.IsPaused());
    EXPECT_EQ(hw.stopCount, 0);
}

TEST(TestCommissioningProxyBgScanRegistry, ResumeWhenNotPausedIsNoop)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

    EXPECT_EQ(reg.Start(kFabric1, kNode1, kBle, kNoBands, kNoTimeout), Status::Success);
    reg.ResumeIfNeeded();
    EXPECT_EQ(hw.startCount, 1); // was already running
}

TEST(TestCommissioningProxyBgScanRegistry, ResumeStillBusyStaysPaused)
{
    MockHardwareControl hw;
    CommissioningProxyBgScanRegistry reg(hw);

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
    {
        CommissioningProxyBgScanRegistry reg(hw);
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
    CommissioningProxyBgScanRegistry reg(hw);

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
