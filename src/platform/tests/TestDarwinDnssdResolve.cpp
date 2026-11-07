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

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <poll.h>
#include <unistd.h>

#include <string>
#include <vector>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Discovery_ImplPlatform.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/dnssd/DnssdImpl.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Dnssd;

namespace {

constexpr char kInstanceName[] = "84EE7F2EE1119951-0000000000000001";
constexpr char kHostname[]     = "0A1B2C3D4E5F.local.";
constexpr uint16_t kPort       = 5540;
constexpr uint8_t kTxt[]       = { 3, 'T', '=', '1' };

constexpr uint32_t kReportedInterface = 7;
constexpr uint32_t kOtherInterface    = 6;

struct ResolveResult
{
    Inet::InterfaceId interface;
    std::string name;
    uint16_t port;
    std::string txt;
    std::vector<std::string> addresses;
};

void OnResolve(void * context, DnssdService * service, const Span<Inet::IPAddress> & addresses, CHIP_ERROR error)
{
    auto results = static_cast<std::vector<ResolveResult> *>(context);
    VerifyOrReturn(service != nullptr);

    ResolveResult result{ service->mInterface, service->mName, service->mPort, {}, {} };
    for (size_t i = 0; i < service->mTextEntrySize; i++)
    {
        const auto & entry = service->mTextEntries[i];
        result.txt += std::string(entry.mKey) + "=" + std::string(reinterpret_cast<const char *>(entry.mData), entry.mDataSize);
    }
    for (const auto & address : addresses)
    {
        char addressString[Inet::IPAddress::kMaxStringLength];
        result.addresses.push_back(address.ToString(addressString));
    }
    results->push_back(result);
}

void OnResolveStopEventLoop(void * context, DnssdService * service, const Span<Inet::IPAddress> & addresses, CHIP_ERROR error)
{
    OnResolve(context, service, addresses, error);
    VerifyOrReturn(service != nullptr && error != CHIP_ERROR_IN_PROGRESS);
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
        [] { TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask(); });
}

void OnResolveTimeout(System::Layer * layer, void * context)
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask();
}

bool WaitForReply(DNSServiceRef serviceRef, const DNSServiceErrorType & replyError)
{
    pollfd fd = { DNSServiceRefSockFD(serviceRef), POLLIN, 0 };
    return poll(&fd, 1, 5000) == 1 && DNSServiceProcessResult(serviceRef) == kDNSServiceErr_NoError &&
        replyError == kDNSServiceErr_NoError;
}

bool IsInterfaceRunning(const char * name)
{
    ifaddrs * addresses = nullptr;
    VerifyOrReturnValue(getifaddrs(&addresses) == 0, false);
    bool running = false;
    for (auto address = addresses; address != nullptr; address = address->ifa_next)
    {
        running = running ||
            (strcmp(address->ifa_name, name) == 0 && (address->ifa_flags & (IFF_UP | IFF_RUNNING)) == (IFF_UP | IFF_RUNNING));
    }
    freeifaddrs(addresses);
    return running;
}

class DiscoveredNodes : public DiscoverNodeDelegate
{
public:
    void OnNodeDiscovered(const DiscoveredNodeData & nodeData) override
    {
        ASSERT_TRUE(nodeData.Is<CommissionNodeData>());
        const auto & data = nodeData.Get<CommissionNodeData>();
        interfaces.push_back(data.interfaceId);
        ports.push_back(data.port);
    }

    std::vector<Inet::InterfaceId> interfaces;
    std::vector<uint16_t> ports;
};

class OperationalResults : public OperationalResolveDelegate
{
public:
    void OnOperationalNodeResolved(const ResolvedNodeData & nodeData) override
    {
        resolvedInterfaces.push_back(nodeData.resolutionData.interfaceId);
    }

    void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override { failures++; }

    std::vector<Inet::InterfaceId> resolvedInterfaces;
    size_t failures = 0;
};

class TestDarwinDnssdResolve : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        ASSERT_TRUE(if_nametoindex("lo0") != kReportedInterface && if_nametoindex("lo0") != kOtherInterface);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

