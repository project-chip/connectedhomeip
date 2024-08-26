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

#include <lib/dnssd/IncrementalResolve.h>

#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/support/ScopedBuffer.h>

using namespace chip;
using namespace chip::Dnssd;
using namespace mdns::Minimal;

namespace {

// Operational names must be <compressed-fabric>-<node>._matter._tcp.local
const auto kTestOperationalName = testing::TestQName<4>({ "1234567898765432-ABCDEFEDCBAABCDE", "_matter", "_tcp", "local" });

// Commissionable names must be <instance>._matterc._udp.local
const auto kTestCommissionableNode = testing::TestQName<4>({ "C5038835313B8B98", "_matterc", "_udp", "local" });

// Commissioner names must be <instance>._matterd._udp.local
const auto kTestCommissionerNode = testing::TestQName<4>({ "C5038835313B8B98", "_matterd", "_udp", "local" });

// Server name that is preloaded by the `PreloadSrvRecord`
const auto kTestHostName = testing::TestQName<2>({ "abcd", "local" });

const auto kIrrelevantHostName = testing::TestQName<2>({ "different", "local" });

void PreloadSrvRecord(SrvRecord & record)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes] = {};
    HeaderRef dummyHeader(headerBuffer);

    // NOTE: record pointers persist beyond  this function, so
    // this data MUST be static
    static uint8_t dataBuffer[256];
    chip::Encoding::BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    EXPECT_TRUE(SrvResourceRecord(kTestOperationalName.Full(), kTestHostName.Full(), 0x1234 /* port */)
                    .Append(dummyHeader, ResourceType::kAnswer, writer));

    ResourceData resource;
    BytesRange packet(dataBuffer, dataBuffer + sizeof(dataBuffer));
    const uint8_t * _ptr = dataBuffer;

    EXPECT_TRUE(resource.Parse(packet, &_ptr));
    EXPECT_TRUE(record.Parse(resource.GetData(), packet));
}

/// Convenience method to have a  serialized QName.
/// Assumes valid QName data that is terminated by null.
template <size_t N>
static SerializedQNameIterator AsSerializedQName(const uint8_t (&v)[N])
{
    // NOTE: the -1 is because we format these items as STRINGS and that
    // appends an extra NULL terminator
    return SerializedQNameIterator(BytesRange(v, v + N - 1), v);
}

void CallOnRecord(IncrementalResolver & resolver, const ResourceRecord & record)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes] = {};
    HeaderRef dummyHeader(headerBuffer);

    uint8_t dataBuffer[256];
    chip::Encoding::BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    EXPECT_TRUE(record.Append(dummyHeader, ResourceType::kAnswer, writer));
    EXPECT_TRUE(writer.Fit());

    ResourceData resource;
    BytesRange packet(dataBuffer, dataBuffer + sizeof(dataBuffer));
    const uint8_t * _ptr = dataBuffer;
    EXPECT_TRUE(resource.Parse(packet, &_ptr));
    EXPECT_EQ(resolver.OnRecord(chip::Inet::InterfaceId::Null(), resource, packet), CHIP_NO_ERROR);
}

TEST(TestIncrementalResolve, TestStoredServerName)
{

    StoredServerName name;

    // name should start of as cleared
    EXPECT_FALSE(name.Get().Next());

    // Data should be storable in server name
    EXPECT_EQ(name.Set(kTestOperationalName.Serialized()), CHIP_NO_ERROR);
    EXPECT_EQ(name.Get(), kTestOperationalName.Serialized());
    EXPECT_NE(name.Get(), kTestCommissionerNode.Serialized());
    EXPECT_NE(name.Get(), kTestCommissionableNode.Serialized());

    EXPECT_EQ(name.Set(kTestCommissionerNode.Serialized()), CHIP_NO_ERROR);
    EXPECT_NE(name.Get(), kTestOperationalName.Serialized());
    EXPECT_EQ(name.Get(), kTestCommissionerNode.Serialized());
    EXPECT_NE(name.Get(), kTestCommissionableNode.Serialized());

    EXPECT_EQ(name.Set(kTestCommissionableNode.Serialized()), CHIP_NO_ERROR);
    EXPECT_NE(name.Get(), kTestOperationalName.Serialized());
    EXPECT_NE(name.Get(), kTestCommissionerNode.Serialized());
    EXPECT_EQ(name.Get(), kTestCommissionableNode.Serialized());

    {
        // setting to a too long value should reset it
        uint8_t largeBuffer[256];
        memset(largeBuffer, 0, sizeof(largeBuffer));

        Encoding::BigEndian::BufferWriter writer(largeBuffer, sizeof(largeBuffer));

        for (unsigned idx = 0; true; idx++)
        {
            writer.Put("\07abcd123"); // will not NULL-terminate, but buffer is 0-filled
            if (!writer.Fit())
            {
                break; // filled all our tests
            }

            if (writer.WritePos() < 64)
            {
                // this is how much data can be fit by the copy
                EXPECT_EQ(name.Set(AsSerializedQName(largeBuffer)), CHIP_NO_ERROR) << "idx = " << idx;
                EXPECT_EQ(name.Get(), AsSerializedQName(largeBuffer)) << "idx = " << idx;
                EXPECT_NE(name.Get(), kTestOperationalName.Serialized()) << "idx = " << idx;
            }
            else
            {
                EXPECT_EQ(name.Set(AsSerializedQName(largeBuffer)), CHIP_ERROR_NO_MEMORY) << "idx = " << idx;
                EXPECT_FALSE(name.Get().Next()) << "idx = " << idx;
            }
        }
    }
}

