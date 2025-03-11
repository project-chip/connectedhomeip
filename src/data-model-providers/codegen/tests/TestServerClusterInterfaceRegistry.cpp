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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>

#include <algorithm>
#include <cstdlib>
#include <random>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace {

constexpr chip::EndpointId kEp1     = 1;
constexpr chip::EndpointId kEp2     = 2;
constexpr chip::EndpointId kEp3     = 3;
constexpr chip::ClusterId kCluster1 = 1;
constexpr chip::ClusterId kCluster2 = 2;
constexpr chip::ClusterId kCluster3 = 3;

class FakeServerClusterInterface : public DefaultServerCluster
{
public:
    FakeServerClusterInterface(EndpointId endpoint, ClusterId cluster) : mPath({ endpoint, cluster }) {}
    FakeServerClusterInterface(const ConcreteClusterPath & path) : mPath(path) {}

    [[nodiscard]] ConcreteClusterPath GetPath() const override { return mPath; }

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

private:
    ConcreteClusterPath mPath;
};

class CannotStartUpCluster : public FakeServerClusterInterface
{
public:
    CannotStartUpCluster(EndpointId endpoint, ClusterId id) : FakeServerClusterInterface(endpoint, id) {}
    CHIP_ERROR Startup(ServerClusterContext * context) override { return CHIP_ERROR_BUSY; }
};

struct TestServerClusterInterfaceRegistry : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestServerClusterInterfaceRegistry, BasicTest)
{
    ServerClusterInterfaceRegistry registry;

    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    FakeServerClusterInterface cluster2(kEp2, kCluster2);
    FakeServerClusterInterface cluster3(kEp2, kCluster3);

    // there should be nothing registered to start with.
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), nullptr);
    EXPECT_EQ(registry.Get({ kInvalidEndpointId, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kInvalidClusterId }), nullptr);

    // registration of invalid values is not acceptable
    {
        // registration has NULL interface
        // next is not null (meaning registration2 looks like already registered)
        ServerClusterRegistration registration1(cluster1);
        ServerClusterRegistration registration2(cluster2);
        registration2.next = &registration1;
        EXPECT_EQ(registry.Register(registration2), CHIP_ERROR_INVALID_ARGUMENT);

        // invalid path in cluster
        FakeServerClusterInterface invalidPathInterface(kInvalidEndpointId, kCluster1);
        ServerClusterRegistration registration3(invalidPathInterface);
        EXPECT_EQ(registry.Register(registration3), CHIP_ERROR_INVALID_ARGUMENT);

        // invalid path in cluster
        FakeServerClusterInterface invalidPathInterface2(kEp1, kInvalidClusterId);
        ServerClusterRegistration registration4(invalidPathInterface);
        EXPECT_EQ(registry.Register(registration4), CHIP_ERROR_INVALID_ARGUMENT);
    }

    ServerClusterRegistration registration1(cluster1);
    ServerClusterRegistration registration2(cluster2);
    ServerClusterRegistration registration3(cluster3);

    // should be able to register
    EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(registration3), CHIP_NO_ERROR);

    // cannot register two implementations on the same path
    {
        FakeServerClusterInterface another1(kEp1, kCluster1);
        ServerClusterRegistration anotherRegisration1(another1);
        EXPECT_EQ(registry.Register(anotherRegisration1), CHIP_ERROR_DUPLICATE_KEY_ID);
    }

    // Items can be found back
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), &cluster2);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), &cluster3);

    EXPECT_EQ(registry.Get({ kEp2, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp3, kCluster2 }), nullptr);

    // repeated calls work
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster1 }), nullptr);

    // remove registrations
    EXPECT_EQ(registry.Unregister({ kEp2, kCluster2 }), &cluster2);
    EXPECT_EQ(registry.Unregister({ kEp2, kCluster2 }), nullptr);

    // Re-adding works
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), &cluster2);

    // clean of an entire endpoint works
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), &cluster3);
    registry.UnregisterAllFromEndpoint(kEp2);
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), nullptr);

    registry.UnregisterAllFromEndpoint(kEp1);
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), nullptr);
}

