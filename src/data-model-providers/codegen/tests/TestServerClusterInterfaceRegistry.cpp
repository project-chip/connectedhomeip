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
#include <data-model-providers/codegen/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>

#include <algorithm>
#include <cstdlib>
#include <random>

using namespace chip;
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
    FakeServerClusterInterface(ClusterId id) : mClusterId(id) {}

    ClusterId GetClusterId() const override { return mClusterId; }

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

private:
    ClusterId mClusterId;
};

struct TestServerClusterInterfaceRegistry : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestServerClusterInterfaceRegistry, BasicTest)
{
    // NOTE: tests DO NOT use the global registry and validate implementation
    //       details.

    ServerClusterInterfaceRegistry registry;

    FakeServerClusterInterface cluster1(kCluster1);
    FakeServerClusterInterface cluster2(kCluster2);
    FakeServerClusterInterface cluster3(kCluster3);

    // there should be nothing registered to start with.
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kEp1, kCluster1)), nullptr);
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kEp1, kCluster2)), nullptr);
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kEp2, kCluster2)), nullptr);
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kEp2, kCluster3)), nullptr);
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kInvalidEndpointId, kCluster2)), nullptr);
    EXPECT_EQ(registry.Get(ConcreteClusterPath(kEp1, kInvalidClusterId)), nullptr);

    // registration of invalid values is not acceptable
    EXPECT_EQ(registry.Register(kEp1, nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registry.Register(kInvalidEndpointId, nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registry.Register(kInvalidEndpointId, &cluster1), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registry.Register(kInvalidEndpointId, &cluster2), CHIP_ERROR_INVALID_ARGUMENT);
    {
        FakeServerClusterInterface badCluster(kInvalidClusterId);
        EXPECT_EQ(registry.Register(kInvalidEndpointId, &badCluster), CHIP_ERROR_INVALID_ARGUMENT);
    }

    // should be able to register
    EXPECT_EQ(registry.Register(kEp1, &cluster1), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(kEp2, &cluster2), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Register(kEp2, &cluster3), CHIP_NO_ERROR);

    // cannot register two implementations on the same path
    {
        FakeServerClusterInterface another1(kCluster1);
        EXPECT_EQ(registry.Register(kEp1, &another1), CHIP_ERROR_DUPLICATE_KEY_ID);
    }

    // Items can be found back
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), &cluster2);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), &cluster3);

    EXPECT_EQ(registry.Get({ kEp2, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp1, kCluster2 }), nullptr);

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

    // Re-adding works exactly once.
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Register(kEp3, &cluster2), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Get({ kEp3, kCluster2 }), &cluster2);
    EXPECT_EQ(registry.Unregister({ kEp3, kCluster2 }), &cluster2);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Unregister({ kEp3, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);

    // cannot get it anymore once removed, others are still valid
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), &cluster3);

    // clean of an entire endpoint works
    registry.UnregisterAllFromEndpoint(kEp2);
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), &cluster1);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), nullptr);

    registry.UnregisterAllFromEndpoint(kEp1);
    EXPECT_EQ(registry.Get({ kEp1, kCluster1 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster2 }), nullptr);
    EXPECT_EQ(registry.Get({ kEp2, kCluster3 }), nullptr);
}

TEST_F(TestServerClusterInterfaceRegistry, StressTest)
{
    // make the test repeatable
    srand(1234);

    std::vector<FakeServerClusterInterface> items;

    static constexpr ClusterId kClusterTestCount   = 200;
    static constexpr EndpointId kEndpointTestCount = 10;
    static constexpr size_t kTestIterations        = 4;

    static_assert(kInvalidClusterId > kClusterTestCount, "Tests assume all clusters IDs [0...] are valid");
    static_assert(kTestIterations > 1, "Tests use different unregister methods. Need 2 or more passes.");

    items.reserve(kClusterTestCount);
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        items.emplace_back(i);
    }

    ServerClusterInterfaceRegistry registry;

    for (size_t test = 0; test < kTestIterations; test++)
    {
        std::vector<EndpointId> endpoint_placement;
        endpoint_placement.reserve(kClusterTestCount);

        for (ClusterId i = 0; i < kClusterTestCount; i++)
        {
            auto endpointId = static_cast<EndpointId>(rand() % kEndpointTestCount);
            endpoint_placement.push_back(endpointId);
            ASSERT_EQ(registry.Register(endpointId, &items[i]), CHIP_NO_ERROR);
        }

        // test that getters work
        for (ClusterId cluster = 0; cluster < kClusterTestCount; cluster++)
        {
            for (EndpointId ep = 0; ep < kEndpointTestCount; ep++)
            {
                if (endpoint_placement[cluster] == ep)
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
                ASSERT_EQ(registry.Get({ endpoint_placement[cluster], static_cast<ClusterId>(cluster) }), &items[cluster]);
                ASSERT_EQ(registry.Unregister({ endpoint_placement[cluster], static_cast<ClusterId>(cluster) }), &items[cluster]);

                // once unregistered, it is not there anymore
                ASSERT_EQ(registry.Get({ endpoint_placement[cluster], static_cast<ClusterId>(cluster) }), nullptr);
                ASSERT_EQ(registry.Unregister({ endpoint_placement[cluster], static_cast<ClusterId>(cluster) }), nullptr);
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

    static constexpr ClusterId kClusterTestCount   = 200;
    static constexpr EndpointId kEndpointTestCount = 10;

    static_assert(kInvalidClusterId > kClusterTestCount, "Tests assume all clusters IDs [0...] are valid");

    items.reserve(kClusterTestCount);
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        items.emplace_back(i);
    }

    ServerClusterInterfaceRegistry registry;

    // place the clusters on the respecitve endpoints
    for (ClusterId i = 0; i < kClusterTestCount; i++)
    {
        ASSERT_EQ(registry.Register(static_cast<EndpointId>(i % kEndpointTestCount), &items[i]), CHIP_NO_ERROR);
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
            ASSERT_EQ(cluster->GetClusterId(), expectedClusterId);
            expectedClusterId -= kEndpointTestCount; // next expected/registered cluster
        }

        // Iterated through all : we overflowed and got a large number
        ASSERT_GE(expectedClusterId, kClusterTestCount);
    }

    // invalid index works and iteration on empty lists is ok
    auto clusters = registry.ClustersOnEndpoint(kEndpointTestCount + 1);
    ASSERT_EQ(clusters.begin(), clusters.end());
}
