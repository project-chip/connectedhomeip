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
#include <lib/dnssd/Advertiser.h>

#include <string>
#include <utility>

#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/minimal_mdns/Query.h>
#include <lib/dnssd/minimal_mdns/QueryBuilder.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/dnssd/minimal_mdns/tests/CheckOnlyServer.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

namespace {

using namespace std;
using namespace chip;
using namespace chip::Dnssd;
using namespace mdns::Minimal;
using namespace mdns::Minimal::test;

// Used for everything.
const QNamePart kDnsSdQueryParts[]  = { "_services", "_dns-sd", "_udp", "local" };
const FullQName kDnsSdQueryName     = FullQName(kDnsSdQueryParts);
constexpr size_t kMdnsMaxPacketSize = 512;
const uint8_t kMac[kMaxMacSize]     = { 1, 2, 3, 4, 5, 6, 7, 8 };
const QNamePart kHostnameParts[]    = { "0102030405060708", "local" };
const FullQName kHostnameName       = FullQName(kHostnameParts);

// Operational records and queries.
const QNamePart kMatterOperationalQueryParts[3] = { "_matter", "_tcp", "local" };
const FullQName kMatterOperationalQueryName     = FullQName(kMatterOperationalQueryParts);
const PeerId kPeerId1                           = PeerId().SetCompressedFabricId(0xBEEFBEEFF00DF00D).SetNodeId(0x1111222233334444);
const PeerId kPeerId2                           = PeerId().SetCompressedFabricId(0x5555666677778888).SetNodeId(0x1212343456567878);
const PeerId kPeerId3                           = PeerId().SetCompressedFabricId(0x3333333333333333).SetNodeId(0x3333333333333333);
const PeerId kPeerId4                           = PeerId().SetCompressedFabricId(0x4444444444444444).SetNodeId(0x4444444444444444);
const PeerId kPeerId5                           = PeerId().SetCompressedFabricId(0x5555555555555555).SetNodeId(0x5555555555555555);
const PeerId kPeerId6                           = PeerId().SetCompressedFabricId(0x6666666666666666).SetNodeId(0x6666666666666666);
const QNamePart kInstanceNameParts1[]           = { "BEEFBEEFF00DF00D-1111222233334444", "_matter", "_tcp", "local" };
const FullQName kInstanceName1                  = FullQName(kInstanceNameParts1);
const QNamePart kInstanceNameParts2[]           = { "5555666677778888-1212343456567878", "_matter", "_tcp", "local" };
const FullQName kInstanceName2                  = FullQName(kInstanceNameParts2);
const QNamePart kTxtRecordEmptyParts[]          = { "=" };
const FullQName kTxtRecordEmptyName             = FullQName(kTxtRecordEmptyParts);
const QNamePart kCompressedIdSubParts1[]        = { "_IBEEFBEEFF00DF00D", "_sub", "_matter", "_tcp", "local" };
FullQName kCompressedIdSubName1                 = FullQName(kCompressedIdSubParts1);
const QNamePart kCompressedIdSubParts2[]        = { "_I5555666677778888", "_sub", "_matter", "_tcp", "local" };
FullQName kCompressedIdSubName2                 = FullQName(kCompressedIdSubParts2);
PtrResourceRecord ptrServiceSubCompressedId1    = PtrResourceRecord(kDnsSdQueryName, kCompressedIdSubName1);
PtrResourceRecord ptrServiceSubCompressedId2    = PtrResourceRecord(kDnsSdQueryName, kCompressedIdSubName2);

OperationalAdvertisingParameters operationalParams1 =
    OperationalAdvertisingParameters()
        .SetPeerId(kPeerId1)
        .SetMac(ByteSpan(kMac))
        .SetPort(CHIP_PORT)
        .EnableIpV4(true)
        .SetTcpSupported(chip::Optional<bool>(false))
        .SetLocalMRPConfig(Optional<ReliableMessageProtocolConfig>::Value(32_ms32, 30_ms32)); // Match SII, SAI below
OperationalAdvertisingParameters operationalParams2 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId2).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
OperationalAdvertisingParameters operationalParams3 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId3).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
OperationalAdvertisingParameters operationalParams4 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId4).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
OperationalAdvertisingParameters operationalParams5 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId5).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
OperationalAdvertisingParameters operationalParams6 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId6).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
const QNamePart txtOperational1Parts[]  = { "SII=32", "SAI=30", "T=0" };
PtrResourceRecord ptrOperationalService = PtrResourceRecord(kDnsSdQueryName, kMatterOperationalQueryName);
PtrResourceRecord ptrOperational1       = PtrResourceRecord(kMatterOperationalQueryName, kInstanceName1);
SrvResourceRecord srvOperational1       = SrvResourceRecord(kInstanceName1, kHostnameName, CHIP_PORT);
TxtResourceRecord txtOperational1       = TxtResourceRecord(kInstanceName1, txtOperational1Parts);
PtrResourceRecord ptrOperational2       = PtrResourceRecord(kMatterOperationalQueryName, kInstanceName2);
SrvResourceRecord srvOperational2       = SrvResourceRecord(kInstanceName2, kHostnameName, CHIP_PORT);
TxtResourceRecord txtOperational2       = TxtResourceRecord(kInstanceName2, kTxtRecordEmptyName);

