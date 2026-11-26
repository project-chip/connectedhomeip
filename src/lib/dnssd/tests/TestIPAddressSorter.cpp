/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/IPAddressSorter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::Dnssd::IPAddressSorter;

namespace pw {

template <>
StatusWithSize ToString<Inet::IPAddress>(const Inet::IPAddress & address, pw::span<char> buffer)
{
    char string[Inet::IPAddress::kMaxStringLength];
    address.ToString(string);
    return pw::string::Format(buffer, "IP<%s>", string);
}

} // namespace pw

namespace {

// Ranks IPv6 address types in the reverse of the default order: unique local, then global, then link-local.
class UniqueLocalFirstScorer : public IPAddressScorer
{
public:
    uint8_t ScoreIPAddress(const Inet::IPAddress & address, Inet::InterfaceId interfaceId) override
    {
        if (address.IsIPv6ULA())
        {
            return 3;
        }
        if (address.IsIPv6GlobalUnicast())
        {
            return 2;
        }
        return 1;
    }
};

// Scores unique local addresses above every default score, and every other address with its default score.
class UniqueLocalBoostingScorer : public IPAddressScorer
{
public:
    uint8_t ScoreIPAddress(const Inet::IPAddress & address, Inet::InterfaceId interfaceId) override
    {
        if (address.IsIPv6ULA())
        {
            return to_underlying(IpScore::kLinkLocal) + 1;
        }
        return IPAddressScorer::ScoreIPAddress(address, interfaceId);
    }
};

class ScopedScorer
{
public:
    explicit ScopedScorer(IPAddressScorer & scorer) : mPreviousScorer(SetIPAddressScorer(&scorer)) {}
    ~ScopedScorer() { SetIPAddressScorer(mPreviousScorer); }

private:
    IPAddressScorer * mPreviousScorer;
};

Inet::IPAddress MakeAddress(const char * string)
{
    Inet::IPAddress address;
    VerifyOrDie(Inet::IPAddress::FromString(string, address));
    return address;
}

Inet::IPAddress LinkLocalAddress()
{
    return MakeAddress("fe80::aabb:ccdd:2233:4455");
}

Inet::IPAddress GlobalAddress()
{
    return MakeAddress("2001::aabb:ccdd:2233:4455");
}

Inet::IPAddress UniqueLocalAddress()
{
    return MakeAddress("fdff:aabb:ccdd:1::4");
}

TEST(TestIPAddressSorter, ScoreIPAddressUsesTheInstalledScorer)
{
    UniqueLocalFirstScorer scorer;
    ScopedScorer scopedScorer(scorer);

    EXPECT_EQ(ScoreIPAddress(UniqueLocalAddress(), Inet::InterfaceId::Null()), 3);
    EXPECT_EQ(ScoreIPAddress(LinkLocalAddress(), Inet::InterfaceId::Null()), 1);
}

TEST(TestIPAddressSorter, SortUsesTheInstalledScorer)
{
    UniqueLocalFirstScorer scorer;
    ScopedScorer scopedScorer(scorer);

    Inet::IPAddress addresses[] = { LinkLocalAddress(), GlobalAddress(), UniqueLocalAddress() };
    Sort(addresses, MATTER_ARRAY_SIZE(addresses), Inet::InterfaceId::Null());

    EXPECT_EQ(addresses[0], UniqueLocalAddress());
    EXPECT_EQ(addresses[1], GlobalAddress());
    EXPECT_EQ(addresses[2], LinkLocalAddress());
}

TEST(TestIPAddressSorter, ClearingTheScorerRestoresTheDefaultScores)
{
    UniqueLocalFirstScorer scorer;
    IPAddressScorer * previousScorer = SetIPAddressScorer(&scorer);
    SetIPAddressScorer(nullptr);

    EXPECT_EQ(ScoreIPAddress(LinkLocalAddress(), Inet::InterfaceId::Null()), to_underlying(IpScore::kLinkLocal));

    SetIPAddressScorer(previousScorer);
}

TEST(TestIPAddressSorter, ScorerCanUseTheDefaultScores)
{
    UniqueLocalBoostingScorer scorer;
    ScopedScorer scopedScorer(scorer);

    EXPECT_EQ(ScoreIPAddress(LinkLocalAddress(), Inet::InterfaceId::Null()), to_underlying(IpScore::kLinkLocal));
    EXPECT_EQ(ScoreIPAddress(UniqueLocalAddress(), Inet::InterfaceId::Null()), to_underlying(IpScore::kLinkLocal) + 1);
}

TEST(TestIPAddressSorter, SetIPAddressScorerReturnsTheScorerItReplaces)
{
    UniqueLocalFirstScorer firstScorer;
    UniqueLocalBoostingScorer secondScorer;

    IPAddressScorer * originalScorer = SetIPAddressScorer(&firstScorer);
    EXPECT_EQ(SetIPAddressScorer(&secondScorer), &firstScorer);
    EXPECT_EQ(SetIPAddressScorer(originalScorer), &secondScorer);
}

} // namespace
