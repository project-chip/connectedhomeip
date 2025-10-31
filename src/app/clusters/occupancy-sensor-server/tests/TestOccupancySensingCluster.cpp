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

#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// Helper function to read a numeric attribute and decode its value.
template <typename T>
CHIP_ERROR ReadNumericAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, T & value)
{
    static_assert(std::is_arithmetic<T>::value, "This helper only supports numeric attribute types.");

    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    if (paths.size() != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<chip::app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
}

struct TestOccupancySensingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOccupancySensingCluster, TestReadClusterRevision)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster(kTestEndpointId);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    uint16_t clusterRevision;
    EXPECT_EQ(ReadNumericAttribute(cluster, Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, OccupancySensing::kRevision);
}

} // namespace