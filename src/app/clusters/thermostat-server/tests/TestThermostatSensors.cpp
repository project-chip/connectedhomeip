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

#include "ThermostatTestCommon.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/clusters/thermostat-server/SensorScheduleTransitionStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatSensorStructWithOwnedMembers.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;
using chip::app::Clusters::Globals::AtomicRequestTypeEnum;

namespace {

constexpr uint8_t kHandle0Data[] = { 0x01 };
constexpr uint8_t kHandle1Data[] = { 0x02 };
constexpr uint8_t kHandle2Data[] = { 0x03 };
const ByteSpan kHandle0(kHandle0Data);
const ByteSpan kHandle1(kHandle1Data);
const ByteSpan kHandle2(kHandle2Data);

template <typename T>
DataModel::ActionReturnStatus AppendItemWithSubject(ThermostatClusterBase & cluster, AttributeId attrId,
                                                    const Access::SubjectDescriptor & subjectDescriptor, const T & item)
{
    ConcreteAttributePath path(kTestEndpointId, Thermostat::Id, attrId);
    WriteOperation writeOp(path);
    writeOp.SetSubjectDescriptor(subjectDescriptor);
    writeOp.SetListOperation(ConcreteDataAttributePath::ListOperation::AppendItem);
    auto decoder = writeOp.DecoderFor(item);
    return cluster.WriteAttribute(writeOp.GetRequest(), decoder);
}

template <typename T>
DataModel::ActionReturnStatus ReplaceAllWithSubject(ThermostatClusterBase & cluster, AttributeId attrId,
                                                    const Access::SubjectDescriptor & subjectDescriptor, const T & listPayload)
{
    ConcreteAttributePath path(kTestEndpointId, Thermostat::Id, attrId);
    WriteOperation writeOp(path);
    writeOp.SetSubjectDescriptor(subjectDescriptor);
    writeOp.SetListOperation(ConcreteDataAttributePath::ListOperation::ReplaceAll);
    auto decoder = writeOp.DecoderFor(listPayload);
    return cluster.WriteAttribute(writeOp.GetRequest(), decoder);
}

TEST(TestThermostatSensorStructWithOwnedMembers, TestStructOperations)
{
    ThermostatSensorStructWithOwnedMembers sensor;

    // Test Name
    EXPECT_EQ(sensor.SetName("Room Sensor"_span), CHIP_NO_ERROR);
    EXPECT_TRUE(sensor.GetName().data_equal("Room Sensor"_span));

    // Name max size 64
    char maxName[kThermostatSensorNameMaxSize];
    memset(maxName, 'A', sizeof(maxName));
    EXPECT_EQ(sensor.SetName(CharSpan(maxName, sizeof(maxName))), CHIP_NO_ERROR);
    EXPECT_TRUE(sensor.GetName().data_equal(CharSpan(maxName, sizeof(maxName))));

    // Name exceeding 64
    char tooLargeName[kThermostatSensorNameMaxSize + 1];
    memset(tooLargeName, 'B', sizeof(tooLargeName));
    EXPECT_EQ(sensor.SetName(CharSpan(tooLargeName, sizeof(tooLargeName))), CHIP_ERROR_INVALID_STRING_LENGTH);

    // Test SensorHandle
    EXPECT_EQ(sensor.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    EXPECT_TRUE(sensor.GetSensorHandle().data_equal(kHandle0));

    // Handle max size 16
    uint8_t maxHandle[kThermostatSensorHandleMaxSize];
    memset(maxHandle, 0xEE, sizeof(maxHandle));
    EXPECT_EQ(sensor.SetSensorHandle(ByteSpan(maxHandle, sizeof(maxHandle))), CHIP_NO_ERROR);
    EXPECT_TRUE(sensor.GetSensorHandle().data_equal(ByteSpan(maxHandle, sizeof(maxHandle))));

    // Handle exceeding 16
    uint8_t tooLargeHandle[kThermostatSensorHandleMaxSize + 1];
    memset(tooLargeHandle, 0xFF, sizeof(tooLargeHandle));
    EXPECT_EQ(sensor.SetSensorHandle(ByteSpan(tooLargeHandle, sizeof(tooLargeHandle))), CHIP_ERROR_INVALID_STRING_LENGTH);

    // Test Cluster
    sensor.SetCluster(app::Clusters::TemperatureMeasurement::Id);
    EXPECT_EQ(sensor.GetCluster(), app::Clusters::TemperatureMeasurement::Id);

    // Test Endpoint
    sensor.SetEndpoint(MakeOptional<EndpointId>(static_cast<EndpointId>(2)));
    EXPECT_TRUE(sensor.GetEndpoint().HasValue());
    EXPECT_EQ(sensor.GetEndpoint().Value(), 2);
    sensor.SetEndpoint(NullOptional);
    EXPECT_FALSE(sensor.GetEndpoint().HasValue());

    // Test Node
    sensor.SetNode(MakeOptional<NodeId>(0x12345678ULL));
    EXPECT_TRUE(sensor.GetNode().HasValue());
    EXPECT_EQ(sensor.GetNode().Value(), 0x12345678ULL);
    sensor.SetNode(NullOptional);
    EXPECT_FALSE(sensor.GetNode().HasValue());

    // Test FabricIndex
    sensor.SetFabricIndex(MakeOptional<FabricIndex>(static_cast<FabricIndex>(1)));
    EXPECT_TRUE(sensor.GetFabricIndex().HasValue());
    EXPECT_EQ(sensor.GetFabricIndex().Value(), 1);
    sensor.SetFabricIndex(NullOptional);
    EXPECT_FALSE(sensor.GetFabricIndex().HasValue());

    // Test Copy assignment
    EXPECT_EQ(sensor.SetName("Original"_span), CHIP_NO_ERROR);
    EXPECT_EQ(sensor.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    sensor.SetCluster(app::Clusters::OccupancySensing::Id);
    sensor.SetEndpoint(MakeOptional<EndpointId>(static_cast<EndpointId>(3)));

    ThermostatSensorStructWithOwnedMembers copy;
    copy = sensor;
    EXPECT_TRUE(copy.GetName().data_equal("Original"_span));
    EXPECT_TRUE(copy.GetSensorHandle().data_equal(kHandle1));
    EXPECT_EQ(copy.GetCluster(), app::Clusters::OccupancySensing::Id);
    EXPECT_TRUE(copy.GetEndpoint().HasValue());
    EXPECT_EQ(copy.GetEndpoint().Value(), 3);

    // Modify original and verify copy remains independent
    EXPECT_EQ(sensor.SetName("Modified"_span), CHIP_NO_ERROR);
    EXPECT_TRUE(copy.GetName().data_equal("Original"_span));

    // Test assignment from base type
    Structs::ThermostatSensorStruct::Type baseType;
    baseType.name         = "Base"_span;
    baseType.sensorHandle = kHandle2;
    baseType.cluster      = app::Clusters::RelativeHumidityMeasurement::Id;
    baseType.endpoint     = MakeOptional<EndpointId>(static_cast<EndpointId>(4));

    ThermostatSensorStructWithOwnedMembers fromBase;
    fromBase = baseType;
    EXPECT_TRUE(fromBase.GetName().data_equal("Base"_span));
    EXPECT_TRUE(fromBase.GetSensorHandle().data_equal(kHandle2));
    EXPECT_EQ(fromBase.GetCluster(), app::Clusters::RelativeHumidityMeasurement::Id);
    EXPECT_TRUE(fromBase.GetEndpoint().HasValue());
    EXPECT_EQ(fromBase.GetEndpoint().Value(), 4);
}

TEST(TestSensorScheduleTransitionStructWithOwnedMembers, TestStructOperations)
{
    SensorScheduleTransitionStructWithOwnedMembers transition;

    // Test DayOfWeek
    BitMask<ScheduleDayOfWeekBitmap> days(ScheduleDayOfWeekBitmap::kMonday, ScheduleDayOfWeekBitmap::kFriday);
    transition.SetDayOfWeek(days);
    EXPECT_EQ(transition.GetDayOfWeek().Raw(), days.Raw());

    // Test TransitionTime
    transition.SetTransitionTime(480);
    EXPECT_EQ(transition.GetTransitionTime(), 480);
    transition.SetTransitionTime(0);
    EXPECT_EQ(transition.GetTransitionTime(), 0);
    transition.SetTransitionTime(1439);
    EXPECT_EQ(transition.GetTransitionTime(), 1439);

    // Test EnabledSensors with Span<const ByteSpan>
    ByteSpan handles[] = { kHandle0, kHandle1 };
    EXPECT_EQ(transition.SetEnabledSensors(Span<const ByteSpan>(handles, 2)), CHIP_NO_ERROR);
    auto enabled = transition.GetEnabledSensors();
    ASSERT_EQ(enabled.size(), 2u);
    EXPECT_TRUE(enabled[0].data_equal(kHandle0));
    EXPECT_TRUE(enabled[1].data_equal(kHandle1));

    // Exceeding max sensors per transition (32)
    ByteSpan largeHandles[kMaxEnabledSensorsPerTransition + 1];
    for (size_t i = 0; i < sizeof(largeHandles) / sizeof(largeHandles[0]); ++i)
    {
        largeHandles[i] = kHandle0;
    }
    EXPECT_EQ(transition.SetEnabledSensors(Span<const ByteSpan>(largeHandles, sizeof(largeHandles) / sizeof(largeHandles[0]))),
              CHIP_ERROR_INVALID_LIST_LENGTH);

    // Handle length exceeding 16
    uint8_t tooLargeData[kMaxSensorHandleSize + 1];
    ByteSpan tooLargeHandle(tooLargeData, sizeof(tooLargeData));
    EXPECT_EQ(transition.SetEnabledSensors(Span<const ByteSpan>(&tooLargeHandle, 1)), CHIP_ERROR_INVALID_STRING_LENGTH);

    // Test EnabledSensors with DataModel::List<const ByteSpan>
    auto listPayload = DataModel::List<const ByteSpan>(handles, 2);
    EXPECT_EQ(transition.SetEnabledSensors(listPayload), CHIP_NO_ERROR);
    enabled = transition.GetEnabledSensors();
    ASSERT_EQ(enabled.size(), 2u);

    // Test Copy assignment
    SensorScheduleTransitionStructWithOwnedMembers copy;
    copy = transition;
    EXPECT_EQ(copy.GetDayOfWeek().Raw(), days.Raw());
    EXPECT_EQ(copy.GetTransitionTime(), 1439);
    ASSERT_EQ(copy.GetEnabledSensors().size(), 2u);
    EXPECT_TRUE(copy.GetEnabledSensors()[0].data_equal(kHandle0));

    // Test assignment from base Struct
    Structs::SensorScheduleTransitionStruct::Type baseStruct;
    baseStruct.dayOfWeek      = ScheduleDayOfWeekBitmap::kTuesday;
    baseStruct.transitionTime = 720;
    ByteSpan singleHandle[]   = { kHandle2 };
    baseStruct.enabledSensors = DataModel::List<const ByteSpan>(singleHandle, 1);

    SensorScheduleTransitionStructWithOwnedMembers fromBase;
    fromBase = baseStruct;
    EXPECT_EQ(fromBase.GetDayOfWeek().Raw(), to_underlying(ScheduleDayOfWeekBitmap::kTuesday));
    EXPECT_EQ(fromBase.GetTransitionTime(), 720);
    ASSERT_EQ(fromBase.GetEnabledSensors().size(), 1u);
    EXPECT_TRUE(fromBase.GetEnabledSensors()[0].data_equal(kHandle2));
}

TEST_F(ThermostatTestFixture, TestSensorsFeatureDisabled)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_FALSE(HasAttribute(cluster, Sensors::Id));
    EXPECT_FALSE(HasAttribute(cluster, AvailableSensors::Id));
    EXPECT_FALSE(HasAttribute(cluster, EnabledSensors::Id));
    EXPECT_FALSE(HasAttribute(cluster, NumberOfSensorScheduleTransitions::Id));
    EXPECT_FALSE(HasAttribute(cluster, SensorSchedule::Id));

    Attributes::Sensors::TypeInfo::DecodableType sensorsList;
    EXPECT_EQ(tester.ReadAttribute(Sensors::Id, sensorsList), Status::UnsupportedAttribute);

    Attributes::AvailableSensors::TypeInfo::DecodableType availableList;
    EXPECT_EQ(tester.ReadAttribute(AvailableSensors::Id, availableList), Status::UnsupportedAttribute);

    Attributes::EnabledSensors::TypeInfo::DecodableType enabledList;
    EXPECT_EQ(tester.ReadAttribute(EnabledSensors::Id, enabledList), Status::UnsupportedAttribute);

    uint8_t numTransitions = 0;
    EXPECT_EQ(tester.ReadAttribute(NumberOfSensorScheduleTransitions::Id, numTransitions), Status::UnsupportedAttribute);

    Attributes::SensorSchedule::TypeInfo::DecodableType scheduleList;
    EXPECT_EQ(tester.ReadAttribute(SensorSchedule::Id, scheduleList), Status::UnsupportedAttribute);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorsFeatureEnabledAttributesAndCommands)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // FeatureMap contains kThermostatSensors
    uint32_t readFeatures = 0;
    EXPECT_EQ(tester.ReadAttribute(FeatureMap::Id, readFeatures), Status::Success);
    EXPECT_TRUE(BitFlags<Feature>(readFeatures).Has(Feature::kThermostatSensors));

    // AttributeList contains all 5 attributes
    EXPECT_TRUE(HasAttribute(cluster, Sensors::Id));
    EXPECT_TRUE(HasAttribute(cluster, AvailableSensors::Id));
    EXPECT_TRUE(HasAttribute(cluster, EnabledSensors::Id));
    EXPECT_TRUE(HasAttribute(cluster, NumberOfSensorScheduleTransitions::Id));
    EXPECT_TRUE(HasAttribute(cluster, SensorSchedule::Id));

    // Accepted commands contains AtomicRequest
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    ASSERT_EQ(cluster.AcceptedCommands(cluster.GetPaths()[0], acceptedCommands), CHIP_NO_ERROR);
    auto accepted = acceptedCommands.TakeBuffer();
    EXPECT_TRUE(std::any_of(accepted.begin(), accepted.end(),
                            [](const auto & entry) { return entry.commandId == Commands::AtomicRequest::Id; }));

    // Generated commands contains AtomicResponse
    ReadOnlyBufferBuilder<CommandId> generatedCommands;
    ASSERT_EQ(cluster.GeneratedCommands(cluster.GetPaths()[0], generatedCommands), CHIP_NO_ERROR);
    auto generated = generatedCommands.TakeBuffer();
    EXPECT_TRUE(std::any_of(generated.begin(), generated.end(), [](CommandId id) { return id == Commands::AtomicResponse::Id; }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestReadSensorsAttributes)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    // Setup sensors in delegate
    ThermostatSensorStructWithOwnedMembers sensor0;
    EXPECT_EQ(sensor0.SetName("Internal Temp Sensor"_span), CHIP_NO_ERROR);
    EXPECT_EQ(sensor0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    sensor0.SetCluster(app::Clusters::TemperatureMeasurement::Id);
    sensor0.SetEndpoint(MakeOptional<EndpointId>(static_cast<EndpointId>(1)));
    mSensorsDelegate.AddSensor(sensor0);

    ThermostatSensorStructWithOwnedMembers sensor1;
    EXPECT_EQ(sensor1.SetName("External Occupancy Sensor"_span), CHIP_NO_ERROR);
    EXPECT_EQ(sensor1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    sensor1.SetCluster(app::Clusters::OccupancySensing::Id);
    sensor1.SetEndpoint(MakeOptional<EndpointId>(static_cast<EndpointId>(2)));
    sensor1.SetNode(MakeOptional<NodeId>(0xAABBCCDDEEFFULL));
    sensor1.SetFabricIndex(MakeOptional<FabricIndex>(Thermostat::kTestFabricIndex));
    mSensorsDelegate.AddSensor(sensor1);

    // Available and Enabled
    ByteSpan defaultAvailable[] = { kHandle0, kHandle1 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(defaultAvailable, 2));

    ByteSpan defaultEnabled[] = { kHandle0 };
    mSensorsDelegate.SetEnabledSensors(Span<const ByteSpan>(defaultEnabled, 1));

    // Transitions
    mSensorsDelegate.mNumberOfSensorScheduleTransitions = 8;
    SensorScheduleTransitionStructWithOwnedMembers trans0;
    trans0.SetDayOfWeek(ScheduleDayOfWeekBitmap::kMonday);
    trans0.SetTransitionTime(480);
    EXPECT_EQ(trans0.SetEnabledSensors(Span<const ByteSpan>(defaultEnabled, 1)), CHIP_NO_ERROR);
    mSensorsDelegate.AddTransition(trans0);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read Sensors
    Attributes::Sensors::TypeInfo::DecodableType readSensors;
    EXPECT_EQ(tester.ReadAttribute(Sensors::Id, readSensors), Status::Success);
    auto sensorIter = readSensors.begin();
    ASSERT_TRUE(sensorIter.Next());
    EXPECT_TRUE(sensorIter.GetValue().name.data_equal("Internal Temp Sensor"_span));
    EXPECT_TRUE(sensorIter.GetValue().sensorHandle.data_equal(kHandle0));
    EXPECT_EQ(sensorIter.GetValue().cluster, app::Clusters::TemperatureMeasurement::Id);
    EXPECT_TRUE(sensorIter.GetValue().endpoint.HasValue());
    EXPECT_EQ(sensorIter.GetValue().endpoint.Value(), 1);

    ASSERT_TRUE(sensorIter.Next());
    EXPECT_TRUE(sensorIter.GetValue().name.data_equal("External Occupancy Sensor"_span));
    EXPECT_TRUE(sensorIter.GetValue().sensorHandle.data_equal(kHandle1));
    EXPECT_EQ(sensorIter.GetValue().cluster, app::Clusters::OccupancySensing::Id);
    EXPECT_TRUE(sensorIter.GetValue().node.HasValue());
    EXPECT_EQ(sensorIter.GetValue().node.Value(), 0xAABBCCDDEEFFULL);
    EXPECT_FALSE(sensorIter.Next());
    EXPECT_EQ(sensorIter.GetStatus(), CHIP_NO_ERROR);

    // Read AvailableSensors
    Attributes::AvailableSensors::TypeInfo::DecodableType readAvailable;
    EXPECT_EQ(tester.ReadAttribute(AvailableSensors::Id, readAvailable), Status::Success);
    auto availIter = readAvailable.begin();
    ASSERT_TRUE(availIter.Next());
    EXPECT_TRUE(availIter.GetValue().data_equal(kHandle0));
    ASSERT_TRUE(availIter.Next());
    EXPECT_TRUE(availIter.GetValue().data_equal(kHandle1));
    EXPECT_FALSE(availIter.Next());
    EXPECT_EQ(availIter.GetStatus(), CHIP_NO_ERROR);

    // Read EnabledSensors
    Attributes::EnabledSensors::TypeInfo::DecodableType readEnabled;
    EXPECT_EQ(tester.ReadAttribute(EnabledSensors::Id, readEnabled), Status::Success);
    auto enabledIter = readEnabled.begin();
    ASSERT_TRUE(enabledIter.Next());
    EXPECT_TRUE(enabledIter.GetValue().data_equal(kHandle0));
    EXPECT_FALSE(enabledIter.Next());
    EXPECT_EQ(enabledIter.GetStatus(), CHIP_NO_ERROR);

    // Read NumberOfSensorScheduleTransitions
    uint8_t readNumTransitions = 0;
    EXPECT_EQ(tester.ReadAttribute(NumberOfSensorScheduleTransitions::Id, readNumTransitions), Status::Success);
    EXPECT_EQ(readNumTransitions, 8);

    // Read SensorSchedule
    Attributes::SensorSchedule::TypeInfo::DecodableType readSchedule;
    EXPECT_EQ(tester.ReadAttribute(SensorSchedule::Id, readSchedule), Status::Success);
    auto scheduleIter = readSchedule.begin();
    ASSERT_TRUE(scheduleIter.Next());
    EXPECT_EQ(scheduleIter.GetValue().dayOfWeek.Raw(), to_underlying(ScheduleDayOfWeekBitmap::kMonday));
    EXPECT_EQ(scheduleIter.GetValue().transitionTime, 480);
    auto transEnabledIter = scheduleIter.GetValue().enabledSensors.begin();
    ASSERT_TRUE(transEnabledIter.Next());
    EXPECT_TRUE(transEnabledIter.GetValue().data_equal(kHandle0));
    EXPECT_FALSE(transEnabledIter.Next());
    EXPECT_FALSE(scheduleIter.Next());
    EXPECT_EQ(scheduleIter.GetStatus(), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestWriteAvailableSensorsReplaceAll)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    // Setup 2 configured sensors
    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ThermostatSensorStructWithOwnedMembers s1;
    EXPECT_EQ(s1.SetName("Sensor 1"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s1);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Write valid AvailableSensors
    ByteSpan handles[] = { kHandle0, kHandle1 };
    auto validPayload  = DataModel::List<const ByteSpan>(handles, 2);
    EXPECT_EQ(tester.WriteAttribute(AvailableSensors::Id, validPayload, ListWritingPattern::ReplaceAll), Status::Success);
    EXPECT_EQ(mSensorsDelegate.mAvailableSensors.size(), 2u);
    EXPECT_TRUE(tester.IsAttributeDirty(AvailableSensors::Id));

    // Clear dirty list
    tester.GetDirtyList().clear();

    // Write unconfigured sensor handle -> ConstraintError
    ByteSpan unconfiguredHandles[] = { kHandle0, kHandle2 };
    auto unconfiguredPayload       = DataModel::List<const ByteSpan>(unconfiguredHandles, 2);
    EXPECT_EQ(tester.WriteAttribute(AvailableSensors::Id, unconfiguredPayload, ListWritingPattern::ReplaceAll),
              Status::ConstraintError);

    // Write duplicate handle -> ConstraintError
    ByteSpan duplicateHandles[] = { kHandle0, kHandle0 };
    auto duplicatePayload       = DataModel::List<const ByteSpan>(duplicateHandles, 2);
    EXPECT_EQ(tester.WriteAttribute(AvailableSensors::Id, duplicatePayload, ListWritingPattern::ReplaceAll),
              Status::ConstraintError);

    // Write handle size > 16 -> ConstraintError
    uint8_t tooLargeData[17]   = { 0 };
    ByteSpan tooLargeHandle    = ByteSpan(tooLargeData, sizeof(tooLargeData));
    ByteSpan oversizeHandles[] = { tooLargeHandle };
    auto oversizePayload       = DataModel::List<const ByteSpan>(oversizeHandles, 1);
    EXPECT_EQ(tester.WriteAttribute(AvailableSensors::Id, oversizePayload, ListWritingPattern::ReplaceAll),
              Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestWriteAvailableSensorsCascadeUpdateToEnabledSensors)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    // Setup 3 configured sensors
    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ThermostatSensorStructWithOwnedMembers s1;
    EXPECT_EQ(s1.SetName("Sensor 1"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s1);

    ThermostatSensorStructWithOwnedMembers s2;
    EXPECT_EQ(s2.SetName("Sensor 2"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s2.SetSensorHandle(kHandle2), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s2);

    // Initial AvailableSensors: { 0, 1, 2 }
    ByteSpan initAvailable[] = { kHandle0, kHandle1, kHandle2 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(initAvailable, 3));

    // Initial EnabledSensors: { 0, 1, 2 }
    mSensorsDelegate.SetEnabledSensors(Span<const ByteSpan>(initAvailable, 3));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Write AvailableSensors with only { 0, 2 } -> sensor 1 is removed
    ByteSpan newAvailable[] = { kHandle0, kHandle2 };
    auto newPayload         = DataModel::List<const ByteSpan>(newAvailable, 2);
    EXPECT_EQ(tester.WriteAttribute(AvailableSensors::Id, newPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // Verify EnabledSensors was automatically filtered to remove sensor 1
    ASSERT_EQ(mSensorsDelegate.mEnabledSensors.size(), 2u);
    EXPECT_TRUE(
        ByteSpan(mSensorsDelegate.mEnabledSensors[0].data(), mSensorsDelegate.mEnabledSensors[0].size()).data_equal(kHandle0));
    EXPECT_TRUE(
        ByteSpan(mSensorsDelegate.mEnabledSensors[1].data(), mSensorsDelegate.mEnabledSensors[1].size()).data_equal(kHandle2));

    // Both attributes should be marked dirty
    EXPECT_TRUE(tester.IsAttributeDirty(AvailableSensors::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(EnabledSensors::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestWriteAvailableSensorsAppendItem)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ThermostatSensorStructWithOwnedMembers s1;
    EXPECT_EQ(s1.SetName("Sensor 1"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s1);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Append configured handle kHandle0
    EXPECT_EQ(AppendItemWithSubject(cluster, AvailableSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle0),
              Status::Success);
    EXPECT_EQ(mSensorsDelegate.mAvailableSensors.size(), 1u);

    // Append configured handle kHandle1
    EXPECT_EQ(AppendItemWithSubject(cluster, AvailableSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle1),
              Status::Success);
    EXPECT_EQ(mSensorsDelegate.mAvailableSensors.size(), 2u);

    // Append duplicate handle kHandle0 -> ConstraintError
    EXPECT_EQ(AppendItemWithSubject(cluster, AvailableSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle0),
              Status::ConstraintError);

    // Append unconfigured handle kHandle2 -> ConstraintError
    EXPECT_EQ(AppendItemWithSubject(cluster, AvailableSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle2),
              Status::ConstraintError);

    // Append handle > 16 bytes -> ConstraintError
    uint8_t tooLargeData[17] = { 0 };
    EXPECT_EQ(AppendItemWithSubject(cluster, AvailableSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(),
                                    ByteSpan(tooLargeData, sizeof(tooLargeData))),
              Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestWriteEnabledSensorsReplaceAll)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    // Configured sensors: s0 and s1
    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ThermostatSensorStructWithOwnedMembers s1;
    EXPECT_EQ(s1.SetName("Sensor 1"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s1);

    // Only s0 is in AvailableSensors (s1 is NOT available)
    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Write EnabledSensors with available sensor s0 -> Success
    ByteSpan enabledValid[] = { kHandle0 };
    auto validPayload       = DataModel::List<const ByteSpan>(enabledValid, 1);
    EXPECT_EQ(tester.WriteAttribute(EnabledSensors::Id, validPayload, ListWritingPattern::ReplaceAll), Status::Success);
    EXPECT_EQ(mSensorsDelegate.mEnabledSensors.size(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(EnabledSensors::Id));

    // Write EnabledSensors with s1 (not available) -> ConstraintError
    ByteSpan enabledInvalid[] = { kHandle0, kHandle1 };
    auto invalidPayload       = DataModel::List<const ByteSpan>(enabledInvalid, 2);
    EXPECT_EQ(tester.WriteAttribute(EnabledSensors::Id, invalidPayload, ListWritingPattern::ReplaceAll), Status::ConstraintError);

    // Write EnabledSensors with duplicate handles -> ConstraintError
    ByteSpan enabledDuplicate[] = { kHandle0, kHandle0 };
    auto duplicatePayload       = DataModel::List<const ByteSpan>(enabledDuplicate, 2);
    EXPECT_EQ(tester.WriteAttribute(EnabledSensors::Id, duplicatePayload, ListWritingPattern::ReplaceAll), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestWriteEnabledSensorsAppendItem)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ThermostatSensorStructWithOwnedMembers s1;
    EXPECT_EQ(s1.SetName("Sensor 1"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s1.SetSensorHandle(kHandle1), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s1);

    // Available: s0 only
    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Append s0 (available) -> Success
    EXPECT_EQ(AppendItemWithSubject(cluster, EnabledSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle0),
              Status::Success);
    EXPECT_EQ(mSensorsDelegate.mEnabledSensors.size(), 1u);

    // Append s0 again (duplicate) -> ConstraintError
    EXPECT_EQ(AppendItemWithSubject(cluster, EnabledSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle0),
              Status::ConstraintError);

    // Append s1 (not in AvailableSensors) -> ConstraintError
    EXPECT_EQ(AppendItemWithSubject(cluster, EnabledSensors::Id, tester.GetCommandHandler().GetSubjectDescriptor(), kHandle1),
              Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleWritingOutsideAtomicSessionFails)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Writing SensorSchedule without an atomic write session returns InvalidInState
    Structs::SensorScheduleTransitionStruct::Type trans;
    trans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    trans.transitionTime = 480;
    ByteSpan handles[]   = { kHandle0 };
    trans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);

    Structs::SensorScheduleTransitionStruct::Type transitions[] = { trans };
    auto listPayload = DataModel::List<const Structs::SensorScheduleTransitionStruct::Type>(transitions, 1);

    EXPECT_EQ(tester.WriteAttribute(SensorSchedule::Id, listPayload, ListWritingPattern::ReplaceAll), Status::InvalidInState);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleAtomicWriteFullLifecycle)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    // Sensor setup
    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // 1. BeginAtomicWrite for SensorSchedule
    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));

    auto result = tester.Invoke(beginReq);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().statusCode, to_underlying(Status::Success));
    }

    // 2. While atomic session is open, writing non-atomic attribute returns InvalidInState
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::InvalidInState);

    // 3. Write SensorSchedule transition during atomic write
    Structs::SensorScheduleTransitionStruct::Type trans;
    trans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    trans.transitionTime = 480;
    ByteSpan handles[]   = { kHandle0 };
    trans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);

    Structs::SensorScheduleTransitionStruct::Type transitions[] = { trans };
    auto listPayload = DataModel::List<const Structs::SensorScheduleTransitionStruct::Type>(transitions, 1);
    EXPECT_EQ(tester.WriteAttribute(SensorSchedule::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // Pending transitions updated, committed still empty
    EXPECT_EQ(mSensorsDelegate.mPendingTransitions.size(), 1u);
    EXPECT_EQ(mSensorsDelegate.mTransitions.size(), 0u);

    // 4. Reading SensorSchedule during atomic session returns pending transitions
    Attributes::SensorSchedule::TypeInfo::DecodableType readPending;
    EXPECT_EQ(tester.ReadAttribute(SensorSchedule::Id, readPending), Status::Success);
    auto pendingIter = readPending.begin();
    ASSERT_TRUE(pendingIter.Next());
    EXPECT_EQ(pendingIter.GetValue().dayOfWeek.Raw(), to_underlying(ScheduleDayOfWeekBitmap::kMonday));
    EXPECT_EQ(pendingIter.GetValue().transitionTime, 480);
    EXPECT_FALSE(pendingIter.Next());

    // 5. CommitAtomicWrite
    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);

    result = tester.Invoke(commitReq);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().statusCode, to_underlying(Status::Success));
    }

    // Committed transitions updated
    EXPECT_EQ(mSensorsDelegate.mTransitions.size(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(SensorSchedule::Id));

    // Non-atomic writes are allowed again
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleAtomicWriteRollback)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // BeginAtomicWrite
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.IsSuccess());

    // Write pending transition
    Structs::SensorScheduleTransitionStruct::Type trans;
    trans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    trans.transitionTime = 480;
    ByteSpan handles[]   = { kHandle0 };
    trans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);

    Structs::SensorScheduleTransitionStruct::Type transitions[] = { trans };
    auto listPayload = DataModel::List<const Structs::SensorScheduleTransitionStruct::Type>(transitions, 1);
    EXPECT_EQ(tester.WriteAttribute(SensorSchedule::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);
    EXPECT_EQ(mSensorsDelegate.mPendingTransitions.size(), 1u);

    // RollbackAtomicWrite
    req.requestType = AtomicRequestTypeEnum::kRollbackWrite;
    result          = tester.Invoke(req);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().statusCode, to_underlying(Status::Success));
    }

    // Pending cleared, committed remains empty
    EXPECT_EQ(mSensorsDelegate.mPendingTransitions.size(), 0u);
    EXPECT_EQ(mSensorsDelegate.mTransitions.size(), 0u);

    // Non-atomic writes allowed again
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleAtomicWriteTimeout)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // BeginAtomicWrite with 2000 ms
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(2000));

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.IsSuccess());

