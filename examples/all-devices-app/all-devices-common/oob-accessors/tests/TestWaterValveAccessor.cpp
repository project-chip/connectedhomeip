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

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/water-valve/WaterValveAccessor.h>
#include <device/types/water-valve/impl/LoggingWaterValve.h>
#include <lib/core/TLV.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

class TestWaterValveAccessor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        EXPECT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.Register(1, mProvider), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().Startup(mClusterContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mRegistry.Register(std::make_unique<WaterValveAccessor>(mDevice)), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mDevice.ValveConfigurationAndControlCluster().Shutdown(ClusterShutdownType::kClusterShutdown);
        mDevice.Unregister(mProvider);
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }

    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
    CodeDrivenDataModelProvider mProvider{ mClusterContext.StorageDelegate(), mClusterContext.AttributePersistenceProvider() };
    LoggingWaterValve mDevice{ mTimerDelegate };
    InMemoryOOBAccessorRegistry mRegistry;
};

// Test 1: Setting CurrentState to kOpen
TEST_F(TestWaterValveAccessor, SetCurrentState_Open)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kOpen);
}

// Test 2: Setting CurrentState to kClosed
TEST_F(TestWaterValveAccessor, SetCurrentState_Closed)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kClosed));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kClosed);
}

// Test 3: Setting CurrentLevel to 60%
TEST_F(TestWaterValveAccessor, SetCurrentLevel_Level60)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
        static_cast<uint8_t>(60));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto level = mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel();
    ASSERT_FALSE(level.IsNull());
    EXPECT_EQ(level.Value(), 60);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kOpen);
}

// Test 4: Setting CurrentLevel to 0 closes valve
TEST_F(TestWaterValveAccessor, SetCurrentLevel_ZeroClosesValve)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
        static_cast<uint8_t>(0));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kClosed);
}

// Test 5: Reopening valve preserves LastOpenLevel
TEST_F(TestWaterValveAccessor, ReopenPreservesLastOpenLevel)
{
    // 1. Set level to 75
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
            static_cast<uint8_t>(75));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 75);

    // 2. Close valve via CurrentState
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
            static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kClosed));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kClosed);

    // 3. Reopen valve via CurrentState
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
            static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 75);
}

// Test 6: Rejects invalid CurrentState (e.g. kTransitioning)
TEST_F(TestWaterValveAccessor, RejectsInvalidCurrentState)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kTransitioning));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Test 7: Rejects invalid CurrentLevel (null or > 100)
TEST_F(TestWaterValveAccessor, RejectsInvalidCurrentLevel)
{
    // Null level
    {
        ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id,
                                       ValveConfigurationAndControl::Attributes::CurrentLevel::Id);
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        EXPECT_EQ(writer.PutNull(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    // Level > 100
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
            static_cast<uint8_t>(101));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
}

// Test 8: Unrecognized attribute returns std::nullopt -> NOT_FOUND in registry
TEST_F(TestWaterValveAccessor, RejectUnsupportedAttribute)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, static_cast<AttributeId>(0xFFFF)), static_cast<uint8_t>(1));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 9: Unhandled action or wrong endpoint
TEST_F(TestWaterValveAccessor, UnhandledActionOrEndpoint)
{
    uint8_t dummy[4] = { 0 };
    EXPECT_EQ(mRegistry.HandleAction("UnknownAction"_span, ByteSpan(dummy, sizeof(dummy))), CHIP_ERROR_NOT_FOUND);

    // Wrong cluster on valid endpoint
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, static_cast<ClusterId>(0xFFFF), ValveConfigurationAndControl::Attributes::CurrentState::Id),
            static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_ERROR_NOT_FOUND);
    }

    // Wrong endpoint
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(99, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 10: In-band OpenValve level is preserved when closed and reopened via OOB SetAttribute(CurrentState)
TEST_F(TestWaterValveAccessor, InBandOpenLevelPreservedOnOOBReopen)
{
    // Open in-band with 40%
    EXPECT_EQ(
        mDevice.ValveConfigurationAndControlCluster().OpenValve(DataModel::MakeNullable<Percent>(40), DataModel::NullNullable),
        CHIP_NO_ERROR);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 40);

    // Close via OOB SetAttribute(CurrentState = kClosed)
    auto closeResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kClosed));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(closeResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(closeResult)),
              CHIP_NO_ERROR);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kClosed);

    // Reopen via OOB CurrentState = kOpen
    auto openResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(openResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(openResult)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 40);
}

// Test 11: In-band OpenValve level is preserved when closed via OOB CurrentLevel = 0 and reopened via OOB CurrentState = kOpen
TEST_F(TestWaterValveAccessor, InBandOpenLevelPreservedOnZeroLevelCloseAndReopen)
{
    // Open in-band with 60%
    EXPECT_EQ(
        mDevice.ValveConfigurationAndControlCluster().OpenValve(DataModel::MakeNullable<Percent>(60), DataModel::NullNullable),
        CHIP_NO_ERROR);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 60);

    // Close via OOB SetAttribute(CurrentLevel = 0)
    auto closeResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
        static_cast<Percent>(0));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(closeResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(closeResult)),
              CHIP_NO_ERROR);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kClosed);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 0);

    // Reopen via OOB CurrentState = kOpen
    auto openResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(openResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(openResult)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(),
              ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 60);
}

// Test 12: DefaultOpenDuration is applied on OOB Open
TEST_F(TestWaterValveAccessor, DefaultOpenDurationAppliedOnOOBOpen)
{
    Testing::ClusterTester tester(mDevice.ValveConfigurationAndControlCluster());
    EXPECT_TRUE(tester
                    .WriteAttribute(ValveConfigurationAndControl::Attributes::DefaultOpenDuration::Id,
                                    DataModel::MakeNullable<uint32_t>(120))
                    .IsSuccess());

    // Reopen via OOB CurrentState = kOpen
    auto openResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id),
        static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(openResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(openResult)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetOpenDuration(), DataModel::MakeNullable<uint32_t>(120));

    // Also test OOB CurrentLevel open applies DefaultOpenDuration
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().CloseValve(), CHIP_NO_ERROR);

    auto levelResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id),
        static_cast<Percent>(80));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(levelResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(levelResult)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetOpenDuration(), DataModel::MakeNullable<uint32_t>(120));
}

} // namespace
