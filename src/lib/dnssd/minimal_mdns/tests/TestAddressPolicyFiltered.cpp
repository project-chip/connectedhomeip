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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy_FilteredImpl.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

/// Yields a fixed number of null-interface endpoints.
///
/// A null interface id is what minimal mDNS uses to listen for IPv4 on all
/// interfaces at once, so it exercises the "no name to match on" path.
class FakeListenIterator : public ListenIterator
{
public:
    explicit FakeListenIterator(size_t count) : mRemaining(count) {}

    bool Next(Inet::InterfaceId * id, Inet::IPAddressType * type) override
    {
        if (mRemaining == 0)
        {
            return false;
        }
        mRemaining--;
        *id   = Inet::InterfaceId::Null();
        *type = Inet::IPAddressType::kIPv6;
        return true;
    }

private:
    size_t mRemaining;
};

class EmptyAddressIterator : public IpAddressIterator
{
public:
    bool Next(Inet::IPAddress &) override { return false; }
};

class FakePolicy : public AddressPolicy
{
public:
    Platform::UniquePtr<ListenIterator> GetListenEndpoints() override
    {
        mListenCalls++;
        if (mReturnNullIterator)
        {
            return Platform::UniquePtr<ListenIterator>();
        }
        return Platform::UniquePtr<ListenIterator>(Platform::New<FakeListenIterator>(mYieldCount));
    }

    Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(Inet::InterfaceId, Inet::IPAddressType) override
    {
        mAddressCalls++;
        return Platform::UniquePtr<IpAddressIterator>(Platform::New<EmptyAddressIterator>());
    }

    size_t mListenCalls      = 0;
    size_t mAddressCalls     = 0;
    size_t mYieldCount       = 0;
    bool mReturnNullIterator = false;
};

class AcceptAll : public InterfacePredicate
{
public:
    bool IsAllowed(const char *) const override { return true; }
};

class RejectAll : public InterfacePredicate
{
public:
    bool IsAllowed(const char *) const override { return false; }
};

size_t CountEndpoints(AddressPolicy & policy)
{
    auto it = policy.GetListenEndpoints();
    if (!it)
    {
        return 0;
    }

    size_t count = 0;
    Inet::InterfaceId id;
    Inet::IPAddressType type;
    while (it->Next(&id, &type))
    {
        count++;
    }
    return count;
}

TEST(TestAddressPolicyFiltered, AllowListMatchesExactNames)
{
    const char * const kNames[] = { "eth0", "vlan3020" };
    InterfaceNameAllowList allowList{ Span<const char * const>(kNames) };

    EXPECT_TRUE(allowList.IsAllowed("eth0"));
    EXPECT_TRUE(allowList.IsAllowed("vlan3020"));

    EXPECT_FALSE(allowList.IsAllowed("eth1"));
    EXPECT_FALSE(allowList.IsAllowed("lxc0c659c903f2b"));

    // Prefixes and extensions of an allowed name must not match.
    EXPECT_FALSE(allowList.IsAllowed("eth"));
    EXPECT_FALSE(allowList.IsAllowed("eth00"));
    EXPECT_FALSE(allowList.IsAllowed(""));
}

TEST(TestAddressPolicyFiltered, AllowListRejectsNullName)
{
    const char * const kNames[] = { "eth0" };
    InterfaceNameAllowList allowList{ Span<const char * const>(kNames) };

    EXPECT_FALSE(allowList.IsAllowed(nullptr));
}

TEST(TestAddressPolicyFiltered, EmptyAllowListRejectsEverything)
{
    InterfaceNameAllowList allowList{ Span<const char * const>{} };

    EXPECT_FALSE(allowList.IsAllowed("eth0"));
}

TEST(TestAddressPolicyFiltered, NullInterfacesAreAlwaysKept)
{
    // Without this, configuring any filter at all would silently disable IPv4
    // discovery.
    FakePolicy underlying;
    underlying.mYieldCount = 3;

    RejectAll rejectAll;
    FilteredAddressPolicy policy(underlying, rejectAll);

    EXPECT_EQ(CountEndpoints(policy), 3u);
}

TEST(TestAddressPolicyFiltered, PassesThroughWhenUnderlyingHasNoEndpoints)
{
    FakePolicy underlying;
    underlying.mYieldCount = 0;

    AcceptAll acceptAll;
    FilteredAddressPolicy policy(underlying, acceptAll);

    EXPECT_EQ(CountEndpoints(policy), 0u);
    EXPECT_EQ(underlying.mListenCalls, 1u);
}

TEST(TestAddressPolicyFiltered, HandlesUnderlyingReturningNoIterator)
{
    FakePolicy underlying;
    underlying.mReturnNullIterator = true;

    AcceptAll acceptAll;
    FilteredAddressPolicy policy(underlying, acceptAll);

    EXPECT_FALSE(policy.GetListenEndpoints());
}

TEST(TestAddressPolicyFiltered, AddressLookupIsDelegatedUnchanged)
{
    FakePolicy underlying;
    RejectAll rejectAll;
    FilteredAddressPolicy policy(underlying, rejectAll);

    // Endpoints only reach this call after passing the predicate, so the
    // filter must not second-guess the addresses of an accepted endpoint.
    auto addresses = policy.GetIpAddressesForEndpoint(Inet::InterfaceId::Null(), Inet::IPAddressType::kIPv6);

    EXPECT_TRUE(addresses);
    EXPECT_EQ(underlying.mAddressCalls, 1u);
}

} // namespace
