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
        for (unsigned i = 0; i < 20; i++)
        {
            memcpy(largeBuffer + i * 8, "\07abcd123", 8);
            largeBuffer[(i + 1) * 8] = 0;

            // Up to 64 bytes supported to copy
            if (i < 7)
            {
                NL_TEST_ASSERT_LOOP(inSuite, i, name.Set(AsSerializedQName(largeBuffer)) == CHIP_NO_ERROR);
                NL_TEST_ASSERT_LOOP(inSuite, i, name.Get() == AsSerializedQName(largeBuffer));
                NL_TEST_ASSERT_LOOP(inSuite, i, name.Get() != AsSerializedQName(kTestOperationalName));
            }
            else
            {
                NL_TEST_ASSERT_LOOP(inSuite, i, name.Set(AsSerializedQName(largeBuffer)) == CHIP_ERROR_NO_MEMORY);
                NL_TEST_ASSERT_LOOP(inSuite, i, !name.Get().Next());
            }
        }
        NL_TEST_ASSERT(inSuite, name.Set(AsSerializedQName(largeBuffer)) == CHIP_ERROR_NO_MEMORY);
        NL_TEST_ASSERT(inSuite, !name.Get().Next());
    }
}

void TestCreation(nlTestSuite * inSuite, void * inContext)
{
    IncrementalResolver resolver;

    NL_TEST_ASSERT(inSuite, !resolver.IsActive());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveCommissionParse());
    NL_TEST_ASSERT(inSuite, !resolver.IsActiveOperationalParse());
    NL_TEST_ASSERT(inSuite,
                   resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kSrvInitialization));
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
    NL_TEST_ASSERT(inSuite, resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetServerName() == AsSerializedQName(kTestServerName));
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
    NL_TEST_ASSERT(inSuite, resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetServerName() == AsSerializedQName(kTestServerName));
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
    NL_TEST_ASSERT(inSuite, resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetServerName() == AsSerializedQName(kTestServerName));
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
    NL_TEST_ASSERT(inSuite, resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    NL_TEST_ASSERT(inSuite, resolver.GetServerName() == AsSerializedQName(kTestServerName));

    // Send an irellevant IP address here
    {
        const uint8_t packetAAAA[] = {                                   //
                                       4,    'x',  'y',  'x',  't',      // QNAME part: xyzt
                                       5,    'l',  'o',  'c',  'a', 'l', // QNAME part: local
                                       0,                                // QNAME ends
                                       0,    28,                         // QType AAAA
                                       0,    1,                          // QClass IN
                                       0x12, 0x34, 0x56, 0x78,           // TTL
                                       0,    16,                         // data size - size for IPv4
                                       0xfe, 0x80, 0x00, 0x00,           // IPv6
                                       0x00, 0x00, 0x00, 0x00,           //
                                       0x02, 0x24, 0x32, 0xff,           //
                                       0xfe, 0x19, 0x35, 0x9b
        };
        BytesRange packet(packetAAAA, packetAAAA + sizeof(packetAAAA));
        BytesRange aaaa_data(packetAAAA, packetAAAA + sizeof(packetAAAA));

        ResourceData data;
        const uint8_t * ptr = packetAAAA;
        NL_TEST_ASSERT(inSuite, data.Parse(aaaa_data, &ptr));

        // Parsing irellevant IP does not take effect
        NL_TEST_ASSERT(inSuite, resolver.OnRecord(data, packet) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, resolver.GetRequiredInformation().HasOnly(IncrementalResolver::RequiredInformation::kIpAddress));
    }

    // Send a useful IP address here
    {
        const uint8_t packetAAAA[] = {                                   //
                                       4,    'a',  'b',  'c',  'd',      // QNAME part: abcd
                                       5,    'l',  'o',  'c',  'a', 'l', // QNAME part: local
                                       0,                                // QNAME ends
                                       0,    28,                         // QType AAAA
                                       0,    1,                          // QClass IN
                                       0x12, 0x34, 0x56, 0x78,           // TTL
                                       0,    16,                         // data size - size for IPv4
                                       0xfe, 0x80, 0x00, 0x00,           // IPv6
                                       0x00, 0x00, 0x00, 0x00,           //
                                       0xab, 0xcd, 0xef, 0x11,           //
                                       0x22, 0x33, 0x44, 0x55
        };
        BytesRange packet(packetAAAA, packetAAAA + sizeof(packetAAAA));
        BytesRange aaaa_data(packetAAAA, packetAAAA + sizeof(packetAAAA));

        ResourceData data;
        const uint8_t * ptr = packetAAAA;
        NL_TEST_ASSERT(inSuite, data.Parse(aaaa_data, &ptr));

        // Parsing valid data makes requirement for IP addresses go away
        NL_TEST_ASSERT(inSuite, resolver.OnRecord(data, packet) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, !resolver.GetRequiredInformation().HasAny());
    }

    // Adding an irrelevant text entry
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const uint8_t packetTxt[] = {
            //
            4,    'a',  'b',  'c',  'd',                // QNAME part: abcd
            5,    'l',  'o',  'c',  'a', 'l',           // QNAME part: local
            0,                                          // QNAME ends
            0,    16,                                   // QType TXT
            0,    1,                                    // QClass IN
            0x12, 0x34, 0x56, 0x78,                     // TTL
            0,    26,                                   // data size - sum of entires
            4,    's',  'o',  'm',  'e',                // some
            7,    'f',  'o',  'o',  '=', 'b', 'a', 'r', // foo=bar
            5,    'x',  '=',  'y',  '=', 'z',           // x=y=z
            2,    'a',  '=',                            // a=
            3,    'T',  '=',  '1'                       // TCP supported = 1
        };
        BytesRange packet(packetTxt, packetTxt + sizeof(packetTxt));
        BytesRange txt_data(packetTxt, packetTxt + sizeof(packetTxt));

        ResourceData data;
        const uint8_t * ptr = packetTxt;
        NL_TEST_ASSERT(inSuite, data.Parse(txt_data, &ptr));

        // Parsing valid data makes requirement for IP addresses go away
        NL_TEST_ASSERT(inSuite, resolver.OnRecord(data, packet) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, !resolver.GetRequiredInformation().HasAny());
    }

    // Adding actual text entries that are useful
    // Note that TXT entries should be addressed to the Record address and
    // NOT to the server name for A/AAAA records
    {
        const uint8_t packetTxt[] = {
            // below is the same as `kTestOperationalname`:
            33,   '1',  '2',  '3',  '4', '5', '6', '7', '8', '9', // QNAME part:
            '8',  '7',  '6',  '5',  '4', '3', '2', '-', 'A', 'B', //    1234567898765432-ABCDEFEDCBAABCDE
            'C',  'D',  'E',  'F',  'E', 'D', 'C', 'B', 'A', 'A', //
            'B',  'C',  'D',  'E',                                //
            7,    '_',  'm',  'a',  't', 't', 'e', 'r',           // QNAME part: _matter
            4,    '_',  't',  'c',  'p',                          // QNAME part: _tcp
            5,    'l',  'o',  'c',  'a', 'l',                     // QNAME part: local
            0,                                                    // QNAME ends
            0,    16,                                             // QType TXT
            0,    1,                                              // QClass IN
            0x12, 0x34, 0x56, 0x78,                               // TTL
            0,    15,                                             // data size - sum of entires
            7,    'f',  'o',  'o',  '=', 'b', 'a', 'r',           // foo=bar
            6,    'S',  'I',  'I',  '=', '2', '3',                // SII=23 (sleepy idle interval)
        };
        BytesRange packet(packetTxt, packetTxt + sizeof(packetTxt));
        BytesRange txt_data(packetTxt, packetTxt + sizeof(packetTxt));

        ResourceData data;
        const uint8_t * ptr = packetTxt;
        NL_TEST_ASSERT(inSuite, data.Parse(txt_data, &ptr));

        // Parsing valid data makes requirement for IP addresses go away
        NL_TEST_ASSERT(inSuite, resolver.OnRecord(data, packet) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, !resolver.GetRequiredInformation().HasAny());
    }

    // At this point taking value should work. Once taken, the resolver
    // is reset.
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

const nlTest sTests[] = {
    // Tests for helper class
    NL_TEST_DEF("StoredServerName", TestStoredServerName), //

    // Actual resolver tests
    NL_TEST_DEF("Creation", TestCreation),                       //
    NL_TEST_DEF("StartOperational", TestStartOperational),       //
    NL_TEST_DEF("StartCommissionable", TestStartCommissionable), //
    NL_TEST_DEF("StartCommissioner", TestStartCommissioner),     //
    NL_TEST_DEF("ParseOperational", TestParseOperational),       //
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