protected:
    ResolveContext * StartResolve(Inet::InterfaceId interface)
    {
        DnssdService service = {};
        Platform::CopyString(service.mName, kInstanceName);
        Platform::CopyString(service.mType, "_matter");
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
        service.mAddressType = Inet::IPAddressType::kAny;

        EXPECT_EQ(ChipDnssdResolve(&service, interface, OnResolve, &mResults), CHIP_NO_ERROR);
        return MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kInstanceName);
    }

    static void AddResolveResult(ResolveContext * context, uint32_t interfaceId)
    {
        context->OnNewInterface(interfaceId, kInstanceName, kHostname, htons(kPort), sizeof(kTxt), kTxt, false);
    }

    static void AddAddress(ResolveContext * context, uint32_t interfaceId, const char * address, bool isSRPResult = false,
                           const char * hostname = kHostname)
    {
        sockaddr_in6 sockaddr = {};
        sockaddr.sin6_len     = sizeof(sockaddr);
        sockaddr.sin6_family  = AF_INET6;
        EXPECT_EQ(inet_pton(AF_INET6, address, &sockaddr.sin6_addr), 1);

        InterfaceKey key = { interfaceId, hostname, isSRPResult };
        EXPECT_EQ(context->OnNewAddress(key, reinterpret_cast<const struct sockaddr *>(&sockaddr)), CHIP_NO_ERROR);
    }

    bool AdvertiseLoopbackHost(uint32_t serviceInterface, const char * name)
    {
        mServiceName                   = std::string(name) + "-" + std::to_string(getpid());
        const std::string hostname     = mServiceName + ".local.";
        DNSServiceErrorType replyError = kDNSServiceErr_Unknown;

        VerifyOrReturnValue(DNSServiceCreateConnection(&mRecords) == kDNSServiceErr_NoError, false, mRecords = nullptr);
        DNSRecordRef record;
        const in6_addr loopbackAddress = in6addr_loopback;
        auto onRecord = [](DNSServiceRef, DNSRecordRef, DNSServiceFlags, DNSServiceErrorType error, void * context) {
            *static_cast<DNSServiceErrorType *>(context) = error;
        };
        VerifyOrReturnValue(DNSServiceRegisterRecord(mRecords, &record, kDNSServiceFlagsShared, if_nametoindex("lo0"),
                                                     hostname.c_str(), kDNSServiceType_AAAA, kDNSServiceClass_IN,
                                                     sizeof(loopbackAddress), &loopbackAddress, 0, onRecord,
                                                     &replyError) == kDNSServiceErr_NoError,
                            false);
        VerifyOrReturnValue(WaitForReply(mRecords, replyError), false);

        replyError      = kDNSServiceErr_Unknown;
        auto onRegister = [](DNSServiceRef, DNSServiceFlags, DNSServiceErrorType error, const char *, const char *, const char *,
                             void * context) { *static_cast<DNSServiceErrorType *>(context) = error; };
        VerifyOrReturnValue(DNSServiceRegister(&mService, kDNSServiceFlagsNoAutoRename, serviceInterface, mServiceName.c_str(),
                                               "_anyif._tcp", "local.", hostname.c_str(), htons(kPort), sizeof(kTxt), kTxt,
                                               onRegister, &replyError) == kDNSServiceErr_NoError,
                            false, mService = nullptr);
        return WaitForReply(mService, replyError);
    }

    void ResolveAdvertisedService(Inet::InterfaceId interface = Inet::InterfaceId::Null())
    {
        DnssdService query = {};
        Platform::CopyString(query.mName, mServiceName.c_str());
        Platform::CopyString(query.mType, "_anyif");
        query.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
        query.mAddressType = Inet::IPAddressType::kIPv6;
        EXPECT_EQ(ChipDnssdResolve(&query, interface, OnResolveStopEventLoop, &mResults), CHIP_NO_ERROR);
        EXPECT_EQ(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(5), OnResolveTimeout, nullptr), CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
        DeviceLayer::SystemLayer().CancelTimer(OnResolveTimeout, nullptr);
        auto context     = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(mServiceName.c_str());
        mServiceResolved = !mResults.empty() || (context != nullptr && context->HasInterface());
        ChipDnssdResolveNoLongerNeeded(mServiceName.c_str());
    }

    void TearDown() override
    {
        if (mService != nullptr)
        {
            DNSServiceRefDeallocate(mService);
            mService = nullptr;
        }
        if (mRecords != nullptr)
        {
            DNSServiceRefDeallocate(mRecords);
            mRecords = nullptr;
        }
    }

    void ExpectLoopbackResult()
    {
        ASSERT_EQ(mResults.size(), 1u);
        EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(if_nametoindex("lo0")));
        EXPECT_EQ(mResults[0].port, kPort);
        EXPECT_EQ(mResults[0].txt, "T=1");
        EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
    }

    std::vector<ResolveResult> mResults;
    std::string mServiceName;
    bool mServiceResolved  = false;
    DNSServiceRef mRecords = nullptr;
    DNSServiceRef mService = nullptr;
};

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveUsesAddressFromAnotherInterface)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    const InterfaceKey otherKey    = { kOtherInterface, kHostname, false };
    const InterfaceKey reportedKey = { kReportedInterface, kHostname, false };
    auto lookupInterface           = context->interfaces.find(otherKey);
    EXPECT_TRUE(lookupInterface != context->interfaces.end() && lookupInterface->second.isDNSLookUpRequested);
    EXPECT_FALSE(context->interfaces[reportedKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].name, kInstanceName);
    EXPECT_EQ(mResults[0].port, kPort);
    EXPECT_EQ(mResults[0].txt, "T=1");
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveReportsEachAddressOnItsOwnInterface)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kReportedInterface, "fe80::7");
    AddAddress(context, kOtherInterface, "fe80::6");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 2u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "fe80::6" });
    EXPECT_EQ(mResults[1].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[1].addresses, std::vector<std::string>{ "fe80::7" });
    for (const auto & result : mResults)
    {
        EXPECT_EQ(result.port, kPort);
        EXPECT_EQ(result.txt, "T=1");
    }
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsEachAddressOnce)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::6");
    AddAddress(context, kOtherInterface, "2001:db8::6");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 2u);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::6" });
    EXPECT_EQ(mResults[1].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresAddressForAnotherHost)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1", false, "FFEEDDCCBBAA.local.");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresAddressWhenPortsDisagree)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    context->OnNewInterface(loopback, kInstanceName, kHostname, htons(kPort + 1), sizeof(kTxt), kTxt, false);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresAddressWhenTxtDisagrees)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);
    constexpr uint8_t kOtherTxt[] = { 3, 'T', '=', '2' };

    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    context->OnNewInterface(loopback, kInstanceName, kHostname, htons(kPort), sizeof(kOtherTxt), kOtherTxt, false);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresDisagreementOfAnotherHost)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    context->OnNewInterface(loopback, kInstanceName, "FFEEDDCCBBAA.local.", htons(kPort + 1), sizeof(kTxt), kTxt, false);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].port, kPort);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveDropsAddressesFiledBeforeResultsDisagree)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    context->OnNewInterface(kReportedInterface + 1, kInstanceName, kHostname, htons(kPort + 1), sizeof(kTxt), kTxt, false);
    AddAddress(context, kOtherInterface, "2001:db8::2");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveDropsAddressesFiledBeforeTxtDisagrees)
{
    constexpr uint8_t kOtherTxt[] = { 3, 'T', '=', '2' };

    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    context->OnNewInterface(kReportedInterface + 1, kInstanceName, kHostname, htons(kPort), sizeof(kOtherTxt), kOtherTxt, false);
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsAddressesFiledBeforeAnotherHostDisagrees)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    context->OnNewInterface(kReportedInterface + 1, kInstanceName, "FFEEDDCCBBAA.local.", htons(kPort + 1), sizeof(kTxt), kTxt,
                            false);
    context->OnNewInterface(kReportedInterface + 1, kInstanceName, kHostname, htons(kPort + 1), sizeof(kTxt), kTxt, true);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsAddressesWhenTheirInterfaceReportsAnotherPort)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    context->OnNewInterface(kOtherInterface, kInstanceName, kHostname, htons(kPort + 1), sizeof(kTxt), kTxt, false);
    const InterfaceKey otherKey = { kOtherInterface, kHostname, false };
    EXPECT_TRUE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].port, kPort + 1);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveUsesAddressFromAnotherInterfaceWithoutTxt)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    context->OnNewInterface(kReportedInterface, kInstanceName, kHostname, htons(kPort), 0, nullptr, false);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].txt, "");
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsAddressesOnceAResultReportsTheirInterface)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1");
    AddResolveResult(context, kOtherInterface);
    context->OnNewInterface(kReportedInterface + 1, kInstanceName, kHostname, htons(kPort + 1), sizeof(kTxt), kTxt, false);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::1" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceDiscoveryReportsAddressFromAnotherInterface)
{
    DiscoveredNodes nodes;
    DnssdService service = {};
    Platform::CopyString(service.mName, kInstanceName);
    Platform::CopyString(service.mType, "_matterc");
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolUdp;
    service.mAddressType = Inet::IPAddressType::kAny;
    ASSERT_EQ(ChipDnssdResolve(&service, Inet::InterfaceId::Null(), &nodes), CHIP_NO_ERROR);

    auto context = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kInstanceName);
    ASSERT_NE(context, nullptr);
    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::6");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(nodes.interfaces, std::vector<Inet::InterfaceId>{ Inet::InterfaceId(kOtherInterface) });
    EXPECT_EQ(nodes.ports, std::vector<uint16_t>{ kPort });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveLooksUpEachHostOnce)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddResolveResult(context, kOtherInterface);
    context->OnNewInterface(kReportedInterface, kInstanceName, kHostname, htons(kPort), sizeof(kTxt), kTxt, true);
    context->OnNewInterface(kReportedInterface, kInstanceName, "FFEEDDCCBBAA.local.", htons(kPort), sizeof(kTxt), kTxt, false);
    context->OnLookUpRequested(InterfaceKey{ kReportedInterface, kHostname, false });

    const InterfaceKey otherKey     = { kOtherInterface, kHostname, false };
    const InterfaceKey srpKey       = { kReportedInterface, kHostname, true };
    const InterfaceKey otherHostKey = { kReportedInterface, "FFEEDDCCBBAA.local.", false };
    EXPECT_TRUE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_FALSE(context->interfaces[srpKey].isDNSLookUpRequested);
    EXPECT_FALSE(context->interfaces[otherHostKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsAHostLookedUpWhenAnotherHostIsLookedUp)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddResolveResult(context, kOtherInterface);
    context->OnNewInterface(kReportedInterface, kInstanceName, "FFEEDDCCBBAA.local.", htons(kPort), sizeof(kTxt), kTxt, false);
    context->OnLookUpRequested(InterfaceKey{ kReportedInterface, kHostname, false });
    context->OnLookUpRequested(InterfaceKey{ kReportedInterface, "FFEEDDCCBBAA.local.", false });

    const InterfaceKey reportedKey = { kReportedInterface, kHostname, false };
    const InterfaceKey otherKey    = { kOtherInterface, kHostname, false };
    EXPECT_TRUE(context->interfaces[reportedKey].isDNSLookUpRequested);
    EXPECT_TRUE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
}