// Commissionable node records and queries.
const QNamePart kMatterCommissionableNodeQueryParts[3] = { "_matterc", "_udp", "local" };
const QNamePart kLongSubPartsFullLen[]                 = { "_L4094", "_sub", "_matterc", "_udp", "local" };
const QNamePart kShortSubPartsFullLen[]                = { "_S15", "_sub", "_matterc", "_udp", "local" };
const QNamePart kCmSubParts[]                          = { "_CM", "_sub", "_matterc", "_udp", "local" };
const QNamePart kLongSubParts[]                        = { "_L22", "_sub", "_matterc", "_udp", "local" };
const QNamePart kShortSubParts[]                       = { "_S2", "_sub", "_matterc", "_udp", "local" };
const QNamePart kVendorSubParts[]                      = { "_V555", "_sub", "_matterc", "_udp", "local" };
const QNamePart kDeviceTypeSubParts[]                  = { "_T70000", "_sub", "_matterc", "_udp", "local" };
const FullQName kMatterCommissionableNodeQueryName     = FullQName(kMatterCommissionableNodeQueryParts);
FullQName kLongSubFullLenName                          = FullQName(kLongSubPartsFullLen);
FullQName kShortSubFullLenName                         = FullQName(kShortSubPartsFullLen);
FullQName kCmSubName                                   = FullQName(kCmSubParts);
FullQName kLongSubName                                 = FullQName(kLongSubParts);
FullQName kShortSubName                                = FullQName(kShortSubParts);
FullQName kVendorSubName                               = FullQName(kVendorSubParts);
FullQName kDeviceTypeSubName                           = FullQName(kDeviceTypeSubParts);
PtrResourceRecord ptrCommissionableNodeService         = PtrResourceRecord(kDnsSdQueryName, kMatterCommissionableNodeQueryName);
PtrResourceRecord ptrServiceSubLFullLen                = PtrResourceRecord(kDnsSdQueryName, kLongSubFullLenName);
PtrResourceRecord ptrServiceSubSFullLen                = PtrResourceRecord(kDnsSdQueryName, kShortSubFullLenName);
PtrResourceRecord ptrServiceSubCM                      = PtrResourceRecord(kDnsSdQueryName, kCmSubName);
PtrResourceRecord ptrServiceSubLong                    = PtrResourceRecord(kDnsSdQueryName, kLongSubName);
PtrResourceRecord ptrServiceSubShort                   = PtrResourceRecord(kDnsSdQueryName, kShortSubName);
PtrResourceRecord ptrServiceSubVendor                  = PtrResourceRecord(kDnsSdQueryName, kVendorSubName);
PtrResourceRecord ptrServiceSubDeviceType              = PtrResourceRecord(kDnsSdQueryName, kDeviceTypeSubName);

// For commissioning, the instance name is chosen randomly by the advertiser, so we have to get this value from it. We can, however,
// pre-populate the records with the ptr.
char instanceNamePrefix[17];
QNamePart instanceNameParts[]           = { instanceNamePrefix, "_matterc", "_udp", "local" };
FullQName instanceName                  = FullQName(instanceNameParts);
PtrResourceRecord ptrCommissionableNode = PtrResourceRecord(kMatterCommissionableNodeQueryName, instanceName);
SrvResourceRecord srvCommissionableNode = SrvResourceRecord(instanceName, kHostnameName, CHIP_PORT);

