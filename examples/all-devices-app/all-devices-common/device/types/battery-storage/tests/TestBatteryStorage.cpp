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

#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/battery-storage/impl/SimulatedBatteryStorage.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/DeviceEnergyManagement/Attributes.h>
#include <clusters/DeviceEnergyManagement/Commands.h>
#include <clusters/ElectricalEnergyMeasurement/AttributeIds.h>
#include <clusters/ElectricalEnergyMeasurement/ClusterId.h>
#include <clusters/ElectricalPowerMeasurement/AttributeIds.h>
#include <clusters/ElectricalPowerMeasurement/ClusterId.h>
#include <clusters/Identify/ClusterId.h>
#include <clusters/PowerSource/AttributeIds.h>
#include <clusters/PowerSource/ClusterId.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

class BatteryStorageTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    Testing::ScopedAttributeChangeListenerRegistration mChangeListenerRegistration{ mProvider, mContext };
    TimerDelegateMock mTimerDelegate;
    SimpleTestEventTriggerDelegate mTestEventTriggerDelegate;
};

constexpr uint64_t kFakeReadingsLoadStart    = 0x0091'0000'0000'0001;
constexpr uint64_t kFakeReadingsGenStart     = 0x0091'0000'0000'0002;
constexpr uint64_t kEventTriggerEndpointMask = 0x0000'FFFF'0000'0000;

uint64_t EventTriggerForEndpoint(uint64_t eventTrigger, EndpointId endpoint)
{
    return (eventTrigger & ~kEventTriggerEndpointMask) | (static_cast<uint64_t>(endpoint) << 32);
}

class FailingEndpointIdAllocator : public EndpointIdAllocator
{
public:
    explicit FailingEndpointIdAllocator(size_t failureIndex) : mFailureIndex(failureIndex) {}

    EndpointId Allocate() override
    {
        if (mAllocationIndex++ == mFailureIndex)
        {
            return kInvalidEndpointId;
        }
        return mNextEndpoint++;
    }

private:
    const size_t mFailureIndex;
    size_t mAllocationIndex  = 0;
    EndpointId mNextEndpoint = 1;
};

class ProviderAttributeReader
{
public:
    explicit ProviderAttributeReader(CodeDrivenDataModelProvider & provider) : mProvider(provider) {}

    template <typename T>
    CHIP_ERROR Read(EndpointId endpoint, ClusterId cluster, AttributeId attribute, T & value)
    {
        mOperations.push_back(std::make_unique<Testing::ReadOperation>(endpoint, cluster, attribute));
        Testing::ReadOperation & operation             = *mOperations.back();
        std::unique_ptr<AttributeValueEncoder> encoder = operation.StartEncoding();
        VerifyOrReturnError(encoder != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(mProvider.ReadAttribute(operation.GetRequest(), *encoder).GetUnderlyingError());
        ReturnErrorOnFailure(operation.FinishEncoding());

        std::vector<Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(operation.GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
        return DataModel::Decode(attributeData[0].dataReader, value);
    }

private:
    CodeDrivenDataModelProvider & mProvider;
    std::vector<std::unique_ptr<Testing::ReadOperation>> mOperations;
};

std::vector<DataModel::EndpointEntry> GetEndpoints(CodeDrivenDataModelProvider & provider)
{
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
    EXPECT_EQ(provider.Endpoints(builder), CHIP_NO_ERROR);
    const auto buffer = builder.TakeBuffer();
    return std::vector<DataModel::EndpointEntry>(buffer.begin(), buffer.end());
}

void ExpectSingleDeviceType(CodeDrivenDataModelProvider & provider, EndpointId endpoint,
                            const DataModel::DeviceTypeEntry & expected)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    ASSERT_EQ(provider.DeviceTypes(endpoint, builder), CHIP_NO_ERROR);
    const auto deviceTypes = builder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    EXPECT_EQ(deviceTypes[0].deviceTypeId, expected.deviceTypeId);
    EXPECT_EQ(deviceTypes[0].deviceTypeRevision, expected.deviceTypeRevision);
}

void ExpectServerClusters(CodeDrivenDataModelProvider & provider, EndpointId endpoint,
                          std::initializer_list<ClusterId> expectedClusters)
{
    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    ASSERT_EQ(provider.ServerClusters(endpoint, builder), CHIP_NO_ERROR);
    const auto clusterEntries = builder.TakeBuffer();

    std::vector<ClusterId> actual;
    for (const auto & entry : clusterEntries)
    {
        actual.push_back(entry.clusterId);
    }
    std::vector<ClusterId> expected(expectedClusters);
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actual, expected);
}

