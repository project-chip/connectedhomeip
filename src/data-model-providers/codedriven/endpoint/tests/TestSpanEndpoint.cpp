#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>

#include <initializer_list>
#include <vector>

#include <clusters/Descriptor/ClusterId.h>
#include <clusters/OnOff/ClusterId.h>

using namespace chip;
using namespace chip::app;
using chip::app::DataModel::DeviceTypeEntry;
using SemanticTag = chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type;

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

    auto result = SpanEndpoint::Builder(id).SetComposition(composition).SetParentId(parentId).Build();
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

    const chip::ClusterId clientClusters[] = { 10, 20 };
    const SemanticTag semanticTags[]       = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                               { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    const DeviceTypeEntry deviceTypes[]    = { { .deviceTypeId = 100, .deviceTypeRevision = 1 },
                                               { .deviceTypeId = 200, .deviceTypeRevision = 2 } };

    auto result = SpanEndpoint::Builder(id)
                      .SetComposition(composition)
                      .SetParentId(parentId)
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

TEST(TestSpanEndpoint, SetAndGetClientClusters)
{
    constexpr EndpointId kTestEndpoint = 1;

    const chip::ClusterId clientClusters[] = { 10, 20, 30 };
    auto result = SpanEndpoint::Builder(kTestEndpoint).SetClientClusters(Span<const chip::ClusterId>(clientClusters)).Build();
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

    const SemanticTag semanticTags[] = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                         { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    auto result = SpanEndpoint::Builder(kTestEndpoint).SetSemanticTags(Span<const SemanticTag>(semanticTags)).Build();
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

    const DeviceTypeEntry deviceTypes[] = { { .deviceTypeId = 100, .deviceTypeRevision = 1 },
                                            { .deviceTypeId = 200, .deviceTypeRevision = 2 } };
    auto result = SpanEndpoint::Builder(kTestEndpoint).SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes)).Build();
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

    auto result =
        SpanEndpoint::Builder(newEntry.id).SetParentId(newEntry.parentId).SetComposition(newEntry.compositionPattern).Build();
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

    auto result = SpanEndpoint::Builder(kTestEndpoint)
                      .SetClientClusters(Span<const chip::ClusterId>())
                      .SetSemanticTags(Span<const SemanticTag>())
                      .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                      .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(result));
    const auto & provider = std::get<SpanEndpoint>(result);

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
