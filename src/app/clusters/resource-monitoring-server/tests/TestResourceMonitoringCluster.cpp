/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/resource-monitoring-server/ResourceMonitoringCluster.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Attributes.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Metadata.h>
#include <clusters/HepaFilterMonitoring/Attributes.h>
#include <clusters/HepaFilterMonitoring/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring::Attributes;
using namespace chip::Test;
using chip::Test::TestServerClusterContext;

constexpr std::bitset<4> kResourceMonitoringFeatureMap{
    static_cast<uint32_t>(ResourceMonitoring::Feature::kCondition) | static_cast<uint32_t>(ResourceMonitoring::Feature::kWarning) |
    static_cast<uint32_t>(ResourceMonitoring::Feature::kReplacementProductList)
};

namespace {

class ScopedSafeAttributePersistence
{
public:
    ScopedSafeAttributePersistence(TestServerClusterContext & context) : mOldPersistence(app::GetSafeAttributePersistenceProvider())
    {
        VerifyOrDie(mPersistence.Init(&context.StorageDelegate()) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistence);
    }
    ~ScopedSafeAttributePersistence() { app::SetSafeAttributePersistenceProvider(mOldPersistence); }

private:
    app::SafeAttributePersistenceProvider * mOldPersistence;
    app::DefaultSafeAttributePersistenceProvider mPersistence;
};

class ImmutableReplacementProductListManager : public chip::app::Clusters::ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR Next(ReplacementProductStruct & item) override
    {
        if (mIndex >= kReplacementProductListMaxSize)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }

        switch (mIndex)
        {
        case 0: {
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kUpc);
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierValue(CharSpan::fromCharString("PRODUCT_0"));
            break;
        case 1:
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8);
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierValue(CharSpan::fromCharString("PRODUCT_1"));
            break;
        case 2:
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kEan);
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierValue(CharSpan::fromCharString("PRODUCT_2"));
            break;
        case 3:
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14);
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierValue(CharSpan::fromCharString("PRODUCT_3"));
            break;
        case 4:
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kOem);
            TEMPORARY_RETURN_IGNORED item.SetProductIdentifierValue(CharSpan::fromCharString("PRODUCT_4"));
            break;
        default:
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
            break;
        }
        }
        mIndex++;
        return CHIP_NO_ERROR;
    }
};

struct TestResourceMonitoringCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        TestServerClusterContext context;
        ScopedSafeAttributePersistence persistence(context);
        ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
        activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);
    }

    void TearDown() override { activatedCarbonFilterMonitoring.Shutdown(); }

    chip::Test::TestServerClusterContext testContext;

    EndpointId kRootEndpointId = 1;

    ResourceMonitoringCluster activatedCarbonFilterMonitoring;

    ImmutableReplacementProductListManager replacementProductListManager;

    TestResourceMonitoringCluster() :
        activatedCarbonFilterMonitoring(
            kRootEndpointId, ActivatedCarbonFilterMonitoring::Id,
            BitFlags<ResourceMonitoring::Feature>{ static_cast<uint32_t>(kResourceMonitoringFeatureMap.to_ulong()) },
            OptionalAttributeSet(), ResourceMonitoring::DegradationDirectionEnum::kDown, true)
    {}
};
} // namespace

TEST_F(TestResourceMonitoringCluster, AttributeTest)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(activatedCarbonFilterMonitoring.Attributes(ConcreteClusterPath(kRootEndpointId, ActivatedCarbonFilterMonitoring::Id),
                                                         attributes),
              CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributesEntries[] = {
        { true, Condition::kMetadataEntry },
        { true, DegradationDirection::kMetadataEntry },
        { true, ReplacementProductList::kMetadataEntry },
    };

    ASSERT_EQ(
        listBuilder.Append(Span(ActivatedCarbonFilterMonitoring::Attributes::kMandatoryMetadata), Span(optionalAttributesEntries)),
        CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestResourceMonitoringCluster, ReadAttributeTest)
{
    ClusterTester tester(activatedCarbonFilterMonitoring);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    uint8_t changeIndication{};
    ASSERT_EQ(tester.ReadAttribute(ChangeIndication::Id, changeIndication), CHIP_NO_ERROR);

    uint8_t degradationDirection{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::DegradationDirection::Id, degradationDirection), CHIP_NO_ERROR);

    uint8_t condition{};
    ASSERT_EQ(tester.ReadAttribute(Condition::Id, condition), CHIP_NO_ERROR);

    bool inPlaceIndicator{};
    ASSERT_EQ(tester.ReadAttribute(InPlaceIndicator::Id, inPlaceIndicator), CHIP_NO_ERROR);

    DataModel::Nullable<uint32_t> lastChangedTime;
    ASSERT_EQ(tester.ReadAttribute(LastChangedTime::Id, lastChangedTime), CHIP_NO_ERROR);

    ActivatedCarbonFilterMonitoring::Attributes::ReplacementProductList::TypeInfo::DecodableType replacementProductList;
    ASSERT_EQ(tester.ReadAttribute(ReplacementProductList::Id, replacementProductList), CHIP_NO_ERROR);

    auto it = replacementProductList.begin();
    it.Next();
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_0")));
    it.Next();
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_1")));
    it.Next();
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_2")));
    it.Next();
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_3")));
    it.Next();
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_4")));

    ASSERT_FALSE(it.Next());
}
