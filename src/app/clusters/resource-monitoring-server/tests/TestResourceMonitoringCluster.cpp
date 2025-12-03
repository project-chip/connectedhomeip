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
#include <app/clusters/testing/ValidateGlobalAttributes.h>
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

namespace {

class ImmutableReplacementProductListManager : public chip::app::Clusters::ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR Next(ReplacementProductStruct & item) override
    {

        struct ReplacementProductStruct
        {
            CharSpan productIdentifierValue;
            ResourceMonitoring::ProductIdentifierTypeEnum productIdentifierType;
        };

        constexpr ReplacementProductStruct kReplacementProducts[] = {
            { "PRODUCT_0"_span, ResourceMonitoring::ProductIdentifierTypeEnum::kUpc },
            { "PRODUCT_1"_span, ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8 },
            { "PRODUCT_2"_span, ResourceMonitoring::ProductIdentifierTypeEnum::kEan },
            { "PRODUCT_3"_span, ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14 },
            { "PRODUCT_4"_span, ResourceMonitoring::ProductIdentifierTypeEnum::kOem },
        };

        VerifyOrReturnError(mIndex < MATTER_ARRAY_SIZE(kReplacementProducts), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

        item.SetProductIdentifierType(kReplacementProducts[mIndex].productIdentifierType);
        VerifyOrDie(item.SetProductIdentifierValue(kReplacementProducts[mIndex].productIdentifierValue) == CHIP_NO_ERROR);
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
        oldPersistence = app::GetSafeAttributePersistenceProvider();
        ASSERT_EQ(safePersistence.Init(&testContext.StorageDelegate()), CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&safePersistence);

        ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
        activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);
    }

    void TearDown() override
    {
        app::SetSafeAttributePersistenceProvider(oldPersistence);
        activatedCarbonFilterMonitoring.Shutdown();
    }

    chip::Test::TestServerClusterContext testContext;
    app::SafeAttributePersistenceProvider * oldPersistence;
    app::DefaultSafeAttributePersistenceProvider safePersistence;

    EndpointId kEndpointId = 1;

    ResourceMonitoringCluster activatedCarbonFilterMonitoring;

    ImmutableReplacementProductListManager replacementProductListManager;

    static constexpr auto kResourceMonitoringFeatureMap =
        BitFlags<ResourceMonitoring::Feature>(ResourceMonitoring::Feature::kCondition, ResourceMonitoring::Feature::kWarning,
                                              ResourceMonitoring::Feature::kReplacementProductList);

    TestResourceMonitoringCluster() :
        activatedCarbonFilterMonitoring(kEndpointId, ActivatedCarbonFilterMonitoring::Id, kResourceMonitoringFeatureMap,
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

    ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(
        activatedCarbonFilterMonitoring,
        { ActivatedCarbonFilterMonitoring::Attributes::ChangeIndication::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::Condition::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::DegradationDirection::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::ReplacementProductList::kMetadataEntry }));
}

TEST_F(TestResourceMonitoringCluster, ReadAttributeTest)
{
    ClusterTester tester(activatedCarbonFilterMonitoring);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, ActivatedCarbonFilterMonitoring::kRevision);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features, kResourceMonitoringFeatureMap.Raw());

    uint8_t changeIndication{};
    ASSERT_EQ(tester.ReadAttribute(ChangeIndication::Id, changeIndication), CHIP_NO_ERROR);
    EXPECT_EQ(changeIndication, static_cast<uint8_t>(ResourceMonitoring::ChangeIndicationEnum::kOk));

    uint8_t degradationDirection{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::DegradationDirection::Id, degradationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(degradationDirection, static_cast<uint8_t>(ResourceMonitoring::DegradationDirectionEnum::kDown));

    uint8_t condition{};
    ASSERT_EQ(tester.ReadAttribute(Condition::Id, condition), CHIP_NO_ERROR);
    EXPECT_EQ(condition, static_cast<uint8_t>(100));

    bool inPlaceIndicator{};
    ASSERT_EQ(tester.ReadAttribute(InPlaceIndicator::Id, inPlaceIndicator), CHIP_NO_ERROR);
    EXPECT_EQ(inPlaceIndicator, true);

    DataModel::Nullable<uint32_t> lastChangedTime;
    ASSERT_EQ(tester.ReadAttribute(LastChangedTime::Id, lastChangedTime), CHIP_NO_ERROR);
    EXPECT_TRUE(lastChangedTime.IsNull());

    ActivatedCarbonFilterMonitoring::Attributes::ReplacementProductList::TypeInfo::DecodableType replacementProductList;
    ASSERT_EQ(tester.ReadAttribute(ReplacementProductList::Id, replacementProductList), CHIP_NO_ERROR);

    auto it = replacementProductList.begin();
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_0")));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_1")));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_2")));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_3")));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal(CharSpan::fromCharString("PRODUCT_4")));

    ASSERT_FALSE(it.Next());
}
