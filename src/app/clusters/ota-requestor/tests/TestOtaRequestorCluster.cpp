/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/ota-requestor/OtaRequestorCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

#include <clusters/Identify/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestOtaRequestorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOtaRequestorCluster, TestCreate)
{
    chip::Test::TestServerClusterContext context;
    OtaRequestorCluster cluster(kTestEndpointId);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestOtaRequestorCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;
    OtaRequestorCluster cluster(kTestEndpointId);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id), attributes),
              CHIP_NO_ERROR);

    const DataModel::AttributeEntry expectedAttributes[] = {
        OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdateState::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedAttributes), {}), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOtaRequestorCluster, AcceptedCommandsTest)
{
    chip::Test::TestServerClusterContext context;
    OtaRequestorCluster cluster(kTestEndpointId);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id), acceptedCommands),
              CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

}  // namespace
