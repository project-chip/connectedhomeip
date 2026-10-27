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
#include <device/types/soil-sensor/OOBAccessors.h>
#include <device/types/soil-sensor/SoilSensor.h>
#include <device/types/soil-sensor/SoilSensorAccessor.h>
#include <device/types/soil-sensor/impl/IncreasingMoistureSoilSensor.h>
#include <devices/Types.h>
#include <lib/core/TLV.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultMoistureLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(kAccuracyRange),
};

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTempConfig = {
    .minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-1000)),
    .maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(5000)),
    .tolerance        = 0,
};

template <typename DeviceType>
DataModel::Nullable<chip::Percent> ReadMoisture(DeviceType & device)
{
    Testing::ClusterTester tester(device.SoilMeasurementCluster());
    SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::DecodableType moisture;
    EXPECT_TRUE(tester.ReadAttribute(SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id, moisture).IsSuccess());
    return moisture;
}

template <typename DeviceType>
DataModel::Nullable<int16_t> ReadTemperature(DeviceType & device)
{
    Testing::ClusterTester tester(device.TemperatureMeasurementCluster());
    TemperatureMeasurement::Attributes::MeasuredValue::TypeInfo::DecodableType temp;
    EXPECT_TRUE(tester.ReadAttribute(TemperatureMeasurement::Attributes::MeasuredValue::Id, temp).IsSuccess());
    return temp;
}

class TestSoilSensorAccessor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        EXPECT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.Register(1, mProvider), CHIP_NO_ERROR);
        RegisterOOBAccessors(mDevice, mRegistry);
    }

    void TearDown() override
    {
        mDevice.Unregister(mProvider);
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }

    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
    CodeDrivenDataModelProvider mProvider{ mClusterContext.StorageDelegate(), mClusterContext.AttributePersistenceProvider() };
    SoilSensor mDevice{ mTimerDelegate, kDefaultMoistureLimits, kDefaultTempConfig };
    InMemoryOOBAccessorRegistry mRegistry;
};

// Test 1: Verify initial attribute values on SoilSensor
TEST_F(TestSoilSensorAccessor, InitialAttributeValues)
{
    auto moisture = ReadMoisture(mDevice);
    EXPECT_TRUE(moisture.IsNull());

    auto temp = ReadTemperature(mDevice);
    EXPECT_TRUE(temp.IsNull());

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    EXPECT_EQ(mDevice.DeviceTypes(builder), CHIP_NO_ERROR);
    auto types = builder.TakeBuffer();
    EXPECT_EQ(types.size(), 1U);
    EXPECT_EQ(types[0].deviceTypeId, Device::Type::kSoilSensor.deviceTypeId);
}

// Test 2: Setting soil moisture to a direct valid percentage
TEST_F(TestSoilSensorAccessor, SetMoisture_DirectValue42)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id),
        static_cast<chip::Percent>(42));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto moisture = ReadMoisture(mDevice);
    ASSERT_FALSE(moisture.IsNull());
    EXPECT_EQ(moisture.Value(), 42);
}

// Test 3: Setting soil moisture to null (e.g. sensor probe fault)
TEST_F(TestSoilSensorAccessor, SetMoisture_NullProbeFault)
{
    // Set a non-null moisture first to verify transition to null
    auto setValidResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id),
        static_cast<chip::Percent>(42));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(setValidResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(setValidResult)),
              CHIP_NO_ERROR);
    EXPECT_EQ(ReadMoisture(mDevice).Value(), 42);

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
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto moisture = ReadMoisture(mDevice);
    EXPECT_TRUE(moisture.IsNull());
}

// Test 4: Setting temperature to a direct valid value (21.50°C -> 2150 in 0.01°C units)
TEST_F(TestSoilSensorAccessor, SetTemperature_DirectValue2150)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id),
        static_cast<int16_t>(2150));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto temp = ReadTemperature(mDevice);
    ASSERT_FALSE(temp.IsNull());
    EXPECT_EQ(temp.Value(), 2150);
}

