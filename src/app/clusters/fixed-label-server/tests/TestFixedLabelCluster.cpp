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

#include <pw_unit_test/framework.h>

#include <app/clusters/fixed-label-server/fixed-label-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/FixedLabel/Attributes.h>
#include <clusters/FixedLabel/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FixedLabel;
using namespace chip::app::Clusters::FixedLabel::Attributes;
using namespace chip::Test;

namespace {

struct TestFixedLabelCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(fixedLabel.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { fixedLabel.Shutdown(); }

    TestFixedLabelCluster() : fixedLabel(kRootEndpointId) {}

    chip::Test::TestServerClusterContext testContext;
    FixedLabelCluster fixedLabel;
};

} // namespace

TEST_F(TestFixedLabelCluster, AttributeTest)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(fixedLabel.Attributes(ConcreteClusterPath(kRootEndpointId, FixedLabel::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    ASSERT_EQ(listBuilder.Append(Span(FixedLabel::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestFixedLabelCluster, ReadAttributeTest)
{
    ClusterTester tester(fixedLabel);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::LabelStruct::DecodableType> labelList;
    ASSERT_EQ(tester.ReadAttribute(LabelList::Id, labelList), CHIP_NO_ERROR);
    auto it = labelList.begin();
    while (it.Next())
    {
        ASSERT_GT(it.GetValue().label.size(), 0u);
    }
}
