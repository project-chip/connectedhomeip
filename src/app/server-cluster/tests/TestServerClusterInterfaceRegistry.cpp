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
#include <app/server-cluster/ServerClusterInterface.h>
#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <algorithm>
#include <cstdlib>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace {

constexpr chip::EndpointId kEp1     = 1;
constexpr chip::EndpointId kEp2     = 2;
constexpr chip::ClusterId kCluster1 = 1;
constexpr chip::ClusterId kCluster2 = 2;
constexpr chip::ClusterId kCluster3 = 3;

class FakeServerClusterInterface : public DefaultServerCluster
{
public:
    FakeServerClusterInterface(const ConcreteClusterPath & path) : DefaultServerCluster(path) {}
    FakeServerClusterInterface(EndpointId endpoint, ClusterId cluster) : DefaultServerCluster({ endpoint, cluster }) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case Globals::Attributes::FeatureMap::Id:
            return encoder.Encode<uint32_t>(0);
        case Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode<uint32_t>(123);
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool HasContext() { return mContext != nullptr; }

    const ConcreteClusterPath & GetPath() const { return mPath; }
};

class MultiPathCluster : public DefaultServerCluster
{
public:
    MultiPathCluster(Span<const ConcreteClusterPath> paths) : DefaultServerCluster(paths[0]), mActualPaths(paths) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    Span<const ConcreteClusterPath> GetPaths() const override { return mActualPaths; }

private:
    Span<const ConcreteClusterPath> mActualPaths;
};

struct TestServerClusterInterfaceRegistry : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestServerClusterInterfaceRegistry, AcceptDifferentEndpointPaths)
{
    {
        const std::array<ConcreteClusterPath, 2> kTestPaths{ {
            { 1, 100 },
            { 2, 88 },
        } };
        MultiPathCluster cluster(kTestPaths);
        ServerClusterRegistration registration(cluster);

        ServerClusterInterfaceRegistry registry;
        ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);
        ASSERT_EQ(registry.Get({ 1, 100 }), &cluster);
        ASSERT_EQ(registry.Get({ 2, 88 }), &cluster);
    }

    {
        const std::array<ConcreteClusterPath, 3> kTestPaths{ {
            { 1, 100 },
            { 1, 200 },
            { 3, 100 },
        } };
        MultiPathCluster cluster(kTestPaths);
        ServerClusterRegistration registration(cluster);

        ServerClusterInterfaceRegistry registry;
        ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);
        ASSERT_EQ(registry.Get({ 1, 100 }), &cluster);
        ASSERT_EQ(registry.Get({ 1, 200 }), &cluster);
        ASSERT_EQ(registry.Get({ 3, 100 }), &cluster);
    }
}

TEST_F(TestServerClusterInterfaceRegistry, LazyRegistrationTest)
{
    LazyRegisteredServerCluster<FakeServerClusterInterface> obj;

    EXPECT_FALSE(obj.IsConstructed());

    obj.Create(kEp1, kCluster1);
    EXPECT_TRUE(obj.IsConstructed());
    EXPECT_EQ(obj.Cluster().GetPath(), ConcreteClusterPath(kEp1, kCluster1));
    EXPECT_EQ(obj.Registration().serverClusterInterface, &obj.Cluster());
    obj.Destroy();
    EXPECT_FALSE(obj.IsConstructed());

    obj.Create(kEp2, kCluster3);
    EXPECT_TRUE(obj.IsConstructed());
    EXPECT_EQ(obj.Cluster().GetPath(), ConcreteClusterPath(kEp2, kCluster3));
    EXPECT_EQ(obj.Registration().serverClusterInterface, &obj.Cluster());
    obj.Destroy();
    EXPECT_FALSE(obj.IsConstructed());
}

TEST_F(TestServerClusterInterfaceRegistry, AllClustersIteration)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    FakeServerClusterInterface cluster2(kEp2, kCluster2);
    FakeServerClusterInterface cluster3(kEp2, kCluster3);

    ServerClusterRegistration registration1(cluster1);
    ServerClusterRegistration registration2(cluster2);
    ServerClusterRegistration registration3(cluster3);

    ServerClusterInterfaceRegistry registry;

    EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration3), CHIP_NO_ERROR);

    std::vector<ServerClusterInterface *> found_clusters;
    for (auto * cluster : registry.AllServerClusterInstances())
    {
        found_clusters.push_back(cluster);
    }

    EXPECT_EQ(found_clusters.size(), 3u);
    EXPECT_NE(std::find(found_clusters.begin(), found_clusters.end(), &cluster1), found_clusters.end());
    EXPECT_NE(std::find(found_clusters.begin(), found_clusters.end(), &cluster2), found_clusters.end());
    EXPECT_NE(std::find(found_clusters.begin(), found_clusters.end(), &cluster3), found_clusters.end());

    EXPECT_SUCCESS(registry.Unregister(&cluster2));

    found_clusters.clear();
    for (auto * cluster : registry.AllServerClusterInstances())
    {
        found_clusters.push_back(cluster);
    }

    EXPECT_EQ(found_clusters.size(), 2u);
    EXPECT_NE(std::find(found_clusters.begin(), found_clusters.end(), &cluster1), found_clusters.end());
    EXPECT_EQ(std::find(found_clusters.begin(), found_clusters.end(), &cluster2), found_clusters.end());
    EXPECT_NE(std::find(found_clusters.begin(), found_clusters.end(), &cluster3), found_clusters.end());
}