void ExpectAttributes(CodeDrivenDataModelProvider & provider, const ConcreteClusterPath & path,
                      std::initializer_list<AttributeId> requiredAttributes,
                      std::initializer_list<AttributeId> disallowedAttributes = {})
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    ASSERT_EQ(provider.Attributes(path, builder), CHIP_NO_ERROR);
    const auto attributes = builder.TakeBuffer();

    for (AttributeId required : requiredAttributes)
    {
        EXPECT_NE(std::find_if(attributes.begin(), attributes.end(),
                               [required](const auto & entry) { return entry.attributeId == required; }),
                  attributes.end());
    }
    for (AttributeId disallowed : disallowedAttributes)
    {
        EXPECT_EQ(std::find_if(attributes.begin(), attributes.end(),
                               [disallowed](const auto & entry) { return entry.attributeId == disallowed; }),
                  attributes.end());
    }
}

void ExpectSingleSemanticTag(ProviderAttributeReader & reader, EndpointId endpoint, uint16_t expectedNamespace, uint8_t expectedTag)
{
    Clusters::Descriptor::Attributes::TagList::TypeInfo::DecodableType tagList;
    ASSERT_EQ(reader.Read(endpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::TagList::Id, tagList),
              CHIP_NO_ERROR);

    auto iterator = tagList.begin();
    ASSERT_TRUE(iterator.Next());
    EXPECT_EQ(iterator.GetValue().namespaceID, expectedNamespace);
    EXPECT_EQ(iterator.GetValue().tag, expectedTag);
    EXPECT_FALSE(iterator.Next());
    EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
}

void ExpectNoRegisteredEndpoints(CodeDrivenDataModelProvider & provider)
{
    EXPECT_TRUE(GetEndpoints(provider).empty());
}

void ExpectAllEndpointIdsInvalid(SimulatedBatteryStorage & device)
{
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetEnergyManagement().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetGridElectricalSensor().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetBatteryElectricalSensor().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetGridPowerSource().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetBatteryPowerSource().GetEndpointId(), kInvalidEndpointId);
}

Protocols::InteractionModel::Status
StatusFromInvokeResult(const Testing::ClusterTester::InvokeResult<DataModel::NullObjectType> & result)
{
    EXPECT_TRUE(result.GetStatusCode().has_value());
    return result.GetStatusCode().has_value() ? result.GetStatusCode()->GetStatus() : Protocols::InteractionModel::Status::Failure;
}

