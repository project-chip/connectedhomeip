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

#include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

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

void PreloadSrvRecord(nlTestSuite * inSuite, SrvRecord & record)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes] = {};
    HeaderRef dummyHeader(headerBuffer);

    // NOTE: record pointers persist beyond  this function, so
    // this data MUST be static
    static uint8_t dataBuffer[256];
    chip::Encoding::BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    NL_TEST_ASSERT(inSuite,
                   SrvResourceRecord(kTestOperationalName.Full(), kTestHostName.Full(), 0x1234 /* port */)
                       .Append(dummyHeader, ResourceType::kAnswer, writer));

    ResourceData resource;
    BytesRange packet(dataBuffer, dataBuffer + sizeof(dataBuffer));
    const uint8_t * _ptr = dataBuffer;

    NL_TEST_ASSERT(inSuite, resource.Parse(packet, &_ptr));
    NL_TEST_ASSERT(inSuite, record.Parse(resource.GetData(), packet));
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

void CallOnRecord(nlTestSuite * inSuite, IncrementalResolver & resolver, const ResourceRecord & record)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes] = {};
    HeaderRef dummyHeader(headerBuffer);

    uint8_t dataBuffer[256];
    chip::Encoding::BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    NL_TEST_ASSERT(inSuite, record.Append(dummyHeader, ResourceType::kAnswer, writer));
    NL_TEST_ASSERT(inSuite, writer.Fit());

    ResourceData resource;
    BytesRange packet(dataBuffer, dataBuffer + sizeof(dataBuffer));
    const uint8_t * _ptr = dataBuffer;
    NL_TEST_ASSERT(inSuite, resource.Parse(packet, &_ptr));
    NL_TEST_ASSERT(inSuite, resolver.OnRecord(chip::Inet::InterfaceId::Null(), resource, packet) == CHIP_NO_ERROR);
}

void TestStoredServerName(nlTestSuite * inSuite, void * inContext)
{

    StoredServerName name;

    // name should start of as cleared
    NL_TEST_ASSERT(inSuite, !name.Get().Next());

    // Data should be storable in server name
    NL_TEST_ASSERT(inSuite, name.Set(kTestOperationalName.Serialized()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() == kTestOperationalName.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() != kTestCommissionerNode.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() != kTestCommissionableNode.Serialized());

    NL_TEST_ASSERT(inSuite, name.Set(kTestCommissionerNode.Serialized()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() != kTestOperationalName.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() == kTestCommissionerNode.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() != kTestCommissionableNode.Serialized());

    NL_TEST_ASSERT(inSuite, name.Set(kTestCommissionableNode.Serialized()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() != kTestOperationalName.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() != kTestCommissionerNode.Serialized());
    NL_TEST_ASSERT(inSuite, name.Get() == kTestCommissionableNode.Serialized());

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
                NL_TEST_ASSERT_LOOP(inSuite, idx, name.Set(AsSerializedQName(largeBuffer)) == CHIP_NO_ERROR);
                NL_TEST_ASSERT_LOOP(inSuite, idx, name.Get() == AsSerializedQName(largeBuffer));
                NL_TEST_ASSERT_LOOP(inSuite, idx, name.Get() != kTestOperationalName.Serialized());
            }
            else
            {
                NL_TEST_ASSERT_LOOP(inSuite, idx, name.Set(AsSerializedQName(largeBuffer)) == CHIP_ERROR_NO_MEMORY);
                NL_TEST_ASSERT_LOOP(inSuite, idx, !name.Get().Next());
            }
        }
    }
}

void TestCreation(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(
        inSuite,
        resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kSrvInitialization));
}

void TestInactiveResetOnInitError(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    // test host name is not a 'matter' name
    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestHostName.Serialized(), srvRecord) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
}

void TestStartOperational(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestOperationalName.Serialized(), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == kTestHostName.Serialized());
}

void TestStartCommissionable(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestCommissionableNode.Serialized(), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == kTestHostName.Serialized());
}

void TestStartCommissioner(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestCommissionerNode.Serialized(), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == kTestHostName.Serialized());
}