TEST_F(TestServerClusterInterfaceRegistry, Context)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    FakeServerClusterInterface cluster2(kEp1, kCluster2);
    FakeServerClusterInterface cluster3(kEp2, kCluster3);

    ServerClusterRegistration registration1(cluster1);
    ServerClusterRegistration registration2(cluster2);
    ServerClusterRegistration registration3(cluster3);

    {
        ServerClusterInterfaceRegistry registry;
        EXPECT_FALSE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_FALSE(cluster3.HasContext());

        // registry is NOT initialized
        EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
        EXPECT_FALSE(cluster1.HasContext());

        // set up the registry
        TestServerClusterContext context;
        EXPECT_EQ(registry.SetContext(context.Create()), CHIP_NO_ERROR);

        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_FALSE(cluster3.HasContext());

        // adding clusters automatically adds the context
        EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster2.HasContext());

        // clearing the context clears all clusters
        registry.ClearContext();
        EXPECT_FALSE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_FALSE(cluster3.HasContext());

        EXPECT_EQ(registry.SetContext(context.Create()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_TRUE(cluster2.HasContext());
        EXPECT_FALSE(cluster3.HasContext());

        EXPECT_EQ(registry.Register(registration3), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster3.HasContext());

        // removing clears the context/shuts clusters down
        EXPECT_EQ(registry.Unregister(&cluster2), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_TRUE(cluster3.HasContext());

        // re-setting context works
        EXPECT_EQ(registry.SetContext(context.Create()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_TRUE(cluster3.HasContext());

        // also not valid, but different
        TestServerClusterContext otherContext;

        EXPECT_EQ(registry.SetContext(otherContext.Create()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
        EXPECT_TRUE(cluster3.HasContext());
    }

    // destructor clears the context
    EXPECT_FALSE(cluster1.HasContext());
    EXPECT_FALSE(cluster2.HasContext());
    EXPECT_FALSE(cluster3.HasContext());
}

TEST_F(TestServerClusterInterfaceRegistry, GetWithCache)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    FakeServerClusterInterface cluster2(kEp1, kCluster2);
    ServerClusterRegistration registration1(cluster1);
    ServerClusterRegistration registration2(cluster2);

    ServerClusterInterfaceRegistry registry;
    EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);

    // First Get should find and cache cluster2
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), &cluster2);

    // Second Get should hit the cache
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), &cluster2);

    // Get a different cluster, which will not be cached
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);

    // Now cluster1 should be cached
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);
}

TEST_F(TestServerClusterInterfaceRegistry, RegisterErrors)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    ServerClusterRegistration registration1(cluster1);
    FakeServerClusterInterface invalidPathInterface(kInvalidEndpointId, kCluster1);
    ServerClusterRegistration invalidPathRegistration(invalidPathInterface);
    FakeServerClusterInterface anotherCluster1(kEp1, kCluster1);
    ServerClusterRegistration anotherRegistration1(anotherCluster1);

    ServerClusterInterfaceRegistry registry;

    // Can't register an interface with an invalid path
    EXPECT_EQ(registry.Register(invalidPathRegistration), CHIP_ERROR_INVALID_ARGUMENT);

    // Can't register a duplicate cluster
    EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration1), CHIP_ERROR_DUPLICATE_KEY_ID);
    EXPECT_EQ(registry.Register(anotherRegistration1), CHIP_ERROR_DUPLICATE_KEY_ID);
}

TEST_F(TestServerClusterInterfaceRegistry, UnregisterErrors)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    FakeServerClusterInterface cluster2(kEp1, kCluster2);
    ServerClusterRegistration registration1(cluster1);

    ServerClusterInterfaceRegistry registry;

    EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);

    // Can't unregister a cluster that was not registered.
    EXPECT_EQ(registry.Unregister(&cluster2), CHIP_ERROR_NOT_FOUND);

    // Can't unregister a null cluster.
    EXPECT_EQ(registry.Unregister(nullptr), CHIP_ERROR_NOT_FOUND);
}
