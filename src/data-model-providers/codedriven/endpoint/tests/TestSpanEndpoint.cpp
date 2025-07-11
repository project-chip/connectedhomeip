#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>

#include <initializer_list>
#include <vector>

#include <clusters/Descriptor/ClusterId.h>
#include <clusters/OnOff/ClusterId.h>

using namespace chip;
using namespace chip::app;
using chip::app::DataModel::DeviceTypeEntry;
using SemanticTag = chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type;

// Mock ServerClusterInterface for testing
class MockServerCluster : public DefaultServerCluster
{
public:
    MockServerCluster(ConcreteClusterPath path, DataVersion dataVersion, BitFlags<DataModel::ClusterQualityFlags> flags) :
        DefaultServerCluster(path), mPaths({ path }), mDataVersion(dataVersion), mFlags(flags)
    {}

    // New constructor for multiple paths
    MockServerCluster(std::initializer_list<ConcreteClusterPath> paths, DataVersion dataVersion,
                      BitFlags<DataModel::ClusterQualityFlags> flags) :
        DefaultServerCluster(paths.size() > 0 ? *paths.begin() : ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId)),
        mPaths(paths), mDataVersion(dataVersion), mFlags(flags)
    {}

    ~MockServerCluster() override = default; // NOLINT(modernize-use-override)

    chip::Span<const ConcreteClusterPath> GetPaths() const override
    {
        return chip::Span<const ConcreteClusterPath>(mPaths.data(), mPaths.size());
    }
    chip::DataVersion GetDataVersion(const ConcreteClusterPath &) const override { return mDataVersion; }
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override { return mFlags; }

    // Implement the pure virtual functions from DefaultServerCluster
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        return DataModel::ActionReturnStatus(CHIP_ERROR_NOT_IMPLEMENTED);
    }
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        return DataModel::ActionReturnStatus(CHIP_ERROR_NOT_IMPLEMENTED);
    }
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        return std::nullopt;
    }
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<chip::CommandId> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        startupCalled = true;
        return DefaultServerCluster::Startup(context);
    }

    void Shutdown() override
    {
        shutdownCalled = true;
        DefaultServerCluster::Shutdown();
    }

    bool startupCalled  = false;
    bool shutdownCalled = false;

private:
    std::vector<ConcreteClusterPath> mPaths;
    DataVersion mDataVersion;
    BitFlags<DataModel::ClusterQualityFlags> mFlags;
};

using namespace chip::app;

TEST(TestSpanEndpoint, InstantiateDefaultConstructor)
{
    // Test default constructor initializes to invalid/default values
    auto result = SpanEndpoint::Builder(kInvalidEndpointId).Build();
    // Building with kInvalidEndpointId should fail, provider is default constructed
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(result));
    ASSERT_EQ(std::get<CHIP_ERROR>(result), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSpanEndpoint, InstantiateWithParameters)
{
    // Test constructor with parameters initializes correctly
    EndpointId id                                     = 1;
    DataModel::EndpointCompositionPattern composition = DataModel::EndpointCompositionPattern::kTree;
    EndpointId parentId                               = 0;
    MockServerCluster descriptorCluster({ id, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    auto result = SpanEndpoint::Builder(id)
                      .SetComposition(composition)
                      .SetParentId(parentId)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters))
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    const auto & entry    = provider.GetEndpointEntry();
    ASSERT_EQ(entry.id, id);
    ASSERT_EQ(entry.parentId, parentId); // parentId is 0
    ASSERT_EQ(entry.compositionPattern, composition);
}

