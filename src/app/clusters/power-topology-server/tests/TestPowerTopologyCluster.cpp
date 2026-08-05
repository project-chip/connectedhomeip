/*
 *
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

#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/power-topology-server/DefaultPowerTopologyCircuitNodeStorage.h>
#include <app/clusters/power-topology-server/PowerTopologyCircuitNodeStorage.h>
#include <app/clusters/power-topology-server/tests/MockPowerTopologyDelegate.h>
#include <app/data-model/List.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PowerTopology/Attributes.h>
#include <clusters/PowerTopology/Events.h>
#include <clusters/PowerTopology/Metadata.h>
#include <clusters/PowerTopology/Structs.h>

#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;
using namespace chip::Testing;

namespace {

using IMStatus = Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

// Build a CircuitNodeStruct for a write. The incoming fabricIndex is ignored by the cluster (the
// accessing fabric of the write is authoritative), so it is left unset here.
Structs::CircuitNodeStruct::Type MakeCircuitNode(NodeId node, Optional<EndpointId> endpoint, Optional<CharSpan> label)
{
    Structs::CircuitNodeStruct::Type value;
    value.node     = node;
    value.endpoint = endpoint;
    value.label    = label;
    return value;
}

// A CircuitNodeStorage backed by a fixed array, with no persistence and no allocation of any kind.
//
// Besides keeping the tests independent of attribute storage, this is the worked example of what a
// memory-constrained platform would supply instead of DefaultCircuitNodeStorage: it holds a plain
// C array and reports its real limit through Capacity().
class FixedCircuitNodeStorage : public CircuitNodeStorage
{
public:
    size_t Capacity() const override { return kCapacity; }
    size_t Count() const override { return mCount; }

    size_t CountForFabric(FabricIndex fabricIndex) const override
    {
        size_t count = 0;
        for (size_t i = 0; i < mCount; i++)
        {
            count += (mNodes[i].fabricIndex == fabricIndex) ? 1 : 0;
        }
        return count;
    }

    CHIP_ERROR GetNodeAtIndex(size_t index, Node & outNode) const override
    {
        VerifyOrReturnError(index < mCount, CHIP_ERROR_INVALID_ARGUMENT);
        outNode = mNodes[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReplaceNodesForFabric(FabricIndex fabricIndex, const Node * nodes, size_t count) override
    {
        VerifyOrReturnError((mCount - CountForFabric(fabricIndex)) + count <= kCapacity, CHIP_ERROR_NO_MEMORY);
        Erase(fabricIndex);
        for (size_t i = 0; i < count; i++)
        {
            mNodes[mCount++] = nodes[i];
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AppendNode(const Node & node) override
    {
        VerifyOrReturnError(mCount < kCapacity, CHIP_ERROR_NO_MEMORY);
        mNodes[mCount++] = node;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveNodesForFabric(FabricIndex fabricIndex) override
    {
        Erase(fabricIndex);
        return CHIP_NO_ERROR;
    }

private:
    static constexpr size_t kCapacity = CircuitNodeStorage::kMaxCircuitNodes;

    void Erase(FabricIndex fabricIndex)
    {
        size_t kept = 0;
        for (size_t i = 0; i < mCount; i++)
        {
            if (mNodes[i].fabricIndex != fabricIndex)
            {
                if (kept != i)
                {
                    mNodes[kept] = mNodes[i];
                }
                kept++;
            }
        }
        mCount = kept;
    }

    Node mNodes[kCapacity];
    size_t mCount = 0;
};

struct TestPowerTopologyCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

TEST_F(TestPowerTopologyCluster, EmptyAttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> noFeatures;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = noFeatures,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, {}));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, AllAttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> allFeatures(Feature::kNodeTopology, Feature::kTreeTopology, Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = allFeatures,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::AvailableEndpoints::kMetadataEntry,
                                            Attributes::ActiveEndpoints::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, NoActiveEndpointsWithoutDYPFTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::AvailableEndpoints::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, OnlyDynamicPowerFlowFeatureTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, FeatureMapReadTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    FeatureMap::TypeInfo::DecodableType featureMapValue{};
    ASSERT_TRUE(tester.ReadAttribute(FeatureMap::Id, featureMapValue).IsSuccess());
    EXPECT_EQ(featureMapValue, features.Raw());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsEmptyTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.ClearAvailableEndpoints(); // Empty list

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsSingleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpoints({ 5 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 5u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsMultipleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpoints({ 1, 2, 5, 10 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 1u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 2u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 5u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 10u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, ReadActiveEndpointsEmptyTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.ClearActiveEndpoints();

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = activeEndpoints.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, ReadActiveEndpointsMultipleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetActiveEndpoints({ 3, 7, 9 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = activeEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 3u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 7u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 9u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, DelegateAvailableEndpointsErrorTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpointsError(CHIP_ERROR_INTERNAL);

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_FALSE(status.IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPowerTopologyCluster, DelegateActiveEndpointsErrorTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetActiveEndpointsError(CHIP_ERROR_INTERNAL);

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_FALSE(status.IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// --- ElectricalCircuitNodes (CIRC feature) -------------------------------------------------------

// ElectricalCircuitNodes appears in the attribute list only when the CIRC feature is enabled.
TEST_F(TestPowerTopologyCluster, ElectricalCircuitNodesGatedByCircFeatureTest)
{
    MockPowerTopologyDelegate dummyDelegate;

    {
        chip::Testing::TestServerClusterContext context;
        BitMask<Feature> noCirc;
        PowerTopologyCluster cluster(PowerTopologyCluster::Config{
            .endpointId = kTestEndpointId,
            .delegate   = dummyDelegate,
            .features   = noCirc,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(IsAttributesListEqualTo(cluster, {}));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        chip::Testing::TestServerClusterContext context;
        BitMask<Feature> circOnly(Feature::kElectricalCircuit);
        FixedCircuitNodeStorage circuitStorage1;
        PowerTopologyCluster cluster(PowerTopologyCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = dummyDelegate,
            .features           = circOnly,
            .circuitNodeStorage = &circuitStorage1,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::ElectricalCircuitNodes::kMetadataEntry,
                                            }));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// A CIRC cluster with no writes reports an empty ElectricalCircuitNodes list.
TEST_F(TestPowerTopologyCluster, ReadElectricalCircuitNodesEmptyTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage2;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage2,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

    auto iter = nodes.begin();
    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A ReplaceAll write of two nodes round-trips: fields are preserved and each node carries the
// accessing fabric index.
TEST_F(TestPowerTopologyCluster, WriteAndReadElectricalCircuitNodesReplaceAllTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage3;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage3,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    Structs::CircuitNodeStruct::Type toWrite[] = {
        MakeCircuitNode(0x1111'2222'3333'4444, MakeOptional<EndpointId>(static_cast<EndpointId>(3)),
                        MakeOptional(CharSpan::fromCharString("main"))),
        MakeCircuitNode(0x5555'6666'7777'8888, NullOptional, NullOptional),
    };
    DataModel::List<Structs::CircuitNodeStruct::Type> list(toWrite);

    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, list, ListWritingPattern::ReplaceAll), IMStatus::Success);

    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

    auto iter = nodes.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().node, 0x1111'2222'3333'4444u);
    ASSERT_TRUE(iter.GetValue().endpoint.HasValue());
    EXPECT_EQ(iter.GetValue().endpoint.Value(), 3u);
    ASSERT_TRUE(iter.GetValue().label.HasValue());
    EXPECT_TRUE(iter.GetValue().label.Value().data_equal(CharSpan::fromCharString("main")));
    EXPECT_EQ(iter.GetValue().fabricIndex, kTestFabricIndex);

    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().node, 0x5555'6666'7777'8888u);
    EXPECT_FALSE(iter.GetValue().endpoint.HasValue());
    EXPECT_FALSE(iter.GetValue().label.HasValue());
    EXPECT_EQ(iter.GetValue().fabricIndex, kTestFabricIndex);

    EXPECT_FALSE(iter.Next());
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The clear-then-append write pattern produces the same result as ReplaceAll.
TEST_F(TestPowerTopologyCluster, WriteElectricalCircuitNodesAppendItemTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage4;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage4,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    Structs::CircuitNodeStruct::Type toWrite[] = {
        MakeCircuitNode(0xAAAA, MakeOptional<EndpointId>(static_cast<EndpointId>(7)), NullOptional),
        MakeCircuitNode(0xBBBB, NullOptional, MakeOptional(CharSpan::fromCharString("branch"))),
    };
    DataModel::List<Structs::CircuitNodeStruct::Type> list(toWrite);

    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, list, ListWritingPattern::ClearAllThenAppendItems),
              IMStatus::Success);

    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

    size_t count = 0;
    auto iter    = nodes.begin();
    while (iter.Next())
    {
        count++;
    }
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A ReplaceAll write replaces only the accessing fabric's slice; other fabrics' nodes survive.
TEST_F(TestPowerTopologyCluster, WriteElectricalCircuitNodesFabricIsolationTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage5;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage5,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Fabric 1 writes one node.
    Structs::CircuitNodeStruct::Type fabric1[] = { MakeCircuitNode(0x1001, NullOptional, NullOptional) };
    tester.SetFabricIndex(1);
    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(fabric1),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::Success);

    // Fabric 2 writes two nodes.
    Structs::CircuitNodeStruct::Type fabric2[] = { MakeCircuitNode(0x2001, NullOptional, NullOptional),
                                                   MakeCircuitNode(0x2002, NullOptional, NullOptional) };
    tester.SetFabricIndex(2);
    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(fabric2),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::Success);

    // Fabric 1 replaces its slice with a different single node; fabric 2's nodes are untouched.
    Structs::CircuitNodeStruct::Type fabric1New[] = { MakeCircuitNode(0x1002, NullOptional, NullOptional) };
    tester.SetFabricIndex(1);
    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(fabric1New),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::Success);

    // Unfiltered read (ClusterTester does not fabric-filter) returns all three nodes with true fabric indices.
    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

    size_t fabric1Count = 0;
    size_t fabric2Count = 0;
    bool sawOldFabric1  = false;
    auto iter           = nodes.begin();
    while (iter.Next())
    {
        const auto & value = iter.GetValue();
        if (value.fabricIndex == 1)
        {
            fabric1Count++;
            EXPECT_EQ(value.node, 0x1002u);
        }
        else if (value.fabricIndex == 2)
        {
            fabric2Count++;
        }
        if (value.node == 0x1001u)
        {
            sawOldFabric1 = true;
        }
    }
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(fabric1Count, 1u);
    EXPECT_EQ(fabric2Count, 2u);
    EXPECT_FALSE(sawOldFabric1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Persisted nodes are restored when a fresh cluster instance starts up against the same storage.
TEST_F(TestPowerTopologyCluster, ElectricalCircuitNodesPersistAcrossRestartTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage6;
    MockPowerTopologyDelegate dummyDelegate;

    Structs::CircuitNodeStruct::Type toWrite[] = {
        MakeCircuitNode(0xDEAD'BEEF, MakeOptional<EndpointId>(static_cast<EndpointId>(9)),
                        MakeOptional(CharSpan::fromCharString("persisted"))),
    };

    // First lifetime: write and shut down.
    {
        PowerTopologyCluster cluster(PowerTopologyCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = dummyDelegate,
            .features           = features,
            .circuitNodeStorage = &circuitStorage6,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(toWrite),
                                        ListWritingPattern::ReplaceAll),
                  IMStatus::Success);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Second lifetime: same storage, a new instance must see the retained nodes on startup.
    {
        PowerTopologyCluster cluster(PowerTopologyCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = dummyDelegate,
            .features           = features,
            .circuitNodeStorage = &circuitStorage6,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
        EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

        auto iter = nodes.begin();
        ASSERT_TRUE(iter.Next());
        EXPECT_EQ(iter.GetValue().node, 0xDEAD'BEEFu);
        ASSERT_TRUE(iter.GetValue().endpoint.HasValue());
        EXPECT_EQ(iter.GetValue().endpoint.Value(), 9u);
        ASSERT_TRUE(iter.GetValue().label.HasValue());
        EXPECT_TRUE(iter.GetValue().label.Value().data_equal(CharSpan::fromCharString("persisted")));
        EXPECT_EQ(iter.GetValue().fabricIndex, kTestFabricIndex);
        EXPECT_FALSE(iter.Next());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Removing a fabric purges only that fabric's ElectricalCircuitNodes; other fabrics' nodes survive.
TEST_F(TestPowerTopologyCluster, ElectricalCircuitNodesPurgedOnFabricRemovalTest)
{
    chip::Testing::TestServerClusterContext context;
    chip::Testing::FabricTestFixture fabricHelper{ &context.StorageDelegate() };

    FabricIndex fabric1 = kTestFabricIndex; // 1
    ASSERT_EQ(fabricHelper.SetUpTestFabric(fabric1), CHIP_NO_ERROR);
    ASSERT_EQ(fabric1, kTestFabricIndex);
    FabricIndex fabric2 = static_cast<FabricIndex>(kTestFabricIndex + 1); // 2
    ASSERT_EQ(fabricHelper.AddAdditionalTestFabric(fabric2), CHIP_NO_ERROR);

    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage7;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .fabricTable        = &fabricHelper.GetFabricTable(),
        .circuitNodeStorage = &circuitStorage7,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // fabric1 writes two nodes; fabric2 writes one node.
    Structs::CircuitNodeStruct::Type f1[] = { MakeCircuitNode(0x1001, NullOptional, NullOptional),
                                              MakeCircuitNode(0x1002, NullOptional, NullOptional) };
    tester.SetFabricIndex(fabric1);
    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(f1),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::Success);

    Structs::CircuitNodeStruct::Type f2[] = { MakeCircuitNode(0x2001, NullOptional, NullOptional) };
    tester.SetFabricIndex(fabric2);
    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(f2),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::Success);

    // Removing fabric1 fires OnFabricRemoved; fabric1's nodes must be purged, fabric2's must remain.
    EXPECT_EQ(fabricHelper.GetFabricTable().Delete(fabric1), CHIP_NO_ERROR);

    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());

    size_t count = 0;
    auto iter    = nodes.begin();
    while (iter.Next())
    {
        count++;
        EXPECT_EQ(iter.GetValue().fabricIndex, fabric2);
        EXPECT_EQ(iter.GetValue().node, 0x2001u);
    }
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A node label longer than the spec maximum is rejected with ConstraintError.
TEST_F(TestPowerTopologyCluster, WriteElectricalCircuitNodesLabelTooLongTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage8;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage8,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    std::string tooLong(PowerTopologyCluster::kMaxNodeLabelLength + 1, 'x');
    Structs::CircuitNodeStruct::Type toWrite[] = {
        MakeCircuitNode(0x1234, NullOptional, MakeOptional(CharSpan(tooLong.data(), tooLong.size()))),
    };

    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id, DataModel::List<Structs::CircuitNodeStruct::Type>(toWrite),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::ConstraintError);

    // The rejected write must not have mutated state.
    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());
    auto iter = nodes.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Writing more nodes than the list maximum is rejected with ResourceExhausted.
TEST_F(TestPowerTopologyCluster, WriteElectricalCircuitNodesResourceExhaustedTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kElectricalCircuit);
    FixedCircuitNodeStorage circuitStorage9;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = dummyDelegate,
        .features           = features,
        .circuitNodeStorage = &circuitStorage9,
    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    std::vector<Structs::CircuitNodeStruct::Type> tooMany;
    for (size_t i = 0; i < PowerTopologyCluster::kMaxCircuitNodes + 1; i++)
    {
        tooMany.push_back(MakeCircuitNode(static_cast<NodeId>(i + 1), NullOptional, NullOptional));
    }

    EXPECT_EQ(tester.WriteAttribute(ElectricalCircuitNodes::Id,
                                    DataModel::List<Structs::CircuitNodeStruct::Type>(tooMany.data(), tooMany.size()),
                                    ListWritingPattern::ReplaceAll),
              IMStatus::ResourceExhausted);

    // The rejected write must not have mutated state.
    ElectricalCircuitNodes::TypeInfo::DecodableType nodes;
    EXPECT_TRUE(tester.ReadAttribute(ElectricalCircuitNodes::Id, nodes).IsSuccess());
    auto iter = nodes.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace

// DefaultCircuitNodeStorage owns the persistence that used to live in the cluster, so it needs its
// own round-trip coverage: the cluster-level tests above deliberately use a non-persisting storage.
TEST_F(TestPowerTopologyCluster, DefaultCircuitNodeStoragePersistsAcrossInstances)
{
    chip::Testing::TestServerClusterContext context;

    CircuitNodeStorage::Node node;
    node.node        = 0xABCD'1234;
    node.fabricIndex = 7;
    node.endpoint.SetValue(static_cast<EndpointId>(4));
    node.hasLabel = true;
    memcpy(node.label, "durable", 7);
    node.labelLength = 7;

    // First instance: store one node, which persists through attribute storage.
    {
        DefaultCircuitNodeStorage storage;
        ASSERT_EQ(storage.Init(context.Get().attributeStorage, kTestEndpointId), CHIP_NO_ERROR);
        EXPECT_EQ(storage.Count(), 0u);
        ASSERT_EQ(storage.ReplaceNodesForFabric(7, &node, 1), CHIP_NO_ERROR);
        EXPECT_EQ(storage.Count(), 1u);
    }

    // Second instance over the same attribute storage must reload it, fields intact.
    {
        DefaultCircuitNodeStorage storage;
        ASSERT_EQ(storage.Init(context.Get().attributeStorage, kTestEndpointId), CHIP_NO_ERROR);
        ASSERT_EQ(storage.Count(), 1u);

        CircuitNodeStorage::Node loaded;
        ASSERT_EQ(storage.GetNodeAtIndex(0, loaded), CHIP_NO_ERROR);
        EXPECT_EQ(loaded.node, 0xABCD'1234u);
        EXPECT_EQ(loaded.fabricIndex, 7);
        ASSERT_TRUE(loaded.endpoint.HasValue());
        EXPECT_EQ(loaded.endpoint.Value(), 4u);
        ASSERT_TRUE(loaded.hasLabel);
        EXPECT_TRUE(CharSpan(loaded.label, loaded.labelLength).data_equal(CharSpan::fromCharString("durable")));

        // Removing the fabric clears it, and that too is persisted.
        ASSERT_EQ(storage.RemoveNodesForFabric(7), CHIP_NO_ERROR);
        EXPECT_EQ(storage.Count(), 0u);
    }

    {
        DefaultCircuitNodeStorage storage;
        ASSERT_EQ(storage.Init(context.Get().attributeStorage, kTestEndpointId), CHIP_NO_ERROR);
        EXPECT_EQ(storage.Count(), 0u);
    }
}