    // Non-atomic write blocked
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::InvalidInState);

    // Advance clock past timeout
    mMockTimerDelegate.AdvanceClock(chip::System::Clock::Milliseconds64(3000));

    // Non-atomic write allowed again
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleAtomicWriteBusyAndMultiSubject)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Node A begins atomic write
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.IsSuccess());

    // Subject for Node B (different node on same fabric)
    Access::SubjectDescriptor subjectB;
    subjectB.fabricIndex = Thermostat::kTestFabricIndex;
    subjectB.authMode    = Access::AuthMode::kCase;
    subjectB.subject     = 0x9999ULL;

    // Node B tries to write SensorSchedule -> Busy
    Structs::SensorScheduleTransitionStruct::Type trans;
    trans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    trans.transitionTime = 480;
    ByteSpan handles[]   = { kHandle0 };
    trans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);

    Structs::SensorScheduleTransitionStruct::Type transitions[] = { trans };
    auto listPayload = DataModel::List<const Structs::SensorScheduleTransitionStruct::Type>(transitions, 1);

    EXPECT_EQ(ReplaceAllWithSubject(cluster, SensorSchedule::Id, subjectB, listPayload), Status::Busy);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorScheduleTransitionValidationErrors)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));
    mSensorsDelegate.mNumberOfSensorScheduleTransitions = 2; // limit to 2 transitions

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Open atomic session
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    EXPECT_TRUE(tester.Invoke(req).IsSuccess());

    // 1. DayOfWeek Away bit set (bit 7) -> ConstraintError
    Structs::SensorScheduleTransitionStruct::Type invalidAwayTrans;
    invalidAwayTrans.dayOfWeek.Set(ScheduleDayOfWeekBitmap::kAway);
    invalidAwayTrans.transitionTime = 480;
    ByteSpan handles[]              = { kHandle0 };
    invalidAwayTrans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(
        AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), invalidAwayTrans),
        Status::ConstraintError);

    // 2. DayOfWeek zero -> ConstraintError
    Structs::SensorScheduleTransitionStruct::Type invalidZeroDaysTrans;
    invalidZeroDaysTrans.dayOfWeek.ClearAll();
    invalidZeroDaysTrans.transitionTime = 480;
    invalidZeroDaysTrans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(
        AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), invalidZeroDaysTrans),
        Status::ConstraintError);

    // 3. TransitionTime > 1439 -> ConstraintError
    Structs::SensorScheduleTransitionStruct::Type invalidTimeTrans;
    invalidTimeTrans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    invalidTimeTrans.transitionTime = 1440;
    invalidTimeTrans.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(
        AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), invalidTimeTrans),
        Status::ConstraintError);

    // 4. EnabledSensors containing unavailable sensor -> ConstraintError
    Structs::SensorScheduleTransitionStruct::Type invalidSensorTrans;
    invalidSensorTrans.dayOfWeek        = ScheduleDayOfWeekBitmap::kMonday;
    invalidSensorTrans.transitionTime   = 480;
    ByteSpan unavailHandles[]           = { kHandle1 }; // kHandle1 not in available
    invalidSensorTrans.enabledSensors   = DataModel::List<const ByteSpan>(unavailHandles, 1);
    EXPECT_EQ(
        AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), invalidSensorTrans),
        Status::ConstraintError);

    // 5. EnabledSensors containing duplicate sensor -> ConstraintError
    Structs::SensorScheduleTransitionStruct::Type duplicateSensorTrans;
    duplicateSensorTrans.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    duplicateSensorTrans.transitionTime = 480;
    ByteSpan dupHandles[]               = { kHandle0, kHandle0 };
    duplicateSensorTrans.enabledSensors = DataModel::List<const ByteSpan>(dupHandles, 2);
    EXPECT_EQ(
        AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), duplicateSensorTrans),
        Status::ConstraintError);

    // 6. ResourceExhausted when exceeding NumberOfSensorScheduleTransitions (limit is 2)
    Structs::SensorScheduleTransitionStruct::Type validTrans1;
    validTrans1.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    validTrans1.transitionTime = 300;
    validTrans1.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), validTrans1),
              Status::Success);

    Structs::SensorScheduleTransitionStruct::Type validTrans2;
    validTrans2.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    validTrans2.transitionTime = 600;
    validTrans2.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), validTrans2),
              Status::Success);

    // 3rd transition exceeds max of 2 -> ResourceExhausted
    Structs::SensorScheduleTransitionStruct::Type validTrans3;
    validTrans3.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    validTrans3.transitionTime = 900;
    validTrans3.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), validTrans3),
              Status::ResourceExhausted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSensorSchedulePrecommitDuplicateTransitions)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kThermostatSensors);

    ThermostatSensorStructWithOwnedMembers s0;
    EXPECT_EQ(s0.SetName("Sensor 0"_span), CHIP_NO_ERROR);
    EXPECT_EQ(s0.SetSensorHandle(kHandle0), CHIP_NO_ERROR);
    mSensorsDelegate.AddSensor(s0);

    ByteSpan available[] = { kHandle0 };
    mSensorsDelegate.SetAvailableSensors(Span<const ByteSpan>(available, 1));
    mSensorsDelegate.mNumberOfSensorScheduleTransitions = 5;

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mSensorsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // BeginAtomicWrite
    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { SensorSchedule::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    EXPECT_TRUE(tester.Invoke(beginReq).IsSuccess());

    // Transition 1: Monday at 600
    Structs::SensorScheduleTransitionStruct::Type trans1;
    trans1.dayOfWeek      = ScheduleDayOfWeekBitmap::kMonday;
    trans1.transitionTime = 600;
    ByteSpan handles[]    = { kHandle0 };
    trans1.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), trans1),
              Status::Success);

    // Transition 2: Monday and Wednesday at 600 (overlaps Monday at same time!)
    Structs::SensorScheduleTransitionStruct::Type trans2;
    trans2.dayOfWeek.Set(ScheduleDayOfWeekBitmap::kMonday);
    trans2.dayOfWeek.Set(ScheduleDayOfWeekBitmap::kWednesday);
    trans2.transitionTime = 600;
    trans2.enabledSensors = DataModel::List<const ByteSpan>(handles, 1);
    EXPECT_EQ(AppendItemWithSubject(cluster, SensorSchedule::Id, tester.GetCommandHandler().GetSubjectDescriptor(), trans2),
              Status::Success);

    // CommitAtomicWrite triggers Precommit which detects overlapping transition
    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);

    auto result = tester.Invoke(commitReq);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        // Overall status is failure and attribute status is ConstraintError
        EXPECT_EQ(result.response.value().statusCode, to_underlying(Status::Failure));
        auto attrStatusIter = result.response.value().attributeStatus.begin();
        ASSERT_TRUE(attrStatusIter.Next());
        EXPECT_EQ(attrStatusIter.GetValue().statusCode, to_underlying(Status::ConstraintError));
        EXPECT_FALSE(attrStatusIter.Next());
    }

    // Committed schedule remains empty
    EXPECT_EQ(mSensorsDelegate.mTransitions.size(), 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