TEST_F(TestDarwinDnssdResolve, InterfaceResolveLooksUpEachInterface)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    auto context = StartResolve(Inet::InterfaceId(loopback));
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, loopback);
    AddResolveResult(context, loopback + 1);
    context->OnLookUpRequested(InterfaceKey{ loopback, kHostname, false });

    const InterfaceKey loopbackKey = { loopback, kHostname, false };
    const InterfaceKey otherKey    = { loopback + 1, kHostname, false };
    EXPECT_TRUE(context->interfaces[loopbackKey].isDNSLookUpRequested);
    EXPECT_FALSE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveKeepsAddressesWhenAResultRepeatsWithAnotherTxt)
{
    constexpr uint8_t kOtherTxt[] = { 3, 'T', '=', '2' };

    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::6");
    context->OnNewInterface(kReportedInterface, kInstanceName, kHostname, htons(kPort), sizeof(kOtherTxt), kOtherTxt, false);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kOtherInterface));
    EXPECT_EQ(mResults[0].txt, "T=1");
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::6" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveDoesNotLookUpAHostAgainForANewInterface)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    context->OnLookUpRequested(InterfaceKey{ kReportedInterface, kHostname, false });
    AddResolveResult(context, kOtherInterface);
    context->OnNewInterface(kOtherInterface, kInstanceName, kHostname, htons(kPort), sizeof(kTxt), kTxt, true);

    const InterfaceKey otherKey = { kOtherInterface, kHostname, false };
    const InterfaceKey srpKey   = { kOtherInterface, kHostname, true };
    EXPECT_TRUE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_FALSE(context->interfaces[srpKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
}

TEST_F(TestDarwinDnssdResolve, InterfaceResolveLooksUpANewInterface)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    auto context = StartResolve(Inet::InterfaceId(loopback));
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, loopback);
    context->OnLookUpRequested(InterfaceKey{ loopback, kHostname, false });
    AddResolveResult(context, loopback + 1);

    const InterfaceKey otherKey = { loopback + 1, kHostname, false };
    EXPECT_FALSE(context->interfaces[otherKey].isDNSLookUpRequested);
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresAddressWithNoInterface)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kDNSServiceInterfaceIndexAny, "2001:db8::1");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresAddressFromAnotherDomain)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::1", true);
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(kReportedInterface));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "2001:db8::7" });
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveIgnoresLinkLocalAddressOnLocalOnly)
{
    auto context = StartResolve(Inet::InterfaceId::Null());
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, kDNSServiceInterfaceIndexLocalOnly);
    AddAddress(context, kDNSServiceInterfaceIndexLocalOnly, "fe80::1");
    AddAddress(context, kDNSServiceInterfaceIndexLocalOnly, "::1");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId::Null());
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolve, InterfaceResolveIgnoresAddressFromAnotherInterface)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    auto context = StartResolve(Inet::InterfaceId(loopback));
    ASSERT_NE(context, nullptr);

    AddResolveResult(context, loopback);
    AddAddress(context, loopback + 1, "2001:db8::1");
    AddAddress(context, loopback, "::1");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(loopback));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolve, OperationalResolveReportsEveryInterfaceWithAddresses)
{
    OperationalResults results;
    auto & discovery = DiscoveryImplPlatform::GetInstance();
    ASSERT_EQ(discovery.Init(nullptr), CHIP_NO_ERROR);
    discovery.SetOperationalDelegate(&results);
    ASSERT_EQ(discovery.ResolveNodeId(PeerId(0x84EE7F2EE1119951, 1)), CHIP_NO_ERROR);

    auto context = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kInstanceName);
    ASSERT_NE(context, nullptr);
    AddResolveResult(context, kOtherInterface);
    AddResolveResult(context, kReportedInterface);
    AddAddress(context, kOtherInterface, "2001:db8::6");
    AddAddress(context, kReportedInterface, "2001:db8::7");
    EXPECT_EQ(context->Finalize(), CHIP_NO_ERROR);
    discovery.SetOperationalDelegate(nullptr);
    discovery.Shutdown();

    EXPECT_EQ(results.failures, 0u);
    EXPECT_EQ(results.resolvedInterfaces,
              (std::vector<Inet::InterfaceId>{ Inet::InterfaceId(kOtherInterface), Inet::InterfaceId(kReportedInterface) }));
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveFindsHostAddressesOnAnotherInterface)
{
    if (!AdvertiseLoopbackHost(kDNSServiceInterfaceIndexLocalOnly, "anyif-localonly"))
    {
        GTEST_SKIP() << "mDNSResponder registration failed";
    }

    ResolveAdvertisedService();
    ExpectLoopbackResult();
}

TEST_F(TestDarwinDnssdResolve, AnyInterfaceResolveLooksUpHostOnEveryInterface)
{
    const uint32_t en0 = if_nametoindex("en0");
    if (en0 == 0 || !IsInterfaceRunning("en0"))
    {
        GTEST_SKIP() << "needs en0 up";
    }

    if (!AdvertiseLoopbackHost(en0, "anyif-en0"))
    {
        GTEST_SKIP() << "mDNSResponder registration failed";
    }

    ResolveAdvertisedService();
    ExpectLoopbackResult();
}

TEST_F(TestDarwinDnssdResolve, InterfaceResolveLooksUpHostOnlyOnThatInterface)
{
    const uint32_t en0 = if_nametoindex("en0");
    if (en0 == 0 || !IsInterfaceRunning("en0"))
    {
        GTEST_SKIP() << "needs en0 up";
    }

    if (!AdvertiseLoopbackHost(en0, "anyif-en0-only"))
    {
        GTEST_SKIP() << "mDNSResponder registration failed";
    }

    ResolveAdvertisedService(Inet::InterfaceId(en0));
    EXPECT_TRUE(mServiceResolved);
    EXPECT_TRUE(mResults.empty());
}

} // namespace