// Test 5: Setting temperature to null (e.g. thermistor sensor fault)
TEST_F(TestSoilSensorAccessor, SetTemperature_NullSensorFault)
{
    // Set a non-null temperature first to verify transition to null
    auto setValidResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id),
        static_cast<int16_t>(2150));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(setValidResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(setValidResult)),
              CHIP_NO_ERROR);
    EXPECT_EQ(ReadTemperature(mDevice).Value(), 2150);

    ConcreteDataAttributePath path(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);

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

    auto temp = ReadTemperature(mDevice);
    EXPECT_TRUE(temp.IsNull());
}

// Test 6: Setting out-of-range moisture or temperature returns ConstraintError
TEST_F(TestSoilSensorAccessor, OutOfRangeValuesReturnConstraintError)
{
    // Out-of-range moisture (> 100%)
    auto moistureResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id),
        static_cast<chip::Percent>(101));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(moistureResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(moistureResult)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Temperature below minMeasuredValue (< -1000, i.e. < -10.00°C)
    auto tempLowResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id),
        static_cast<int16_t>(-1001));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(tempLowResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(tempLowResult)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Temperature above maxMeasuredValue (> 5000, i.e. > 50.00°C)
    auto tempHighResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id),
        static_cast<int16_t>(5001));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(tempHighResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(tempHighResult)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Test 7: Attribute update for unsupported cluster returns std::nullopt -> NOT_FOUND in registry
TEST_F(TestSoilSensorAccessor, RejectUnsupportedCluster)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, Identify::Id, static_cast<AttributeId>(0)), static_cast<uint16_t>(1));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 8: Unrecognized attribute returns std::nullopt -> NOT_FOUND in registry
TEST_F(TestSoilSensorAccessor, RejectUnsupportedAttribute)
{
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(1, SoilMeasurement::Id, static_cast<AttributeId>(0xFFFF)), static_cast<uint8_t>(1));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 9: Unhandled action or wrong endpoint
TEST_F(TestSoilSensorAccessor, UnhandledActionOrEndpoint)
{
    uint8_t dummy[4] = { 0 };
    EXPECT_EQ(mRegistry.HandleAction("UnknownAction"_span, ByteSpan(dummy, sizeof(dummy))), CHIP_ERROR_NOT_FOUND);

    // Wrong endpoint
    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(99, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id),
        static_cast<chip::Percent>(10));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(mRegistry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_ERROR_NOT_FOUND);
}

// Test 10: IncreasingMoistureSoilSensor registers, supports OOB accessors, and advances simulation from OOB state
TEST_F(TestSoilSensorAccessor, IncreasingMoistureSoilSensor_RegisterAndOOB)
{
    IncreasingMoistureSoilSensor sensor;
    EXPECT_EQ(sensor.Register(2, mProvider), CHIP_NO_ERROR);

    InMemoryOOBAccessorRegistry registry;
    RegisterOOBAccessors(sensor, registry);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(2, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id),
        static_cast<chip::Percent>(77));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));
    EXPECT_EQ(registry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(buildResult)),
              CHIP_NO_ERROR);

    auto moisture = ReadMoisture(sensor);
    ASSERT_FALSE(moisture.IsNull());
    EXPECT_EQ(moisture.Value(), 77);

    auto tempResult = OOBDataSerializer::BuildSetAttributeRequest(
        ConcreteAttributePath(2, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id),
        static_cast<int16_t>(2150));
    ASSERT_FALSE(std::holds_alternative<CHIP_ERROR>(tempResult));
    EXPECT_EQ(registry.HandleAction(OOBDataSerializer::kSetAttributeAction, std::get<ReadOnlyBuffer<uint8_t>>(tempResult)),
              CHIP_NO_ERROR);
    EXPECT_EQ(ReadTemperature(sensor).Value(), 2150);

    // Verify TimerFired advances from current cluster values rather than resetting to private defaults
    sensor.TimerFired();
    EXPECT_EQ(ReadMoisture(sensor).Value(), 78);
    EXPECT_EQ(ReadTemperature(sensor).Value(), 2250);

    sensor.Unregister(mProvider);
}

} // namespace
