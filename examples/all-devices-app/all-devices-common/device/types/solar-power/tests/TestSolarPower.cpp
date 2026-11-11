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
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/solar-power/PanelElectricalSensor.h>
#include <device/types/solar-power/impl/SimulatedSolarPower.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerFourParallelPanels.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerThreePhase.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerTwoStrings.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerTwoStringsFourPanels.h>
#include <providers/AllDevicesExampleDeviceInfoProviderImpl.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/DeviceEnergyManagement/ClusterId.h>
#include <clusters/ElectricalEnergyMeasurement/ClusterId.h>
#include <clusters/ElectricalPowerMeasurement/ClusterId.h>
#include <clusters/Identify/ClusterId.h>
#include <clusters/PowerSource/ClusterId.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <clusters/TemperatureMeasurement/ClusterId.h>
#include <clusters/UserLabel/ClusterId.h>
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

using NS = Clusters::Globals::SemanticTagNamespaceId;

constexpr uint8_t kTagAcPhase1 = 0x02;
constexpr uint8_t kTagAcPhase2 = 0x03;
constexpr uint8_t kTagAcPhase3 = 0x04;

class SolarPowerTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { DeviceLayer::SetDeviceInfoProvider(&mDeviceInfoProvider); }
    void TearDown() override { DeviceLayer::SetDeviceInfoProvider(nullptr); }

protected:
    SolarPower::Context MakeContext()
    {
        return {
            .timerDelegate            = mTimerDelegate,
            .testEventTriggerDelegate = mTestEventTriggerDelegate,
            .fabricTable              = mContext.GetFabricTable(),
        };
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    Testing::ScopedAttributeChangeListenerRegistration mChangeListenerRegistration{ mProvider, mContext };
    TimerDelegateMock mTimerDelegate;
    SimpleTestEventTriggerDelegate mTestEventTriggerDelegate;
    DeviceLayer::AllDevicesExampleDeviceInfoProviderImpl mDeviceInfoProvider;
};

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

void ExpectSemanticTags(ProviderAttributeReader & reader, EndpointId endpoint,
                        std::initializer_list<std::pair<uint16_t, uint8_t>> expectedTags)
{
    Clusters::Descriptor::Attributes::TagList::TypeInfo::DecodableType tagList;
    ASSERT_EQ(reader.Read(endpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::TagList::Id, tagList),
              CHIP_NO_ERROR);

    std::vector<std::pair<uint16_t, uint8_t>> actual;
    auto iterator = tagList.begin();
    while (iterator.Next())
    {
        const auto & tag = iterator.GetValue();
        actual.emplace_back(tag.namespaceID, tag.tag);
    }
    ASSERT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);

    std::vector<std::pair<uint16_t, uint8_t>> expected(expectedTags);
    EXPECT_EQ(actual, expected);
}

void ExpectNoRegisteredEndpoints(CodeDrivenDataModelProvider & provider)
{
    EXPECT_TRUE(GetEndpoints(provider).empty());
}

void ExpectBaselineEndpointIdsInvalid(SimulatedSolarPower & device)
{
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetEnergyManagement().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetInverterTemperatureSensor().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetOutputElectricalSensor().GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetGridPowerSource().GetEndpointId(), kInvalidEndpointId);
}

void ExpectPanelEndpointIdsInvalid(SimulatedElectricalSensor & sensor)
{
    EXPECT_EQ(sensor.GetEndpointId(), kInvalidEndpointId);
}