TEST_F(TestServerClusterInterfaceRegistry, StressTest)
{
    // make the test repeatable
    srand(1234);

    std::vector<FakeServerClusterInterface> items;
    std::vector<ServerClusterRegistration> registrations;

    static constexpr ClusterId kClusterTestCount   = 200;
    static constexpr EndpointId kEndpointTestCount = 10;
    static constexpr size_t kTestIterations        = 4;

    static_assert(kInvalidClusterId > kClusterTestCount, "Tests assume all clusters IDs [0...] are valid");
    static_assert(kTestIterations > 1, "Tests use different unregister methods. Need 2 or more passes.");

    items.reserve(kClusterTestCount);
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        auto endpointId = static_cast<EndpointId>(rand() % kEndpointTestCount);
        items.emplace_back(endpointId, i);
    }

    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        registrations.emplace_back(items[i]);
    }

    ServerClusterInterfaceRegistry registry;

    for (size_t test = 0; test < kTestIterations; test++)
    {
        for (ClusterId i = 0; i < kClusterTestCount; i++)
        {
            ASSERT_EQ(registry.Register(registrations[i]), CHIP_NO_ERROR);
        }

        // test that getters work
        for (ClusterId cluster = 0; cluster < kClusterTestCount; cluster++)
        {
            for (EndpointId ep = 0; ep < kEndpointTestCount; ep++)
            {
                if (items[cluster].GetPath().mEndpointId == ep)
                {
                    ASSERT_EQ(registry.Get({ ep, cluster }), &items[cluster]);
                }
                else
                {
                    ASSERT_EQ(registry.Get({ ep, cluster }), nullptr);
                }
            }
        }

        // clear endpoints. Stress test, unregister in different ways (bulk vs individual)
        if (test % 2 == 1)
        {
            // shuffle unregister
            std::vector<size_t> unregister_order;
            unregister_order.reserve(kClusterTestCount);
            for (size_t i = 0; i < kClusterTestCount; i++)
            {
                unregister_order.push_back(i);
            }

            std::default_random_engine eng(static_cast<std::default_random_engine::result_type>(rand()));
            std::shuffle(unregister_order.begin(), unregister_order.end(), eng);

            // unregister
            for (auto cluster : unregister_order)
            {
                // item MUST exist and be accessible
                ASSERT_EQ(registry.Get(items[cluster].GetPath()), &items[cluster]);
                ASSERT_EQ(registry.Unregister(items[cluster].GetPath()), &items[cluster]);

                // once unregistered, it is not there anymore
                ASSERT_EQ(registry.Get(items[cluster].GetPath()), nullptr);
                ASSERT_EQ(registry.Unregister(items[cluster].GetPath()), nullptr);
            }
        }
        else
        {
            // bulk unregister
            for (EndpointId ep = 0; ep < kEndpointTestCount; ep++)
            {
                registry.UnregisterAllFromEndpoint(ep);
            }
        }

        // all endpoints should be clear
        for (ClusterId cluster = 0; cluster < kClusterTestCount; cluster++)
        {
            for (EndpointId ep = 0; ep < kEndpointTestCount; ep++)
            {
                ASSERT_EQ(registry.Get({ ep, cluster }), nullptr);
            }
        }
    }
}

TEST_F(TestServerClusterInterfaceRegistry, ClustersOnEndpoint)
{
    std::vector<FakeServerClusterInterface> items;
    std::vector<ServerClusterRegistration> registrations;

    static constexpr ClusterId kClusterTestCount   = 200;
    static constexpr EndpointId kEndpointTestCount = 10;

    static_assert(kInvalidClusterId > kClusterTestCount, "Tests assume all clusters IDs [0...] are valid");

    items.reserve(kClusterTestCount);
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        items.emplace_back(static_cast<EndpointId>(i % kEndpointTestCount), i);
    }
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        registrations.emplace_back(items[i]);
    }

    ServerClusterInterfaceRegistry registry;

    // place the clusters on the respecitve endpoints
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        ASSERT_EQ(registry.Register(registrations[i]), CHIP_NO_ERROR);
    }

    // this IS implementation defined: we always register at "HEAD" so the listing is in
    // INVERSE order of registering.
    for (EndpointId ep = 0; ep < kEndpointTestCount; ep++)
    {
        // Move to the end since we iterate in reverse order
        ClusterId expectedClusterId = ep + kEndpointTestCount * (kClusterTestCount / kEndpointTestCount);
        if (expectedClusterId >= kClusterTestCount)
        {
            expectedClusterId -= kEndpointTestCount;
        }

        // ensure that iteration happens exactly as we expect: reverse order and complete
        for (auto cluster : registry.ClustersOnEndpoint(ep))
        {
            ASSERT_LT(expectedClusterId, kClusterTestCount);
            ASSERT_EQ(cluster->GetPath(), ConcreteClusterPath(ep, expectedClusterId));
            expectedClusterId -= kEndpointTestCount; // next expected/registered cluster
        }

        // Iterated through all : we overflowed and got a large number
        ASSERT_GE(expectedClusterId, kClusterTestCount);
    }

    // invalid index works and iteration on empty lists is ok
    auto clusters = registry.ClustersOnEndpoint(kEndpointTestCount + 1);
    ASSERT_EQ(clusters.begin(), clusters.end());
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
        EXPECT_EQ(registry.Unregister(cluster2.GetPath()), &cluster2);
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

TEST_F(TestServerClusterInterfaceRegistry, StartupErrors)
{
    FakeServerClusterInterface cluster1(kEp1, kCluster1);
    CannotStartUpCluster cluster2(kEp2, kCluster2);

    ServerClusterRegistration registration1(cluster1);
    ServerClusterRegistration registration2(cluster2);

    {
        ServerClusterInterfaceRegistry registry;
        EXPECT_FALSE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());

        // register without context works because startup not called yet
        EXPECT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
        EXPECT_EQ(registry.Register(registration2), CHIP_NO_ERROR);

        EXPECT_FALSE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());

        TestServerClusterContext context;
        EXPECT_EQ(registry.SetContext(context.Create()), CHIP_ERROR_HAD_FAILURES);
        EXPECT_TRUE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());

        registry.ClearContext();
        EXPECT_FALSE(cluster1.HasContext());
        EXPECT_FALSE(cluster2.HasContext());
    }
}
