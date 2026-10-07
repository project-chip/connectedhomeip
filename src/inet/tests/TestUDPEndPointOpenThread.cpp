/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <inet/InetConfig.h>
#include <inet/UDPEndPointImpl.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <pw_unit_test/framework.h>

#include <openthread-system.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/message.h>
#include <openthread/tasklet.h>

#include <cstring>
#include <vector>

using namespace chip;
using namespace chip::Inet;

namespace {

constexpr uint8_t kIcmp6CodePortUnreach   = 4;
constexpr uint8_t kIpProtoUdp             = 17;
constexpr uint8_t kIpProtoIcmp6           = 58;
constexpr uint16_t kPeerPort              = 5540;
constexpr const char kPeerAddressString[] = "2001:db8::2";

struct Report
{
    int count = 0;
    IPAddress peerAddress;
    uint16_t peerPort = 0;
    std::vector<uint8_t> quoted;
};

void RecordReport(UDPEndPoint * endPoint, const IPPacketInfo & pktInfo, ByteSpan quotedPayload)
{
    auto * report = static_cast<Report *>(endPoint->mAppState);
    report->count++;
    report->peerAddress = pktInfo.DestAddress;
    report->peerPort    = pktInfo.DestPort;
    report->quoted.assign(quotedPayload.begin(), quotedPayload.end());
}

void IgnoreDatagram(UDPEndPoint *, System::PacketBufferHandle &&, const IPPacketInfo *) {}

void Put16(std::vector<uint8_t> & out, uint16_t value)
{
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

void PutAddress(std::vector<uint8_t> & out, const otIp6Address & address)
{
    out.insert(out.end(), address.mFields.m8, address.mFields.m8 + sizeof(address.mFields.m8));
}

void PutIp6Header(std::vector<uint8_t> & out, uint16_t payloadLength, uint8_t nextHeader, const otIp6Address & source,
                  const otIp6Address & destination)
{
    out.insert(out.end(), { 0x60, 0, 0, 0 });
    Put16(out, payloadLength);
    out.push_back(nextHeader);
    out.push_back(64);
    PutAddress(out, source);
    PutAddress(out, destination);
}

uint16_t Icmp6Checksum(const otIp6Address & source, const otIp6Address & destination, const std::vector<uint8_t> & icmp)
{
    std::vector<uint8_t> pseudo;
    PutAddress(pseudo, source);
    PutAddress(pseudo, destination);
    Put16(pseudo, 0);
    Put16(pseudo, static_cast<uint16_t>(icmp.size()));
    pseudo.insert(pseudo.end(), { 0, 0, 0, kIpProtoIcmp6 });
    pseudo.insert(pseudo.end(), icmp.begin(), icmp.end());
    if (pseudo.size() % 2)
    {
        pseudo.push_back(0);
    }
    uint32_t sum = 0;
    for (size_t i = 0; i < pseudo.size(); i += 2)
    {
        sum += static_cast<uint32_t>(pseudo[i] << 8 | pseudo[i + 1]);
    }
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return static_cast<uint16_t>(~sum);
}

class TestUDPEndPointOpenThread : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        static char arg0[]   = "TestUDPEndPointOpenThread";
        static char logArg[] = "--log-file=/tmp/TestUDPEndPointOpenThread.log";
        static char nodeId[] = "1";
        char * args[]        = { arg0, logArg, nodeId };
        otSysInit(3, args);
        sInstance = otInstanceInitSingle();
        ASSERT_NE(sInstance, nullptr);
        ASSERT_EQ(otIp6SetEnabled(sInstance, true), OT_ERROR_NONE);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        ASSERT_EQ(sUDP.Init(DeviceLayer::SystemLayer()), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        sUDP.Shutdown();
        DeviceLayer::PlatformMgr().Shutdown();
        otInstanceFinalize(sInstance);
        Platform::MemoryShutdown();
    }

    void SetUp() override { DeviceLayer::PlatformMgr().LockChipStack(); }
    void TearDown() override { DeviceLayer::PlatformMgr().UnlockChipStack(); }

protected:
    void OpenEndPoint(UDPEndPointHandle & ep, Report & report)
    {
        ASSERT_EQ(sUDP.NewEndPoint(ep), CHIP_NO_ERROR);
        EndPointStateOpenThread::OpenThreadEndpointInitParam params;
        params.openThreadInstancePtr = sInstance;
        ep->SetNativeParams(&params);
        ASSERT_EQ(ep->Bind(IPAddressType::kIPv6, IPAddress::Any, 0), CHIP_NO_ERROR);
        ASSERT_EQ(ep->Listen(IgnoreDatagram, nullptr, &report), CHIP_NO_ERROR);
        ep->SetPortUnreachableHandler(RecordReport);
    }

    static otIp6Address LinkLocalAddress()
    {
        for (const otNetifAddress * addr = otIp6GetUnicastAddresses(sInstance); addr != nullptr; addr = addr->mNext)
        {
            if (addr->mAddress.mFields.m8[0] == 0xfe && (addr->mAddress.mFields.m8[1] & 0xc0) == 0x80)
            {
                return addr->mAddress;
            }
        }
        return {};
    }

    // Delivers a port-unreachable from `sender` quoting a datagram from our link-local address and `localPort` to
    // `quotedDestination`:kPeerPort carrying `payload`.
    static void DeliverPortUnreachable(const char * sender, const char * quotedDestination, uint16_t localPort,
                                       const char * payload)
    {
        otIp6Address local = LinkLocalAddress();
        otIp6Address from;
        otIp6Address to;
        ASSERT_EQ(otIp6AddressFromString(sender, &from), OT_ERROR_NONE);
        ASSERT_EQ(otIp6AddressFromString(quotedDestination, &to), OT_ERROR_NONE);

        const size_t payloadLen = strlen(payload);
        std::vector<uint8_t> quoted;
        PutIp6Header(quoted, static_cast<uint16_t>(8 + payloadLen), kIpProtoUdp, local, to);
        Put16(quoted, localPort);
        Put16(quoted, kPeerPort);
        Put16(quoted, static_cast<uint16_t>(8 + payloadLen));
        Put16(quoted, 0);
        quoted.insert(quoted.end(), payload, payload + payloadLen);

        std::vector<uint8_t> icmp = { OT_ICMP6_TYPE_DST_UNREACH, kIcmp6CodePortUnreach, 0, 0, 0, 0, 0, 0 };
        icmp.insert(icmp.end(), quoted.begin(), quoted.end());
        const uint16_t checksum = Icmp6Checksum(from, local, icmp);
        icmp[2]                 = static_cast<uint8_t>(checksum >> 8);
        icmp[3]                 = static_cast<uint8_t>(checksum);

        std::vector<uint8_t> packet;
        PutIp6Header(packet, static_cast<uint16_t>(icmp.size()), kIpProtoIcmp6, from, local);
        packet.insert(packet.end(), icmp.begin(), icmp.end());

        otMessage * message = otIp6NewMessage(sInstance, nullptr);
        ASSERT_NE(message, nullptr);
        ASSERT_EQ(otMessageAppend(message, packet.data(), static_cast<uint16_t>(packet.size())), OT_ERROR_NONE);
        ASSERT_EQ(otIp6Send(sInstance, message), OT_ERROR_NONE);
        otTaskletsProcess(sInstance);
    }

    // Runs the event loop long enough for the endpoint's deferred dispatch.
    static void ServiceEvents()
    {
        ASSERT_EQ(DeviceLayer::SystemLayer().StartTimer(
                      System::Clock::Milliseconds32(50),
                      [](System::Layer *, void *) { RETURN_SAFELY_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask(); },
                      nullptr),
                  CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().UnlockChipStack();
        DeviceLayer::PlatformMgr().RunEventLoop();
        DeviceLayer::PlatformMgr().LockChipStack();
    }

    static otInstance * sInstance;
    static UDPEndPointManagerImpl sUDP;
};

otInstance * TestUDPEndPointOpenThread::sInstance = nullptr;
UDPEndPointManagerImpl TestUDPEndPointOpenThread::sUDP;

TEST_F(TestUDPEndPointOpenThread, ReportsPortUnreachableFromQuotedDestination)
{
    Report report;
    UDPEndPointHandle ep;
    OpenEndPoint(ep, report);

    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, ep->GetBoundPort(), "hello");
    ServiceEvents();

    EXPECT_EQ(report.count, 1);
    IPAddress peer;
    ASSERT_TRUE(IPAddress::FromString(kPeerAddressString, peer));
    EXPECT_TRUE(report.peerAddress == peer);
    EXPECT_EQ(report.peerPort, kPeerPort);
    EXPECT_EQ(report.quoted, std::vector<uint8_t>({ 'h', 'e', 'l', 'l', 'o' }));

    ep->Close();
}

TEST_F(TestUDPEndPointOpenThread, IgnoresReportFromAnotherSender)
{
    Report report;
    UDPEndPointHandle ep;
    OpenEndPoint(ep, report);

    DeliverPortUnreachable("2001:db8::3", kPeerAddressString, ep->GetBoundPort(), "hello");
    ServiceEvents();

    EXPECT_EQ(report.count, 0);

    ep->Close();
}

TEST_F(TestUDPEndPointOpenThread, ReportsToTheEndPointBoundToTheQuotedPort)
{
    Report reportA;
    Report reportB;
    UDPEndPointHandle a;
    UDPEndPointHandle b;
    OpenEndPoint(a, reportA);
    OpenEndPoint(b, reportB);
    ASSERT_NE(a->GetBoundPort(), b->GetBoundPort());

    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, a->GetBoundPort(), "hello");
    ServiceEvents();
    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, b->GetBoundPort(), "hello");
    ServiceEvents();

    EXPECT_EQ(reportA.count, 1);
    EXPECT_EQ(reportB.count, 1);

    a->Close();
    b->Close();
}

TEST_F(TestUDPEndPointOpenThread, LaterReportForSameEndPointReplacesPendingOne)
{
    Report report;
    UDPEndPointHandle ep;
    OpenEndPoint(ep, report);

    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, ep->GetBoundPort(), "first");
    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, ep->GetBoundPort(), "second");
    ServiceEvents();

    EXPECT_EQ(report.count, 1);
    EXPECT_EQ(report.quoted, std::vector<uint8_t>({ 's', 'e', 'c', 'o', 'n', 'd' }));

    ep->Close();
}

TEST_F(TestUDPEndPointOpenThread, ReportForAnotherEndPointWhilePendingIsDropped)
{
    Report reportA;
    Report reportB;
    UDPEndPointHandle a;
    UDPEndPointHandle b;
    OpenEndPoint(a, reportA);
    OpenEndPoint(b, reportB);

    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, a->GetBoundPort(), "hello");
    DeliverPortUnreachable(kPeerAddressString, kPeerAddressString, b->GetBoundPort(), "hello");
    ServiceEvents();

    EXPECT_EQ(reportA.count, 1);
    EXPECT_EQ(reportB.count, 0);

    a->Close();
    b->Close();
}

} // namespace
