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
#include <device/types/solar-power/impl/SimulatedSolarPower.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerFourParallelPanels.h>
#include <device/types/solar-power/impl/SimulatedSolarPowerTwoStringsFourPanels.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/DeviceEnergyManagement/ClusterId.h>
#include <clusters/ElectricalEnergyMeasurement/AttributeIds.h>
#include <clusters/ElectricalEnergyMeasurement/ClusterId.h>
#include <clusters/ElectricalPowerMeasurement/AttributeIds.h>
#include <clusters/ElectricalPowerMeasurement/ClusterId.h>
#include <clusters/Identify/ClusterId.h>
#include <clusters/PowerSource/AttributeIds.h>
#include <clusters/PowerSource/ClusterId.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <clusters/TemperatureMeasurement/ClusterId.h>
#include <clusters/UserLabel/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInfoProvider.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

class MockDeviceInfoProvider : public DeviceLayer::DeviceInfoProvider
{
public:
    DeviceLayer::FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override { return nullptr; }
    DeviceLayer::UserLabelIterator * IterateUserLabel(EndpointId endpoint) override { return nullptr; }
    DeviceLayer::SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override { return nullptr; }
    DeviceLayer::SupportedLocalesIterator * IterateSupportedLocales() override { return nullptr; }

protected:
    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override
    {
        val = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_NO_ERROR; }
};

MockDeviceInfoProvider sDeviceInfoProvider;

class SolarPowerTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        DeviceLayer::SetDeviceInfoProvider(&sDeviceInfoProvider);
    }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    Testing::ScopedAttributeChangeListenerRegistration mChangeListenerRegistration{ mProvider, mContext };
    TimerDelegateMock mTimerDelegate;
    SimpleTestEventTriggerDelegate mTestEventTriggerDelegate;

    SimulatedSolarPower::Context SolarContext()
    {
        return {
            .timerDelegate            = mTimerDelegate,
            .testEventTriggerDelegate = mTestEventTriggerDelegate,
            .fabricTable              = mContext.GetFabricTable(),
        };
    }
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
                      std::initializer_list<AttributeId> requiredAttributes)
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

void ExpectSemanticTags(ProviderAttributeReader & reader, EndpointId endpoint,
                        std::initializer_list<std::pair<uint16_t, uint8_t>> expectedTags)
{
    Clusters::Descriptor::Attributes::TagList::TypeInfo::DecodableType tagList;
    ASSERT_EQ(reader.Read(endpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::TagList::Id, tagList),
              CHIP_NO_ERROR);

    size_t count  = 0;
    auto iterator = tagList.begin();
    for (; iterator.Next(); count++)
    {
        ASSERT_LT(count, expectedTags.size());
        auto it = expectedTags.begin();
        std::advance(it, count);
        EXPECT_EQ(iterator.GetValue().namespaceID, it->first);
        EXPECT_EQ(iterator.GetValue().tag, it->second);
    }
    EXPECT_EQ(count, expectedTags.size());
    EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
}

} // namespace

