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

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/power-source/BatteryPowerSource.h>
#include <device/types/power-source/BatteryPowerSourceAccessor.h>
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

class TestBatteryPowerSourceAccessor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        EXPECT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.Register(1, mProvider), CHIP_NO_ERROR);
        EXPECT_EQ(mRegistry.Register(
                      std::make_unique<BatteryPowerSourceAccessor>(mDevice.BatteryPowerSourceCluster(), mDevice.GetEndpointId())),
                  CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mDevice.Unregister(mProvider);
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }

    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
    CodeDrivenDataModelProvider mProvider{ mClusterContext.StorageDelegate(), mClusterContext.AttributePersistenceProvider() };
    BatteryPowerSource mDevice{ "Test Battery"_span, Clusters::PowerSource::BatReplaceabilityEnum::kUserReplaceable,
                                mTimerDelegate };
    InMemoryOOBAccessorRegistry mRegistry;
};

// Initial state: verifies non-null default battery percentage
TEST_F(TestBatteryPowerSourceAccessor, InitialAttributeValues)
{
    auto battery = mDevice.BatteryPowerSourceCluster().GetBatPercentRemaining();
    ASSERT_FALSE(battery.IsNull());
    EXPECT_EQ(battery.Value(), 200);
}

// Test 1: Setting battery percentage (e.g. 170 = 85%)
TEST_F(TestBatteryPowerSourceAccessor, SetBatPercentRemaining_ValidValue)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id), static_cast<uint8_t>(170));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto battery = mDevice.BatteryPowerSourceCluster().GetBatPercentRemaining();
    ASSERT_FALSE(battery.IsNull());
    EXPECT_EQ(battery.Value(), 170);
}

// Test 2: Setting null battery percentage
TEST_F(TestBatteryPowerSourceAccessor, SetBatPercentRemaining_Null)
{
    ConcreteDataAttributePath path(1, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id);

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
              CHIP_NO_ERROR);

    auto battery = mDevice.BatteryPowerSourceCluster().GetBatPercentRemaining();
    EXPECT_TRUE(battery.IsNull());
}

// Test 3: Setting battery percentage > 200 returns ConstraintError
TEST_F(TestBatteryPowerSourceAccessor, SetBatPercentRemaining_ConstraintError)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id), static_cast<uint8_t>(201));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Test 4: Setting battery voltage (e.g. 3300 mV)
TEST_F(TestBatteryPowerSourceAccessor, SetBatVoltage_ValidValue)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, PowerSource::Id, PowerSource::Attributes::BatVoltage::Id), static_cast<uint32_t>(3300));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto voltage = mDevice.BatteryPowerSourceCluster().GetBatVoltage();
    ASSERT_FALSE(voltage.IsNull());
    EXPECT_EQ(voltage.Value(), 3300U);
}

// Test 5: Rejects unsupported attributes on PowerSource cluster (returns std::nullopt -> NOT_FOUND in registry)
TEST_F(TestBatteryPowerSourceAccessor, RejectUnsupportedAttribute)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, PowerSource::Id, PowerSource::Attributes::Order::Id), static_cast<uint8_t>(1));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 6: Unhandled action, wrong cluster, or wrong endpoint
TEST_F(TestBatteryPowerSourceAccessor, UnhandledActionOrEndpoint)
{
    // Wrong action
    uint8_t dummy[4] = { 0 };
    EXPECT_EQ(mRegistry.HandleAction("UnknownAction"_span, ByteSpan(dummy, sizeof(dummy))), CHIP_ERROR_NOT_FOUND);

    // Wrong cluster (OnOff)
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(1, Clusters::OnOff::Id, Clusters::OnOff::Attributes::OnOff::Id), true);
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_ERROR_NOT_FOUND);
    }

    // Wrong endpoint (endpoint 99)
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
            ConcreteAttributePath(99, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id), static_cast<uint8_t>(50));
        ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
        EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
                  CHIP_ERROR_NOT_FOUND);
    }
}

} // namespace