TEST(TestSpanEndpoint, InstantiateWithAllParameters)
{
    EndpointId id                                     = 1;
    DataModel::EndpointCompositionPattern composition = DataModel::EndpointCompositionPattern::kTree;
    EndpointId parentId                               = 0;

    MockServerCluster descriptorCluster({ id, chip::app::Clusters::Descriptor::Id }, 1, {});
    MockServerCluster serverCluster1({ id, 10 }, 1, {});
    MockServerCluster serverCluster2(ConcreteClusterPath(id, 20), 1, {});

    ServerClusterInterface * serverClusters[] = { &descriptorCluster, &serverCluster1, &serverCluster2 };
    const chip::ClusterId clientClusters[]    = { 10, 20 };
    const SemanticTag semanticTags[]          = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                                  { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    const DeviceTypeEntry deviceTypes[]       = { { .deviceTypeId = 100, .deviceTypeRevision = 1 },
                                                  { .deviceTypeId = 200, .deviceTypeRevision = 2 } };

    auto result = SpanEndpoint::Builder(id)
                      .SetComposition(composition)
                      .SetParentId(parentId)
                      .SetServerClusters(Span<chip::app::ServerClusterInterface *>(serverClusters))
                      .SetClientClusters(Span<const chip::ClusterId>(clientClusters))
                      .SetSemanticTags(Span<const SemanticTag>(semanticTags))
                      .SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes))
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    const auto & entry    = provider.GetEndpointEntry();
    ASSERT_EQ(entry.id, id);
    ASSERT_EQ(entry.parentId, parentId); // parentId is 0
    ASSERT_EQ(entry.compositionPattern, composition);

    ReadOnlyBufferBuilder<chip::app::ServerClusterInterface *> serverBuilder;
    ASSERT_EQ(provider.ServerClusters(serverBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(serverBuilder.Size(), std::size(serverClusters));

    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(provider.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), std::size(clientClusters));

    ReadOnlyBufferBuilder<SemanticTag> tagBuilder;
    ASSERT_EQ(provider.SemanticTags(tagBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(tagBuilder.Size(), std::size(semanticTags));

    ReadOnlyBufferBuilder<DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(provider.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), std::size(deviceTypes));
}

TEST(TestSpanEndpoint, SetAndGetServerClusters)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    MockServerCluster serverCluster1(ConcreteClusterPath(kTestEndpoint, 1), 1, {});
    MockServerCluster serverCluster2(ConcreteClusterPath(kTestEndpoint, 2), 1, {});

    ServerClusterInterface * serverClusters[] = { &descriptorCluster, &serverCluster1, &serverCluster2 };

    auto result = SpanEndpoint::Builder(kTestEndpoint).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    ReadOnlyBufferBuilder<ServerClusterInterface *> builder;
    ASSERT_EQ(provider.ServerClusters(builder), CHIP_NO_ERROR);
    auto retrievedClusters = builder.TakeBuffer();
    ASSERT_EQ(retrievedClusters.size(), std::size(serverClusters));
    EXPECT_EQ(retrievedClusters[0], &descriptorCluster);
    EXPECT_EQ(retrievedClusters[1], &serverCluster1);
    EXPECT_EQ(retrievedClusters[2], &serverCluster2);

    EXPECT_EQ(provider.GetServerCluster(chip::app::Clusters::Descriptor::Id), &descriptorCluster);
    EXPECT_EQ(provider.GetServerCluster(1), &serverCluster1);
    EXPECT_EQ(provider.GetServerCluster(2), &serverCluster2);
    EXPECT_EQ(provider.GetServerCluster(3), nullptr); // Non-existent
}

TEST(TestSpanEndpoint, SetInvalidServerClusters)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    MockServerCluster serverCluster1(ConcreteClusterPath(kTestEndpoint, 1), 1, {});

    ServerClusterInterface * serverClustersWithNull[] = { &descriptorCluster, &serverCluster1, nullptr };
    auto result =
        SpanEndpoint::Builder(kTestEndpoint).SetServerClusters(Span<ServerClusterInterface *>(serverClustersWithNull)).Build();
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(result));
    EXPECT_EQ(std::get<CHIP_ERROR>(result), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSpanEndpoint, SetAndGetClientClusters)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    const chip::ClusterId clientClusters[] = { 10, 20, 30 };
    auto result                            = SpanEndpoint::Builder(kTestEndpoint)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters))
                      .SetClientClusters(Span<const chip::ClusterId>(clientClusters))
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    ReadOnlyBufferBuilder<chip::ClusterId> builder;
    ASSERT_EQ(provider.ClientClusters(builder), CHIP_NO_ERROR);
    auto retrievedClusters = builder.TakeBuffer();

    ASSERT_EQ(retrievedClusters.size(), std::size(clientClusters));
    EXPECT_TRUE(retrievedClusters.data_equal(Span<const chip::ClusterId>(clientClusters, std::size(clientClusters))));
}

TEST(TestSpanEndpoint, SetAndGetSemanticTags)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    const SemanticTag semanticTags[] = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                         { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    auto result                      = SpanEndpoint::Builder(kTestEndpoint)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters))
                      .SetSemanticTags(Span<const SemanticTag>(semanticTags))
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    ReadOnlyBufferBuilder<SemanticTag> builder;
    ASSERT_EQ(provider.SemanticTags(builder), CHIP_NO_ERROR);
    auto retrievedTags = builder.TakeBuffer();
    ASSERT_EQ(retrievedTags.size(), std::size(semanticTags));
    // SemanticTagStruct does not have operator== by default, manual check or compare members if needed
    EXPECT_EQ(retrievedTags[0].mfgCode, semanticTags[0].mfgCode);
    EXPECT_EQ(retrievedTags[1].namespaceID, semanticTags[1].namespaceID);
}

