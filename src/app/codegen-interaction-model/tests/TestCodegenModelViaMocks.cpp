/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/codegen-interaction-model/CodegenDataModel.h>

#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <lib/core/DataModelTypes.h>

#include <gtest/gtest.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::InteractionModel;
using namespace chip::app::Clusters::Globals::Attributes;

namespace {

constexpr EndpointId kEndpointIdThatIsMissing = kMockEndpointMin - 1;

static_assert(kEndpointIdThatIsMissing != kInvalidEndpointId);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint1);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint2);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint3);

// clang-format off
const MockNodeConfig gTestNodeConfig({
    MockEndpointConfig(kMockEndpoint1, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id,
        }, {
            MockEventId(1), MockEventId(2),
        }),
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
        }),
    }),
    MockEndpointConfig(kMockEndpoint2, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id,
        }),
        MockClusterConfig(
            MockClusterId(2), 
            {
               ClusterRevision::Id,
               FeatureMap::Id,
               MockAttributeId(1),
               MockAttributeConfig(MockAttributeId(2), ZCL_ARRAY_ATTRIBUTE_TYPE),
            },          /* attributes */
            {},         /* events */
            {1, 2, 23}, /* acceptedCommands */
            {2, 10}     /* generatedCommands */
        ),
        MockClusterConfig(MockClusterId(3), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
        }),
    }),
    MockEndpointConfig(kMockEndpoint3, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
        }),
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
        }),
        MockClusterConfig(MockClusterId(3), {
            ClusterRevision::Id, FeatureMap::Id,
        }),
        MockClusterConfig(MockClusterId(4), {
            ClusterRevision::Id, FeatureMap::Id,
        }),
    }),
});
// clang-format on

struct UseMockNodeConfig
{
    UseMockNodeConfig(const MockNodeConfig & config) { SetMockNodeConfig(config); }
    ~UseMockNodeConfig() { ResetMockNodeConfig(); }
};

} // namespace

TEST(TestCodegenModelViaMocks, IterateOverEndpoints)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // This iteration relies on the hard-coding that occurs when mock_ember is used
    EXPECT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);

    /// Some out of order requests should work as well
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    ASSERT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    ASSERT_EQ(model.FirstEndpoint(), kMockEndpoint1);
}

TEST(TestCodegenModelViaMocks, IterateOverClusters)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    chip::Test::ResetVersion();

    EXPECT_FALSE(model.FirstCluster(kEndpointIdThatIsMissing).path.HasValidIds());
    EXPECT_FALSE(model.FirstCluster(kInvalidEndpointId).path.HasValidIds());

    // mock endpoint 1 has 2 mock clusters: 1 and 2
    ClusterEntry entry = model.FirstCluster(kMockEndpoint1);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(1));
    EXPECT_EQ(entry.info.dataVersion, 0u);
    EXPECT_EQ(entry.info.flags.Raw(), 0u);

    chip::Test::BumpVersion();

    entry = model.NextCluster(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.info.dataVersion, 1u);
    EXPECT_EQ(entry.info.flags.Raw(), 0u);

    entry = model.NextCluster(entry.path);
    EXPECT_FALSE(entry.path.HasValidIds());

    // mock endpoint 3 has 4 mock clusters: 1 through 4
    entry = model.FirstCluster(kMockEndpoint3);
    for (uint16_t clusterId = 1; clusterId <= 4; clusterId++)
    {
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint3);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(clusterId));
        entry = model.NextCluster(entry.path);
    }
    EXPECT_FALSE(entry.path.HasValidIds());

    // repeat calls should work
    for (int i = 0; i < 10; i++)
    {
        entry = model.FirstCluster(kMockEndpoint1);
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(1));
    }

    for (int i = 0; i < 10; i++)
    {
        ClusterEntry nextEntry = model.NextCluster(entry.path);
        ASSERT_TRUE(nextEntry.path.HasValidIds());
        EXPECT_EQ(nextEntry.path.mEndpointId, kMockEndpoint1);
        EXPECT_EQ(nextEntry.path.mClusterId, MockClusterId(2));
    }
}

TEST(TestCodegenModelViaMocks, GetClusterInfo)
{

    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    chip::Test::ResetVersion();

    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId)).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).has_value());

    // now get the value
    std::optional<ClusterInfo> info = model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)));
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->dataVersion, 0u); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->flags.Raw(), 0u); // NOLINT(bugprone-unchecked-optional-access)

    chip::Test::BumpVersion();
    info = model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)));
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->dataVersion, 1u); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->flags.Raw(), 0u); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(TestCodegenModelViaMocks, IterateOverAttributes)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());

    // should be able to iterate over valid paths
    AttributeEntry entry = model.FirstAttribute(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, ClusterRevision::Id);
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, FeatureMap::Id);
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(1));
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(2));
    ASSERT_TRUE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_FALSE(entry.path.HasValidIds());

    // repeated calls should work
    for (int i = 0; i < 10; i++)
    {
        entry = model.FirstAttribute(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
        ASSERT_TRUE(entry.path.HasValidIds());
        ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
        ASSERT_EQ(entry.path.mAttributeId, ClusterRevision::Id);
        ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));
    }

    for (int i = 0; i < 10; i++)
    {
        entry = model.NextAttribute(ConcreteAttributePath(kMockEndpoint2, MockClusterId(2), MockAttributeId(1)));
        ASSERT_TRUE(entry.path.HasValidIds());
        ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
        ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(2));
        ASSERT_TRUE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));
    }
}

TEST(TestCodegenModelViaMocks, GetAttributeInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // various non-existent or invalid paths should return no info data
    ASSERT_FALSE(
        model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, MockClusterId(1), FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), kInvalidAttributeId)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10))).has_value());

    // valid info
    std::optional<AttributeInfo> info =
        model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), FeatureMap::Id));
    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)

    info = model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint2, MockClusterId(2), MockAttributeId(2)));
    ASSERT_TRUE(info.has_value());
    EXPECT_TRUE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)
}

// global attributes are EXPLICITLY not supported
TEST(TestCodegenModelViaMocks, GlobalAttributeInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    std::optional<AttributeInfo> info = model.GetAttributeInfo(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id));

    ASSERT_FALSE(info.has_value());

    info = model.GetAttributeInfo(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id));
    ASSERT_FALSE(info.has_value());
}

TEST(TestCodegenModelViaMocks, IterateOverAcceptedCommands)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());

    // should be able to iterate over valid paths
    CommandEntry entry = model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 1u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 2u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 23u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_FALSE(entry.path.HasValidIds());
}
