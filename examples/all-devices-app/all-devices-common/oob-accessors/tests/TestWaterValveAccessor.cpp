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

#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/water-valve/WaterValve.h>
#include <device/types/water-valve/WaterValveAccessor.h>
#include <lib/core/TLV.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

class TestWaterValveAccessor : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

protected:
    TestPersistentStorageDelegate mStorage;
    DefaultAttributePersistenceProvider mAttrStorage;
    CodeDrivenDataModelProvider mProvider;
    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
    WaterValve mDevice;

    TestWaterValveAccessor() : mProvider(mStorage, mAttrStorage), mDevice(mTimerDelegate)
    {
        EXPECT_EQ(mAttrStorage.Init(&mStorage), CHIP_NO_ERROR);
    }

    void SetUp() override
    {
        EXPECT_EQ(mDevice.Register(1, mProvider), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().Startup(mClusterContext.Get()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mDevice.ValveConfigurationAndControlCluster().Shutdown(ClusterShutdownType::kClusterShutdown);
        mDevice.Unregister(mProvider);
    }
};

// Test 1: Setting CurrentState to kOpen
TEST_F(TestWaterValveAccessor, SetCurrentState_Open)
{
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kOpen);
}

// Test 2: Setting CurrentState to kClosed
TEST_F(TestWaterValveAccessor, SetCurrentState_Closed)
{
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kClosed)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kClosed);
}

// Test 3: Setting CurrentLevel to 60%
TEST_F(TestWaterValveAccessor, SetCurrentLevel_Level60)
{
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(60)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

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
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(0)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto state = mDevice.ValveConfigurationAndControlCluster().GetCurrentState();
    ASSERT_FALSE(state.IsNull());
    EXPECT_EQ(state.Value(), ValveConfigurationAndControl::ValveStateEnum::kClosed);
}

// Test 5: Reopening valve preserves LastOpenLevel
TEST_F(TestWaterValveAccessor, ReopenPreservesLastOpenLevel)
{
    WaterValveAccessor accessor(mDevice);

    // 1. Set level to 75
    {
        ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id);
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(75)), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        EXPECT_EQ(accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size())), CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.LastOpenLevel(), 75);

    // 2. Close valve via CurrentState
    {
        ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kClosed)), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        EXPECT_EQ(accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size())), CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(), ValveConfigurationAndControl::ValveStateEnum::kClosed);

    // 3. Reopen valve via CurrentState
    {
        ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen)), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        EXPECT_EQ(accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size())), CHIP_NO_ERROR);
    }
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentState().Value(), ValveConfigurationAndControl::ValveStateEnum::kOpen);
    EXPECT_EQ(mDevice.ValveConfigurationAndControlCluster().GetCurrentLevel().Value(), 75);
}

// Test 6: Rejects invalid CurrentState (e.g. kTransitioning or out-of-range)
TEST_F(TestWaterValveAccessor, RejectsInvalidCurrentState)
{
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kTransitioning)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Test 7: Rejects null or out-of-range CurrentLevel
TEST_F(TestWaterValveAccessor, RejectsInvalidCurrentLevel)
{
    WaterValveAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentLevel::Id);

    // Null level
    {
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

        auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        auto status = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(status.value(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
}

// Test 8: Unhandled action or wrong endpoint
TEST_F(TestWaterValveAccessor, UnhandledActionOrEndpoint)
{
    WaterValveAccessor accessor(mDevice);

    uint8_t dummy[4] = { 0 };
    auto actionStatus = accessor.HandleAction("UnknownAction"_span, ByteSpan(dummy, sizeof(dummy)));
    EXPECT_FALSE(actionStatus.has_value());

    ConcreteDataAttributePath path(99, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::CurrentState::Id);
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(ValveConfigurationAndControl::ValveStateEnum::kOpen)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto epStatus = accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    EXPECT_FALSE(epStatus.has_value());
}