TEST(TestSpanEndpoint, SetAndGetDeviceTypes)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    const DeviceTypeEntry deviceTypes[] = { { .deviceTypeId = 100, .deviceTypeRevision = 1 },
                                            { .deviceTypeId = 200, .deviceTypeRevision = 2 } };
    auto result                         = SpanEndpoint::Builder(kTestEndpoint)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters))
                      .SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes))
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider = std::get<SpanEndpoint>(result);
    ReadOnlyBufferBuilder<DeviceTypeEntry> builder;
    ASSERT_EQ(provider.DeviceTypes(builder), CHIP_NO_ERROR);
    auto retrievedDeviceTypes = builder.TakeBuffer();
    ASSERT_EQ(retrievedDeviceTypes.size(), std::size(deviceTypes));
    EXPECT_EQ(retrievedDeviceTypes[0], deviceTypes[0]);
    EXPECT_EQ(retrievedDeviceTypes[1], deviceTypes[1]);
}

TEST(TestSpanEndpoint, BuildWithSpecificEndpointEntry)
{
    DataModel::EndpointEntry newEntry = { .id                 = 10,
                                          .parentId           = 1,
                                          .compositionPattern = DataModel::EndpointCompositionPattern::kTree };
    MockServerCluster descriptorCluster({ newEntry.id, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    auto result = SpanEndpoint::Builder(newEntry.id)
                      .SetParentId(newEntry.parentId)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters))
                      .SetComposition(newEntry.compositionPattern)
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));

    const auto & provider       = std::get<SpanEndpoint>(result);
    const auto & retrievedEntry = provider.GetEndpointEntry();
    ASSERT_EQ(retrievedEntry.id, newEntry.id);
    ASSERT_EQ(retrievedEntry.parentId, newEntry.parentId);
    ASSERT_EQ(retrievedEntry.compositionPattern, newEntry.compositionPattern);
}

TEST(TestSpanEndpoint, BuildWithEmptySpans)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &descriptorCluster };

    auto result = SpanEndpoint::Builder(kTestEndpoint)
                      .SetServerClusters(Span<ServerClusterInterface *>(serverClusters)) // Provide minimal valid server clusters
                      .SetClientClusters(Span<const chip::ClusterId>())
                      .SetSemanticTags(Span<const SemanticTag>())
                      .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));
    const auto & provider = std::get<SpanEndpoint>(result);

    // Verify server clusters are present (at least the descriptor)
    ReadOnlyBufferBuilder<ServerClusterInterface *> serverBuilder;
    ASSERT_EQ(provider.ServerClusters(serverBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(serverBuilder.Size(), 1u);                // Should have the descriptor cluster
    ASSERT_EQ(provider.GetServerCluster(123), nullptr); // Check GetServerCluster with no clusters

    // Test SetClientClusters with an empty span
    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(provider.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), 0u);

    // Test SetSemanticTags with an empty span
    ReadOnlyBufferBuilder<SemanticTag> tagBuilder;
    ASSERT_EQ(provider.SemanticTags(tagBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(tagBuilder.Size(), 0u);

    // Test SetDeviceTypes with an empty span
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(provider.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), 0u);
}

TEST(TestSpanEndpoint, BuildFailsWithEmptyServerClusterList)
{
    constexpr EndpointId kTestEndpoint = 1; // A valid endpoint ID for testing
    auto result                        = SpanEndpoint::Builder(kTestEndpoint)
                      .SetServerClusters(Span<ServerClusterInterface *>()) // Empty span
                      .Build();
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(result)) << "Build should fail with an empty server cluster list.";
    // This assertion expects that the Build() method (or future versions) will validate this.
    EXPECT_EQ(std::get<CHIP_ERROR>(result), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSpanEndpoint, BuildFailsWithoutDescriptorCluster)
{
    constexpr EndpointId kTestEndpoint = 1; // A valid endpoint ID for testing
    MockServerCluster onOffCluster({ kTestEndpoint, chip::app::Clusters::OnOff::Id }, 1, {});
    ServerClusterInterface * serverClusters[] = { &onOffCluster };

    auto result = SpanEndpoint::Builder(kTestEndpoint).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(result)) << "Build should fail without Descriptor cluster.";
    // This assertion expects that the Build() method (or future versions) will validate this.
    EXPECT_EQ(std::get<CHIP_ERROR>(result), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSpanEndpoint, BuildSucceedsWithDescriptorCluster)
{
    constexpr EndpointId kTestEndpoint = 1; // A valid endpoint ID for testing
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    MockServerCluster onOffCluster({ kTestEndpoint, chip::app::Clusters::OnOff::Id }, 1, {}); // Another optional cluster

    ServerClusterInterface * serverClusters[] = { &descriptorCluster, &onOffCluster };

    auto result = SpanEndpoint::Builder(kTestEndpoint).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result))
        << "Build should succeed with Descriptor cluster. Error: "
        << (std::holds_alternative<CHIP_ERROR>(result) ? chip::ErrorStr(std::get<CHIP_ERROR>(result)) : "Not a CHIP_ERROR");

    if (std::holds_alternative<SpanEndpoint>(result))
    {
        const auto & provider = std::get<SpanEndpoint>(result);
        EXPECT_EQ(provider.GetEndpointEntry().id, kTestEndpoint);
        EXPECT_NE(provider.GetServerCluster(chip::app::Clusters::Descriptor::Id), nullptr);
        EXPECT_NE(provider.GetServerCluster(chip::app::Clusters::OnOff::Id), nullptr);
    }
}

