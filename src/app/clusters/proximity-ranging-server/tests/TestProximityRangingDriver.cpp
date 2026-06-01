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
// OnMeasurementData, and remapping of the adapter-supplied
// kSessionEndTimeReached terminal status to kPeerNotFound when no measurement
// passed the filter. Timer-driven scheduling is intentionally NOT a driver
// responsibility — adapters drive their own measurement cadence and end-time
// cutoff.

#include <pw_unit_test/framework.h>

#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ProximityRanging;

/// Records every StartSession/StopSession call and exposes its Callback so
/// tests can drive OnMeasurementData / OnRangingSessionStopped directly.
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

    ResultCodeEnum StartSession(uint8_t sessionId, const StartSessionParams & params) override
    {
        mLastStartSessionId = sessionId;
        mLastStartParams    = params;
        mStartCalls++;
        if (mStartResult == ResultCodeEnum::kAccepted)
        {
            mActiveIds.push_back(sessionId);
        }
        return mStartResult;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        mStopCalls++;
        for (auto it = mActiveIds.begin(); it != mActiveIds.end(); ++it)
        {
            if (*it == sessionId)
            {
                mActiveIds.erase(it);
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
        while (!mActiveIds.empty())
        {
            uint8_t id = mActiveIds.back();
            mActiveIds.pop_back();
            if (mCallback != nullptr)
            {
                mCallback->OnRangingSessionStopped(id, RangingSessionStatusEnum::kSessionEndTimeReached);
            }
        }
    }

    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & out) override
    {
        VerifyOrReturnError(out.size() >= mActiveIds.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        for (size_t i = 0; i < mActiveIds.size(); i++)
        {
            out[i] = mActiveIds[i];
        }
        out.reduce_size(mActiveIds.size());
        return CHIP_NO_ERROR;
    }

    Callback * GetCallback() const { return mCallback; }

    ResultCodeEnum mStartResult = ResultCodeEnum::kAccepted;
    int mStartCalls             = 0;
    int mStopCalls              = 0;
    uint8_t mLastStartSessionId = 0;
    StartSessionParams mLastStartParams{};
    std::vector<uint8_t> mActiveIds;

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
    void OnAttributeChanged(AttributeId) override {}

    std::vector<Measurement> measurements;
    std::vector<Stopped> stops;
};

constexpr uint8_t kSessionId = 7;

Commands::StartRangingRequest::DecodableType MakeBleRequest()
{
    Commands::StartRangingRequest::DecodableType req{};
    req.technology        = RangingTechEnum::kBLEBeaconRSSIRanging;
    req.trigger.startTime = 0;
    req.trigger.endTime   = 60;
    Structs::BLERangingDeviceRoleConfigStruct::Type role;
    role.role            = RangingRoleEnum::kBLEBeaconRole;
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
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
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    ASSERT_EQ(driver.HandleStartRanging(kSessionId, MakeBleRequest()), ResultCodeEnum::kAccepted);

    ble.GetCallback()->OnRangingSessionStopped(kSessionId, RangingSessionStatusEnum::kHardwareError);
    ASSERT_EQ(cb.stops.size(), 1u);
    EXPECT_EQ(cb.stops[0].status, RangingSessionStatusEnum::kHardwareError);

    driver.Shutdown();
}

// 10. Driver passes the trigger field through to the adapter (used to be the
//     full request, now it's the narrowed StartSessionParams). reportingCondition
//     must NOT appear on the adapter side.
TEST_F(TestProximityRangingDriver, StartSessionParamsForwardsTriggerNotReporting)
{
    MockRangingAdapter ble(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &ble };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };
    RecordingClusterCallback cb;
    ASSERT_EQ(driver.Init(cb), CHIP_NO_ERROR);

    auto request              = MakeBleRequest();
    request.trigger.startTime = 5;
    request.trigger.endTime   = 25;
    request.trigger.rangingInstanceInterval.SetValue(3);
    Structs::ReportingConditionStruct::Type rc;
    rc.minDistanceCondition.SetValue(10);
    request.reportingCondition.SetValue(rc);
    ASSERT_EQ(driver.HandleStartRanging(kSessionId, request), ResultCodeEnum::kAccepted);

    EXPECT_EQ(ble.mLastStartParams.trigger.startTime, 5u);
    EXPECT_EQ(ble.mLastStartParams.trigger.endTime, 25u);
    ASSERT_TRUE(ble.mLastStartParams.trigger.rangingInstanceInterval.HasValue());
    EXPECT_EQ(ble.mLastStartParams.trigger.rangingInstanceInterval.Value(), 3u);

    driver.Shutdown();
}

} // namespace