TEST(TestIncrementalResolve, TestCreation)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());
    EXPECT_FALSE(resolver.IsActiveCommissionParse());
    EXPECT_FALSE(resolver.IsActiveOperationalParse());
    EXPECT_TRUE(
        resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kSrvInitialization));
}

TEST(TestIncrementalResolve, TestInactiveResetOnInitError)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    // test host name is not a 'matter' name
    EXPECT_NE(resolver.InitializeParsing(kTestHostName.Serialized(), 0, srvRecord), CHIP_NO_ERROR);

    EXPECT_FALSE(resolver.IsActive());
    EXPECT_FALSE(resolver.IsActiveCommissionParse());
    EXPECT_FALSE(resolver.IsActiveOperationalParse());
}

TEST(TestIncrementalResolve, TestStartOperational)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    EXPECT_EQ(resolver.InitializeParsing(kTestOperationalName.Serialized(), 1, srvRecord), CHIP_NO_ERROR);

    EXPECT_TRUE(resolver.IsActive());
    EXPECT_FALSE(resolver.IsActiveCommissionParse());
    EXPECT_TRUE(resolver.IsActiveOperationalParse());
    EXPECT_TRUE(resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    EXPECT_EQ(resolver.GetTargetHostName(), kTestHostName.Serialized());
}

TEST(TestIncrementalResolve, TestStartCommissionable)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    EXPECT_EQ(resolver.InitializeParsing(kTestCommissionableNode.Serialized(), 0, srvRecord), CHIP_NO_ERROR);

    EXPECT_TRUE(resolver.IsActive());
    EXPECT_TRUE(resolver.IsActiveCommissionParse());
    EXPECT_FALSE(resolver.IsActiveOperationalParse());
    EXPECT_TRUE(resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    EXPECT_EQ(resolver.GetTargetHostName(), kTestHostName.Serialized());
}

TEST(TestIncrementalResolve, TestStartCommissioner)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    EXPECT_EQ(resolver.InitializeParsing(kTestCommissionerNode.Serialized(), 0, srvRecord), CHIP_NO_ERROR);

    EXPECT_TRUE(resolver.IsActive());
    EXPECT_TRUE(resolver.IsActiveCommissionParse());
    EXPECT_FALSE(resolver.IsActiveOperationalParse());
    EXPECT_TRUE(resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    EXPECT_EQ(resolver.GetTargetHostName(), kTestHostName.Serialized());
}