CommissionAdvertisingParameters commissionableNodeParamsSmall =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac))
        .SetLongDiscriminator(0xFFE)
        .SetShortDiscriminator(0xF)
        .SetCommissioningMode(CommissioningMode::kDisabled);
const QNamePart txtCommissionableNodeParamsSmallParts[] = { "CM=0", "D=4094" };
FullQName txtCommissionableNodeParamsSmallName          = FullQName(txtCommissionableNodeParamsSmallParts);
TxtResourceRecord txtCommissionableNodeParamsSmall      = TxtResourceRecord(instanceName, txtCommissionableNodeParamsSmallName);

CommissionAdvertisingParameters commissionableNodeParamsLargeBasic =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac, sizeof(kMac)))
        .SetLongDiscriminator(22)
        .SetShortDiscriminator(2)
        .SetVendorId(chip::Optional<uint16_t>(555))
        .SetDeviceType(chip::Optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledBasic)
        .SetDeviceName(chip::Optional<const char *>("testy-test"))
        .SetPairingHint(chip::Optional<uint16_t>(3))
        .SetPairingInstruction(chip::Optional<const char *>("Pair me"))
        .SetProductId(chip::Optional<uint16_t>(897))
        .SetRotatingDeviceId(chip::Optional<const char *>("id_that_spins"));
QNamePart txtCommissionableNodeParamsLargeBasicParts[] = { "D=22",          "VP=555+897",       "CM=1",       "DT=70000",
                                                           "DN=testy-test", "RI=id_that_spins", "PI=Pair me", "PH=3" };
FullQName txtCommissionableNodeParamsLargeBasicName    = FullQName(txtCommissionableNodeParamsLargeBasicParts);
TxtResourceRecord txtCommissionableNodeParamsLargeBasic =
    TxtResourceRecord(instanceName, txtCommissionableNodeParamsLargeBasicName);

CommissionAdvertisingParameters commissionableNodeParamsLargeEnhanced =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac, sizeof(kMac)))
        .SetLongDiscriminator(22)
        .SetShortDiscriminator(2)
        .SetVendorId(chip::Optional<uint16_t>(555))
        .SetDeviceType(chip::Optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledEnhanced)
        .SetDeviceName(chip::Optional<const char *>("testy-test"))
        .SetPairingHint(chip::Optional<uint16_t>(3))
        .SetPairingInstruction(chip::Optional<const char *>("Pair me"))
        .SetProductId(chip::Optional<uint16_t>(897))
        .SetRotatingDeviceId(chip::Optional<const char *>("id_that_spins"))
        .SetTcpSupported(chip::Optional<bool>(true))
        // 3600005 is more than the max so should be adjusted down
        .SetLocalMRPConfig(Optional<ReliableMessageProtocolConfig>::Value(3600000_ms32, 3600005_ms32));
QNamePart txtCommissionableNodeParamsLargeEnhancedParts[] = { "D=22",          "VP=555+897",       "CM=2",       "DT=70000",
                                                              "DN=testy-test", "RI=id_that_spins", "PI=Pair me", "PH=3",
                                                              "SAI=3600000",   "SII=3600000",      "T=1" };
FullQName txtCommissionableNodeParamsLargeEnhancedName    = FullQName(txtCommissionableNodeParamsLargeEnhancedParts);
TxtResourceRecord txtCommissionableNodeParamsLargeEnhanced =
    TxtResourceRecord(instanceName, txtCommissionableNodeParamsLargeEnhancedName);

