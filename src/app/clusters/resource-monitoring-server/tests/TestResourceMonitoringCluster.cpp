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
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Attributes.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Metadata.h>
#include <clusters/HepaFilterMonitoring/Attributes.h>
#include <clusters/HepaFilterMonitoring/Metadata.h>

#include <app/clusters/resource-monitoring-server/MigrateResourceMonitoringServerStorage.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring::Attributes;
using namespace chip::Testing;

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
    }

    void TearDown() override
    {
        app::SetSafeAttributePersistenceProvider(oldPersistence);
        activatedCarbonFilterMonitoring.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    chip::Testing::TestServerClusterContext testContext;
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
                                        ResourceMonitoringCluster::OptionalAttributeSet()
                                            .Set<Attributes::InPlaceIndicator::Id>()
                                            .Set<Attributes::LastChangedTime::Id>(),
                                        ResourceMonitoring::DegradationDirectionEnum::kDown, true)
    {}
};
} // namespace

TEST_F(TestResourceMonitoringCluster, AttributeTest)
{
    ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
    activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);

    ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(
        activatedCarbonFilterMonitoring,
        { ActivatedCarbonFilterMonitoring::Attributes::ChangeIndication::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::Condition::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::DegradationDirection::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::InPlaceIndicator::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::kMetadataEntry,
          ActivatedCarbonFilterMonitoring::Attributes::ReplacementProductList::kMetadataEntry }));
}

TEST_F(TestResourceMonitoringCluster, ReadAttributeTest)
{
    ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
    activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);

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
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal("PRODUCT_0"_span));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal("PRODUCT_1"_span));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal("PRODUCT_2"_span));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal("PRODUCT_3"_span));
    ASSERT_TRUE(it.Next());
    ASSERT_TRUE(it.GetValue().productIdentifierValue.data_equal("PRODUCT_4"_span));

    ASSERT_FALSE(it.Next());
}

// Verify that a value stored in SafeAttributePersistenceProvider via WriteScalarValue (little-endian)
// is correctly migrated to AttributePersistenceProvider and readable through the cluster.
TEST_F(TestResourceMonitoringCluster, MigrationTest_LastChangedTimeMigratesCorrectly)
{

    const ConcreteAttributePath path(kEndpointId, ActivatedCarbonFilterMonitoring::Id, LastChangedTime::Id);
    constexpr uint32_t kValue = 0x12345678u;

    // Store via the safe provider (encodes little-endian internally)
    ASSERT_EQ(safePersistence.WriteScalarValue(path, kValue), CHIP_NO_ERROR);

    // Run migration: safe provider -> attribute persistence provider
    ASSERT_EQ(MigrateResourceMonitoringServerStorage(kEndpointId, ActivatedCarbonFilterMonitoring::Id, safePersistence,
                                                     testContext.AttributePersistenceProvider()),
              CHIP_NO_ERROR);

    ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
    activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);

    // Value should now be readable through the cluster
    ClusterTester tester(activatedCarbonFilterMonitoring);
    DataModel::Nullable<uint32_t> lastChangedTime;
    ASSERT_EQ(tester.ReadAttribute(LastChangedTime::Id, lastChangedTime), CHIP_NO_ERROR);
    ASSERT_FALSE(lastChangedTime.IsNull());
    EXPECT_EQ(lastChangedTime.Value(), kValue);

    // Value must be deleted from the safe provider (one-time migration guarantee)
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safePersistence.SafeReadValue(path, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Verify that raw big-endian bytes written directly to SafeAttributePersistenceProvider are
// migrated verbatim (on little-endian hosts) and can be decoded with Encoding::BigEndian::Get32.
TEST_F(TestResourceMonitoringCluster, MigrationTest_BigEndianBytesRoundTrip)
{
    const ConcreteAttributePath path(kEndpointId, ActivatedCarbonFilterMonitoring::Id, LastChangedTime::Id);
    constexpr uint32_t kTestValue = 0xDEADBEEFu;

    // Encode the value in big-endian byte order
    uint8_t buf[sizeof(uint32_t)];
    Encoding::BigEndian::Put32(buf, kTestValue);

    // Write raw bytes to the safe provider, bypassing the little-endian WriteScalarValue helper
    ASSERT_EQ(safePersistence.SafeWriteValue(path, ByteSpan(buf, sizeof(buf))), CHIP_NO_ERROR);

    // Run migration
    ASSERT_EQ(MigrateResourceMonitoringServerStorage(kEndpointId, ActivatedCarbonFilterMonitoring::Id, safePersistence,
                                                     testContext.AttributePersistenceProvider()),
              CHIP_NO_ERROR);
    ASSERT_EQ(activatedCarbonFilterMonitoring.Startup(testContext.Get()), CHIP_NO_ERROR);
    activatedCarbonFilterMonitoring.SetReplacementProductListManagerInstance(&replacementProductListManager);

    // Read the raw bytes back from the attribute persistence provider
    uint8_t readBuf[sizeof(uint32_t)] = {};
    MutableByteSpan readBuffer(readBuf);
    ASSERT_EQ(testContext.AttributePersistenceProvider().ReadValue(path, readBuffer), CHIP_NO_ERROR);
    ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));

    // Decode as big-endian and confirm the value is preserved verbatim
    EXPECT_EQ(Encoding::BigEndian::Get32(readBuffer.data()), kTestValue);

    // Value must be deleted from the safe provider (one-time migration guarantee)
    {
        uint8_t safeReadBuf[sizeof(uint32_t)] = {};
        MutableByteSpan safeReadBuffer(safeReadBuf);
        EXPECT_EQ(safePersistence.SafeReadValue(path, safeReadBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}