TEST(TestIncrementalResolve, TestParseOperational)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    EXPECT_EQ(resolver.InitializeParsing(kTestOperationalName.Serialized(), 1, srvRecord), CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    EXPECT_TRUE(resolver.IsActiveOperationalParse());
    EXPECT_TRUE(resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    EXPECT_EQ(resolver.GetTargetHostName(), kTestHostName.Serialized());

    // Send an IP for an irrelevant host name
    {
        Inet::IPAddress addr;
        EXPECT_TRUE(Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));

        CallOnRecord(resolver, IPResourceRecord(kIrrelevantHostName.Full(), addr));
    }

    // Send a useful IP address here
    {
        Inet::IPAddress addr;
        EXPECT_TRUE(Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "T=1"                             // TCP supported
        };

        CallOnRecord(resolver, TxtResourceRecord(kTestHostName.Full(), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "foo=bar", // unused data
            "SII=23"   // session idle interval
        };

        CallOnRecord(resolver, TxtResourceRecord(kTestOperationalName.Full(), entries));
    }

    // Resolver should have all data
    EXPECT_FALSE(resolver.GetMissingRequiredInformation().HasAny());

    // At this point taking value should work. Once taken, the resolver is reset.
    ResolvedNodeData nodeData;
    EXPECT_EQ(resolver.Take(nodeData), CHIP_NO_ERROR);
    EXPECT_FALSE(resolver.IsActive());

    // validate data as it was passed in
    EXPECT_EQ(nodeData.operationalData.peerId,
              PeerId().SetCompressedFabricId(0x1234567898765432LL).SetNodeId(0xABCDEFEDCBAABCDELL));
    EXPECT_FALSE(nodeData.operationalData.hasZeroTTL);
    EXPECT_EQ(nodeData.resolutionData.numIPs, 1u);
    EXPECT_EQ(nodeData.resolutionData.port, 0x1234);
    EXPECT_FALSE(nodeData.resolutionData.supportsTcpServer);
    EXPECT_FALSE(nodeData.resolutionData.supportsTcpClient);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().has_value());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryIntervalIdle(), std::make_optional(chip::System::Clock::Milliseconds32(23)));

    Inet::IPAddress addr;
    EXPECT_TRUE(Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
    EXPECT_EQ(nodeData.resolutionData.ipAddress[0], addr);
}

TEST(TestIncrementalResolve, TestParseCommissionable)
{
    IncrementalResolver resolver;

    EXPECT_FALSE(resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(srvRecord);

    EXPECT_EQ(resolver.InitializeParsing(kTestCommissionableNode.Serialized(), 0, srvRecord), CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    EXPECT_TRUE(resolver.IsActiveCommissionParse());
    EXPECT_TRUE(resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    EXPECT_EQ(resolver.GetTargetHostName(), kTestHostName.Serialized());

    // Send an IP for an irrelevant host name
    {
        Inet::IPAddress addr;

        EXPECT_TRUE(Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));
        CallOnRecord(resolver, IPResourceRecord(kIrrelevantHostName.Full(), addr));
    }

    // Send a useful IP address here
    {
        Inet::IPAddress addr;
        EXPECT_TRUE(Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send another IP address
    {
        Inet::IPAddress addr;
        EXPECT_TRUE(Inet::IPAddress::FromString("fe80::f0f1:f2f3:f4f5:1234", addr));
        CallOnRecord(resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "SII=123"                         // session idle interval
        };

        CallOnRecord(resolver, TxtResourceRecord(kTestHostName.Full(), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "foo=bar",    // unused data
            "SAI=321",    // session active interval
            "D=22345",    // Long discriminator
            "VP=321+654", // VendorProduct
            "DN=mytest"   // Device name
        };

        CallOnRecord(resolver, TxtResourceRecord(kTestCommissionableNode.Full(), entries));
    }

    // Resolver should have all data
    EXPECT_FALSE(resolver.GetMissingRequiredInformation().HasAny());

    // At this point taking value should work. Once taken, the resolver is reset.
    DiscoveredNodeData discoveredNodeData;
    EXPECT_TRUE(resolver.Take(discoveredNodeData) == CHIP_NO_ERROR);
    EXPECT_FALSE(resolver.IsActive());

    EXPECT_TRUE(discoveredNodeData.Is<CommissionNodeData>());
    CommissionNodeData nodeData = discoveredNodeData.Get<CommissionNodeData>();

    // validate data as it was passed in
    EXPECT_EQ(nodeData.numIPs, 2u);
    EXPECT_EQ(nodeData.port, 0x1234);
    EXPECT_FALSE(nodeData.supportsTcpClient);
    EXPECT_FALSE(nodeData.supportsTcpServer);
    EXPECT_EQ(nodeData.GetMrpRetryIntervalActive(), std::make_optional(chip::System::Clock::Milliseconds32(321)));
    EXPECT_FALSE(nodeData.GetMrpRetryIntervalIdle().has_value());

    Inet::IPAddress addr;
    EXPECT_TRUE(Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
    EXPECT_EQ(nodeData.ipAddress[0], addr);
    EXPECT_TRUE(Inet::IPAddress::FromString("fe80::f0f1:f2f3:f4f5:1234", addr));
    EXPECT_EQ(nodeData.ipAddress[1], addr);

    // parsed txt data for discovered nodes
    EXPECT_EQ(nodeData.longDiscriminator, 22345);
    EXPECT_EQ(nodeData.vendorId, 321);
    EXPECT_EQ(nodeData.productId, 654);
    EXPECT_STREQ(nodeData.deviceName, "mytest");
}
} // namespace
