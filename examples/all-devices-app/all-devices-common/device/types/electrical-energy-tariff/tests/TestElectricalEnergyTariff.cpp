/*
 *
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

#include <device/types/electrical-energy-tariff/ElectricalEnergyTariff.h>
#include <device/types/electrical-energy-tariff/impl/SimulatedElectricalEnergyTariff.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/ClusterId.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <devices/Types.h>

#include <algorithm>
#include <memory>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::CommodityPrice::Attributes;
using chip::Testing::ClusterTester;

namespace {

constexpr EndpointId kEndpointId = 1;

using SemanticTag = EndpointComposition::SemanticTag;

const SemanticTag kPositionTopTag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kPosition),
    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kTop),
};

const SemanticTag kElectricalEnergyTag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = ElectricalEnergyTariff::kCommodityTariffCommodityNamespaceId,
    .tag         = ElectricalEnergyTariff::kCommodityTariffCommodityElectricalEnergyTag,
};

const SemanticTag kPositionAndElectricalEnergyTags[] = { kPositionTopTag, kElectricalEnergyTag };

const SemanticTag kTagsWithoutElectricalEnergy[] = { kPositionTopTag };

// Same namespace and tag values as ElectricalEnergy, but a manufacturer tag instead of a standard one.
const SemanticTag kManufacturerElectricalEnergyTags[] = {
    {
        .mfgCode     = DataModel::MakeNullable(VendorId::TestVendor1),
        .namespaceID = ElectricalEnergyTariff::kCommodityTariffCommodityNamespaceId,
        .tag         = ElectricalEnergyTariff::kCommodityTariffCommodityElectricalEnergyTag,
    },
};

struct DecodedTag
{
    bool mfgCodeIsNull;
    uint8_t namespaceID;
    uint8_t tag;

    bool operator==(const DecodedTag & other) const
    {
        return mfgCodeIsNull == other.mfgCodeIsNull && namespaceID == other.namespaceID && tag == other.tag;
    }
};

std::vector<DecodedTag> ExpectedTags(Span<const SemanticTag> tags)
{
    std::vector<DecodedTag> result;
    for (const auto & tag : tags)
    {
        result.push_back({ tag.mfgCode.IsNull(), tag.namespaceID, tag.tag });
    }
    return result;
}

EndpointComposition WithTags(Span<const SemanticTag> tags)
{
    return EndpointComposition(kInvalidEndpointId, DataModel::EndpointCompositionPattern::kTree, tags);
}

/// Electrical Energy Tariff whose cluster hooks only count calls, used to test the generic type.
class TestTariff : public ElectricalEnergyTariff
{
public:
    using ElectricalEnergyTariff::ElectricalEnergyTariff;

    bool activeTariff                        = false;
    CHIP_ERROR registrationError             = CHIP_NO_ERROR;
    unsigned registerOptionalClustersCalls   = 0;
    unsigned unregisterOptionalClustersCalls = 0;

protected:
    bool IsActiveTariff() const override { return activeTariff; }

    CHIP_ERROR RegisterOptionalClusters(EndpointId, CodeDrivenDataModelProvider &) override
    {
        registerOptionalClustersCalls++;
        return registrationError;
    }

    void UnregisterOptionalClusters(CodeDrivenDataModelProvider &) override { unregisterOptionalClustersCalls++; }
};

class ElectricalEnergyTariffTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };

    std::vector<ClusterId> GetServerClusterIds(EndpointId endpoint)
    {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        EXPECT_EQ(mProvider.ServerClusters(endpoint, builder), CHIP_NO_ERROR);
        auto clusters = builder.TakeBuffer();
        std::vector<ClusterId> ids;
        for (const auto & entry : clusters)
        {
            ids.push_back(entry.clusterId);
        }
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    void ExpectElectricalEnergyTariffDeviceType(EndpointId endpoint)
    {
        ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
        ASSERT_EQ(mProvider.DeviceTypes(endpoint, builder), CHIP_NO_ERROR);
        auto deviceTypes = builder.TakeBuffer();
        ASSERT_EQ(deviceTypes.size(), 1u);
        EXPECT_EQ(deviceTypes[0].deviceTypeId, Device::Type::kElectricalEnergyTariff.deviceTypeId);
        EXPECT_EQ(deviceTypes[0].deviceTypeRevision, Device::Type::kElectricalEnergyTariff.deviceTypeRevision);
    }

    bool HasNoEndpoints()
    {
        ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
        EXPECT_EQ(mProvider.Endpoints(builder), CHIP_NO_ERROR);
        return builder.TakeBuffer().empty();
    }

    bool HasDescriptorTagListAttribute(EndpointId endpoint)
    {
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
        EXPECT_EQ(mProvider.Attributes(ConcreteClusterPath(endpoint, Clusters::Descriptor::Id), builder), CHIP_NO_ERROR);
        const auto attributes = builder.TakeBuffer();
        return std::any_of(attributes.begin(), attributes.end(),
                           [](const auto & entry) { return entry.attributeId == Clusters::Descriptor::Attributes::TagList::Id; });
    }

    std::vector<DecodedTag> ReadDescriptorTagList(EndpointId endpoint)
    {
        std::vector<DecodedTag> result;

        Testing::ReadOperation operation(endpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::TagList::Id);
        std::unique_ptr<AttributeValueEncoder> encoder = operation.StartEncoding();
        EXPECT_NE(encoder, nullptr);
        VerifyOrReturnValue(encoder != nullptr, result);
        EXPECT_EQ(mProvider.ReadAttribute(operation.GetRequest(), *encoder).GetUnderlyingError(), CHIP_NO_ERROR);
        EXPECT_EQ(operation.FinishEncoding(), CHIP_NO_ERROR);

        std::vector<Testing::DecodedAttributeData> attributeData;
        EXPECT_EQ(operation.GetEncodedIBs().Decode(attributeData), CHIP_NO_ERROR);
        EXPECT_EQ(attributeData.size(), 1u);
        VerifyOrReturnValue(attributeData.size() == 1u, result);

        Clusters::Descriptor::Attributes::TagList::TypeInfo::DecodableType tagList;
        EXPECT_EQ(DataModel::Decode(attributeData[0].dataReader, tagList), CHIP_NO_ERROR);
        auto iterator = tagList.begin();
        while (iterator.Next())
        {
            const auto & tag = iterator.GetValue();
            result.push_back({ tag.mfgCode.IsNull(), tag.namespaceID, tag.tag });
        }
        EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
        return result;
    }

    /// Expects that `device` registered nothing: no endpoint, and no cluster hook was called.
    void ExpectNothingRegistered(const TestTariff & device)
    {
        EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
        EXPECT_EQ(device.registerOptionalClustersCalls, 0u);
        EXPECT_EQ(device.unregisterOptionalClustersCalls, 0u);
        EXPECT_TRUE(HasNoEndpoints());
    }
};

TEST_F(ElectricalEnergyTariffTestFixture, RegistersDescriptorWithDefaultTagList)
{
    TestTariff device;

    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);
    EXPECT_EQ(device.registerOptionalClustersCalls, 1u);

    EXPECT_EQ(GetServerClusterIds(kEndpointId), std::vector<ClusterId>{ Clusters::Descriptor::Id });
    ExpectElectricalEnergyTariffDeviceType(kEndpointId);
    EXPECT_TRUE(HasDescriptorTagListAttribute(kEndpointId));
    EXPECT_EQ(ReadDescriptorTagList(kEndpointId), ExpectedTags(ElectricalEnergyTariff::DefaultTagList()));

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 1u);
    EXPECT_TRUE(HasNoEndpoints());
}

TEST_F(ElectricalEnergyTariffTestFixture, CompositionTagListOverridesDefault)
{
    TestTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider, WithTags(Span(kPositionAndElectricalEnergyTags))), CHIP_NO_ERROR);

    EXPECT_EQ(ReadDescriptorTagList(kEndpointId), ExpectedTags(Span(kPositionAndElectricalEnergyTags)));

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, RejectsDefaultTagListWithoutElectricalEnergy)
{
    TestTariff device{ Span(kTagsWithoutElectricalEnergy) };

    EXPECT_EQ(device.Register(kEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    ExpectNothingRegistered(device);
}

TEST_F(ElectricalEnergyTariffTestFixture, RejectsCompositionTagListWithoutElectricalEnergy)
{
    TestTariff device;

    EXPECT_EQ(device.Register(kEndpointId, mProvider, WithTags(Span(kTagsWithoutElectricalEnergy))), CHIP_ERROR_INVALID_ARGUMENT);
    ExpectNothingRegistered(device);
}

TEST_F(ElectricalEnergyTariffTestFixture, RejectsManufacturerElectricalEnergyTag)
{
    TestTariff device;

    EXPECT_EQ(device.Register(kEndpointId, mProvider, WithTags(Span(kManufacturerElectricalEnergyTags))),
              CHIP_ERROR_INVALID_ARGUMENT);
    ExpectNothingRegistered(device);
}

TEST_F(ElectricalEnergyTariffTestFixture, ActiveTariffRequiresCurrentTag)
{
    TestTariff device;
    device.activeTariff = true;

    EXPECT_EQ(device.Register(kEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    ExpectNothingRegistered(device);

    ASSERT_EQ(device.Register(kEndpointId, mProvider, WithTags(ElectricalEnergyTariff::ActiveTariffTagList())), CHIP_NO_ERROR);
    EXPECT_EQ(ReadDescriptorTagList(kEndpointId), ExpectedTags(ElectricalEnergyTariff::ActiveTariffTagList()));

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, RejectsRepeatedRegistrationAndAllowsReuse)
{
    TestTariff device;
    EXPECT_EQ(device.Register(kInvalidEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);

    ASSERT_EQ(device.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(device.Register(2, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(device.GetEndpointId(), 1);
    device.Unregister(mProvider);

    ASSERT_EQ(device.Register(2, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), 2);
    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, RollsBackWhenOptionalRegistrationFails)
{
    TestTariff device;
    device.registrationError = CHIP_ERROR_INTERNAL;

    EXPECT_EQ(device.Register(kEndpointId, mProvider), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 1u);
    EXPECT_TRUE(HasNoEndpoints());

    device.registrationError = CHIP_NO_ERROR;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);
    device.Unregister(mProvider);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 2u);
}

TEST_F(ElectricalEnergyTariffTestFixture, RegistersWithEndpointIdAllocator)
{
    constexpr EndpointId kFirstAllocatedEndpoint = 5;
    ConsecutiveEndpointIdAllocator allocator(kFirstAllocatedEndpoint);
    TestTariff device;

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kFirstAllocatedEndpoint);

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedDefaultsToActiveTariffTagList)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(ReadDescriptorTagList(kEndpointId), ExpectedTags(ElectricalEnergyTariff::ActiveTariffTagList()));

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedRejectsTagListWithoutCurrent)
{
    SimulatedElectricalEnergyTariff device{ ElectricalEnergyTariff::DefaultTagList() };

    EXPECT_EQ(device.Register(kEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(HasNoEndpoints());
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedRegistersCommodityPrice)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);

    std::vector<ClusterId> expectedClusterIds = { Clusters::Descriptor::Id, Clusters::CommodityPrice::Id };
    std::sort(expectedClusterIds.begin(), expectedClusterIds.end());
    EXPECT_EQ(GetServerClusterIds(kEndpointId), expectedClusterIds);
    ExpectElectricalEnergyTariffDeviceType(kEndpointId);
    EXPECT_EQ(device.CommodityPriceCluster().GetPaths()[0].mEndpointId, kEndpointId);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(HasNoEndpoints());
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedSetsInitialPrice)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);

    ClusterTester tester{ device.CommodityPriceCluster() };

    TariffUnit::TypeInfo::DecodableType tariffUnit = Clusters::Globals::TariffUnitEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(TariffUnit::Id, tariffUnit), CHIP_NO_ERROR);
    EXPECT_EQ(tariffUnit, Clusters::Globals::TariffUnitEnum::kKWh);

    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(tester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    ASSERT_FALSE(currency.IsNull());
    EXPECT_EQ(currency.Value().currency, SimulatedElectricalEnergyTariff::kCurrencyEur);
    EXPECT_EQ(currency.Value().decimalPoints, SimulatedElectricalEnergyTariff::kCurrencyDecimalPoints);

    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(tester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, 0u);
    EXPECT_TRUE(currentPrice.Value().periodEnd.IsNull());
    ASSERT_TRUE(currentPrice.Value().price.HasValue());
    EXPECT_EQ(currentPrice.Value().price.Value(), SimulatedElectricalEnergyTariff::kPricePerKWh);

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedUnregistersCleanlyAfterProviderStartup)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
