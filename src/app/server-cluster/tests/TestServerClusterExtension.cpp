/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <access/Privilege.h>
#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterExtension.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/shared/GlobalIds.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <protocols/interaction_model/Constants.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::Test;
using namespace chip::Protocols::InteractionModel;

constexpr uint32_t kMockRevision   = 123;
constexpr uint32_t kMockFeatureMap = 0x112233;

class MockServerCluster : public DefaultServerCluster
{
public:
    MockServerCluster(const ConcreteClusterPath & path) : DefaultServerCluster(path) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case chip::app::Clusters::Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(kMockRevision);
        case chip::app::Clusters::Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(kMockFeatureMap);
        default:
            return Status::UnsupportedAttribute;
        }
    }
};

class TestableServerClusterExtension : public ServerClusterExtension
{
public:
    TestableServerClusterExtension(const ConcreteClusterPath & path, ServerClusterInterface & underlying) :
        ServerClusterExtension(path, underlying)
    {}

    void TestNotifyAttributeChanged(AttributeId id) { NotifyAttributeChanged(id); }
};

TEST(TestServerClusterExtension, TestExtensionPath)
{
    const ConcreteClusterPath mockPath = { 1, 2 };
    MockServerCluster underlying(mockPath);
    const ConcreteClusterPath extensionPath = { 1, 2 };

    ServerClusterExtension extension(extensionPath, underlying);

    // The extension should return the paths of its underlying interface.
    ASSERT_EQ(extension.GetPaths().size(), 1u);
    ASSERT_EQ(extension.GetPaths()[0], mockPath);
}

TEST(TestServerClusterExtension, TestGetDataVersion)
{
    const ConcreteClusterPath mockPath = { 2, 3 };
    MockServerCluster underlying(mockPath);

    TestableServerClusterExtension extension(mockPath, underlying);

    // Initially, version is the same as underlying (since mVersionDelta is 0).
    ASSERT_EQ(extension.GetDataVersion(mockPath), underlying.GetDataVersion(mockPath));

    // When NotifyAttributeChanged is called WITHOUT a context, mVersionDelta should still increment,
    // but the underlying listener is not marked dirty.
    // Since mContext is initially nullptr, mVersionDelta should NOT increase in the original code
    // (due to VerifyOrReturn). So the version should remain the same as underlying initially
    extension.TestNotifyAttributeChanged(4);
    ASSERT_EQ(extension.GetDataVersion(mockPath),
              underlying.GetDataVersion(mockPath)); // Should still be the same as no context is set

    // Set a context and then notify change. This time mVersionDelta should increment.
    TestServerClusterContext context;
    ASSERT_EQ(extension.Startup(context.Get()), CHIP_NO_ERROR);

    DataVersion oldVersion = extension.GetDataVersion(mockPath);
    extension.TestNotifyAttributeChanged(5);
    ASSERT_EQ(extension.GetDataVersion(mockPath), oldVersion + 1);
}

TEST(TestServerClusterExtension, TestNotifyAttributeChangedWithContext)
{
    const ConcreteClusterPath mockPath = { 1, 2 };
    MockServerCluster underlying(mockPath);
    TestableServerClusterExtension extension(mockPath, underlying);

    // Verify that NotifyAttributeChanged does NOT mark dirty when no context is set.
    extension.TestNotifyAttributeChanged(123);
    // No context set, so no dirty list updates. Default TestServerClusterContext has an empty list.
    // We cannot easily check mVersionDelta directly as it's protected in ServerClusterExtension.
    // The effect of mVersionDelta is visible through GetDataVersion.

    // Create a ServerClusterContext and verify that attribute change notifications are processed.
    TestServerClusterContext context;
    ASSERT_EQ(extension.Startup(context.Get()), CHIP_NO_ERROR);

    // Clear any previous dirty marks from calls before Startup.
    context.ChangeListener().DirtyList().clear();

    DataVersion oldVersion = extension.GetDataVersion(mockPath);
    extension.TestNotifyAttributeChanged(234);
    ASSERT_NE(extension.GetDataVersion(mockPath), oldVersion);

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    ASSERT_EQ(context.ChangeListener().DirtyList()[0], AttributePathParams(mockPath.mEndpointId, mockPath.mClusterId, 234));
}

} // namespace