void TestParseOperational(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestOperationalName.Serialized(), srvRecord) == CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    NL_TEST_ASSERT(inSuite, resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == kTestHostName.Serialized());

    // Send an IP for an irrelevant host name
    {
        Inet::IPAddress addr;
        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));

        CallOnRecord(inSuite, resolver, IPResourceRecord(kIrrelevantHostName.Full(), addr));
    }

    // Send a useful IP address here
    {
        Inet::IPAddress addr;
        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "T=1"                             // TCP supported
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(kTestHostName.Full(), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "foo=bar", // unused data
            "SII=23"   // sleepy idle interval
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(kTestOperationalName.Full(), entries));
    }

    // Resolver should have all data
    NL_TEST_ASSERT(inSuite, !resolver.GetMissingRequiredInformation().HasAny());

    // At this point taking value should work. Once taken, the resolver is reset.
    ResolvedNodeData nodeData;
    NL_TEST_ASSERT(inSuite, resolver.Take(nodeData) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    // validate data as it was passed in
    NL_TEST_ASSERT(inSuite,
                   nodeData.operationalData.peerId ==
                       PeerId().SetCompressedFabricId(0x1234567898765432LL).SetNodeId(0xABCDEFEDCBAABCDELL));
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.numIPs == 1);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.port == 0x1234);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.supportsTcp);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().Value() == chip::System::Clock::Milliseconds32(23));

    Inet::IPAddress addr;
    NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.ipAddress[0] == addr);
}

void TestParseCommissionable(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(kTestCommissionableNode.Serialized(), srvRecord) == CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformationBitFlags::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == kTestHostName.Serialized());

    // Send an IP for an irrelevant host name
    {
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(kIrrelevantHostName.Full(), addr));
    }

    // Send a useful IP address here
    {
        Inet::IPAddress addr;
        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send another IP address
    {
        Inet::IPAddress addr;
        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::f0f1:f2f3:f4f5:1234", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(kTestHostName.Full(), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "SII=123"                         // Sleepy idle interval
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(kTestHostName.Full(), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * entries[] = {
            "foo=bar",    // unused data
            "SAI=321",    // sleepy active interval
            "D=22345",    // Long discriminator
            "VP=321+654", // VendorProduct
            "DN=mytest"   // Device name
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(kTestCommissionableNode.Full(), entries));
    }

    // Resolver should have all data
    NL_TEST_ASSERT(inSuite, !resolver.GetMissingRequiredInformation().HasAny());

    // At this point taking value should work. Once taken, the resolver is reset.
    DiscoveredNodeData nodeData;
    NL_TEST_ASSERT(inSuite, resolver.Take(nodeData) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    // validate data as it was passed in
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.numIPs == 2);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.port == 0x1234);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.supportsTcp);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    NL_TEST_ASSERT(inSuite,
                   nodeData.resolutionData.GetMrpRetryIntervalActive().Value() == chip::System::Clock::Milliseconds32(321));
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    Inet::IPAddress addr;
    NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.ipAddress[0] == addr);
    NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::f0f1:f2f3:f4f5:1234", addr));
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.ipAddress[1] == addr);

    // parsed txt data for discovered nodes
    NL_TEST_ASSERT(inSuite, nodeData.commissionData.longDiscriminator == 22345);
    NL_TEST_ASSERT(inSuite, nodeData.commissionData.vendorId == 321);
    NL_TEST_ASSERT(inSuite, nodeData.commissionData.productId == 654);
    NL_TEST_ASSERT(inSuite, strcmp(nodeData.commissionData.deviceName, "mytest") == 0);
}

const nlTest sTests[] = {
    // Tests for helper class
    NL_TEST_DEF("StoredServerName", TestStoredServerName), //

    // Actual resolver tests
    NL_TEST_DEF("Creation", TestCreation),                                 //
    NL_TEST_DEF("InactiveResetOnInitError", TestInactiveResetOnInitError), //
    NL_TEST_DEF("StartOperational", TestStartOperational),                 //
    NL_TEST_DEF("StartCommissionable", TestStartCommissionable),           //
    NL_TEST_DEF("StartCommissioner", TestStartCommissioner),               //
    NL_TEST_DEF("ParseOperational", TestParseOperational),                 //
    NL_TEST_DEF("ParseCommissionable", TestParseCommissionable),           //
    NL_TEST_SENTINEL()                                                     //
};

} // namespace

int TestChipDnsSdIncrementalResolve(void)
{
    nlTestSuite theSuite = { "IncrementalResolve", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestChipDnsSdIncrementalResolve)
