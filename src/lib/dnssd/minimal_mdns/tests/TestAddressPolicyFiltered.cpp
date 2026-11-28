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

#include <inet/InetInterface.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy_FilteredImpl.h>
#include <lib/support/CHIPMem.h>

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

/// Yields the given interfaces in order, all as IPv6.
class ListedListenIterator : public ListenIterator
{
public:
    ListedListenIterator(const Inet::InterfaceId * ids, size_t count) : mIds(ids), mCount(count) {}

    bool Next(Inet::InterfaceId * id, Inet::IPAddressType * type) override
    {
        if (mIndex == mCount)
        {
            return false;
        }
        *id   = mIds[mIndex++];
        *type = Inet::IPAddressType::kIPv6;
        return true;
    }

private:
    const Inet::InterfaceId * mIds;
    size_t mCount;
    size_t mIndex = 0;
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
        if (mListedCount > 0)
        {
            return Platform::UniquePtr<ListenIterator>(Platform::New<ListedListenIterator>(mListed, mListedCount));
        }
        return Platform::UniquePtr<ListenIterator>(Platform::New<FakeListenIterator>(mYieldCount));
    }

    Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(Inet::InterfaceId id, Inet::IPAddressType type) override
    {
        mAddressCalls++;
        mLastAddressInterface = id;
        mLastAddressType      = type;
        return Platform::UniquePtr<IpAddressIterator>(Platform::New<EmptyAddressIterator>());
    }

    size_t mListenCalls      = 0;
    size_t mAddressCalls     = 0;
    size_t mYieldCount       = 0;
    bool mReturnNullIterator = false;

    const Inet::InterfaceId * mListed = nullptr;
    size_t mListedCount               = 0;

    Inet::InterfaceId mLastAddressInterface;
    Inet::IPAddressType mLastAddressType = Inet::IPAddressType::kUnknown;
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

class TestAddressPolicyFiltered : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestAddressPolicyFiltered, AllowListMatchesExactNames)
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

TEST_F(TestAddressPolicyFiltered, AllowListRejectsNullName)
{
    const char * const kNames[] = { "eth0" };
    InterfaceNameAllowList allowList{ Span<const char * const>(kNames) };

    EXPECT_FALSE(allowList.IsAllowed(nullptr));
}

TEST_F(TestAddressPolicyFiltered, EmptyAllowListRejectsEverything)
{
    InterfaceNameAllowList allowList{ Span<const char * const>{} };

    EXPECT_FALSE(allowList.IsAllowed("eth0"));
}

TEST_F(TestAddressPolicyFiltered, NullInterfacesAreAlwaysKept)
{
    // Without this, configuring any filter at all would silently disable IPv4
    // discovery.
    FakePolicy underlying;
    underlying.mYieldCount = 3;

    RejectAll rejectAll;
    FilteredAddressPolicy policy(underlying, rejectAll);

    EXPECT_EQ(CountEndpoints(policy), 3u);
}

TEST_F(TestAddressPolicyFiltered, OnlyAllowedNamedInterfacesArePassed)
{
    // The filter resolves names through the operating system, so this needs
    // two real interfaces rather than made-up ids.
    Inet::InterfaceId ids[2];
    char names[2][Inet::InterfaceId::kMaxIfNameLength];
    size_t found = 0;
    for (Inet::InterfaceIterator it; it.HasCurrent() && found < 2; it.Next())
    {
        if (it.GetInterfaceName(names[found], sizeof(names[found])) == CHIP_NO_ERROR)
        {
            ids[found++] = it.GetInterfaceId();
        }
    }
    if (found < 2)
    {
        GTEST_SKIP() << "Needs two network interfaces";
    }

    FakePolicy underlying;
    underlying.mListed      = ids;
    underlying.mListedCount = 2;

    const char * const kNames[] = { names[1] };
    InterfaceNameAllowList allowList{ Span<const char * const>(kNames) };
    FilteredAddressPolicy policy(underlying, allowList);

    auto it = policy.GetListenEndpoints();
    ASSERT_TRUE(it);

    Inet::InterfaceId id;
    Inet::IPAddressType type;
    ASSERT_TRUE(it->Next(&id, &type));
    EXPECT_TRUE(id == ids[1]);
    EXPECT_FALSE(it->Next(&id, &type));
}

TEST_F(TestAddressPolicyFiltered, PassesThroughWhenUnderlyingHasNoEndpoints)
{
    FakePolicy underlying;
    underlying.mYieldCount = 0;

    AcceptAll acceptAll;
    FilteredAddressPolicy policy(underlying, acceptAll);

    EXPECT_EQ(CountEndpoints(policy), 0u);
    EXPECT_EQ(underlying.mListenCalls, 1u);
}

TEST_F(TestAddressPolicyFiltered, HandlesUnderlyingReturningNoIterator)
{
    FakePolicy underlying;
    underlying.mReturnNullIterator = true;

    AcceptAll acceptAll;
    FilteredAddressPolicy policy(underlying, acceptAll);

    EXPECT_FALSE(policy.GetListenEndpoints());
}

TEST_F(TestAddressPolicyFiltered, AddressLookupIsDelegatedUnchanged)
{
    FakePolicy underlying;
    RejectAll rejectAll;
    FilteredAddressPolicy policy(underlying, rejectAll);

    Inet::InterfaceIterator interfaces;
    if (!interfaces.HasCurrent())
    {
        GTEST_SKIP() << "Needs a network interface";
    }
    Inet::InterfaceId interfaceId = interfaces.GetInterfaceId();

    // Endpoints only reach this call after passing the predicate, so the
    // filter must not second-guess the addresses of an accepted endpoint.
    // kAny rather than kIPv4, which IPv6-only builds do not define.
    auto addresses = policy.GetIpAddressesForEndpoint(interfaceId, Inet::IPAddressType::kAny);

    EXPECT_TRUE(addresses);
    EXPECT_EQ(underlying.mAddressCalls, 1u);
    EXPECT_TRUE(underlying.mLastAddressInterface == interfaceId);
    EXPECT_EQ(underlying.mLastAddressType, Inet::IPAddressType::kAny);
}

} // namespace
