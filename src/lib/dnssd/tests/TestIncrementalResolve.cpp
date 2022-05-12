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

#include <lib/dnssd/minimal_mdns/records/IP.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Dnssd;
using namespace mdns::Minimal;

namespace {

// Operational names must be <compressed-fabric>-<node>._matter._tcp.local
constexpr uint8_t kTestOperationalName[] = "\0411234567898765432-ABCDEFEDCBAABCDE\07_matter\04_tcp\05local\00";

// Commissionable names must be <instance>._matterc._udp.local
constexpr uint8_t kTestCommissionableNode[] = "\020C5038835313B8B98\10_matterc\04_udp\05local\00";

// Commissioner names must be <instance>._matterd._udp.local
constexpr uint8_t kTestCommissionerNode[] = "\020C5038835313B8B98\10_matterd\04_udp\05local\00";

// Server name that is preloaded by the `PreloadSrvRecord`
constexpr uint8_t kTestServerName[] = "\04abcd\05local\00";

void PreloadSrvRecord(nlTestSuite * inSuite, SrvRecord & record)
{
    // NOTE: record pointers persist beyond  this function, so
    // this data MUST be static
    static const uint8_t data[] = {
        0,    12,                       // Priority
        0,    3,                        // weight
        0x12, 0x34,                     // port
        4,    'a',  'b', 'c', 'd',      // QNAME part: abcd
        5,    'l',  'o', 'c', 'a', 'l', // QNAME part: local
        0,                              // QNAME ends
    };

    BytesRange packet(data, data + sizeof(data));
    BytesRange srv_data(data, data + sizeof(data));

    NL_TEST_ASSERT(inSuite, record.Parse(srv_data, packet));
}

/// Convenience method to have a  serialized QName:
///
/// static const uint8_t kData[] = "datahere\00";
///  AsSerializedQName(kData);
///
/// NOTE: this MUST be using the string "" format to add an extra NULL
/// terminator that this method discards.
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
    NL_TEST_ASSERT(inSuite, resolver.OnRecord(resource, packet) == CHIP_NO_ERROR);
}

void TestStoredServerName(nlTestSuite * inSuite, void * inContext)
{

    StoredServerName name;

    // name should start of as cleared
    NL_TEST_ASSERT(inSuite, !name.Get().Next());

    // Data should be storable in server name
    NL_TEST_ASSERT(inSuite, name.Set(AsSerializedQName(kTestOperationalName)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() == AsSerializedQName(kTestOperationalName));
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestCommissionerNode));
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestCommissionableNode));

    NL_TEST_ASSERT(inSuite, name.Set(AsSerializedQName(kTestCommissionerNode)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestOperationalName));
    NL_TEST_ASSERT(inSuite, name.Get() == AsSerializedQName(kTestCommissionerNode));
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestCommissionableNode));

    NL_TEST_ASSERT(inSuite, name.Set(AsSerializedQName(kTestCommissionableNode)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestOperationalName));
    NL_TEST_ASSERT(inSuite, name.Get() != AsSerializedQName(kTestCommissionerNode));
    NL_TEST_ASSERT(inSuite, name.Get() == AsSerializedQName(kTestCommissionableNode));

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
                NL_TEST_ASSERT_LOOP(inSuite, idx, name.Get() != AsSerializedQName(kTestOperationalName));
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
    NL_TEST_ASSERT(inSuite,
                   resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kSrvInitialization));
}

void TestStartOperational(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(AsSerializedQName(kTestOperationalName), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite, resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == AsSerializedQName(kTestServerName));
}

void TestStartCommissionable(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(AsSerializedQName(kTestCommissionableNode), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite, resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == AsSerializedQName(kTestServerName));
}

void TestStartCommissioner(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(AsSerializedQName(kTestCommissionerNode), srvRecord) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, resolver.IsActive());
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite, resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == AsSerializedQName(kTestServerName));
}

void TestParseOperational(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());

    SrvRecord srvRecord;
    PreloadSrvRecord(inSuite, srvRecord);

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(AsSerializedQName(kTestOperationalName), srvRecord) == CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    NL_TEST_ASSERT(inSuite, resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite, resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == AsSerializedQName(kTestServerName));

    // Send an IP for an irrelevant host name
    {
        const char * path[] = { "xyzt", "local" };
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(FullQName(path), addr));
    }

    // Send a useful IP address here
    {
        const char * path[] = { "abcd", "local" };
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(FullQName(path), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * path[]    = { "abcd", "local" };
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "T=1"                             // TCP supported
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(FullQName(path), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * path[]    = { "1234567898765432-ABCDEFEDCBAABCDE", "_matter", "_tcp", "local" };
        const char * entries[] = {
            "foo=bar", // unused data
            "SII=23"   // sleepy idle interval
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(FullQName(path), entries));
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

    NL_TEST_ASSERT(inSuite, resolver.InitializeParsing(AsSerializedQName(kTestCommissionableNode), srvRecord) == CHIP_NO_ERROR);

    // once initialized, parsing should be ready however no IP address is available
    NL_TEST_ASSERT(inSuite, resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, resolver.GetMissingRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetTargetHostName() == AsSerializedQName(kTestServerName));

    // Send an IP for an irrelevant host name
    {
        const char * path[] = { "xyzt", "local" };
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(FullQName(path), addr));
    }

    // Send a useful IP address here
    {
        const char * path[] = { "abcd", "local" };
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::abcd:ef11:2233:4455", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(FullQName(path), addr));
    }

    // Send another IP address
    {
        const char * path[] = { "abcd", "local" };
        Inet::IPAddress addr;

        NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("fe80::f0f1:f2f3:f4f5:1234", addr));
        CallOnRecord(inSuite, resolver, IPResourceRecord(FullQName(path), addr));
    }

    // Send a TXT record for an irrelevant host name
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * path[]    = { "abcd", "local" };
        const char * entries[] = {
            "some", "foo=bar", "x=y=z", "a=", // unused data
            "SII=123"                         // Sleepy idle interval
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(FullQName(path), entries));
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const char * path[]    = { "C5038835313B8B98", "_matterc", "_udp", "local" };
        const char * entries[] = {
            "foo=bar",    // unused data
            "SAI=321",    // sleepy active interval
            "D=22345",    // Long discriminator
            "VP=321+654", // VendorProduct
            "DN=mytest"   // Device name
        };

        CallOnRecord(inSuite, resolver, TxtResourceRecord(FullQName(path), entries));
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
    NL_TEST_DEF("Creation", TestCreation),                       //
    NL_TEST_DEF("StartOperational", TestStartOperational),       //
    NL_TEST_DEF("StartCommissionable", TestStartCommissionable), //
    NL_TEST_DEF("StartCommissioner", TestStartCommissioner),     //
    NL_TEST_DEF("ParseOperational", TestParseOperational),       //
    NL_TEST_DEF("ParseCommissionable", TestParseCommissionable), //
    NL_TEST_SENTINEL()                                           //
};

} // namespace

int TestChipDnsSdIncrementalResolve(void)
{
    nlTestSuite theSuite = { "IncrementalResolve", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestChipDnsSdIncrementalResolve)
