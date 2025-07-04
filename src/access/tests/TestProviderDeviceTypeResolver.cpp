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

#include "access/ProviderDeviceTypeResolver.h"
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/Provider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/CHIPMem.h> 

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

namespace {

class FakeProvider final : public Provider
{
public:
    // Devices are represented by endpoints, and each endpoint can have multiple device types.
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DeviceTypeEntry> & builder) override
    {
        if (endpointId == 1)
        {
            // Hardcoded device types for endpoint 1
            constexpr DeviceTypeEntry types[] = {
                { .deviceTypeId = 0x0000'0001, .deviceTypeRevision = 1 },
                { .deviceTypeId = 0x0000'0002, .deviceTypeRevision = 1 },
            };
            return builder.AppendElements(chip::Span(types));
        }
        if (endpointId == 2)
        {
            // Hardcoded device types for endpoint 2
            constexpr DeviceTypeEntry types[] = {
                { .deviceTypeId = 0x0000'0003, .deviceTypeRevision = 1 },
            };
            return builder.AppendElements(chip::Span(types));
        }
        return CHIP_NO_ERROR;
    }

    // The following methods are not used in this test, but must be implemented as they are pure virtual in the Provider interface.
    CHIP_ERROR SemanticTags(EndpointId, ReadOnlyBufferBuilder<Clusters::Descriptor::Structs::SemanticTagStruct::Type> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClientClusters(EndpointId, ReadOnlyBufferBuilder<ClusterId> &)                                            override { return CHIP_NO_ERROR; }
    CHIP_ERROR ServerClusters(EndpointId, ReadOnlyBufferBuilder<ServerClusterEntry> &)                                   override { return CHIP_NO_ERROR; }
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<EndpointEntry> &)                                                         override { return CHIP_NO_ERROR; }
    CHIP_ERROR EventInfo(const ConcreteEventPath &, EventEntry &)                                                        override { return CHIP_NO_ERROR; }
    CHIP_ERROR Attributes(const ConcreteClusterPath &, ReadOnlyBufferBuilder<AttributeEntry> &)                          override { return CHIP_NO_ERROR; }
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath &, ReadOnlyBufferBuilder<CommandId> &)                        override { return CHIP_NO_ERROR; }
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath &, ReadOnlyBufferBuilder<AcceptedCommandEntry> &)              override { return CHIP_NO_ERROR; }
    CHIP_ERROR Shutdown()                                                                                                override { return CHIP_NO_ERROR; }
    void Temporary_ReportAttributeChanged(const AttributePathParams &)                        override {}
    ActionReturnStatus ReadAttribute(const ReadAttributeRequest &, AttributeValueEncoder &)   override { return Protocols::InteractionModel::Status::Success; }
    ActionReturnStatus WriteAttribute(const WriteAttributeRequest &, AttributeValueDecoder &) override { return Protocols::InteractionModel::Status::Success; }
    void ListAttributeWriteNotification(const ConcreteAttributePath &, ListWriteOperation)    override {}
    std::optional<ActionReturnStatus> InvokeCommand(const InvokeRequest &, TLV::TLVReader &, CommandHandler *) override { return Protocols::InteractionModel::Status::Success; }
};

// Hook required by DynamicProviderDeviceTypeResolver
FakeProvider gFakeProvider;
Provider * GetFakeProvider() { return &gFakeProvider; }

} // unnamed namespace

namespace chip::Access {

class TestDeviceTypeResolver : public ::testing::Test
{
public:
    // In a smart home context, the physical device that can act as a resolver is typically
    // a gateway or hub, e.g. a controller that can communicate with various devices.
    DynamicProviderDeviceTypeResolver resolver{ &GetFakeProvider };

    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::Platform::MemoryShutdown();
    }
};

// Checks that the system can correctly identify when a specific device type (like a smart bulb or sensor) 
// is actually present on a given endpoint (for example, a particular room or appliance port).
TEST_F(TestDeviceTypeResolver, PositiveMatches)
{
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(0x0000'0001, 1));
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(0x0000'0002, 1));
    EXPECT_TRUE(resolver.IsDeviceTypeOnEndpoint(0x0000'0003, 2));
}

// Checks that the system does not mistakenly identify a device type as present on an endpoint where it 
// doesn’t actually exist (for example, asking if a light switch is in the kitchen when it isn't).
TEST_F(TestDeviceTypeResolver, NegativeMatches)
{
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(0x0000'0004, 1));  // wrong device type
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(0x0000'0001, 2));  // wrong endpoint
    EXPECT_FALSE(resolver.IsDeviceTypeOnEndpoint(0x0000'0001, 99)); // unknown endpoint
}

} // namespace chip::Access
