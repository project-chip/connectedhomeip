/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <access/ProviderDeviceTypeResolver.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ReadOnlyBuffer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

namespace {

constexpr EndpointId kTestEndpointRoot  = 1; // Example: could represent the main/root device
constexpr EndpointId kTestEndpointLight = 2; // Example: could represent a lighting device

// In real applications, DeviceTypeId could represent specific
// device types like "Smart Bulb", "Thermostat", etc.
constexpr DeviceTypeId kDeviceTypeId1 = 0x0000'0001;
constexpr DeviceTypeId kDeviceTypeId2 = 0x0000'0002;
constexpr DeviceTypeId kDeviceTypeId3 = 0x0000'0003;
constexpr DeviceTypeId kDeviceTypeId4 = 0x0000'0004;

class FakeProvider final : public Provider
{
public:
    // Devices are represented by endpoints, and each endpoint can have multiple device types.
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DeviceTypeEntry> & builder) override
    {
        if (endpointId == kTestEndpointRoot)
        {
            // Hardcoded device types for endpoint 1
            constexpr DeviceTypeEntry types[] = {
                { .deviceTypeId = kDeviceTypeId1, .deviceTypeRevision = 1 },
                { .deviceTypeId = kDeviceTypeId2, .deviceTypeRevision = 1 },
            };
            return builder.AppendElements(chip::Span(types));
        }
        if (endpointId == kTestEndpointLight)
        {
            // Hardcoded device types for endpoint 2
            constexpr DeviceTypeEntry types[] = {
                { .deviceTypeId = kDeviceTypeId3, .deviceTypeRevision = 1 },
            };
            return builder.AppendElements(chip::Span(types));
        }
        return CHIP_NO_ERROR;
    }

    // The following methods are not used in this test, but must be implemented as they are pure virtual in the Provider interface.
    CHIP_ERROR ClientClusters(EndpointId, ReadOnlyBufferBuilder<ClusterId> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ServerClusters(EndpointId, ReadOnlyBufferBuilder<ServerClusterEntry> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<EndpointEntry> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR EventInfo(const ConcreteEventPath &, EventEntry &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR Attributes(const ConcreteClusterPath &, ReadOnlyBufferBuilder<AttributeEntry> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath &, ReadOnlyBufferBuilder<CommandId> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath &, ReadOnlyBufferBuilder<AcceptedCommandEntry> &) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }
    void Temporary_ReportAttributeChanged(const AttributePathParams &) override {}
    ActionReturnStatus ReadAttribute(const ReadAttributeRequest &, AttributeValueEncoder &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    ActionReturnStatus WriteAttribute(const WriteAttributeRequest &, AttributeValueDecoder &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    void ListAttributeWriteNotification(const ConcreteAttributePath &, ListWriteOperation, FabricIndex) override {}
    std::optional<ActionReturnStatus> InvokeCommand(const InvokeRequest &, TLV::TLVReader &, CommandHandler *) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
};

// Hook required by DynamicProviderDeviceTypeResolver
FakeProvider gFakeProvider;
Provider * GetFakeProvider()
{
    return &gFakeProvider;
}

} // unnamed namespace

namespace chip::Access {

class TestDeviceTypeResolver : public ::testing::Test
{
public:
    // In a smart home context, the physical device that can act as a resolver is typically
    // a gateway or hub, e.g. a controller that can communicate with various devices.
    DynamicProviderDeviceTypeResolver resolver{ &GetFakeProvider };

    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Checks that the system can correctly identify when a specific device type (like a smart bulb or sensor)
// is actually present on a given endpoint (for example, a particular room or appliance port).
TEST_F(TestDeviceTypeResolver, PositiveMatches)
{
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId1, kTestEndpointRoot));
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId2, kTestEndpointRoot));
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId3, kTestEndpointLight));
}

// Checks that the system does not mistakenly identify a device type as present on an endpoint where it
// doesn’t actually exist (for example, asking if a light switch is in the kitchen when it isn't).
TEST_F(TestDeviceTypeResolver, NegativeMatches)
{
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId4, kTestEndpointRoot));  // wrong device type
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId1, kTestEndpointLight)); // wrong endpoint
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(kDeviceTypeId1, 99));                 // unknown endpoint
}

} // namespace chip::Access
