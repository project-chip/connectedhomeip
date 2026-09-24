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

struct TestFullWiredPowerSourceCluster : public TestBase
{
};

TEST_F(TestFullWiredPowerSourceCluster, AttributeTest)
{
    FullWiredPowerSourceConfig config(CharSpan{}, WiredCurrentTypeEnum::kAc);
    FullWiredPowerSourceCluster cluster(kTestEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster,
        { Status::kMetadataEntry, Order::kMetadataEntry, Description::kMetadataEntry, WiredAssessedInputVoltage::kMetadataEntry,
          WiredAssessedInputFrequency::kMetadataEntry, WiredCurrentType::kMetadataEntry, WiredAssessedCurrent::kMetadataEntry,
          WiredNominalVoltage::kMetadataEntry, WiredMaximumCurrent::kMetadataEntry, WiredPresent::kMetadataEntry,
          ActiveWiredFaults::kMetadataEntry, EndpointList::kMetadataEntry }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFullWiredPowerSourceCluster, ReadAttributeTest)
{
    FullWiredPowerSourceConfig config(CharSpan{}, WiredCurrentTypeEnum::kAc);
    FullWiredPowerSourceCluster cluster(kTestEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ReadAttribute<Status::TypeInfo>(tester);
    ReadAttribute<Order::TypeInfo>(tester);
    ReadAttribute<Description::TypeInfo>(tester);
    ReadAttribute<WiredAssessedInputVoltage::TypeInfo>(tester);
    ReadAttribute<WiredAssessedInputFrequency::TypeInfo>(tester);
    ReadAttribute<WiredCurrentType::TypeInfo>(tester);
    ReadAttribute<WiredAssessedCurrent::TypeInfo>(tester);
    ReadAttribute<WiredNominalVoltage::TypeInfo>(tester);
    ReadAttribute<WiredMaximumCurrent::TypeInfo>(tester);
    ReadAttribute<WiredPresent::TypeInfo>(tester);
    ReadAttribute<ActiveWiredFaults::TypeInfo>(tester);
    ReadAttribute<EndpointList::TypeInfo>(tester);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFullWiredPowerSourceCluster, TestGetters)
{
    FullWiredPowerSourceConfig config(CharSpan{}, WiredCurrentTypeEnum::kAc);
    FullWiredPowerSourceCluster cluster(kTestEndpointId, config);

    cluster.GetStatus();
    cluster.GetOrder();
    cluster.GetDescription();
    cluster.GetWiredAssessedInputVoltage();
    cluster.GetWiredAssessedInputFrequency();
    cluster.GetWiredCurrentType();
    cluster.GetWiredAssessedCurrent();
    cluster.GetWiredNominalVoltage();
    cluster.GetWiredMaximumCurrent();
    cluster.GetWiredPresent();
    // this will do nothing, because if the buffer is smaller than needed, it will be filled as much as possible.
    auto noBuf = Span<WiredFaultEnum>{};
    cluster.GetActiveWiredFaults(noBuf);
    cluster.GetEndpointList();
}

TEST_F(TestFullWiredPowerSourceCluster, TestSetters)
{
    FullWiredPowerSourceConfig config(CharSpan{}, WiredCurrentTypeEnum::kAc);
    FullWiredPowerSourceCluster cluster(kTestEndpointId, config);

    EXPECT_EQ(cluster.SetStatus({}), CHIP_NO_ERROR);
    cluster.SetOrder({});
    cluster.SetWiredAssessedInputVoltage({});
    cluster.SetWiredAssessedInputFrequency({});
    cluster.SetWiredAssessedCurrent({});
    cluster.SetWiredPresent({});
    cluster.SetActiveWiredFaults({});
    EXPECT_EQ(cluster.SetEndpointList({}), CHIP_NO_ERROR);
}

TEST_F(TestFullWiredPowerSourceCluster, TestBounds)
{
    CharSpan longTestText =
        "Very very long text used for descriptions and designations, totally longer than one hundred bytes. For testing purposes"_span;

    FullWiredPowerSourceConfig config(longTestText, WiredCurrentTypeEnum::kAc);
    FullWiredPowerSourceCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    TestStringAttributeReadLength<Description::TypeInfo>(tester);
}

} // namespace
