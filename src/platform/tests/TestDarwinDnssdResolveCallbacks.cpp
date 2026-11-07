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
#include <net/if.h>

#include <string>
#include <vector>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/dnssd/DnssdImpl.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Dnssd;

namespace {

constexpr char kHostname[] = "0A1B2C3D4E5F.local.";
constexpr uint16_t kPort   = 5540;
constexpr uint8_t kTxt[]   = { 3, 'T', '=', '1' };

struct ResolveResult
{
    Inet::InterfaceId interface;
    std::vector<std::string> addresses;
};

void OnResolved(void * context, DnssdService * service, const Span<Inet::IPAddress> & addresses, CHIP_ERROR error)
{
    VerifyOrReturn(service != nullptr);

    ResolveResult result{ service->mInterface, {} };
    for (const auto & address : addresses)
    {
        char addressString[Inet::IPAddress::kMaxStringLength];
        result.addresses.push_back(address.ToString(addressString));
    }
    static_cast<std::vector<ResolveResult> *>(context)->push_back(result);
}

class TestDarwinDnssdResolveCallbacks : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        static unsigned sResolveCount = 0;
        mInstanceName                 = "resolve-callbacks-" + std::to_string(++sResolveCount);
        mFullName                     = mInstanceName + "._matter._tcp.local.";

        DnssdService service = {};
        Platform::CopyString(service.mName, mInstanceName.c_str());
        Platform::CopyString(service.mType, "_matter");
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
        service.mAddressType = Inet::IPAddressType::kAny;

        ASSERT_EQ(ChipDnssdResolve(&service, Inet::InterfaceId::Null(), OnResolved, &mResults), CHIP_NO_ERROR);
        ASSERT_TRUE(IsResolving());
    }

    void TearDown() override { ChipDnssdResolveNoLongerNeeded(mInstanceName.c_str()); }

    bool IsResolving() { return MdnsContexts::GetInstance().GetExistingResolveForInstanceName(mInstanceName.c_str()) != nullptr; }

    ResolveContextWithType * CallbackContext()
    {
        return &MdnsContexts::GetInstance().GetExistingResolveForInstanceName(mInstanceName.c_str())->resolveContextWithNonSRPType;
    }

    void DeliverResolve(DNSServiceFlags flags, uint32_t interfaceId)
    {
        ASSERT_TRUE(IsResolving());
        OnResolve(nullptr, flags, interfaceId, kDNSServiceErr_NoError, mFullName.c_str(), kHostname, htons(kPort), sizeof(kTxt),
                  kTxt, CallbackContext());
    }

    void DeliverAddress(DNSServiceFlags flags, uint32_t interfaceId, const char * address)
    {
        sockaddr_in6 sockaddr = {};
        sockaddr.sin6_len     = sizeof(sockaddr);
        sockaddr.sin6_family  = AF_INET6;
        EXPECT_EQ(inet_pton(AF_INET6, address, &sockaddr.sin6_addr), 1);
        ASSERT_TRUE(IsResolving());
        OnGetAddrInfo(nullptr, flags | kDNSServiceFlagsAdd, interfaceId, kDNSServiceErr_NoError, kHostname,
                      reinterpret_cast<const struct sockaddr *>(&sockaddr), 120, CallbackContext());
    }

    std::string mInstanceName;
    std::string mFullName;
    std::vector<ResolveResult> mResults;
};

TEST_F(TestDarwinDnssdResolveCallbacks, ResolveFinishesWhenAddressEndsTheBatch)
{
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);
    DeliverAddress(0, kDNSServiceInterfaceIndexLocalOnly, "::1");

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolveCallbacks, ResolveFinishesWhenRepeatedResultEndsTheBatch)
{
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);
    DeliverAddress(kDNSServiceFlagsMoreComing, kDNSServiceInterfaceIndexLocalOnly, "::1");
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolveCallbacks, ResolveWaitsForAddressAfterRepeatedResult)
{
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);
    EXPECT_TRUE(mResults.empty());

    DeliverAddress(0, kDNSServiceInterfaceIndexLocalOnly, "::1");
    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolveCallbacks, ResolveWaitsForAddressesOfNewResult)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);
    DeliverAddress(kDNSServiceFlagsMoreComing, kDNSServiceInterfaceIndexLocalOnly, "::1");
    DeliverResolve(0, loopback);
    EXPECT_TRUE(mResults.empty());

    DeliverAddress(0, loopback, "fe80::1");
    ASSERT_EQ(mResults.size(), 2u);
    EXPECT_EQ(mResults[0].interface, Inet::InterfaceId(loopback));
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "fe80::1" });
    EXPECT_EQ(mResults[1].addresses, std::vector<std::string>{ "::1" });
}

TEST_F(TestDarwinDnssdResolveCallbacks, ResolveFinishesLikeAddressBatchEndWhileAnotherLookupIsPending)
{
    const uint32_t loopback = if_nametoindex("lo0");
    ASSERT_NE(loopback, 0u);

    DeliverResolve(kDNSServiceFlagsMoreComing, kDNSServiceInterfaceIndexLocalOnly);
    DeliverResolve(0, loopback);
    DeliverAddress(kDNSServiceFlagsMoreComing, kDNSServiceInterfaceIndexLocalOnly, "::1");
    DeliverResolve(0, kDNSServiceInterfaceIndexLocalOnly);

    ASSERT_EQ(mResults.size(), 1u);
    EXPECT_EQ(mResults[0].addresses, std::vector<std::string>{ "::1" });
}

} // namespace