// Our server doesn't do anything with this, blank is fine.
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
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.RemoveServices() == CHIP_NO_ERROR);

    auto & server = static_cast<CheckOnlyServer &>(GlobalMinimalMdnsServer::Server());
    server.SetTestSuite(inSuite);
    server.Reset();

    // Start a single operational advertiser
    ChipLogProgress(Discovery, "Testing single operational advertiser");
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);

    // Test for PTR response to _services request.
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrServiceSubCompressedId1);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);

    // These check that the requested records added are sent out correctly.
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Want PTR response to _matter.tcp. We will also get the SRV and TXT as additionals.
    // We won't get any A/AAAA because this is a test and we don't have addresses.
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    // For now, we don't check TXT records content, just that they exist. Operational currently
    // sends a TXT record regardless of content being present or not.
    server.AddExpectedRecord(&ptrOperational1);
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);

    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // If we try to re-advertise with the same operational parameters, we should not get duplicates
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);
    ChipLogProgress(Discovery, "Testing single operational advertiser with Advertise called twice");
    // We should get a single PTR back for _services
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrServiceSubCompressedId1);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Same records should come back for _matter._tcp.local queries.
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrOperational1);
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Adding a second operational advertiser.
    ChipLogProgress(Discovery, "Adding a second operational Advertiser");
    server.Reset();
    // Mac is the same, peer id is different
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);

    // For now, we'll get back two copies of the PTR. Not sure if that's totally correct, but for now, that's expected.
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrServiceSubCompressedId1);
    server.AddExpectedRecord(&ptrServiceSubCompressedId2);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for _matter._tcp.local will respond with all records from both parameter sets
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrOperational1);
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    server.AddExpectedRecord(&ptrOperational2);
    server.AddExpectedRecord(&srvOperational2);
    server.AddExpectedRecord(&txtOperational2);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for each SRV record should give only records specific to that fabric.
    ChipLogProgress(Discovery, "Testing response to instance name for fabric 1");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name for fabric 2");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvOperational2);
    server.AddExpectedRecord(&txtOperational2);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // All devices should support at least 5 operational network additions (spec min)
    // however larger devices may support more.
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams5) == CHIP_NO_ERROR);
}