void ExpectParentChildTree(CodeDrivenDataModelProvider & provider, EndpointId parentEndpoint,
                           std::initializer_list<EndpointId> childEndpoints)
{
    const auto endpoints = GetEndpoints(provider);
    const auto parent    = std::find_if(endpoints.begin(), endpoints.end(),
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
}

void ExpectPanelSensorClusters(CodeDrivenDataModelProvider & provider, EndpointId endpoint)
{
    ExpectServerClusters(provider, endpoint,
                         { Clusters::Descriptor::Id, Clusters::ElectricalEnergyMeasurement::Id,
                           Clusters::ElectricalPowerMeasurement::Id, Clusters::PowerTopology::Id, Clusters::UserLabel::Id });
}

template <typename MakeDeviceFn, typename ExpectInvalidFn>
void TestRollBackForEndpointCount(CodeDrivenDataModelProvider & provider, size_t endpointCount, MakeDeviceFn makeDevice,
                                  ExpectInvalidFn expectInvalid)
{
    for (size_t failureIndex = 0; failureIndex < endpointCount; ++failureIndex)
    {
        FailingEndpointIdAllocator allocator(failureIndex);
        auto device = makeDevice();

        EXPECT_NE(device->Register(allocator, provider), CHIP_NO_ERROR);
        expectInvalid(*device);
        ExpectNoRegisteredEndpoints(provider);
    }
}

template <typename MakeDeviceFn, typename ExpectInvalidFn>
void TestUnregisterAndRegisterAgain(CodeDrivenDataModelProvider & provider, Testing::TestServerClusterContext & context,
                                    MakeDeviceFn makeDevice, size_t endpointCount, ExpectInvalidFn expectInvalid)
{
    ConsecutiveEndpointIdAllocator allocator;
    auto device = makeDevice();

    ASSERT_EQ(device->Register(allocator, provider), CHIP_NO_ERROR);
    ASSERT_EQ(provider.Startup(context.ImContext()), CHIP_NO_ERROR);
    const EndpointId firstParentEndpoint = device->GetEndpointId();

    device->Unregister(provider);
    expectInvalid(*device);
    ExpectNoRegisteredEndpoints(provider);

    ASSERT_EQ(device->Register(allocator, provider), CHIP_NO_ERROR);
    EXPECT_NE(device->GetEndpointId(), firstParentEndpoint);
    EXPECT_EQ(GetEndpoints(provider).size(), endpointCount);

    device->Unregister(provider);
    expectInvalid(*device);
    ExpectNoRegisteredEndpoints(provider);
    EXPECT_EQ(provider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, BaselineRegistersRequiredTopologyAndDeviceTypes)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(MakeContext(), {});

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    const EndpointId parentEndpoint     = device.GetEndpointId();
    const EndpointId energyEndpoint     = device.GetEnergyManagement().GetEndpointId();
    const EndpointId outputEndpoint     = device.GetOutputElectricalSensor().GetEndpointId();
    const EndpointId gridSourceEndpoint = device.GetGridPowerSource().GetEndpointId();
    const EndpointId inverterEndpoint   = device.GetInverterTemperatureSensor().GetEndpointId();

    ASSERT_EQ(GetEndpoints(mProvider).size(), 5u);
    ExpectParentChildTree(mProvider, parentEndpoint, { energyEndpoint, outputEndpoint, gridSourceEndpoint, inverterEndpoint });

    ExpectSingleDeviceType(mProvider, parentEndpoint, Device::Type::kSolarPower);
    ExpectSingleDeviceType(mProvider, energyEndpoint, Device::Type::kDeviceEnergyManagement);
    ExpectSingleDeviceType(mProvider, outputEndpoint, Device::Type::kElectricalSensor);
    ExpectSingleDeviceType(mProvider, gridSourceEndpoint, Device::Type::kPowerSource);
    ExpectSingleDeviceType(mProvider, inverterEndpoint, Device::Type::kTemperatureSensor);

    ExpectServerClusters(mProvider, parentEndpoint, { Clusters::Descriptor::Id, Clusters::Identify::Id });
    ExpectServerClusters(mProvider, energyEndpoint,
                         { Clusters::Descriptor::Id, Clusters::Identify::Id, Clusters::DeviceEnergyManagement::Id });
    ExpectPanelSensorClusters(mProvider, outputEndpoint);
    ExpectServerClusters(mProvider, gridSourceEndpoint, { Clusters::Descriptor::Id, Clusters::PowerSource::Id });
    ExpectServerClusters(mProvider, inverterEndpoint,
                         { Clusters::Descriptor::Id, Clusters::Identify::Id, Clusters::TemperatureMeasurement::Id });

    EXPECT_EQ(device.GetEnergyManagement().GetESAType(), Clusters::DeviceEnergyManagement::ESATypeEnum::kSolarPV);
    EXPECT_TRUE(device.GetEnergyManagement().GetESACanGenerate());

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, BaselineRegistersRequiredSemanticTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(MakeContext(), {});

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ProviderAttributeReader reader(mProvider);
    const uint16_t powerSourceNs = to_underlying(NS::kPowerSource);
    const uint16_t locationNs    = to_underlying(NS::kLocation);
    ExpectSemanticTags(reader, device.GetOutputElectricalSensor().GetEndpointId(), { { powerSourceNs, 0x01 } });
    ExpectSemanticTags(reader, device.GetGridPowerSource().GetEndpointId(), { { powerSourceNs, 0x01 } });
    ExpectSemanticTags(reader, device.GetInverterTemperatureSensor().GetEndpointId(),
                       { { locationNs, to_underlying(Clusters::Globals::LocationTag::kOutdoor) } });

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, FourParallelPanelsRegistersTopologyAndTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPowerFourParallelPanels device(MakeContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ASSERT_EQ(GetEndpoints(mProvider).size(), 9u);

    ProviderAttributeReader reader(mProvider);
    const uint16_t numberNs = to_underlying(NS::kNumber);
    for (size_t index = 0; index < SimulatedSolarPowerFourParallelPanels::kParallelPanelCount; ++index)
    {
        PanelElectricalSensor & panel = device.GetParallelPanelSensor(index);
        ExpectPanelSensorClusters(mProvider, panel.GetEndpointId());
        ExpectSemanticTags(reader, panel.GetEndpointId(), { { numberNs, static_cast<uint8_t>(index + 1) } });
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, TwoStringsRegistersTopologyAndTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPowerTwoStrings device(MakeContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ASSERT_EQ(GetEndpoints(mProvider).size(), 7u);

    ProviderAttributeReader reader(mProvider);
    const uint16_t numberNs = to_underlying(NS::kNumber);
    ExpectSemanticTags(reader, device.GetStringSensor(0).GetEndpointId(), { { numberNs, 1 } });
    ExpectSemanticTags(reader, device.GetStringSensor(1).GetEndpointId(), { { numberNs, 2 } });

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, TwoStringsFourPanelsRegistersTopologyAndTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPowerTwoStringsFourPanels device(MakeContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ASSERT_EQ(GetEndpoints(mProvider).size(), 9u);

    ProviderAttributeReader reader(mProvider);
    const uint16_t numberNs   = to_underlying(NS::kNumber);
    const uint16_t positionNs = to_underlying(NS::kPosition);
    ExpectSemanticTags(reader, device.GetPanelSensor(0).GetEndpointId(),
                       { { numberNs, 1 }, { positionNs, to_underlying(Clusters::Globals::PositionTag::kLeft) } });
    ExpectSemanticTags(reader, device.GetPanelSensor(1).GetEndpointId(),
                       { { numberNs, 1 }, { positionNs, to_underlying(Clusters::Globals::PositionTag::kRight) } });
    ExpectSemanticTags(reader, device.GetPanelSensor(2).GetEndpointId(),
                       { { numberNs, 2 }, { positionNs, to_underlying(Clusters::Globals::PositionTag::kLeft) } });
    ExpectSemanticTags(reader, device.GetPanelSensor(3).GetEndpointId(),
                       { { numberNs, 2 }, { positionNs, to_underlying(Clusters::Globals::PositionTag::kRight) } });

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, ThreePhaseRegistersTopologyAndAcPhaseTags)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPowerThreePhase device(MakeContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ASSERT_EQ(GetEndpoints(mProvider).size(), 8u);

    ProviderAttributeReader reader(mProvider);
    const uint16_t electricalMeasurementNs = to_underlying(NS::kElectricalMeasurement);
    ExpectSemanticTags(reader, device.GetPhaseSensor(0).GetEndpointId(), { { electricalMeasurementNs, kTagAcPhase1 } });
    ExpectSemanticTags(reader, device.GetPhaseSensor(1).GetEndpointId(), { { electricalMeasurementNs, kTagAcPhase2 } });
    ExpectSemanticTags(reader, device.GetPhaseSensor(2).GetEndpointId(), { { electricalMeasurementNs, kTagAcPhase3 } });

    for (size_t index = 0; index < 3; ++index)
    {
        ExpectPanelSensorClusters(mProvider, device.GetPhaseSensor(index).GetEndpointId());
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, BaselineRollsBackPartialRegistrationFailures)
{
    TestRollBackForEndpointCount(
        mProvider, 5u, [this]() { return std::make_unique<SimulatedSolarPower>(MakeContext(), SolarPower::Parameters{}); },
        [](SimulatedSolarPower & device) { ExpectBaselineEndpointIdsInvalid(device); });
}

TEST_F(SolarPowerTestFixture, FourParallelPanelsRollsBackPartialRegistrationFailures)
{
    TestRollBackForEndpointCount(
        mProvider, 9u, [this]() { return std::make_unique<SimulatedSolarPowerFourParallelPanels>(MakeContext()); },
        [](SimulatedSolarPowerFourParallelPanels & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < SimulatedSolarPowerFourParallelPanels::kParallelPanelCount; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetParallelPanelSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, TwoStringsRollsBackPartialRegistrationFailures)
{
    TestRollBackForEndpointCount(
        mProvider, 7u, [this]() { return std::make_unique<SimulatedSolarPowerTwoStrings>(MakeContext()); },
        [](SimulatedSolarPowerTwoStrings & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            ExpectPanelEndpointIdsInvalid(device.GetStringSensor(0));
            ExpectPanelEndpointIdsInvalid(device.GetStringSensor(1));
        });
}

TEST_F(SolarPowerTestFixture, TwoStringsFourPanelsRollsBackPartialRegistrationFailures)
{
    TestRollBackForEndpointCount(
        mProvider, 9u, [this]() { return std::make_unique<SimulatedSolarPowerTwoStringsFourPanels>(MakeContext()); },
        [](SimulatedSolarPowerTwoStringsFourPanels & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < SimulatedSolarPowerTwoStringsFourPanels::kPanelEndpointCount; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetPanelSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, ThreePhaseRollsBackPartialRegistrationFailures)
{
    TestRollBackForEndpointCount(
        mProvider, 8u, [this]() { return std::make_unique<SimulatedSolarPowerThreePhase>(MakeContext()); },
        [](SimulatedSolarPowerThreePhase & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < 3; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetPhaseSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, BaselineCanUnregisterAndRegisterAgainAfterProviderStartup)
{
    TestUnregisterAndRegisterAgain(
        mProvider, mContext, [this]() { return std::make_unique<SimulatedSolarPower>(MakeContext(), SolarPower::Parameters{}); },
        5u, [](SimulatedSolarPower & device) { ExpectBaselineEndpointIdsInvalid(device); });
}

TEST_F(SolarPowerTestFixture, FourParallelPanelsCanUnregisterAndRegisterAgainAfterProviderStartup)
{
    TestUnregisterAndRegisterAgain(
        mProvider, mContext, [this]() { return std::make_unique<SimulatedSolarPowerFourParallelPanels>(MakeContext()); }, 9u,
        [](SimulatedSolarPowerFourParallelPanels & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < SimulatedSolarPowerFourParallelPanels::kParallelPanelCount; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetParallelPanelSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, TwoStringsCanUnregisterAndRegisterAgainAfterProviderStartup)
{
    TestUnregisterAndRegisterAgain(
        mProvider, mContext, [this]() { return std::make_unique<SimulatedSolarPowerTwoStrings>(MakeContext()); }, 7u,
        [](SimulatedSolarPowerTwoStrings & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            ExpectPanelEndpointIdsInvalid(device.GetStringSensor(0));
            ExpectPanelEndpointIdsInvalid(device.GetStringSensor(1));
        });
}

TEST_F(SolarPowerTestFixture, TwoStringsFourPanelsCanUnregisterAndRegisterAgainAfterProviderStartup)
{
    TestUnregisterAndRegisterAgain(
        mProvider, mContext, [this]() { return std::make_unique<SimulatedSolarPowerTwoStringsFourPanels>(MakeContext()); }, 9u,
        [](SimulatedSolarPowerTwoStringsFourPanels & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < SimulatedSolarPowerTwoStringsFourPanels::kPanelEndpointCount; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetPanelSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, ThreePhaseCanUnregisterAndRegisterAgainAfterProviderStartup)
{
    TestUnregisterAndRegisterAgain(
        mProvider, mContext, [this]() { return std::make_unique<SimulatedSolarPowerThreePhase>(MakeContext()); }, 8u,
        [](SimulatedSolarPowerThreePhase & device) {
            ExpectBaselineEndpointIdsInvalid(device);
            for (size_t index = 0; index < 3; ++index)
            {
                ExpectPanelEndpointIdsInvalid(device.GetPhaseSensor(index));
            }
        });
}

TEST_F(SolarPowerTestFixture, UnregistersCleanlyAfterProviderStartup)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(MakeContext(), {});

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    ExpectBaselineEndpointIdsInvalid(device);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