TEST(TestSpanEndpoint, MultiPathClusterPathResolution)
{
    constexpr EndpointId kEndpointA   = 1;
    constexpr EndpointId kEndpointB   = 2;
    constexpr ClusterId kDescriptorId = chip::app::Clusters::Descriptor::Id;
    constexpr ClusterId kOnOffId      = chip::app::Clusters::OnOff::Id;

    // Scenario 1: Test Builder::Build() succeeds even if the Descriptor cluster path is not first.
    {
        // This cluster interface provides the Descriptor cluster for Endpoint A,
        // but a path for another endpoint/cluster is listed first.
        MockServerCluster descriptorCluster({ { kEndpointB, kOnOffId }, // Path for another endpoint/cluster is first.
                                              { kEndpointA, kDescriptorId } },
                                            1, {});
        ServerClusterInterface * serverClusters[] = { &descriptorCluster };

        // Try to build an endpoint for Endpoint A.
        auto buildResult =
            SpanEndpoint::Builder(kEndpointA).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();

        // A correct implementation iterates, finds `{kEndpointA, kDescriptorId}`, and succeeds.
        ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(buildResult))
            << "Build should succeed because the implementation now correctly checks all paths for the Descriptor cluster.";
    }

    // Scenario 2: Test GetServerCluster() succeeds even if the desired cluster path is not first.
    {
        // This cluster interface provides the OnOff cluster for Endpoint A,
        // but a path for another endpoint/cluster is listed first.
        MockServerCluster onOffCluster({ { kEndpointB, kDescriptorId }, // Path for another endpoint/cluster is first.
                                         { kEndpointA, kOnOffId } },
                                       1, {});
        // We also need a valid descriptor cluster for Endpoint A to successfully build the endpoint.
        MockServerCluster descriptorForEpA({ kEndpointA, kDescriptorId }, 1, {});
        ServerClusterInterface * serverClusters[] = { &descriptorForEpA, &onOffCluster };

        // Build an endpoint for Endpoint A. This should succeed.
        auto buildResult =
            SpanEndpoint::Builder(kEndpointA).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
        ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(buildResult)) << "Build for GetServerCluster test failed unexpectedly.";
        const auto & provider = std::get<SpanEndpoint>(buildResult);

        // Try to get the OnOff cluster.
        ServerClusterInterface * retrievedCluster = provider.GetServerCluster(kOnOffId);

        // A correct implementation iterates through all paths of `onOffCluster`, finds `{kEndpointA, kOnOffId}`,
        // and returns `&onOffCluster`.
        EXPECT_EQ(retrievedCluster, &onOffCluster)
            << "GetServerCluster should succeed because the implementation now correctly checks all paths.";
    }
}

TEST(TestSpanEndpoint, BuildFailsWithInvalidClusterAfterDescriptor)
{
    constexpr EndpointId kTestEndpoint = 1;
    MockServerCluster descriptorCluster({ kTestEndpoint, chip::app::Clusters::Descriptor::Id }, 1, {});
    MockServerCluster onOffCluster({ kTestEndpoint, chip::app::Clusters::OnOff::Id }, 1, {});

    // This list has the descriptor first, but an invalid cluster later.
    // The builder must iterate through the entire list to validate all entries.
    ServerClusterInterface * serverClusters[] = { &descriptorCluster, &onOffCluster, nullptr };

    auto result = SpanEndpoint::Builder(kTestEndpoint).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();

    // The build should fail because of the nullptr entry, even though the descriptor was found.
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(result))
        << "Build should fail due to an invalid cluster even if the descriptor is found first.";
    EXPECT_EQ(std::get<CHIP_ERROR>(result), CHIP_ERROR_INVALID_ARGUMENT);
}
