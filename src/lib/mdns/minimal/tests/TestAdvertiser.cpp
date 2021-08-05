/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <mdns/Advertiser.h>

#include <string>
#include <utility>

#include <mdns/Advertiser.h>
#include <mdns/MinimalMdnsServer.h>
#include <mdns/minimal/Query.h>
#include <mdns/minimal/QueryBuilder.h>
#include <mdns/minimal/core/QName.h>
#include <mdns/minimal/records/Ptr.h>
#include <mdns/minimal/records/Srv.h>
#include <mdns/minimal/records/Txt.h>
#include <mdns/minimal/tests/CheckOnlyServer.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>

#include <nlunit-test.h>

namespace {

using namespace std;
using namespace chip;
using namespace chip::Mdns;
using namespace mdns::Minimal;
using namespace mdns::Minimal::test;

const QNamePart kDnsSdQueryParts[]              = { "_services", "_dns-sd", "_udp", "local" };
const FullQName kDnsSdQueryName                 = FullQName(kDnsSdQueryParts);
const QNamePart kMatterOperationalQueryParts[3] = { "_matter", "_tcp", "local" };
const FullQName kMatterOperationalQueryName     = FullQName(kMatterOperationalQueryParts);

constexpr size_t kMdnsMaxPacketSize = 512;

const uint8_t kMac[kMaxMacSize]        = { 1, 2, 3, 4, 5, 6, 7, 8 };
const PeerId kPeerId1                  = PeerId().SetFabricId(0xBEEFBEEFF00DF00D).SetNodeId(0x1111222233334444);
const PeerId kPeerId2                  = PeerId().SetFabricId(0x5555666677778888).SetNodeId(0x1212343456567878);
const PeerId kPeerId3                  = PeerId().SetFabricId(0x3333333333333333).SetNodeId(0x3333333333333333);
const PeerId kPeerId4                  = PeerId().SetFabricId(0x4444444444444444).SetNodeId(0x4444444444444444);
const PeerId kPeerId5                  = PeerId().SetFabricId(0x5555555555555555).SetNodeId(0x5555555555555555);
const PeerId kPeerId6                  = PeerId().SetFabricId(0x6666666666666666).SetNodeId(0x6666666666666666);
const QNamePart kInstanceNameParts1[]  = { "BEEFBEEFF00DF00D-1111222233334444", "_matter", "_tcp", "local" };
const FullQName kInstanceName1         = FullQName(kInstanceNameParts1);
const QNamePart kInstanceNameParts2[]  = { "5555666677778888-1212343456567878", "_matter", "_tcp", "local" };
const FullQName kInstanceName2         = FullQName(kInstanceNameParts2);
const QNamePart kHostnameParts[]       = { "0102030405060708", "local" };
const FullQName kHostnameName          = FullQName(kHostnameParts);
const QNamePart kTxtRecordEmptyParts[] = { "=" };
const FullQName kTxtRecordEmptyName    = FullQName(kTxtRecordEmptyParts);

Inet::IPPacketInfo packetInfo;

CHIP_ERROR SendQuery(FullQName qname)
{
    System::PacketBufferHandle queryBuffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    if (queryBuffer.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    QueryBuilder queryBuilder(std::move(queryBuffer));
    queryBuilder.Header().SetMessageId(0);
    Query query = Query(qname).SetClass(QClass::IN).SetType(QType::ANY);
    queryBuilder.AddQuery(query);
    queryBuffer = queryBuilder.ReleasePacket();

    // Send the query to the advertiser directly.
    BytesRange range = BytesRange(queryBuffer->Start(), queryBuffer->Start() + queryBuffer->DataLength());
    GlobalMinimalMdnsServer::Instance().OnQuery(range, &packetInfo);
    return CHIP_NO_ERROR;
}

void OperationalAdverts(nlTestSuite * inSuite, void * inContext)
{
    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();
    mdnsAdvertiser.StopPublishDevice();
    auto & server = reinterpret_cast<CheckOnlyServer &>(GlobalMinimalMdnsServer::Server());
    server.SetTestSuite(inSuite);
    server.Reset();

    // Start a single operational advertiser
    ChipLogProgress(Discovery, "Testing single operational advertiser");
    OperationalAdvertisingParameters params =
        OperationalAdvertisingParameters().SetPeerId(kPeerId1).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params) == CHIP_NO_ERROR);

