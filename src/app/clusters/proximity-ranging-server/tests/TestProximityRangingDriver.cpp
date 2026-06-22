/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// Driver-direct unit tests covering the responsibilities the driver owns
// independently of the cluster: ReportingCondition filtering on
// OnMeasurementData, remapping of the adapter-supplied
// kSessionEndTimeReached terminal status to kPeerNotFound when no
// measurement passed the filter, and the driver-owned StartTime / EndTime
// scheduling that gates adapter->PrepareSession → StartSession → StopSession.

#include <pw_unit_test/framework.h>

#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ProximityRanging;

/// Records every PrepareSession/StartSession/StopSession call and exposes
/// its Callback so tests can drive OnMeasurementData /
/// OnRangingSessionStopped directly.
///
/// Under the single-shot model the driver re-invokes StartSession once per
/// rangingInstanceInterval tick. Sessions stay in "prepared" state across
/// invocations; only StopSession releases them. The mock counts every call
/// so tests can assert on cadence-driven behavior.
class MockRangingAdapter : public RangingAdapter
{
public:
    explicit MockRangingAdapter(RangingTechEnum technology) : mTechnology(technology) {}

    RangingTechEnum GetTechnology() const override { return mTechnology; }
    Structs::RangingCapabilitiesStruct::Type GetCapabilities() const override
    {
        Structs::RangingCapabilitiesStruct::Type cap;
        cap.technology = mTechnology;
        return cap;
    }

    ResultCodeEnum PrepareSession(uint8_t sessionId, const StartSessionParams & params) override
    {
        mLastPrepareSessionId = sessionId;
        mLastPrepareParams    = params;
        mPrepareCalls++;
        if (mPrepareResult == ResultCodeEnum::kAccepted)
        {
            mPreparedIds.push_back(sessionId);
        }
        return mPrepareResult;
    }

    CHIP_ERROR StartSession(uint8_t sessionId) override
    {
        mLastStartSessionId = sessionId;
        mStartCalls++;
        for (auto id : mPreparedIds)
        {
            if (id == sessionId)
            {
                return mStartError;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        mLastStopSessionId = sessionId;
        mStopCalls++;
        for (auto it = mPreparedIds.begin(); it != mPreparedIds.end(); ++it)
        {
            if (*it == sessionId)
            {
                mPreparedIds.erase(it);
                if (mCallback != nullptr)
                {
                    mCallback->OnRangingSessionStopped(sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
                }
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    void StopAllSessions() override
    {
        while (!mPreparedIds.empty())
        {
            uint8_t id = mPreparedIds.back();
            mPreparedIds.pop_back();
            if (mCallback != nullptr)
            {
                mCallback->OnRangingSessionStopped(id, RangingSessionStatusEnum::kSessionEndTimeReached);
            }
        }
    }

    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & out) override
    {
        VerifyOrReturnError(out.size() >= mPreparedIds.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        for (size_t i = 0; i < mPreparedIds.size(); i++)
        {
            out[i] = mPreparedIds[i];
        }
        out.reduce_size(mPreparedIds.size());
        return CHIP_NO_ERROR;
    }

    std::optional<WiFiUsdConfig> GetWiFiUsdConfig() override { return mWiFiUsdConfig; }

    Callback * GetCallback() const { return mCallback; }

    ResultCodeEnum mPrepareResult = ResultCodeEnum::kAccepted;
    /// Configurable return for StartSession. Set to CHIP_ERROR_BUSY to
    /// simulate "previous measurement still in flight".
    CHIP_ERROR mStartError        = CHIP_NO_ERROR;
    int mPrepareCalls             = 0;
    int mStartCalls               = 0;
    int mStopCalls                = 0;
    uint8_t mLastPrepareSessionId = 0;
    uint8_t mLastStartSessionId   = 0;
    uint8_t mLastStopSessionId    = 0;
    StartSessionParams mLastPrepareParams{};
    std::optional<WiFiUsdConfig> mWiFiUsdConfig;
    /// Sessions for which PrepareSession returned kAccepted and StopSession
    /// has not yet been invoked. Single-shot model: there is no separate
    /// "active" list — measurements happen synchronously when the driver
    /// calls StartSession (or via the test driving the callback directly).
    std::vector<uint8_t> mPreparedIds;

private:
    RangingTechEnum mTechnology;
};

/// Capture-only ProximityRangingDriver::Callback for asserting on the
/// cluster-bound stream after the driver applies its filter / status override.
class RecordingClusterCallback : public ProximityRangingDriver::Callback
{
public:
    struct Measurement
    {
        uint8_t sessionId;
        Structs::RangingMeasurementDataStruct::Type measurement;
    };
    struct Stopped
    {
        uint8_t sessionId;
        RangingSessionStatusEnum status;
    };

    void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) override
    {
        measurements.push_back({ sessionId, measurement });
    }
    void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status) override { stops.push_back({ sessionId, status }); }
    void OnAttributeChanged(AttributeId attrId) override { attributeChanges.push_back(attrId); }

    std::vector<Measurement> measurements;
    std::vector<Stopped> stops;
    std::vector<AttributeId> attributeChanges;
};

constexpr uint8_t kSessionId = 7;

Commands::StartRangingRequest::DecodableType MakeBleRequest()
{
    Commands::StartRangingRequest::DecodableType req{};
    req.technology        = RangingTechEnum::kBLEBeaconRSSIRanging;
    req.trigger.startTime = 0;
    req.trigger.endTime   = 60;
    // kBLEScanningRole = active initiator. Passive-responder roles
    // (kBLEBeaconRole etc.) cause the driver to suppress the kPeerNotFound
    // remap, which most of these tests are not exercising.
    Structs::BLERangingDeviceRoleConfigStruct::Type role;
    role.role            = RangingRoleEnum::kBLEScanningRole;
    role.peerBLEDeviceID = 0x1234;
    req.BLERangingDeviceRoleConfig.SetValue(role);
    return req;
}

class TestProximityRangingDriver : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// 1. min distance filter: measurement below min is dropped, peerFound stays false.
TEST_F(TestProximityRangingDriver, OnMeasurementDataDistanceFilterMin)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request = MakeBleRequest();
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(50);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type below;
    below.distance.SetNonNull(static_cast<uint16_t>(20));
    ble.GetCallback()->OnMeasurementData(kSessionId, below);
    EXPECT_TRUE(cb.measurements.empty());

    Structs::RangingMeasurementDataStruct::Type ok;
    ok.distance.SetNonNull(static_cast<uint16_t>(60));
    ble.GetCallback()->OnMeasurementData(kSessionId, ok);
    EXPECT_EQ(cb.measurements.size(), 1u);

    driver.Shutdown();
}

// 2. max distance filter: measurement above max is dropped.
TEST_F(TestProximityRangingDriver, OnMeasurementDataDistanceFilterMax)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request = MakeBleRequest();
    Structs::ReportingConditionStruct::Type rc;
    rc.maxDistanceCondition.SetValue(100);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type above;
    above.distance.SetNonNull(static_cast<uint16_t>(200));
    ble.GetCallback()->OnMeasurementData(kSessionId, above);
    EXPECT_TRUE(cb.measurements.empty());

    Structs::RangingMeasurementDataStruct::Type ok;
    ok.distance.SetNonNull(static_cast<uint16_t>(50));
    ble.GetCallback()->OnMeasurementData(kSessionId, ok);
    EXPECT_EQ(cb.measurements.size(), 1u);

    driver.Shutdown();
}

// 3. distance is null + a min/max condition is present → drop.
TEST_F(TestProximityRangingDriver, OnMeasurementDataDistanceNullWithCondition)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request = MakeBleRequest();
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(50);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type m;
    m.distance.SetNull();
    ble.GetCallback()->OnMeasurementData(kSessionId, m);
    EXPECT_TRUE(cb.measurements.empty());

    driver.Shutdown();
}

// 4. distance is null + no distance condition → forward.
TEST_F(TestProximityRangingDriver, OnMeasurementDataDistanceNullNoCondition)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    ASSERT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type m;
    m.distance.SetNull();
    ble.GetCallback()->OnMeasurementData(kSessionId, m);
    EXPECT_EQ(cb.measurements.size(), 1u);

    driver.Shutdown();
}

// 5. errorMargin filter: measurement with errorMargin > condition is dropped.
TEST_F(TestProximityRangingDriver, OnMeasurementDataErrorMarginFilter)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request = MakeBleRequest();
    Structs::ReportingConditionStruct::Type rc;
    rc.errorMarginCondition.SetValue(10);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type sloppy;
    sloppy.distance.SetNonNull(static_cast<uint16_t>(100));
    sloppy.errorMargin.SetValue(static_cast<uint16_t>(50));
    ble.GetCallback()->OnMeasurementData(kSessionId, sloppy);
    EXPECT_TRUE(cb.measurements.empty());

    Structs::RangingMeasurementDataStruct::Type tight;
    tight.distance.SetNonNull(static_cast<uint16_t>(100));
    tight.errorMargin.SetValue(static_cast<uint16_t>(5));
    ble.GetCallback()->OnMeasurementData(kSessionId, tight);
    EXPECT_EQ(cb.measurements.size(), 1u);

    driver.Shutdown();
}

// 6. No reportingCondition → all measurements forwarded.
TEST_F(TestProximityRangingDriver, OnMeasurementDataNoReportingCondition)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    ASSERT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type m;
    m.distance.SetNonNull(static_cast<uint16_t>(123));
    m.errorMargin.SetValue(static_cast<uint16_t>(99));
    ble.GetCallback()->OnMeasurementData(kSessionId, m);
    EXPECT_EQ(cb.measurements.size(), 1u);

    driver.Shutdown();
}

// 7. Adapter terminates with kSessionEndTimeReached after no measurement
//    passed the filter → driver remaps to kPeerNotFound.
TEST_F(TestProximityRangingDriver, TerminationStatusPeerNotFoundWhenNoMeasurementPassedFilter)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request = MakeBleRequest();
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(1000);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type tooClose;
    tooClose.distance.SetNonNull(static_cast<uint16_t>(50));
    ble.GetCallback()->OnMeasurementData(kSessionId, tooClose);
    EXPECT_TRUE(cb.measurements.empty());

    ble.GetCallback()->OnRangingSessionStopped(kSessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kPeerNotFound);

    driver.Shutdown();
}

// 8. At least one passing measurement → cluster sees kSessionEndTimeReached.
TEST_F(TestProximityRangingDriver, TerminationStatusEndTimeReached)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    ASSERT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type m;
    m.distance.SetNonNull(static_cast<uint16_t>(100));
    ble.GetCallback()->OnMeasurementData(kSessionId, m);
    EXPECT_EQ(cb.measurements.size(), 1u);

    ble.GetCallback()->OnRangingSessionStopped(kSessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kSessionEndTimeReached);

    driver.Shutdown();
}

// 9. Adapter-driven kHardwareError passes through verbatim regardless of
//    peerFound state.
TEST_F(TestProximityRangingDriver, AdapterDrivenHardwareErrorPassthrough)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    ASSERT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kAccepted);

    ble.GetCallback()->OnRangingSessionStopped(kSessionId, RangingSessionStatusEnum::kHardwareError);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kHardwareError);

    driver.Shutdown();
}

// 10. StartSessionParams forwards role / band fields verbatim and intentionally
//     does NOT carry rangingInstanceInterval, startTime, or endTime — those
//     are entirely driver-owned. The peerBLEDeviceID set by MakeBleRequest
//     proves role configs are forwarded.
TEST_F(TestProximityRangingDriver, StartSessionParamsForwardsRoleConfigsNotTriggerOrInterval)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 25;
    request.trigger.rangingInstanceInterval.SetValue(3);
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(10);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    // Adapter-facing params carry the role config (peerBLEDeviceID from
    // MakeBleRequest is 0x1234) but no trigger / interval fields exist on
    // StartSessionParams at all.
    ASSERT_TRUE(ble.mLastPrepareParams.bleRoleConfig.has_value());
    Structs::BLERangingDeviceRoleConfigStruct::DecodableType empty{};
    EXPECT_EQ(ble.mLastPrepareParams.bleRoleConfig.value_or(empty).peerBLEDeviceID, 0x1234u);

    driver.Shutdown();
}

