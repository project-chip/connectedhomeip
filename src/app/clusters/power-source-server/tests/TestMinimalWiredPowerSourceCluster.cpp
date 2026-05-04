/*
 *    Copyright (c) 2026 Project CHIP Authors
 */

#include "PowerSourceClusterTestCommon.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PowerSource/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;
using namespace chip::Testing;
using namespace chip::app::Clusters::PowerSource::TestSupport;

struct TestMinimalWiredPowerSourceCluster : public TestBase
{};

TEST_F(TestMinimalWiredPowerSourceCluster, AttributeTest)
{
    MinimalWiredPowerSourceConfig config(kTestEndpointId, CharSpan{}, WiredCurrentTypeEnum::kAc);
    MinimalWiredPowerSourceCluster cluster(config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster,
        {
            Status::kMetadataEntry,
            Order::kMetadataEntry,
            Description::kMetadataEntry,
            WiredCurrentType::kMetadataEntry,
            EndpointList::kMetadataEntry
        }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMinimalWiredPowerSourceCluster, ReadAttributeTest)
{
    MinimalWiredPowerSourceConfig config(kTestEndpointId, CharSpan{}, WiredCurrentTypeEnum::kAc);
    MinimalWiredPowerSourceCluster cluster(config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ReadAttribute<Status::TypeInfo>(tester);
    ReadAttribute<Order::TypeInfo>(tester);
    ReadAttribute<Description::TypeInfo>(tester);
    ReadAttribute<WiredCurrentType::TypeInfo>(tester);
    ReadAttribute<EndpointList::TypeInfo>(tester);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMinimalWiredPowerSourceCluster, TestGetters)
{
    MinimalWiredPowerSourceConfig config(kTestEndpointId, CharSpan{}, WiredCurrentTypeEnum::kAc);
    MinimalWiredPowerSourceCluster cluster(config);

    cluster.GetStatus();
    cluster.GetOrder();
    cluster.GetDescription();
    cluster.GetWiredCurrentType();
    cluster.GetEndpointList();
}

TEST_F(TestMinimalWiredPowerSourceCluster, TestSetters)
{
    MinimalWiredPowerSourceConfig config(kTestEndpointId, CharSpan{}, WiredCurrentTypeEnum::kAc);
    MinimalWiredPowerSourceCluster cluster(config);

    EXPECT_EQ(cluster.SetStatus({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetOrder({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetEndpointList({}), CHIP_NO_ERROR);
}

TEST_F(TestMinimalWiredPowerSourceCluster, TestPersistence)
{
    MinimalWiredPowerSourceConfig config(kTestEndpointId, CharSpan{}, WiredCurrentTypeEnum::kAc);

    TestOrderPersistence<MinimalWiredPowerSourceCluster>(config);
}

TEST_F(TestMinimalWiredPowerSourceCluster, TestBounds)
{
    CharSpan longTestText =
        "Very very long text used for descriptions and designations, totally longer than one hundred bytes. For testing purposes"_span;

    MinimalWiredPowerSourceConfig config(kTestEndpointId, longTestText, WiredCurrentTypeEnum::kAc);
    MinimalWiredPowerSourceCluster cluster(config);
    ClusterTester tester(cluster);
    TestStringAttributeReadLength<Description::TypeInfo>(tester);
}

} // namespace