    // Test for PTR response to _services request.
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    PtrResourceRecord ptr_service = PtrResourceRecord(kDnsSdQueryName, kMatterOperationalQueryName);
    server.AddExpectedRecord(&ptr_service);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);

    // These check that the requested records added are sent out correctly.
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Want PTR response to _matter.tcp. We will also get the SRV and TXT as additionals.
    // We won't get any A/AAAA because this is a test and we don't have addresses.
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    PtrResourceRecord ptr_matter1 = PtrResourceRecord(kMatterOperationalQueryName, kInstanceName1);
    SrvResourceRecord srv1        = SrvResourceRecord(kInstanceName1, kHostnameName, CHIP_PORT);
    // For now, we don't check TXT records content, just that they exist. Operational currently
    // sends a TXT record regardless of content being present or not.
    TxtResourceRecord txt1 = TxtResourceRecord(kInstanceName1, kTxtRecordEmptyName);
    server.AddExpectedRecord(&ptr_matter1);
    server.AddExpectedRecord(&srv1);
    server.AddExpectedRecord(&txt1);

    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srv1);
    server.AddExpectedRecord(&txt1);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // If we try to re-advertise with the same operational parameters, we should not get duplicates
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params) == CHIP_NO_ERROR);
    ChipLogProgress(Discovery, "Testing single operational advertiser with Advertise called twice");
    // We should get a single PTR back for _services
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.Reset();
    server.AddExpectedRecord(&ptr_service);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Same records should come back for _matter._tcp.local queries.
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    server.AddExpectedRecord(&ptr_matter1);
    server.AddExpectedRecord(&srv1);
    server.AddExpectedRecord(&txt1);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Adding a second operational advertiser.
    ChipLogProgress(Discovery, "Adding a second operational Advertiser");
    server.Reset();
    // Mac is the same, peer id is different
    OperationalAdvertisingParameters params2 =
        OperationalAdvertisingParameters().SetPeerId(kPeerId2).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params2) == CHIP_NO_ERROR);

    // For now, we'll get back two copies of the PTR. Not sure if that's totally correct, but for now, that's expected.
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.AddExpectedRecord(&ptr_service);
    server.AddExpectedRecord(&ptr_service);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for _matter._tcp.local will respond with all records from both parameter sets
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    PtrResourceRecord ptr_matter2 = PtrResourceRecord(kMatterOperationalQueryName, kInstanceName2);
    SrvResourceRecord srv2        = SrvResourceRecord(kInstanceName2, kHostnameName, CHIP_PORT);
    TxtResourceRecord txt2        = TxtResourceRecord(kInstanceName2, kTxtRecordEmptyName);
    server.AddExpectedRecord(&ptr_matter1);
    server.AddExpectedRecord(&srv1);
    server.AddExpectedRecord(&txt1);
    server.AddExpectedRecord(&ptr_matter2);
    server.AddExpectedRecord(&srv2);
    server.AddExpectedRecord(&txt2);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for each SRV record should give only records specific to that fabric.
    ChipLogProgress(Discovery, "Testing response to instance name for fabric 1");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srv1);
    server.AddExpectedRecord(&txt1);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name for fabric 2");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srv2);
    server.AddExpectedRecord(&txt2);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // We should be able to add up to 5 operational networks total
    OperationalAdvertisingParameters params3 =
        OperationalAdvertisingParameters().SetPeerId(kPeerId3).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params3) == CHIP_NO_ERROR);
    OperationalAdvertisingParameters params4 =
        OperationalAdvertisingParameters().SetPeerId(kPeerId4).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params4) == CHIP_NO_ERROR);
    OperationalAdvertisingParameters params5 =
        OperationalAdvertisingParameters().SetPeerId(kPeerId5).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params5) == CHIP_NO_ERROR);

    // Adding a 6th should return an error
    OperationalAdvertisingParameters params6 =
        OperationalAdvertisingParameters().SetPeerId(kPeerId6).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(params6) == CHIP_ERROR_NO_MEMORY);
}

const nlTest sTests[] = {
    NL_TEST_DEF("OperationalAdverts", OperationalAdverts), //
    NL_TEST_SENTINEL()                                     //
};

} // namespace

int TestAdvertiser(void)
{
    chip::Platform::MemoryInit();
    nlTestSuite theSuite = { "AdvertiserImplMinimal", sTests, nullptr, nullptr };
    CheckOnlyServer server(&theSuite);
    test::ServerSwapper swapper(&server);
    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();
    mdnsAdvertiser.Start(nullptr, CHIP_PORT);
    nlTestRunner(&theSuite, &server);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAdvertiser)