// 11. startTime == 0 → driver invokes PrepareSession then StartSession
//     synchronously, no timer fire required.
TEST_F(TestProximityRangingDriver, ZeroStartTimeFiresPrepareThenStartSynchronously)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    EXPECT_EQ(ble.mPrepareCalls, 1);
    EXPECT_EQ(ble.mStartCalls, 1);
    EXPECT_EQ(ble.mLastPrepareSessionId, kSessionId);
    EXPECT_EQ(ble.mLastStartSessionId, kSessionId);

    driver.Shutdown();
}

// 12. startTime > 0 → driver invokes PrepareSession immediately but defers
//     StartSession until the start timer fires. SessionIDList is dirty
//     immediately because the sessionId is acceptance-visible.
TEST_F(TestProximityRangingDriver, NonZeroStartTimeDefersStartUntilTimerFires)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 5;
    request.trigger.endTime   = 60;
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    EXPECT_EQ(ble.mPrepareCalls, 1);
    EXPECT_EQ(ble.mStartCalls, 0);
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 1u);
    EXPECT_FALSE(cb.attributeChanges.empty());

    // Advance just shy of the start deadline; StartSession must still not
    // have been invoked.
    timer.AdvanceClock(System::Clock::Milliseconds32(4'000));
    EXPECT_EQ(ble.mStartCalls, 0);

    // Cross the 5s threshold; the driver must invoke StartSession exactly once.
    timer.AdvanceClock(System::Clock::Milliseconds32(2'000));
    EXPECT_EQ(ble.mStartCalls, 1);

    driver.Shutdown();
}

// 13. End timer fires while session is active → driver invokes StopSession.
//     Uses startTime == 0 so only the end timer is armed (TimerDelegateMock
//     tracks only one timer at a time, so testing both start and end timers
//     simultaneously requires a richer mock; the more interesting scenario
//     for the driver is the end-time cutoff itself, exercised here).
TEST_F(TestProximityRangingDriver, EndTimeFiresWhileSessionActive)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 10;
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);
    EXPECT_EQ(ble.mStartCalls, 1);

    timer.AdvanceClock(System::Clock::Milliseconds32(11'000));
    EXPECT_EQ(ble.mStopCalls, 1);
    EXPECT_EQ(ble.mLastStopSessionId, kSessionId);
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 0u);

    driver.Shutdown();
}

// 14. Adapter rejects in PrepareSession → no session record committed,
//     SessionIDList is not made dirty, no timers armed.
TEST_F(TestProximityRangingDriver, PrepareSessionRejectedByAdapter)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    ble.mPrepareResult          = ResultCodeEnum::kRejectedInfeasibleRanging;
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    EXPECT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kRejectedInfeasibleRanging);
    EXPECT_EQ(ble.mPrepareCalls, 1);
    EXPECT_EQ(ble.mStartCalls, 0);
    EXPECT_EQ(ble.mStopCalls, 0);
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 0u);
    EXPECT_TRUE(cb.attributeChanges.empty());

    driver.Shutdown();
}