TEST_F(SolarPowerTestFixture, UnregistersCleanlyAfterProviderStartup)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(SolarContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, RegistersRequiredTopologyAndDeviceTypes)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(SolarContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    const EndpointId parentEndpoint      = device.GetEndpointId();
    const EndpointId sensorEndpoint      = device.GetOutputElectricalSensor().GetEndpointId();
    const EndpointId powerSourceEndpoint = device.GetGridPowerSource().GetEndpointId();
    const EndpointId demEndpoint         = device.GetEnergyManagement().GetEndpointId();
    const EndpointId temperatureEndpoint = device.GetInverterTemperatureSensor().GetEndpointId();
    const EndpointId childEndpoints[]    = { sensorEndpoint, powerSourceEndpoint, demEndpoint, temperatureEndpoint };

    const auto endpoints = GetEndpoints(mProvider);
    ASSERT_EQ(endpoints.size(), 5u);

    for (EndpointId childEndpoint : childEndpoints)
    {
        const auto child = std::find_if(endpoints.begin(), endpoints.end(),
                                        [childEndpoint](const auto & entry) { return entry.id == childEndpoint; });
        ASSERT_NE(child, endpoints.end());
        EXPECT_EQ(child->parentId, parentEndpoint);
        EXPECT_EQ(child->compositionPattern, DataModel::EndpointCompositionPattern::kTree);
    }

    ExpectSingleDeviceType(mProvider, parentEndpoint, Device::Type::kSolarPower);
    ExpectSingleDeviceType(mProvider, sensorEndpoint, Device::Type::kElectricalSensor);
    ExpectSingleDeviceType(mProvider, powerSourceEndpoint, Device::Type::kPowerSource);
    ExpectSingleDeviceType(mProvider, demEndpoint, Device::Type::kDeviceEnergyManagement);
    ExpectSingleDeviceType(mProvider, temperatureEndpoint, Device::Type::kTemperatureSensor);

    ExpectServerClusters(mProvider, parentEndpoint, { Clusters::Descriptor::Id, Clusters::Identify::Id });
    ExpectServerClusters(mProvider, sensorEndpoint,
                         { Clusters::Descriptor::Id, Clusters::ElectricalEnergyMeasurement::Id,
                           Clusters::ElectricalPowerMeasurement::Id, Clusters::PowerTopology::Id });
    ExpectServerClusters(mProvider, powerSourceEndpoint, { Clusters::Descriptor::Id, Clusters::PowerSource::Id });

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, RegistersGridSemanticTagsOnComposedEndpoints)
{
    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(SolarContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ProviderAttributeReader reader(mProvider);
    ExpectSingleSemanticTag(reader, device.GetOutputElectricalSensor().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x01);
    ExpectSingleSemanticTag(reader, device.GetGridPowerSource().GetEndpointId(), CommonNamespace::kPowerSourceId, 0x01);

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, RegistersRequiredMeasurementFeaturesAndAttributes)
{
    using EemFeature = Clusters::ElectricalEnergyMeasurement::Feature;
    using EpmFeature = Clusters::ElectricalPowerMeasurement::Feature;
    using PsFeature  = Clusters::PowerSource::Feature;

    ConsecutiveEndpointIdAllocator allocator;
    SimulatedSolarPower device(SolarContext());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    ProviderAttributeReader reader(mProvider);
    uint32_t featureMap = 0;

    const EndpointId sensorEndpoint = device.GetOutputElectricalSensor().GetEndpointId();
    ASSERT_EQ(reader.Read(sensorEndpoint, Clusters::ElectricalPowerMeasurement::Id,
                          Clusters::ElectricalPowerMeasurement::Attributes::FeatureMap::Id, featureMap),
              CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitMask<EpmFeature>(EpmFeature::kAlternatingCurrent).Raw());
    ExpectAttributes(mProvider, ConcreteClusterPath(sensorEndpoint, Clusters::ElectricalPowerMeasurement::Id),
                     { Clusters::ElectricalPowerMeasurement::Attributes::Voltage::Id,
                       Clusters::ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id });

    ASSERT_EQ(reader.Read(sensorEndpoint, Clusters::ElectricalEnergyMeasurement::Id,
                          Clusters::ElectricalEnergyMeasurement::Attributes::FeatureMap::Id, featureMap),
              CHIP_NO_ERROR);
    EXPECT_TRUE(BitMask<EemFeature>(featureMap).Has(EemFeature::kExportedEnergy));

    const EndpointId powerSourceEndpoint = device.GetGridPowerSource().GetEndpointId();
    ASSERT_EQ(
        reader.Read(powerSourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::FeatureMap::Id, featureMap),
        CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitMask<PsFeature>(PsFeature::kWired).Raw());

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, FourParallelPanelsRegistersPanelSensorsWithUserLabel)
{
    SimulatedSolarPowerFourParallelPanels device(SolarContext());

    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    EXPECT_EQ(GetEndpoints(mProvider).size(), 9u);

    ProviderAttributeReader reader(mProvider);
    for (size_t panelIndex = 0; panelIndex < SimulatedSolarPowerFourParallelPanels::kParallelPanelCount; panelIndex++)
    {
        const EndpointId panelEndpoint = device.GetParallelPanelSensor(panelIndex).GetEndpointId();
        ExpectSingleDeviceType(mProvider, panelEndpoint, Device::Type::kElectricalSensor);
        ExpectServerClusters(mProvider, panelEndpoint,
                             { Clusters::Descriptor::Id, Clusters::UserLabel::Id, Clusters::ElectricalEnergyMeasurement::Id,
                               Clusters::ElectricalPowerMeasurement::Id, Clusters::PowerTopology::Id });
        ExpectSingleSemanticTag(reader, panelEndpoint, CommonNamespace::kNumberId, static_cast<uint8_t>(panelIndex + 1));
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SolarPowerTestFixture, TwoStringsFourPanelsRegistersPerPanelEndpointsWithStringAndPositionTags)
{
    SimulatedSolarPowerTwoStringsFourPanels device(SolarContext());

    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    EXPECT_EQ(GetEndpoints(mProvider).size(), 9u);

    ProviderAttributeReader reader(mProvider);
    struct PanelExpectation
    {
        uint8_t stringNumber;
        Clusters::Globals::PositionTag panelPosition;
    };
    const PanelExpectation expectations[] = {
        { 1, Clusters::Globals::PositionTag::kLeft },
        { 1, Clusters::Globals::PositionTag::kRight },
        { 2, Clusters::Globals::PositionTag::kLeft },
        { 2, Clusters::Globals::PositionTag::kRight },
    };

    for (size_t index = 0; index < SimulatedSolarPowerTwoStringsFourPanels::kPanelEndpointCount; index++)
    {
        const EndpointId panelEndpoint = device.GetPanelSensor(index).GetEndpointId();
        ExpectServerClusters(mProvider, panelEndpoint,
                             { Clusters::Descriptor::Id, Clusters::UserLabel::Id, Clusters::ElectricalEnergyMeasurement::Id,
                               Clusters::ElectricalPowerMeasurement::Id, Clusters::PowerTopology::Id });
        ExpectSemanticTags(reader, panelEndpoint,
                           { { CommonNamespace::kNumberId, expectations[index].stringNumber },
                             { CommonNamespace::kPositionId, static_cast<uint8_t>(expectations[index].panelPosition) } });
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}