TEST_F(BatteryStorageTestFixture, UnregistersCleanlyAfterProviderStartup)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RegistersRequiredTopologyAndDeviceTypes)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    const EndpointId parentEndpoint        = device.GetEndpointId();
    const EndpointId energyEndpoint        = device.GetEnergyManagement().GetEndpointId();
    const EndpointId gridSensorEndpoint    = device.GetGridElectricalSensor().GetEndpointId();
    const EndpointId batterySensorEndpoint = device.GetBatteryElectricalSensor().GetEndpointId();
    const EndpointId gridSourceEndpoint    = device.GetGridPowerSource().GetEndpointId();
    const EndpointId batterySourceEndpoint = device.GetBatteryPowerSource().GetEndpointId();
    const EndpointId childEndpoints[]      = { energyEndpoint, gridSensorEndpoint, batterySensorEndpoint, gridSourceEndpoint,
                                               batterySourceEndpoint };

    const auto endpoints = GetEndpoints(mProvider);
    ASSERT_EQ(endpoints.size(), 6u);
    const auto parent = std::find_if(endpoints.begin(), endpoints.end(),
                                     [parentEndpoint](const auto & entry) { return entry.id == parentEndpoint; });
    ASSERT_NE(parent, endpoints.end());
    EXPECT_EQ(parent->parentId, kInvalidEndpointId);
    EXPECT_EQ(parent->compositionPattern, DataModel::EndpointCompositionPattern::kTree);

    for (EndpointId childEndpoint : childEndpoints)
    {
        const auto child = std::find_if(endpoints.begin(), endpoints.end(),
                                        [childEndpoint](const auto & entry) { return entry.id == childEndpoint; });
        ASSERT_NE(child, endpoints.end());
        EXPECT_EQ(child->parentId, parentEndpoint);
        EXPECT_EQ(child->compositionPattern, DataModel::EndpointCompositionPattern::kTree);
    }

    ProviderAttributeReader reader(mProvider);
    Clusters::Descriptor::Attributes::PartsList::TypeInfo::DecodableType partsList;
    ASSERT_EQ(reader.Read(parentEndpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id, partsList),
              CHIP_NO_ERROR);
    std::vector<EndpointId> actualParts;
    auto partsIterator = partsList.begin();
    while (partsIterator.Next())
    {
        actualParts.push_back(partsIterator.GetValue());
    }
    EXPECT_EQ(partsIterator.GetStatus(), CHIP_NO_ERROR);
    std::vector<EndpointId> expectedParts(std::begin(childEndpoints), std::end(childEndpoints));
    std::sort(actualParts.begin(), actualParts.end());
    std::sort(expectedParts.begin(), expectedParts.end());
    EXPECT_EQ(actualParts, expectedParts);

    ExpectSingleDeviceType(mProvider, parentEndpoint, Device::Type::kBatteryStorage);
    ExpectSingleDeviceType(mProvider, energyEndpoint, Device::Type::kDeviceEnergyManagement);
    ExpectSingleDeviceType(mProvider, gridSensorEndpoint, Device::Type::kElectricalSensor);
    ExpectSingleDeviceType(mProvider, batterySensorEndpoint, Device::Type::kElectricalSensor);
    ExpectSingleDeviceType(mProvider, gridSourceEndpoint, Device::Type::kPowerSource);
    ExpectSingleDeviceType(mProvider, batterySourceEndpoint, Device::Type::kPowerSource);

    ExpectServerClusters(mProvider, parentEndpoint, { Clusters::Descriptor::Id, Clusters::Identify::Id });
    ExpectServerClusters(mProvider, energyEndpoint,
                         { Clusters::Descriptor::Id, Clusters::Identify::Id, Clusters::DeviceEnergyManagement::Id });
    for (EndpointId sensorEndpoint : { gridSensorEndpoint, batterySensorEndpoint })
    {
        ExpectServerClusters(mProvider, sensorEndpoint,
                             { Clusters::Descriptor::Id, Clusters::ElectricalEnergyMeasurement::Id,
                               Clusters::ElectricalPowerMeasurement::Id, Clusters::PowerTopology::Id });
    }
    for (EndpointId sourceEndpoint : { gridSourceEndpoint, batterySourceEndpoint })
    {
        ExpectServerClusters(mProvider, sourceEndpoint, { Clusters::Descriptor::Id, Clusters::PowerSource::Id });
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RegistersRequiredSemanticTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ProviderAttributeReader reader(mProvider);
    ExpectSingleSemanticTag(reader, device.GetGridElectricalSensor().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x01);
    ExpectSingleSemanticTag(reader, device.GetGridPowerSource().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x01);
    ExpectSingleSemanticTag(reader, device.GetBatteryElectricalSensor().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x03);
    ExpectSingleSemanticTag(reader, device.GetBatteryPowerSource().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x03);

    ExpectAttributes(mProvider, ConcreteClusterPath(device.GetEnergyManagement().GetEndpointId(), Clusters::Descriptor::Id), {},
                     { Clusters::Descriptor::Attributes::TagList::Id });

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RegistersRequiredFeaturesAndAttributes)
{
    using DemFeature = Clusters::DeviceEnergyManagement::Feature;
    using EemFeature = Clusters::ElectricalEnergyMeasurement::Feature;
    using EpmFeature = Clusters::ElectricalPowerMeasurement::Feature;
    using PsFeature  = Clusters::PowerSource::Feature;

    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ProviderAttributeReader reader(mProvider);
    uint32_t featureMap             = 0;
    const EndpointId energyEndpoint = device.GetEnergyManagement().GetEndpointId();
    ASSERT_EQ(reader.Read(energyEndpoint, Clusters::DeviceEnergyManagement::Id,
                          Clusters::DeviceEnergyManagement::Attributes::FeatureMap::Id, featureMap),
              CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitMask<DemFeature>(DemFeature::kPowerAdjustment).Raw());
    EXPECT_EQ(device.GetEnergyManagement().GetESAType(), Clusters::DeviceEnergyManagement::ESATypeEnum::kBatteryStorage);
    EXPECT_TRUE(device.GetEnergyManagement().GetESACanGenerate());

    struct SensorExpectation
    {
        SimulatedElectricalSensor & sensor;
        EpmFeature powerFeature;
        bool requiresImportedEnergy;
    };
    SensorExpectation sensorExpectations[] = {
        { device.GetGridElectricalSensor(), EpmFeature::kAlternatingCurrent, true },
        { device.GetBatteryElectricalSensor(), EpmFeature::kDirectCurrent, false },
    };
    for (const auto & expectation : sensorExpectations)
    {
        const EndpointId endpoint = expectation.sensor.GetEndpointId();
        ASSERT_EQ(reader.Read(endpoint, Clusters::ElectricalPowerMeasurement::Id,
                              Clusters::ElectricalPowerMeasurement::Attributes::FeatureMap::Id, featureMap),
                  CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, BitMask<EpmFeature>(expectation.powerFeature).Raw());
        ExpectAttributes(mProvider, ConcreteClusterPath(endpoint, Clusters::ElectricalPowerMeasurement::Id),
                         { Clusters::ElectricalPowerMeasurement::Attributes::Voltage::Id,
                           Clusters::ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id });

        ASSERT_EQ(reader.Read(endpoint, Clusters::ElectricalEnergyMeasurement::Id,
                              Clusters::ElectricalEnergyMeasurement::Attributes::FeatureMap::Id, featureMap),
                  CHIP_NO_ERROR);
        const BitMask<EemFeature> eemFeatures(featureMap);
        EXPECT_TRUE(eemFeatures.Has(EemFeature::kExportedEnergy));
        if (expectation.requiresImportedEnergy)
        {
            EXPECT_TRUE(eemFeatures.Has(EemFeature::kImportedEnergy));
        }
    }

    const EndpointId gridSourceEndpoint = device.GetGridPowerSource().GetEndpointId();
    ASSERT_EQ(
        reader.Read(gridSourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::FeatureMap::Id, featureMap),
        CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitMask<PsFeature>(PsFeature::kWired).Raw());

    const EndpointId batterySourceEndpoint = device.GetBatteryPowerSource().GetEndpointId();
    ASSERT_EQ(reader.Read(batterySourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::FeatureMap::Id,
                          featureMap),
              CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitMask<PsFeature>(PsFeature::kBattery, PsFeature::kRechargeable).Raw());
    ExpectAttributes(
        mProvider, ConcreteClusterPath(batterySourceEndpoint, Clusters::PowerSource::Id),
        { Clusters::PowerSource::Attributes::BatVoltage::Id, Clusters::PowerSource::Attributes::BatPercentRemaining::Id,
          Clusters::PowerSource::Attributes::BatTimeRemaining::Id, Clusters::PowerSource::Attributes::ActiveBatFaults::Id,
          Clusters::PowerSource::Attributes::BatCapacity::Id, Clusters::PowerSource::Attributes::BatTimeToFullCharge::Id,
          Clusters::PowerSource::Attributes::BatChargingCurrent::Id,
          Clusters::PowerSource::Attributes::ActiveBatChargeFaults::Id });
    uint32_t batteryCapacity = 0;
    ASSERT_EQ(reader.Read(batterySourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::BatCapacity::Id,
                          batteryCapacity),
              CHIP_NO_ERROR);
    EXPECT_EQ(batteryCapacity, 2'000u);

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RollsBackPartialRegistrationFailures)
{
    constexpr size_t kEndpointCount = 6;

    for (size_t failureIndex = 0; failureIndex < kEndpointCount; ++failureIndex)
    {
        FailingEndpointIdAllocator allocator(failureIndex);
        SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

        EXPECT_NE(device.Register(allocator, mProvider), CHIP_NO_ERROR);
        ExpectAllEndpointIdsInvalid(device);
        ExpectNoRegisteredEndpoints(mProvider);
        EXPECT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(EventTriggerForEndpoint(kFakeReadingsLoadStart, 1)),
                  CHIP_ERROR_INVALID_ARGUMENT);
    }
}

TEST_F(BatteryStorageTestFixture, CanUnregisterAndRegisterAgain)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);
    const EndpointId firstParentEndpoint = device.GetEndpointId();

    device.Unregister(mProvider);
    ExpectAllEndpointIdsInvalid(device);
    ExpectNoRegisteredEndpoints(mProvider);

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    EXPECT_NE(device.GetEndpointId(), firstParentEndpoint);
    EXPECT_EQ(GetEndpoints(mProvider).size(), 6u);

    SimulatedElectricalSensor & batterySensor = device.GetBatteryElectricalSensor();
    ASSERT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(
                  EventTriggerForEndpoint(kFakeReadingsGenStart, batterySensor.GetEndpointId())),
              CHIP_NO_ERROR);
    EXPECT_LT(batterySensor.GetActivePower().Value(), 0);

    device.Unregister(mProvider);
    ExpectAllEndpointIdsInvalid(device);
    ExpectNoRegisteredEndpoints(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RejectsDuplicateRegistration)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);
    const EndpointId parentEndpoint = device.GetEndpointId();
    const auto endpointsBefore      = GetEndpoints(mProvider);

    ConsecutiveEndpointIdAllocator duplicateAllocator(50);
    EXPECT_EQ(device.Register(duplicateAllocator, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(duplicateAllocator.Allocate(), 50);
    EXPECT_EQ(device.GetEndpointId(), parentEndpoint);
    EXPECT_EQ(GetEndpoints(mProvider).size(), endpointsBefore.size());

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, HandlesPowerAdjustmentCommands)
{
    using namespace Clusters::DeviceEnergyManagement;

    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    EnergyManagement & energyManagement = device.GetEnergyManagement();
    Testing::ClusterTester tester(energyManagement.EnergyManagementCluster());

    ASSERT_FALSE(energyManagement.GetPowerAdjustmentCapability().IsNull());
    EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);

    Commands::PowerAdjustRequest::Type powerAdjustRequest;
    powerAdjustRequest.power    = 500;
    powerAdjustRequest.duration = 60;

    struct PowerAdjustmentCase
    {
        AdjustmentCauseEnum requestCause;
        PowerAdjustReasonEnum expectedReason;
    };
    constexpr PowerAdjustmentCase kPowerAdjustmentCases[] = {
        { AdjustmentCauseEnum::kLocalOptimization, PowerAdjustReasonEnum::kLocalOptimizationAdjustment },
        { AdjustmentCauseEnum::kGridOptimization, PowerAdjustReasonEnum::kGridOptimizationAdjustment },
    };

    for (const auto & testCase : kPowerAdjustmentCases)
    {
        powerAdjustRequest.cause = testCase.requestCause;
        EXPECT_TRUE(tester.Invoke(Commands::PowerAdjustRequest::Id, powerAdjustRequest).IsSuccess());
        EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kPowerAdjustActive);
        EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, testCase.expectedReason);

        Commands::CancelPowerAdjustRequest::Type cancelRequest;
        EXPECT_TRUE(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelRequest).IsSuccess());
        EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kOnline);
        EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);
    }

    const EndpointId endpoint = energyManagement.GetEndpointId();
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(endpoint, Id, Attributes::ESAState::Id)));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(endpoint, Id, Attributes::PowerAdjustmentCapability::Id)));

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RejectsInvalidPowerAdjustmentCommands)
{
    using namespace Clusters::DeviceEnergyManagement;
    using Protocols::InteractionModel::Status;

    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    EnergyManagement & energyManagement = device.GetEnergyManagement();
    Testing::ClusterTester tester(energyManagement.EnergyManagementCluster());
    Commands::CancelPowerAdjustRequest::Type cancelRequest;

    EXPECT_EQ(StatusFromInvokeResult(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelRequest)), Status::InvalidInState);
    EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);

    Commands::PowerAdjustRequest::Type powerAdjustRequest;
    powerAdjustRequest.cause    = AdjustmentCauseEnum::kLocalOptimization;
    powerAdjustRequest.power    = 1001;
    powerAdjustRequest.duration = 60;
    EXPECT_EQ(StatusFromInvokeResult(tester.Invoke(Commands::PowerAdjustRequest::Id, powerAdjustRequest)), Status::ConstraintError);

    powerAdjustRequest.power    = 500;
    powerAdjustRequest.duration = 3601;
    EXPECT_EQ(StatusFromInvokeResult(tester.Invoke(Commands::PowerAdjustRequest::Id, powerAdjustRequest)), Status::ConstraintError);
    EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);

    powerAdjustRequest.duration = 60;
    ASSERT_TRUE(tester.Invoke(Commands::PowerAdjustRequest::Id, powerAdjustRequest).IsSuccess());
    ASSERT_TRUE(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelRequest).IsSuccess());
    EXPECT_EQ(StatusFromInvokeResult(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelRequest)), Status::InvalidInState);
    EXPECT_EQ(energyManagement.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(energyManagement.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(BatteryStorageTestFixture, RoutesEnergyReportingTriggersToTargetSensor)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedBatteryStorage device({ .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate });

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    SimulatedElectricalSensor & gridSensor    = device.GetGridElectricalSensor();
    SimulatedElectricalSensor & batterySensor = device.GetBatteryElectricalSensor();

    ASSERT_EQ(
        mTestEventTriggerDelegate.HandleEventTriggers(EventTriggerForEndpoint(kFakeReadingsLoadStart, gridSensor.GetEndpointId())),
        CHIP_NO_ERROR);
    EXPECT_GT(gridSensor.GetActivePower().Value(), 0);
    EXPECT_EQ(batterySensor.GetActivePower().Value(), 0);

    ASSERT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(
                  EventTriggerForEndpoint(kFakeReadingsGenStart, batterySensor.GetEndpointId())),
              CHIP_NO_ERROR);
    EXPECT_GT(gridSensor.GetActivePower().Value(), 0);
    EXPECT_LT(batterySensor.GetActivePower().Value(), 0);

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
