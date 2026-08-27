/*
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

#include <pw_unit_test/framework.h>

#include <app/clusters/commodity-price-server/CommodityPriceCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/Commands.h>
#include <clusters/CommodityPrice/Events.h>
#include <clusters/CommodityPrice/Metadata.h>

#include <string>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;
using namespace chip::app::Clusters::CommodityPrice::Attributes;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

constexpr EndpointId kTestEndpointId = 1;

using OptionalCommandSet = CommodityPriceCluster::OptionalCommandSet;

constexpr OptionalCommandSet kAllOptionalCommands =
    OptionalCommandSet().Set<Commands::GetDetailedPriceRequest::Id>().Set<Commands::GetDetailedForecastRequest::Id>();

constexpr uint32_t kPeriodStart     = 1700000000;
constexpr uint32_t kPeriodDuration  = 3600;
constexpr int64_t kPrice            = 1234;
constexpr uint16_t kCurrencyEuro    = 978;
constexpr uint8_t kCurrencyDecimals = 2;

constexpr CharSpan kPriceDescription     = "peak"_span;
constexpr CharSpan kComponentDescription = "standing charge"_span;
constexpr int64_t kComponentPrice        = 100;

Structs::CommodityPriceStruct::Type MakePrice(uint32_t periodStart = kPeriodStart, int64_t price = kPrice)
{
    Structs::CommodityPriceStruct::Type value;

    value.periodStart = periodStart;
    value.periodEnd.SetNonNull(periodStart + kPeriodDuration);
    value.price.SetValue(price);

    return value;
}

/// Stores a CurrentPrice that carries both a description and a described component.
void SetDetailedPrice(CommodityPriceCluster & cluster)
{
    Structs::CommodityPriceComponentStruct::Type component;
    component.price = kComponentPrice;
    component.description.SetValue(kComponentDescription);

    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.description.SetValue(kPriceDescription);
    price.components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(&component, 1));

    ASSERT_EQ(cluster.SetCurrentPrice(price), CHIP_NO_ERROR);
}

/// Stores a single entry PriceForecast carrying both a description and a described component.
void SetDetailedForecast(CommodityPriceCluster & cluster)
{
    Structs::CommodityPriceComponentStruct::Type component;
    component.price = kComponentPrice;
    component.description.SetValue(kComponentDescription);

    Structs::CommodityPriceStruct::Type entry = MakePrice();
    entry.description.SetValue(kPriceDescription);
    entry.components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(&component, 1));

    ASSERT_EQ(cluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(&entry, 1)), CHIP_NO_ERROR);
}

BitMask<CommodityPriceDetailBitmap> AllDetails()
{
    return BitMask<CommodityPriceDetailBitmap>(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents);
}

struct TestCommodityPriceCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mCluster.Startup(mTester.GetServerClusterContext()), CHIP_NO_ERROR); }

    CommodityPriceCluster mCluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), kAllOptionalCommands };
    ClusterTester mTester{ mCluster };
};

TEST_F(TestCommodityPriceCluster, ReadClusterRevision)
{
    ClusterRevision::TypeInfo::DecodableType clusterRevision = 0;
    ASSERT_EQ(mTester.ReadAttribute(ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestCommodityPriceCluster, ReadFeatureMap)
{
    FeatureMap::TypeInfo::DecodableType featureMap = 0;
    ASSERT_EQ(mTester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kForecasting));
}

TEST_F(TestCommodityPriceCluster, AttributeListWithForecasting)
{
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster,
                                        std::vector<DataModel::AttributeEntry>{
                                            TariffUnit::kMetadataEntry,
                                            Currency::kMetadataEntry,
                                            CurrentPrice::kMetadataEntry,
                                            PriceForecast::kMetadataEntry,
                                        }));
}

TEST_F(TestCommodityPriceCluster, AttributeListWithoutForecasting)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        std::vector<DataModel::AttributeEntry>{
                                            TariffUnit::kMetadataEntry,
                                            Currency::kMetadataEntry,
                                            CurrentPrice::kMetadataEntry,
                                        }));
}

TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithAllOptionalCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedPriceRequest::kMetadataEntry,
                                                  Commands::GetDetailedForecastRequest::kMetadataEntry,
                                              }));
}

// Both commands are optionally conformant, so opting into neither is a valid configuration.
TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithNoOptionalCommands)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), OptionalCommandSet() };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, std::vector<DataModel::AcceptedCommandEntry>{}));
}

// GetDetailedForecastRequest requires FORE in addition to the opt-in, so it must be absent here even
// though the application asked for it.
TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithoutForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedPriceRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithForecastRequestOnly)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting),
                                   OptionalCommandSet().Set<Commands::GetDetailedForecastRequest::Id>() };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedForecastRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithAllOptionalCommands)
{
    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(mCluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedPriceResponse::Id,
                                                   Commands::GetDetailedForecastResponse::Id,
                                               }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithNoOptionalCommands)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), OptionalCommandSet() };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster, std::vector<CommandId>{}));
}

// GetDetailedForecastResponse follows its request, which requires FORE on top of the opt-in.
TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithoutForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedPriceResponse::Id,
                                               }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithForecastRequestOnly)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting),
                                   OptionalCommandSet().Set<Commands::GetDetailedForecastRequest::Id>() };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedForecastResponse::Id,
                                               }));
}

TEST_F(TestCommodityPriceCluster, ReadTariffUnitInitialValue)
{
    TariffUnit::TypeInfo::DecodableType tariffUnit = Globals::TariffUnitEnum::kUnknownEnumValue;
    ASSERT_EQ(mTester.ReadAttribute(TariffUnit::Id, tariffUnit), CHIP_NO_ERROR);
    EXPECT_EQ(tariffUnit, Globals::TariffUnitEnum::kKWh);
}

TEST_F(TestCommodityPriceCluster, ReadCurrencyInitialValueIsNull)
{
    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(mTester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    EXPECT_TRUE(currency.IsNull());
}

TEST_F(TestCommodityPriceCluster, ReadCurrentPriceInitialValueIsNull)
{
    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(mTester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    EXPECT_TRUE(currentPrice.IsNull());
}

TEST_F(TestCommodityPriceCluster, ReadPriceForecastInitialValueIsEmpty)
{
    PriceForecast::TypeInfo::DecodableType priceForecast;
    ASSERT_EQ(mTester.ReadAttribute(PriceForecast::Id, priceForecast), CHIP_NO_ERROR);

    auto iter = priceForecast.begin();
    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
}

// PriceForecast is not part of the attribute list without FORE, so reading it is rejected before it
// ever reaches ReadAttribute.
TEST_F(TestCommodityPriceCluster, ReadPriceForecastWithoutForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };
    ClusterTester tester{ cluster };
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    PriceForecast::TypeInfo::DecodableType priceForecast;
    EXPECT_EQ(tester.ReadAttribute(PriceForecast::Id, priceForecast), Status::UnsupportedAttribute);
}

TEST_F(TestCommodityPriceCluster, SetTariffUnitUpdatesTheAttribute)
{
    ASSERT_EQ(mCluster.SetTariffUnit(Globals::TariffUnitEnum::kKVAh), CHIP_NO_ERROR);

    TariffUnit::TypeInfo::DecodableType tariffUnit = Globals::TariffUnitEnum::kUnknownEnumValue;
    ASSERT_EQ(mTester.ReadAttribute(TariffUnit::Id, tariffUnit), CHIP_NO_ERROR);
    EXPECT_EQ(tariffUnit, Globals::TariffUnitEnum::kKVAh);
    EXPECT_TRUE(mTester.IsAttributeDirty(TariffUnit::Id));
}

TEST_F(TestCommodityPriceCluster, SetTariffUnitRejectsAnUnknownValue)
{
    EXPECT_EQ(mCluster.SetTariffUnit(static_cast<Globals::TariffUnitEnum>(0x02)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    TariffUnit::TypeInfo::DecodableType tariffUnit = Globals::TariffUnitEnum::kUnknownEnumValue;
    ASSERT_EQ(mTester.ReadAttribute(TariffUnit::Id, tariffUnit), CHIP_NO_ERROR);
    EXPECT_EQ(tariffUnit, Globals::TariffUnitEnum::kKWh);
}

TEST_F(TestCommodityPriceCluster, SetCurrencyUpdatesTheAttribute)
{
    Globals::Structs::CurrencyStruct::Type euro;
    euro.currency      = kCurrencyEuro;
    euro.decimalPoints = kCurrencyDecimals;

    ASSERT_EQ(mCluster.SetCurrency(euro), CHIP_NO_ERROR);

    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(mTester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    ASSERT_FALSE(currency.IsNull());
    EXPECT_EQ(currency.Value().currency, kCurrencyEuro);
    EXPECT_EQ(currency.Value().decimalPoints, kCurrencyDecimals);
    EXPECT_TRUE(mTester.IsAttributeDirty(Currency::Id));
}

TEST_F(TestCommodityPriceCluster, SetCurrencyToTheSameValueIsNotReported)
{
    Globals::Structs::CurrencyStruct::Type euro;
    euro.currency      = kCurrencyEuro;
    euro.decimalPoints = kCurrencyDecimals;

    ASSERT_EQ(mCluster.SetCurrency(euro), CHIP_NO_ERROR);
    mTester.GetDirtyList().clear();

    ASSERT_EQ(mCluster.SetCurrency(euro), CHIP_NO_ERROR);
    EXPECT_FALSE(mTester.IsAttributeDirty(Currency::Id));

    // A change limited to the decimal points is still a change.
    euro.decimalPoints = kCurrencyDecimals + 1;
    ASSERT_EQ(mCluster.SetCurrency(euro), CHIP_NO_ERROR);
    EXPECT_TRUE(mTester.IsAttributeDirty(Currency::Id));
}

// The spec caps the currency code at 999, the highest ISO 4217 numeric code.
TEST_F(TestCommodityPriceCluster, SetCurrencyRejectsAnOutOfRangeCode)
{
    Globals::Structs::CurrencyStruct::Type invalid;
    invalid.currency = kMaxCurrencyValue + 1;

    EXPECT_EQ(mCluster.SetCurrency(invalid), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(mTester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    EXPECT_TRUE(currency.IsNull());
}

TEST_F(TestCommodityPriceCluster, SetCurrencyToNull)
{
    Globals::Structs::CurrencyStruct::Type euro;
    euro.currency = kCurrencyEuro;
    ASSERT_EQ(mCluster.SetCurrency(euro), CHIP_NO_ERROR);

    ASSERT_EQ(mCluster.SetCurrency(DataModel::NullNullable), CHIP_NO_ERROR);

    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(mTester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    EXPECT_TRUE(currency.IsNull());
}

// The description and the components are stored, but never exposed through the attribute.
TEST_F(TestCommodityPriceCluster, SetCurrentPriceStripsDetailsFromTheAttribute)
{
    Structs::CommodityPriceComponentStruct::Type components[2];
    components[0].price = 100;
    components[0].description.SetValue("standing charge"_span);
    components[1].price = 200;

    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.description.SetValue("peak"_span);
    price.components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(components, 2));

    ASSERT_EQ(mCluster.SetCurrentPrice(price), CHIP_NO_ERROR);

    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(mTester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, kPeriodStart);
    ASSERT_FALSE(currentPrice.Value().periodEnd.IsNull());
    EXPECT_EQ(currentPrice.Value().periodEnd.Value(), kPeriodStart + kPeriodDuration);
    ASSERT_TRUE(currentPrice.Value().price.HasValue());
    EXPECT_EQ(currentPrice.Value().price.Value(), kPrice);
    EXPECT_FALSE(currentPrice.Value().description.HasValue());
    EXPECT_FALSE(currentPrice.Value().components.HasValue());
    EXPECT_TRUE(mTester.IsAttributeDirty(CurrentPrice::Id));
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceGeneratesAPriceChangeEvent)
{
    ASSERT_EQ(mCluster.SetCurrentPrice(MakePrice()), CHIP_NO_ERROR);

    auto generatedEvent = mTester.GetNextGeneratedEvent();
    ASSERT_TRUE(generatedEvent.has_value());

    Events::PriceChange::DecodableType event;
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): checked above
    ASSERT_EQ(generatedEvent->GetEventData(event), CHIP_NO_ERROR);
    ASSERT_FALSE(event.currentPrice.IsNull());
    EXPECT_EQ(event.currentPrice.Value().periodStart, kPeriodStart);
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceToNull)
{
    ASSERT_EQ(mCluster.SetCurrentPrice(MakePrice()), CHIP_NO_ERROR);

    ASSERT_EQ(mCluster.SetCurrentPrice(DataModel::NullNullable), CHIP_NO_ERROR);

    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(mTester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    EXPECT_TRUE(currentPrice.IsNull());
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceRejectsAPeriodEndingBeforeItStarts)
{
    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.periodEnd.SetNonNull(price.periodStart - 1);

    EXPECT_EQ(mCluster.SetCurrentPrice(price), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceRejectsAPriceWithoutAValue)
{
    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.price.ClearValue();

    EXPECT_EQ(mCluster.SetCurrentPrice(price), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceRejectsAnOverlongDescription)
{
    const std::string description(kMaxDescriptionLength + 1, 'x');

    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.description.SetValue(CharSpan(description.data(), description.size()));

    EXPECT_EQ(mCluster.SetCurrentPrice(price), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestCommodityPriceCluster, SetCurrentPriceRejectsTooManyComponents)
{
    Structs::CommodityPriceComponentStruct::Type components[kMaxComponentsPerPriceEntry + 1];

    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.components.SetValue(
        DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(components, kMaxComponentsPerPriceEntry + 1));

    EXPECT_EQ(mCluster.SetCurrentPrice(price), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// A rejected value must not disturb the price that is already stored.
TEST_F(TestCommodityPriceCluster, SetCurrentPriceKeepsThePreviousValueOnRejection)
{
    ASSERT_EQ(mCluster.SetCurrentPrice(MakePrice()), CHIP_NO_ERROR);

    Structs::CommodityPriceStruct::Type invalid = MakePrice(kPeriodStart + kPeriodDuration);
    invalid.price.ClearValue();
    ASSERT_EQ(mCluster.SetCurrentPrice(invalid), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(mTester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, kPeriodStart);
}

// A description and a component list that are present but empty take up no room in the storage
// buffers, which are then never allocated. The copy still has to hold on to the distinction between
// an absent optional and an empty one.
TEST_F(TestCommodityPriceCluster, SetCurrentPriceWithEmptyOptionalDetails)
{
    Structs::CommodityPriceStruct::Type price = MakePrice();
    price.description.SetValue(""_span);
    price.components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>());

    ASSERT_EQ(mCluster.SetCurrentPrice(price), CHIP_NO_ERROR);

    Commands::GetDetailedPriceRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    const auto & storedPrice = result.response->currentPrice;
    ASSERT_FALSE(storedPrice.IsNull());
    EXPECT_EQ(storedPrice.Value().periodStart, kPeriodStart);

    ASSERT_TRUE(storedPrice.Value().description.HasValue());
    EXPECT_TRUE(storedPrice.Value().description.Value().empty());

    ASSERT_TRUE(storedPrice.Value().components.HasValue());
    auto components = storedPrice.Value().components.Value().begin();
    EXPECT_FALSE(components.Next());
    EXPECT_EQ(components.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCommodityPriceCluster, SetForecastUpdatesTheAttribute)
{
    // The entries are deep copied, so they and everything they point at are gone by the time the
    // attribute is read.
    {
        const std::string description = "off peak";
        Structs::CommodityPriceComponentStruct::Type component;
        component.price = 100;
        component.description.SetValue(CharSpan(description.data(), description.size()));

        Structs::CommodityPriceStruct::Type entries[2];
        entries[0] = MakePrice();
        entries[0].description.SetValue(CharSpan(description.data(), description.size()));
        entries[0].components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(&component, 1));
        entries[1] = MakePrice(kPeriodStart + kPeriodDuration, kPrice * 2);

        ASSERT_EQ(mCluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(entries, 2)), CHIP_NO_ERROR);
    }

    PriceForecast::TypeInfo::DecodableType priceForecast;
    ASSERT_EQ(mTester.ReadAttribute(PriceForecast::Id, priceForecast), CHIP_NO_ERROR);

    auto iter = priceForecast.begin();

    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().periodStart, kPeriodStart);
    ASSERT_TRUE(iter.GetValue().price.HasValue());
    EXPECT_EQ(iter.GetValue().price.Value(), kPrice);
    EXPECT_FALSE(iter.GetValue().description.HasValue());
    EXPECT_FALSE(iter.GetValue().components.HasValue());

    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().periodStart, kPeriodStart + kPeriodDuration);
    ASSERT_TRUE(iter.GetValue().price.HasValue());
    EXPECT_EQ(iter.GetValue().price.Value(), kPrice * 2);

    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_TRUE(mTester.IsAttributeDirty(PriceForecast::Id));
}

TEST_F(TestCommodityPriceCluster, SetForecastToAnEmptyList)
{
    Structs::CommodityPriceStruct::Type entry = MakePrice();
    ASSERT_EQ(mCluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(&entry, 1)), CHIP_NO_ERROR);

    ASSERT_EQ(mCluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>()), CHIP_NO_ERROR);

    PriceForecast::TypeInfo::DecodableType priceForecast;
    ASSERT_EQ(mTester.ReadAttribute(PriceForecast::Id, priceForecast), CHIP_NO_ERROR);

    auto iter = priceForecast.begin();
    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
}

// An entry whose details are empty sits alongside one that fills the shared buffers, so here the
// buffers do exist and the empty entry must neither consume nor skip any room in them.
TEST_F(TestCommodityPriceCluster, SetForecastMixesEmptyAndPopulatedDetails)
{
    Structs::CommodityPriceComponentStruct::Type component;
    component.price = kComponentPrice;
    component.description.SetValue(kComponentDescription);

    Structs::CommodityPriceStruct::Type entries[2];
    entries[0] = MakePrice();
    entries[0].description.SetValue(""_span);
    entries[0].components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>());
    entries[1] = MakePrice(kPeriodStart + kPeriodDuration, kPrice * 2);
    entries[1].description.SetValue(kPriceDescription);
    entries[1].components.SetValue(DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(&component, 1));

    ASSERT_EQ(mCluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(entries, 2)), CHIP_NO_ERROR);

    Commands::GetDetailedForecastRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    auto forecast = result.response->priceForecast.begin();

    ASSERT_TRUE(forecast.Next());
    EXPECT_EQ(forecast.GetValue().periodStart, kPeriodStart);
    ASSERT_TRUE(forecast.GetValue().description.HasValue());
    EXPECT_TRUE(forecast.GetValue().description.Value().empty());
    ASSERT_TRUE(forecast.GetValue().components.HasValue());
    auto emptyComponents = forecast.GetValue().components.Value().begin();
    EXPECT_FALSE(emptyComponents.Next());

    ASSERT_TRUE(forecast.Next());
    EXPECT_EQ(forecast.GetValue().periodStart, kPeriodStart + kPeriodDuration);
    ASSERT_TRUE(forecast.GetValue().description.HasValue());
    EXPECT_TRUE(forecast.GetValue().description.Value().data_equal(kPriceDescription));
    ASSERT_TRUE(forecast.GetValue().components.HasValue());
    auto components = forecast.GetValue().components.Value().begin();
    ASSERT_TRUE(components.Next());
    EXPECT_EQ(components.GetValue().price, kComponentPrice);
    ASSERT_TRUE(components.GetValue().description.HasValue());
    EXPECT_TRUE(components.GetValue().description.Value().data_equal(kComponentDescription));
    EXPECT_FALSE(components.Next());

    EXPECT_FALSE(forecast.Next());
    EXPECT_EQ(forecast.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCommodityPriceCluster, SetForecastRejectsTooManyEntries)
{
    Structs::CommodityPriceStruct::Type entries[kMaxForecastEntries + 1];
    for (size_t i = 0; i < kMaxForecastEntries + 1; i++)
    {
        entries[i] = MakePrice(static_cast<uint32_t>(kPeriodStart + (i * kPeriodDuration)));
    }

    EXPECT_EQ(mCluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(entries, kMaxForecastEntries + 1)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Without FORE there is no PriceForecast attribute to hold the value.
TEST_F(TestCommodityPriceCluster, SetForecastRequiresTheForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    Structs::CommodityPriceStruct::Type entry = MakePrice();
    EXPECT_EQ(cluster.SetForecast(Span<const Structs::CommodityPriceStruct::Type>(&entry, 1)), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestReturnsWhatWasAskedFor)
{
    SetDetailedPrice(mCluster);

    Commands::GetDetailedPriceRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    const auto & currentPrice = result.response->currentPrice;
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, kPeriodStart);

    ASSERT_TRUE(currentPrice.Value().description.HasValue());
    EXPECT_TRUE(currentPrice.Value().description.Value().data_equal(kPriceDescription));

    ASSERT_TRUE(currentPrice.Value().components.HasValue());
    auto iter = currentPrice.Value().components.Value().begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().price, kComponentPrice);
    ASSERT_TRUE(iter.GetValue().description.HasValue());
    EXPECT_TRUE(iter.GetValue().description.Value().data_equal(kComponentDescription));
    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestWithoutDetailsStripsThePrice)
{
    SetDetailedPrice(mCluster);

    // The Details argument defaults to none of the bits being set.
    auto result = mTester.Invoke(Commands::GetDetailedPriceRequest::Type{});
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    const auto & currentPrice = result.response->currentPrice;
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, kPeriodStart);
    EXPECT_FALSE(currentPrice.Value().description.HasValue());
    EXPECT_FALSE(currentPrice.Value().components.HasValue());
}

TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestWithDescriptionOnly)
{
    SetDetailedPrice(mCluster);

    Commands::GetDetailedPriceRequest::Type request;
    request.details.Set(CommodityPriceDetailBitmap::kDescription);

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    const auto & currentPrice = result.response->currentPrice;
    ASSERT_FALSE(currentPrice.IsNull());
    ASSERT_TRUE(currentPrice.Value().description.HasValue());
    EXPECT_TRUE(currentPrice.Value().description.Value().data_equal(kPriceDescription));
    EXPECT_FALSE(currentPrice.Value().components.HasValue());
}

TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestWithoutAPrice)
{
    Commands::GetDetailedPriceRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    EXPECT_TRUE(result.response->currentPrice.IsNull());
}

// The spec defines two Details bits; anything else is not a valid request.
TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestRejectsUndefinedDetailBits)
{
    Commands::GetDetailedPriceRequest::Type request;
    request.details.SetRaw(0x04);

    auto result = mTester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestCommodityPriceCluster, GetDetailedPriceRequestIsRejectedWhenNotOptedInto)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), OptionalCommandSet() };
    ClusterTester tester{ cluster };
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto result = tester.Invoke(Commands::GetDetailedPriceRequest::Type{});
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedCommand));
}

TEST_F(TestCommodityPriceCluster, GetDetailedForecastRequestReturnsWhatWasAskedFor)
{
    SetDetailedForecast(mCluster);

    Commands::GetDetailedForecastRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    auto forecast = result.response->priceForecast.begin();
    ASSERT_TRUE(forecast.Next());
    EXPECT_EQ(forecast.GetValue().periodStart, kPeriodStart);

    ASSERT_TRUE(forecast.GetValue().description.HasValue());
    EXPECT_TRUE(forecast.GetValue().description.Value().data_equal(kPriceDescription));

    ASSERT_TRUE(forecast.GetValue().components.HasValue());
    auto components = forecast.GetValue().components.Value().begin();
    ASSERT_TRUE(components.Next());
    EXPECT_EQ(components.GetValue().price, kComponentPrice);
    ASSERT_TRUE(components.GetValue().description.HasValue());
    EXPECT_TRUE(components.GetValue().description.Value().data_equal(kComponentDescription));
    EXPECT_FALSE(components.Next());

    EXPECT_FALSE(forecast.Next());
    EXPECT_EQ(forecast.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCommodityPriceCluster, GetDetailedForecastRequestWithoutDetailsStripsTheEntries)
{
    SetDetailedForecast(mCluster);

    auto result = mTester.Invoke(Commands::GetDetailedForecastRequest::Type{});
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    auto forecast = result.response->priceForecast.begin();
    ASSERT_TRUE(forecast.Next());
    EXPECT_EQ(forecast.GetValue().periodStart, kPeriodStart);
    EXPECT_FALSE(forecast.GetValue().description.HasValue());
    EXPECT_FALSE(forecast.GetValue().components.HasValue());
    EXPECT_FALSE(forecast.Next());
}

TEST_F(TestCommodityPriceCluster, GetDetailedForecastRequestWithoutAForecast)
{
    Commands::GetDetailedForecastRequest::Type request;
    request.details = AllDetails();

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): IsSuccess() implies a response
    auto forecast = result.response->priceForecast.begin();
    EXPECT_FALSE(forecast.Next());
    EXPECT_EQ(forecast.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCommodityPriceCluster, GetDetailedForecastRequestRejectsUndefinedDetailBits)
{
    Commands::GetDetailedForecastRequest::Type request;
    request.details.SetRaw(0x04);

    auto result = mTester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

} // namespace