// 15. Shutdown cancels per-session timers so they cannot fire post-shutdown.
TEST_F(TestProximityRangingDriver, ShutdownCancelsPendingTimers)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 5;
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    driver.Shutdown();

    // After Shutdown, advancing past the start deadline must not produce a
    // late StartSession invocation.
    const int startCallsAtShutdown = ble.mStartCalls;
    timer.AdvanceClock(System::Clock::Milliseconds32(60'000));
    EXPECT_EQ(ble.mStartCalls, startCallsAtShutdown);
}

// 16. Periodic ranging: driver re-invokes StartSession on every interval tick
//     (anchored from the moment StartSession was issued, not from when the
//     measurement returns). With startTime==0 and interval=2s, advancing the
//     clock 5 seconds should produce the initial StartSession plus two
//     additional ticks (at t=2s and t=4s).
TEST_F(TestProximityRangingDriver, PeriodicRangingDriverReinvokesStartSession)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    request.trigger.rangingInstanceInterval.SetValue(2);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    // Initial synchronous StartSession invocation.
    EXPECT_EQ(ble.mStartCalls, 1);

    // First periodic tick at t=2s.
    timer.AdvanceClock(System::Clock::Milliseconds32(2'000));
    EXPECT_EQ(ble.mStartCalls, 2);

    // Second periodic tick at t=4s.
    timer.AdvanceClock(System::Clock::Milliseconds32(2'000));
    EXPECT_EQ(ble.mStartCalls, 3);

    driver.Shutdown();
}

// 17. Instant ranging: the FIRST measurement that satisfies ReportingCondition
//     causes the driver to call adapter->StopSession synchronously, terminating
//     the session before EndTime fires. The cluster sees a single
//     RangingResult event followed by RangingSessionStatus(SessionEndTimeReached).
TEST_F(TestProximityRangingDriver, InstantRangingTerminatesOnFirstPassingMeasurement)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    // No rangingInstanceInterval -> instant ranging.
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);
    EXPECT_EQ(ble.mStartCalls, 1);

    // Adapter delivers a passing measurement. Driver forwards it AND calls
    // StopSession (instant ranging termination).
    Structs::RangingMeasurementDataStruct::Type m;
    m.distance.SetNonNull(static_cast<uint16_t>(100));
    ble.GetCallback()->OnMeasurementData(kSessionId, m);

    EXPECT_EQ(cb.measurements.size(), 1u);
    EXPECT_EQ(ble.mStopCalls, 1);
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 0u);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kSessionEndTimeReached);

    driver.Shutdown();
}

// 18. Instant ranging + filtered measurement: the driver drops the
//     measurement and does NOT re-issue StartSession. Per spec, instant
//     ranging is a single ranging attempt; the session lingers until
//     EndTime fires, at which point the kPeerNotFound remap produces the
//     spec-correct outcome.
TEST_F(TestProximityRangingDriver, InstantRangingFilteredMeasurementDoesNotRetry)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(1000); // 100cm measurement will be filtered
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);
    EXPECT_EQ(ble.mStartCalls, 1);

    Structs::RangingMeasurementDataStruct::Type tooClose;
    tooClose.distance.SetNonNull(static_cast<uint16_t>(100));
    ble.GetCallback()->OnMeasurementData(kSessionId, tooClose);

    // Measurement was filtered: not forwarded; driver did NOT re-issue
    // StartSession; session stays alive until EndTime.
    EXPECT_TRUE(cb.measurements.empty());
    EXPECT_EQ(ble.mStopCalls, 0);
    EXPECT_EQ(ble.mStartCalls, 1);
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 1u);

    driver.Shutdown();
}

// 19a. Passive-responder sessions (BLE beacon, Wi-Fi publisher, BLT-CS
//      reflector) do NOT trigger the kPeerNotFound remap when the session
//      ends without any measurement. The absence of measurements is the
//      expected outcome for a responder, not a peer-not-found condition.
TEST_F(TestProximityRangingDriver, PassiveResponderSessionDoesNotRemapToPeerNotFound)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    // Start a passive-responder session (kBLEBeaconRole).
    Commands::StartRangingRequest::DecodableType request{};
    request.technology        = RangingTechEnum::kBLEBeaconRSSIRanging;
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    Structs::BLERangingDeviceRoleConfigStruct::Type role;
    role.role            = RangingRoleEnum::kBLEBeaconRole; // passive responder
    role.peerBLEDeviceID = 0x1234;
    request.BLERangingDeviceRoleConfig.SetValue(role);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    // No measurement is delivered; adapter terminates with kSessionEndTimeReached
    // (e.g. driver-issued StopSession at end-time). Driver MUST NOT remap to
    // kPeerNotFound for the responder side.
    ble.GetCallback()->OnRangingSessionStopped(kSessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kSessionEndTimeReached);

    driver.Shutdown();
}

