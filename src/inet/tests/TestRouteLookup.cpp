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

#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <inet/InetConfig.h>
#include <inet/InetInterface.h>
#include <inet/RouteLookup.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::Inet;

namespace {

IPAddress Loopback()
{
    IPAddress loopback;
    VerifyOrDie(IPAddress::FromString("::1", loopback));
    return loopback;
}

bool HostAnswersRouteLookups()
{
    return !LookUpRouteTo(Loopback(), InterfaceId::Null()).Is<UnknownRoute>();
}

bool HostHasAddress(const IPAddress & address)
{
    for (InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        IPAddress interfaceAddress;
        if (it.GetAddress(interfaceAddress) == CHIP_NO_ERROR && interfaceAddress == address)
        {
            return true;
        }
    }
    return false;
}

TEST(TestRouteLookup, RouteToLoopbackIsFromTheLoopbackAddress)
{
    if (!HostHasAddress(Loopback()))
    {
        GTEST_SKIP() << "This host has no IPv6 loopback address";
    }

    RouteLookupResult result = LookUpRouteTo(Loopback(), InterfaceId::Null());
    if (result.Is<UnknownRoute>())
    {
        GTEST_SKIP() << "This host does not answer route lookups (no sockets, no IPv6, or a sandbox that blocks them)";
    }

    ASSERT_TRUE(result.Is<RouteFromLocalAddress>());
    EXPECT_TRUE(result.Get<RouteFromLocalAddress>().localAddress == Loopback());
}

TEST(TestRouteLookup, RouteToOwnLinkLocalAddressIsFromThatAddressOnItsInterface)
{
    if (!HostAnswersRouteLookups())
    {
        GTEST_SKIP() << "This host does not answer route lookups (no sockets, no IPv6, or a sandbox that blocks them)";
    }

    IPAddress linkLocal;
    InterfaceId interfaceId;
    for (InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        if (it.GetAddress(linkLocal) == CHIP_NO_ERROR && linkLocal.IsIPv6LinkLocal())
        {
            interfaceId = it.GetInterfaceId();
            break;
        }
    }
    if (!interfaceId.IsPresent())
    {
        GTEST_SKIP() << "This host has no link-local address";
    }

    RouteLookupResult result = LookUpRouteTo(linkLocal, interfaceId);

    ASSERT_TRUE(result.Is<RouteFromLocalAddress>());
    EXPECT_TRUE(result.Get<RouteFromLocalAddress>().localAddress == linkLocal);
    EXPECT_TRUE(result.Get<RouteFromLocalAddress>().interfaceId == interfaceId);
}

#if INET_CONFIG_ENABLE_IPV4
TEST(TestRouteLookup, RouteToIPv4LoopbackIsFromTheLoopbackAddress)
{
    if (!HostAnswersRouteLookups())
    {
        GTEST_SKIP() << "This host does not answer route lookups (no sockets, no IPv6, or a sandbox that blocks them)";
    }

    IPAddress loopback;
    ASSERT_TRUE(IPAddress::FromString("127.0.0.1", loopback));
    if (!HostHasAddress(loopback))
    {
        GTEST_SKIP() << "This host has no IPv4 loopback address";
    }

    RouteLookupResult result = LookUpRouteTo(loopback, InterfaceId::Null());

    ASSERT_TRUE(result.Is<RouteFromLocalAddress>());
    EXPECT_TRUE(result.Get<RouteFromLocalAddress>().localAddress == loopback);
}
#endif // INET_CONFIG_ENABLE_IPV4

} // namespace
