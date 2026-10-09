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
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/power-source/BatteryPowerSource.h>
#include <device/types/power-source/BatteryPowerSourceAccessor.h>
#include <device/types/soil-sensor/SoilSensor.h>
#include <device/types/soil-sensor/SoilSensorAccessor.h>
#include <lib/core/TLV.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

class TestSoilSensorAccessor : public ::testing::Test
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
    DefaultTimerDelegate mTimerDelegate;
    CodeDrivenDataModelProvider mProvider;
    SoilSensor mDevice;

    TestSoilSensorAccessor() : mProvider(mStorage, mAttrStorage), mDevice(mTimerDelegate, /* includeTemperature = */ true)
    {
        EXPECT_EQ(mAttrStorage.Init(&mStorage), CHIP_NO_ERROR);
    }

    void SetUp() override { EXPECT_EQ(mDevice.Register(1, mProvider), CHIP_NO_ERROR); }

    void TearDown() override { mDevice.Unregister(mProvider); }
};

// Initial state: verifies non-null default values on first registration
TEST_F(TestSoilSensorAccessor, InitialAttributeValues)
{
    auto moisture = mDevice.SoilMeasurementCluster().GetSoilMoistureMeasuredValue();
    ASSERT_FALSE(moisture.IsNull());
    EXPECT_EQ(moisture.Value(), 50);

    auto temp = mDevice.TemperatureMeasurementCluster().GetMeasuredValue();
    ASSERT_FALSE(temp.IsNull());
    EXPECT_EQ(temp.Value(), 2100);

    auto battery = mDevice.PowerSourceCluster().GetBatPercentRemaining();
    ASSERT_FALSE(battery.IsNull());
    EXPECT_EQ(battery.Value(), 200);
}

// Test 1: Setting 42% moisture
TEST_F(TestSoilSensorAccessor, SetMoisture_DirectValue42)
{
    SoilSensorAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(42)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto moisture = mDevice.SoilMeasurementCluster().GetSoilMoistureMeasuredValue();
    ASSERT_FALSE(moisture.IsNull());
    EXPECT_EQ(moisture.Value(), 42);
}

// Test 2: Setting null moisture triggers probe fault
TEST_F(TestSoilSensorAccessor, SetMoisture_NullProbeFault)
{
    SoilSensorAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id);

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
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto moisture = mDevice.SoilMeasurementCluster().GetSoilMoistureMeasuredValue();
    EXPECT_TRUE(moisture.IsNull());
}

// Test 3: Setting temperature to 0 (ground frost alert)
TEST_F(TestSoilSensorAccessor, SetTemperature_ZeroDegreesFrostAlert)
{
    SoilSensorAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<int16_t>(0)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto temp = mDevice.TemperatureMeasurementCluster().GetMeasuredValue();
    ASSERT_FALSE(temp.IsNull());
    EXPECT_EQ(temp.Value(), 0);
}

// Test 3b: Setting 2150 (21.50 deg C)
TEST_F(TestSoilSensorAccessor, SetTemperature_2150Degrees)
{
    SoilSensorAccessor accessor(mDevice);
    ConcreteDataAttributePath path(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<int16_t>(2150)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto temp = mDevice.TemperatureMeasurementCluster().GetMeasuredValue();
    ASSERT_FALSE(temp.IsNull());
    EXPECT_EQ(temp.Value(), 2150);
}

// Test 4: Setting battery via BatteryPowerSourceAccessor
TEST_F(TestSoilSensorAccessor, SetBattery_24HalfPercent)
{
    BatteryPowerSourceAccessor batteryAccessor(mDevice.PowerSourceCluster(), mDevice.GetEndpointId());
    ConcreteDataAttributePath path(1, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(24)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        batteryAccessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto battery = mDevice.PowerSourceCluster().GetBatPercentRemaining();
    ASSERT_FALSE(battery.IsNull());
    EXPECT_EQ(battery.Value(), 24);
}

// Test 5: Standalone BatteryPowerSourceAccessor on BatteryPowerSource device
TEST_F(TestSoilSensorAccessor, StandaloneBatteryPowerSourceAccessor)
{
    DefaultTimerDelegate timerDelegate;
    BatteryPowerSource batteryDevice("Test Battery"_span, Clusters::PowerSource::BatReplaceabilityEnum::kUserReplaceable,
                                     timerDelegate);
    EXPECT_EQ(batteryDevice.Register(3, mProvider), CHIP_NO_ERROR);

    BatteryPowerSourceAccessor accessor(batteryDevice);
    ConcreteDataAttributePath path(3, PowerSource::Id, PowerSource::Attributes::BatPercentRemaining::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(80)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_NO_ERROR);

    auto battery = batteryDevice.BatteryPowerSourceCluster().GetBatPercentRemaining();
    ASSERT_FALSE(battery.IsNull());
    EXPECT_EQ(battery.Value(), 80);

    batteryDevice.Unregister(mProvider);
}

// Test 6: Optional temperature support (without temperature cluster)
TEST_F(TestSoilSensorAccessor, OptionalTemperature_Disabled)
{
    DefaultTimerDelegate timerDelegate;
    SoilSensor soilSensorWithoutTemp(timerDelegate, /* includeTemperature = */ false);
    EXPECT_EQ(soilSensorWithoutTemp.Register(2, mProvider), CHIP_NO_ERROR);
    EXPECT_FALSE(soilSensorWithoutTemp.HasTemperature());

    SoilSensorAccessor accessor(soilSensorWithoutTemp);
    ConcreteDataAttributePath path(2, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<int16_t>(2500)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto status =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status.value(), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));

    soilSensorWithoutTemp.Unregister(mProvider);
}

// Test 7: Wrong action or wrong endpoint
TEST_F(TestSoilSensorAccessor, UnhandledActionOrEndpoint)
{
    SoilSensorAccessor accessor(mDevice);

    // Wrong action
    uint8_t dummy[4]  = { 0 };
    auto actionStatus = accessor.HandleAction("UnknownAction"_span, ByteSpan(dummy, sizeof(dummy)));
    EXPECT_FALSE(actionStatus.has_value());

    // Wrong endpoint (e.g. endpoint 99)
    ConcreteDataAttributePath path(99, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id);
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(50)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    auto epStatus =
        accessor.HandleAction(OOBDataSerializer::kSetAttributeAction, ByteSpan(requestBuffer.data(), requestBuffer.size()));
    EXPECT_FALSE(epStatus.has_value());
}