// 19. Periodic ranging + adapter returns CHIP_ERROR_BUSY on a tick: the driver
//     must continue scheduling subsequent ticks; the busy tick is just skipped.
TEST_F(TestProximityRangingDriver, PeriodicRangingBusyAdapterDoesNotRetireSession)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    ble.mStartError             = CHIP_ERROR_BUSY;
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    request.trigger.rangingInstanceInterval.SetValue(2);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    // Even though every StartSession returns CHIP_ERROR_BUSY, the session
    // remains alive and the periodic schedule keeps firing.
    EXPECT_EQ(ble.mStartCalls, 1);
    timer.AdvanceClock(System::Clock::Milliseconds32(2'000));
    EXPECT_EQ(ble.mStartCalls, 2);
    timer.AdvanceClock(System::Clock::Milliseconds32(2'000));
    EXPECT_EQ(ble.mStartCalls, 3);

    EXPECT_EQ(driver.GetNumActiveSessionIds(), 1u);
    EXPECT_EQ(ble.mStopCalls, 0);

    driver.Shutdown();
}

// 21. Init can only be called once between Shutdown calls. A second Init
//     without an intervening Shutdown surfaces CHIP_ERROR_INCORRECT_STATE so
//     a second cluster cannot silently steal the callback wiring.
TEST_F(TestProximityRangingDriver, InitTwiceFailsWithoutShutdown)
{
    TimerDelegateMock timer;
    ProximityRangingDriver driver{ {}, timer };

    struct Sink : public ProximityRangingDriver::Callback
    {
        void OnMeasurementData(uint8_t, const Structs::RangingMeasurementDataStruct::Type &) override {}
        void OnSessionStopped(uint8_t, RangingSessionStatusEnum) override {}
        void OnAttributeChanged(AttributeId) override {}
    } sink;

    EXPECT_EQ(driver.Init(sink), CHIP_NO_ERROR);
    EXPECT_EQ(driver.Init(sink), CHIP_ERROR_INCORRECT_STATE);

    driver.Shutdown();
    // After Shutdown, Init must succeed again.
    EXPECT_EQ(driver.Init(sink), CHIP_NO_ERROR);
    driver.Shutdown();
}

// 22. Shutdown drives StopSession on every active session and releases the
//     pool slots so a re-Init starts clean.
TEST_F(TestProximityRangingDriver, ShutdownStopsActiveSessions)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    ASSERT_EQ(driver.HandleStartRanging(11, request), ResultCodeEnum::kAccepted);
    ASSERT_EQ(driver.HandleStartRanging(12, request), ResultCodeEnum::kAccepted);
    ASSERT_EQ(driver.GetNumActiveSessionIds(), 2u);

    // Shutdown must drive each adapter's StopSession path; the mock then
    // routes OnRangingSessionStopped back, which RetireSession releases.
    driver.Shutdown();
    EXPECT_EQ(driver.GetNumActiveSessionIds(), 0u);
}

// 23. GetActiveSessionIds returns CHIP_ERROR_BUFFER_TOO_SMALL when the
//     caller-supplied span cannot hold every active session ID.
TEST_F(TestProximityRangingDriver, GetActiveSessionIdsBufferTooSmall)
{
    TimerDelegateMock timer;
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 0;
    ASSERT_EQ(driver.HandleStartRanging(1, request), ResultCodeEnum::kAccepted);
    ASSERT_EQ(driver.HandleStartRanging(2, request), ResultCodeEnum::kAccepted);

    uint8_t buffer[1] = {};
    Span<uint8_t> tooSmall(buffer, 1);
    EXPECT_EQ(driver.GetActiveSessionIds(tooSmall), CHIP_ERROR_BUFFER_TOO_SMALL);

    driver.Shutdown();
}

// 24. GetWiFiUsdConfig falls back to the kWiFiNextGenerationRanging adapter
//     when no kWiFiRoundTripTimeRanging adapter is bound. Exercises the
//     second-branch lookup.
TEST_F(TestProximityRangingDriver, GetWiFiUsdConfigViaNextGenerationAdapter)
{
    TimerDelegateMock timer;
    MockRangingAdapter ngAdapter(RangingTechEnum::kWiFiNextGenerationRanging);
    WiFiUsdConfig cfg{};
    memset(cfg.deviceIdentityKey, 0x5A, sizeof(cfg.deviceIdentityKey));
    ngAdapter.mWiFiUsdConfig    = cfg;
    RangingAdapter * adapters[] = { &ngAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters), timer };

    auto resolved = driver.GetWiFiUsdConfig();
    ASSERT_TRUE(resolved.has_value());
    WiFiUsdConfig empty{};
    EXPECT_EQ(resolved.value_or(empty).deviceIdentityKey[0], 0x5A);
}

} // namespace