void CommissionableAdverts(nlTestSuite * inSuite, void * inContext)
{
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.RemoveServices() == CHIP_NO_ERROR);

    auto & server = static_cast<CheckOnlyServer &>(GlobalMinimalMdnsServer::Server());
    server.SetTestSuite(inSuite);
    server.Reset();

    // Start a single operational advertiser
    ChipLogProgress(Discovery, "Testing commissionable advertiser");
    // Start very basic - only the mandatory values (short and long discriminator and commissioning modes)
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(commissionableNodeParamsSmall) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);

    // Test for PTR response to _services request.
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local for small parameters");
    server.AddExpectedRecord(&ptrCommissionableNodeService);
    server.AddExpectedRecord(&ptrServiceSubLFullLen);
    server.AddExpectedRecord(&ptrServiceSubSFullLen);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    // These check that the requested records added are sent out correctly.
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Want PTR response to _matterc._udp. We will also get the SRV and TXT as additionals.
    // We won't get any A/AAAA because this is a test and we don't have addresses.
    // First fill in the instance name - FullQNames already have this space included.
    NL_TEST_ASSERT(inSuite,
                   mdnsAdvertiser.GetCommissionableInstanceName(instanceNamePrefix, sizeof(instanceNamePrefix)) == CHIP_NO_ERROR);

    ChipLogProgress(Discovery, "Testing response to _matterc._udp.local for small parameters");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNode);
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsSmall);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterCommissionableNodeQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name for small parameters");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsSmall);
    NL_TEST_ASSERT(inSuite, SendQuery(instanceName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Add more parameters, check that the subtypes and TXT values get set correctly.
    // Also check that we get proper values when the discriminators are small (no leading 0's)
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(commissionableNodeParamsLargeBasic) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local for large basic parameters");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNodeService);
    server.AddExpectedRecord(&ptrServiceSubLong);
    server.AddExpectedRecord(&ptrServiceSubShort);
    server.AddExpectedRecord(&ptrServiceSubCM);
    server.AddExpectedRecord(&ptrServiceSubVendor);
    server.AddExpectedRecord(&ptrServiceSubDeviceType);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to _matterc._udp.local for large basic parameters");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNode);
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeBasic);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterCommissionableNodeQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name for large basic parameters");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeBasic);
    NL_TEST_ASSERT(inSuite, SendQuery(instanceName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(commissionableNodeParamsLargeEnhanced) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local for large enhanced parameters");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNodeService);
    server.AddExpectedRecord(&ptrServiceSubLong);
    server.AddExpectedRecord(&ptrServiceSubShort);
    server.AddExpectedRecord(&ptrServiceSubCM);
    server.AddExpectedRecord(&ptrServiceSubVendor);
    server.AddExpectedRecord(&ptrServiceSubDeviceType);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to _matterc._udp.local for large enhanced parameters");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNode);
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeEnhanced);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterCommissionableNodeQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to instance name for large enhanced parameters");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeEnhanced);
    NL_TEST_ASSERT(inSuite, SendQuery(instanceName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());
}

void CommissionableAndOperationalAdverts(nlTestSuite * inSuite, void * inContext)
{
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.RemoveServices() == CHIP_NO_ERROR);

    auto & server = static_cast<CheckOnlyServer &>(GlobalMinimalMdnsServer::Server());
    server.SetTestSuite(inSuite);
    server.Reset();

    // Add two operational and a commissionable and test that we get the correct values back.
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(operationalParams2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.Advertise(commissionableNodeParamsLargeEnhanced) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsAdvertiser.FinalizeServiceUpdate() == CHIP_NO_ERROR);

    // Services listing should have two operational ptrs, the base commissionable node ptr and the various _sub ptrs
    ChipLogProgress(Discovery, "Checking response to _services._dns-sd._udp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrOperationalService);
    server.AddExpectedRecord(&ptrCommissionableNodeService);
    server.AddExpectedRecord(&ptrServiceSubLong);
    server.AddExpectedRecord(&ptrServiceSubShort);
    server.AddExpectedRecord(&ptrServiceSubCM);
    server.AddExpectedRecord(&ptrServiceSubVendor);
    server.AddExpectedRecord(&ptrServiceSubDeviceType);
    server.AddExpectedRecord(&ptrServiceSubCompressedId1);
    server.AddExpectedRecord(&ptrServiceSubCompressedId2);
    NL_TEST_ASSERT(inSuite, SendQuery(kDnsSdQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for _matter._tcp.local will respond with all records from both operational records, but no commissionable.
    ChipLogProgress(Discovery, "Testing response to _matter._tcp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrOperational1);
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    server.AddExpectedRecord(&ptrOperational2);
    server.AddExpectedRecord(&srvOperational2);
    server.AddExpectedRecord(&txtOperational2);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterOperationalQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Responses to _matterc query should return commissionable node, but no operational.
    ChipLogProgress(Discovery, "Testing response to _matterc._udp.local");
    server.Reset();
    server.AddExpectedRecord(&ptrCommissionableNode);
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeEnhanced);
    NL_TEST_ASSERT(inSuite, SendQuery(kMatterCommissionableNodeQueryName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    // Requests for each SRV record should give only records specific to that fabric.
    ChipLogProgress(Discovery, "Testing response to operational instance name for fabric 1");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvOperational1);
    server.AddExpectedRecord(&txtOperational1);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to operational instance name for fabric 2");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvOperational2);
    server.AddExpectedRecord(&txtOperational2);
    NL_TEST_ASSERT(inSuite, SendQuery(kInstanceName2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());

    ChipLogProgress(Discovery, "Testing response to commissionable instance name");
    server.Reset();
    // Just the SRV and TXT should return
    server.AddExpectedRecord(&srvCommissionableNode);
    server.AddExpectedRecord(&txtCommissionableNodeParamsLargeEnhanced);
    NL_TEST_ASSERT(inSuite, SendQuery(instanceName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, server.GetHeaderFound());
}

const nlTest sTests[] = {
    NL_TEST_DEF("OperationalAdverts", OperationalAdverts),                                   //
    NL_TEST_DEF("CommissionableNodeAdverts", CommissionableAdverts),                         //
    NL_TEST_DEF("CommissionableAndOperationalAdverts", CommissionableAndOperationalAdverts), //
    NL_TEST_SENTINEL()                                                                       //
};

} // namespace

int TestAdvertiser(void)
{
    chip::Platform::MemoryInit();
    chip::Test::IOContext context;
    context.Init();
    nlTestSuite theSuite = { "AdvertiserImplMinimal", sTests, nullptr, nullptr };
    CheckOnlyServer server(&theSuite);
    test::ServerSwapper swapper(&server);
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    mdnsAdvertiser.Init(context.GetUDPEndPointManager());
    nlTestRunner(&theSuite, &server);
    server.Shutdown();
    context.Shutdown();
    mdnsAdvertiser.RemoveServices();
    mdnsAdvertiser.Shutdown();
    chip::Platform::MemoryShutdown();

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAdvertiser)
