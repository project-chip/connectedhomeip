#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>

#include <vector>

#include <clusters/Descriptor/ClusterId.h>
#include <clusters/OnOff/ClusterId.h>
#include <devices/Types.h>

using namespace chip;
using namespace chip::app;
using chip::app::DataModel::DeviceTypeEntry;
using SemanticTag = chip::app::Clusters::Globals::Structs::SemanticTagStruct::Type;

using namespace chip::app;

TEST(TestSpanEndpoint, InstantiateWithAllParameters)
{
    const chip::ClusterId clientClusters[] = { 10, 20 };
    const SemanticTag semanticTags[]       = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                               { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    const DeviceTypeEntry deviceTypes[]    = { Device::Type::kDoorLock, Device::Type::kSpeaker };

    auto endpoint = SpanEndpoint::Builder()
                        .SetClientClusters(Span<const chip::ClusterId>(clientClusters))
                        .SetSemanticTags(Span<const SemanticTag>(semanticTags))
                        .SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes))
                        .Build();

    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(endpoint.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), std::size(clientClusters));

    ReadOnlyBufferBuilder<SemanticTag> tagBuilder;
    ASSERT_EQ(endpoint.SemanticTags(tagBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(tagBuilder.Size(), std::size(semanticTags));

    ReadOnlyBufferBuilder<DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(endpoint.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), std::size(deviceTypes));
}

TEST(TestSpanEndpoint, SetAndGetClientClusters)
{
    const chip::ClusterId clientClusters[] = { 10, 20, 30 };
    auto endpoint = SpanEndpoint::Builder().SetClientClusters(Span<const chip::ClusterId>(clientClusters)).Build();
    ReadOnlyBufferBuilder<chip::ClusterId> builder;
    ASSERT_EQ(endpoint.ClientClusters(builder), CHIP_NO_ERROR);
    auto retrievedClusters = builder.TakeBuffer();

    ASSERT_EQ(retrievedClusters.size(), std::size(clientClusters));
    EXPECT_TRUE(retrievedClusters.data_equal(Span<const chip::ClusterId>(clientClusters, std::size(clientClusters))));
}

TEST(TestSpanEndpoint, SetAndGetSemanticTags)
{
    const SemanticTag semanticTags[] = { { .mfgCode = chip::VendorId::TestVendor1, .namespaceID = 1, .tag = 1 },
                                         { .mfgCode = chip::VendorId::TestVendor2, .namespaceID = 2, .tag = 2 } };
    auto endpoint                    = SpanEndpoint::Builder().SetSemanticTags(Span<const SemanticTag>(semanticTags)).Build();
    ReadOnlyBufferBuilder<SemanticTag> builder;
    ASSERT_EQ(endpoint.SemanticTags(builder), CHIP_NO_ERROR);
    auto retrievedTags = builder.TakeBuffer();
    ASSERT_EQ(retrievedTags.size(), std::size(semanticTags));
    // SemanticTagStruct does not have operator== by default, manual check or compare members if needed
    EXPECT_EQ(retrievedTags[0].mfgCode, semanticTags[0].mfgCode);
    EXPECT_EQ(retrievedTags[1].namespaceID, semanticTags[1].namespaceID);
}

TEST(TestSpanEndpoint, SetAndGetDeviceTypes)
{
    const DeviceTypeEntry deviceTypes[] = { Device::Type::kAirPurifier, Device::Type::kClosure };

    auto endpoint = SpanEndpoint::Builder().SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes)).Build();
    ReadOnlyBufferBuilder<DeviceTypeEntry> builder;
    ASSERT_EQ(endpoint.DeviceTypes(builder), CHIP_NO_ERROR);
    auto retrievedDeviceTypes = builder.TakeBuffer();
    ASSERT_EQ(retrievedDeviceTypes.size(), std::size(deviceTypes));
    EXPECT_EQ(retrievedDeviceTypes[0], deviceTypes[0]);
    EXPECT_EQ(retrievedDeviceTypes[1], deviceTypes[1]);
}

TEST(TestSpanEndpoint, BuildWithEmptySpans)
{
    auto endpoint = SpanEndpoint::Builder()
                        .SetClientClusters(Span<const chip::ClusterId>())
                        .SetSemanticTags(Span<const SemanticTag>())
                        .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                        .Build();

    // Test SetClientClusters with an empty span
    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(endpoint.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), 0u);

    // Test SetSemanticTags with an empty span
    ReadOnlyBufferBuilder<SemanticTag> tagBuilder;
    ASSERT_EQ(endpoint.SemanticTags(tagBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(tagBuilder.Size(), 0u);

    // Test SetDeviceTypes with an empty span
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(endpoint.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), 0u);
}
