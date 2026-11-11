/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::Testing;
using namespace chip::app;

namespace {

constexpr ClusterId kServerClusterId = MockClusterId(1);

const MockNodeConfig gTestNodeConfig({
    MockEndpointConfig(kMockEndpoint1, { MockClusterConfig(kServerClusterId) }),
});

class CountingCluster : public DefaultServerCluster
{
public:
    CountingCluster(ClusterId clusterId, EndpointId endpointId = kMockEndpoint1) : DefaultServerCluster({ endpointId, clusterId })
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        mStartupCount++;
        return DefaultServerCluster::Startup(context);
    }

    void Shutdown(ClusterShutdownType shutdownType) override
    {
        mShutdownCount++;
        DefaultServerCluster::Shutdown(shutdownType);
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    int mStartupCount  = 0;
    int mShutdownCount = 0;
};

class TestCodegenRegisteredServerCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { SetMockNodeConfig(gTestNodeConfig); }
    void TearDown() override
    {
        Registry().ClearContext();
        ResetMockNodeConfig();
    }

    static ServerClusterInterfaceRegistry & Registry() { return CodegenDataModelProvider::Instance().Registry(); }

    void StartRegistry() { EXPECT_SUCCESS(Registry().SetContext(ServerClusterContext{ mContext.Get() })); }

private:
    TestServerClusterContext mContext;
};

TEST_F(TestCodegenRegisteredServerCluster, RegisterAndDestroy)
{
    StartRegistry();
    CountingCluster * cluster;
    {
        CodegenRegisteredServerCluster<CountingCluster> registered(kServerClusterId);
        cluster = &registered.Cluster();

        EXPECT_SUCCESS(registered.Register());
        EXPECT_TRUE(registered.IsRegistered());
        EXPECT_EQ(Registry().Get({ kMockEndpoint1, kServerClusterId }), cluster);
        EXPECT_EQ(registered.Cluster().mStartupCount, 1);
        EXPECT_EQ(registered.Cluster().mShutdownCount, 0);
    }
    EXPECT_EQ(Registry().Get({ kMockEndpoint1, kServerClusterId }), nullptr);
}

TEST_F(TestCodegenRegisteredServerCluster, ExplicitUnregister)
{
    StartRegistry();
    CodegenRegisteredServerCluster<CountingCluster> registered(kServerClusterId);

    EXPECT_SUCCESS(registered.Register());
    registered.Unregister();
    EXPECT_FALSE(registered.IsRegistered());
    EXPECT_EQ(Registry().Get({ kMockEndpoint1, kServerClusterId }), nullptr);
    EXPECT_EQ(registered.Cluster().mShutdownCount, 1);

    // Unregistering again does nothing
    registered.Unregister();
    EXPECT_EQ(registered.Cluster().mShutdownCount, 1);
}

TEST_F(TestCodegenRegisteredServerCluster, ShutdownOnlyOncePerStartup)
{
    CodegenRegisteredServerCluster<CountingCluster> registered(kServerClusterId);
    EXPECT_SUCCESS(registered.Register());
    EXPECT_EQ(registered.Cluster().mStartupCount, 0);

    StartRegistry();
    EXPECT_EQ(registered.Cluster().mStartupCount, 1);

    Registry().ClearContext();
    EXPECT_EQ(registered.Cluster().mShutdownCount, 1);

    // Unregistering a cluster that was already shut down must not shut it down again
    registered.Unregister();
    EXPECT_EQ(registered.Cluster().mShutdownCount, 1);
}

TEST_F(TestCodegenRegisteredServerCluster, NeverRegistered)
{
    StartRegistry();
    {
        CodegenRegisteredServerCluster<CountingCluster> registered(kServerClusterId);
        registered.Unregister();
        EXPECT_EQ(registered.Cluster().mShutdownCount, 0);
    }
}

TEST_F(TestCodegenRegisteredServerCluster, RegisterRequiresEmberServerCluster)
{
    StartRegistry();

    CodegenRegisteredServerCluster<CountingCluster> missingCluster(MockClusterId(2));
    EXPECT_EQ(missingCluster.Register(), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(missingCluster.IsRegistered());
    EXPECT_EQ(Registry().Get({ kMockEndpoint1, MockClusterId(2) }), nullptr);
    EXPECT_EQ(missingCluster.Cluster().mStartupCount, 0);

    CodegenRegisteredServerCluster<CountingCluster> missingEndpoint(kServerClusterId, kMockEndpoint2);
    EXPECT_EQ(missingEndpoint.Register(), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(missingEndpoint.IsRegistered());
    EXPECT_EQ(Registry().Get({ kMockEndpoint2, kServerClusterId }), nullptr);
}

TEST_F(TestCodegenRegisteredServerCluster, RegisterTwice)
{
    StartRegistry();
    CodegenRegisteredServerCluster<CountingCluster> registered(kServerClusterId);

    EXPECT_SUCCESS(registered.Register());
    EXPECT_EQ(registered.Register(), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registered.Cluster().mStartupCount, 1);
}

} // namespace
