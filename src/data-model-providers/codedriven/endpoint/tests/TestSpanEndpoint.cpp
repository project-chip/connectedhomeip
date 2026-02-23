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

using namespace chip::app;

TEST(TestSpanEndpoint, InstantiateWithAllParameters)
{
    const chip::ClusterId clientClusters[] = { 10, 20 };
    const DeviceTypeEntry deviceTypes[]    = { Device::Type::kDoorLock, Device::Type::kSpeaker };

    auto endpoint = SpanEndpoint::Builder()
                        .SetClientClusters(Span<const chip::ClusterId>(clientClusters))
                        .SetDeviceTypes(Span<const DeviceTypeEntry>(deviceTypes))
                        .Build();

    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(endpoint.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), std::size(clientClusters));

    ReadOnlyBufferBuilder<DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(endpoint.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), std::size(deviceTypes));

    // A basic test that device types content looks reasonable
    auto buffer = deviceTypeBuilder.TakeBuffer();

    EXPECT_EQ(buffer[0].deviceTypeId, Device::kDoorLockDeviceTypeId);
    EXPECT_EQ(buffer[0].deviceTypeRevision, Device::kDoorLockDeviceTypeRevision);

    // compare content as well
    EXPECT_EQ(buffer[1].deviceTypeId, deviceTypes[1].deviceTypeId);
    EXPECT_EQ(buffer[1].deviceTypeRevision, deviceTypes[1].deviceTypeRevision);
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
                        .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                        .Build();

    // Test SetClientClusters with an empty span
    ReadOnlyBufferBuilder<chip::ClusterId> clientBuilder;
    ASSERT_EQ(endpoint.ClientClusters(clientBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(clientBuilder.Size(), 0u);

    // Test SetDeviceTypes with an empty span
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypeBuilder;
    ASSERT_EQ(endpoint.DeviceTypes(deviceTypeBuilder), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeBuilder.Size(), 0u);
}
