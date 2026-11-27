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

#include <inet/InetConfig.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <inet/UDPEndPointImpl.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace mdns::Minimal;

// An interface index no system hands out.
constexpr InterfaceId::PlatformType kNoSuchInterface = 0x7fffffff;

/// Yields an interface that does not exist and/or every interface that is up
/// and multicast capable, each of the latter for IPv6 and then IPv4.
class SomeInterfaces : public ListenIterator
{
public:
    SomeInterfaces(bool vanished, bool real) : mVanished(vanished), mReal(real) {}

    bool Next(InterfaceId * id, IPAddressType * type) override
    {
        if (mVanished)
        {
            mVanished = false;
            *id       = InterfaceId(kNoSuchInterface);
            *type     = IPAddressType::kIPv6;
            return true;
        }
        if (!mReal)
        {
            return false;
        }
#if INET_CONFIG_ENABLE_IPV4
        if (mIPv4Next)
        {
            mIPv4Next = false;
            *id       = mIterator.GetInterfaceId();
            *type     = IPAddressType::kIPv4;
            mIterator.Next();
            return true;
        }
#endif
        for (; mIterator.HasCurrent(); mIterator.Next())
        {
            if (!mIterator.IsUp() || !mIterator.SupportsMulticast())
            {
                continue;
            }
            *id   = mIterator.GetInterfaceId();
            *type = IPAddressType::kIPv6;
#if INET_CONFIG_ENABLE_IPV4
            mIPv4Next = true;
#else
            mIterator.Next();
#endif
            return true;
        }
        return false;
    }

private:
    bool mVanished;
    bool mReal;
    bool mIPv4Next = false;
    InterfaceIterator mIterator;
};

/// Hands out UDP endpoints from a pool of one, as a platform with a single endpoint left would.
class OneUdpEndPoint : public EndPointManager<UDPEndPoint>
{
public:
    Loop ForEachEndPoint(const EndPointVisitor visitor) override
    {
        return mPool.ForEachActiveObject([&](UDPEndPointImpl * endPoint) {
            TypedEndPointHandle handle(endPoint);
            return visitor(handle);
        });
    }

protected:
    TypedEndPointHandle CreateEndPoint() override { return mPool.CreateObject(*this); }

private:
    void ReleaseEndPoint(UDPEndPoint * endPoint) override { mPool.ReleaseObject(static_cast<UDPEndPointImpl *>(endPoint)); }

    BitMapObjectPool<UDPEndPointImpl, 1> mPool;
};

struct Listed
{
    InterfaceId id;
    IPAddressType type;
};

/// Yields the given interfaces and address types.
class TheseInterfaces : public ListenIterator
{
public:
    template <size_t N>
    explicit TheseInterfaces(const Listed (&listed)[N]) : mListed(listed), mCount(N)
    {}
    TheseInterfaces() = default;

    bool Next(InterfaceId * id, IPAddressType * type) override
    {
        if (mNext == mCount)
        {
            return false;
        }
        *id   = mListed[mNext].id;
        *type = mListed[mNext].type;
        mNext++;
        return true;
    }

private:
    const Listed * mListed = nullptr;
    size_t mCount          = 0;
    size_t mNext           = 0;
};

/// Records the endpoints a Listen() call opens.
class AddedEndpoints : public ServerBase::ListenDelegate
{
public:
    void OnEndpointAdded(InterfaceId id, IPAddressType type) override
    {
        if (mCount < MATTER_ARRAY_SIZE(mAdded))
        {
            mAdded[mCount] = { id, type };
        }
        mCount++;
    }

    size_t Count() const { return mCount; }
    bool Has(InterfaceId id, IPAddressType type) const
    {
        for (size_t i = 0; i < mCount && i < MATTER_ARRAY_SIZE(mAdded); i++)
        {
            if (mAdded[i].id == id && mAdded[i].type == type)
            {
                return true;
            }
        }
        return false;
    }
    void Clear() { mCount = 0; }

private:
    Listed mAdded[4];
    size_t mCount = 0;
};

class TestMinimalMdnsServer : public ::testing::Test
{
public:
    // The server posts kDnssdInitialized to the platform, so the platform stack is used.
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

    static EndPointManager<UDPEndPoint> * EndPoints() { return DeviceLayer::UDPEndPointManager(); }

    /// Skips the test on a host without an interface mDNS can use at all.
    static void RequireAUsableInterface()
    {
        Server<10> server;
        SomeInterfaces interfaces(/* vanished = */ false, /* real = */ true);
        if (server.Listen(EndPoints(), &interfaces, 0) != CHIP_NO_ERROR)
        {
            GTEST_SKIP() << "no usable interface on this host";
        }
        server.Shutdown();
    }

    /// An interface the server can listen on for IPv6; skips the test if there is none.
    static InterfaceId UsableInterface()
    {
        for (InterfaceIterator it; it.HasCurrent(); it.Next())
        {
            if (!it.IsUp() || !it.SupportsMulticast())
            {
                continue;
            }
            Server<10> server;
            AddedEndpoints added;
            const Listed listed[] = { { it.GetInterfaceId(), IPAddressType::kIPv6 } };
            TheseInterfaces interfaces(listed);
            if (server.Listen(EndPoints(), &interfaces, 0, &added) == CHIP_NO_ERROR && added.Count() == 1)
            {
                server.Shutdown();
                return it.GetInterfaceId();
            }
            server.Shutdown();
        }
        return InterfaceId::Null();
    }
};

TEST_F(TestMinimalMdnsServer, SkipsAnInterfaceThatIsGone)
{
    RequireAUsableInterface();

    Server<10> server;
    SomeInterfaces interfaces(/* vanished = */ true, /* real = */ true);

    EXPECT_EQ(server.Listen(EndPoints(), &interfaces, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
}

TEST_F(TestMinimalMdnsServer, NothingBoundIsNotListening)
{
    Server<10> server;
    SomeInterfaces interfaces(/* vanished = */ true, /* real = */ false);

    EXPECT_EQ(server.Listen(EndPoints(), &interfaces, 0), CHIP_NO_ERROR);
    EXPECT_FALSE(server.IsListening());
}

TEST_F(TestMinimalMdnsServer, KeepsWhatIsBoundWhenEndpointsRunOut)
{
    RequireAUsableInterface();

    // The first usable interface takes the only endpoint, the next finds none.
    OneUdpEndPoint endPoints;
    ASSERT_EQ(endPoints.Init(DeviceLayer::SystemLayer()), CHIP_NO_ERROR);

    Server<10> server;
    SomeInterfaces interfaces(/* vanished = */ false, /* real = */ true);

    EXPECT_EQ(server.Listen(&endPoints, &interfaces, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
    endPoints.Shutdown();
}

TEST_F(TestMinimalMdnsServer, KeepsTheEndpointsOfInterfacesStillListed)
{
    const InterfaceId usable = UsableInterface();
    if (!usable.IsPresent())
    {
        GTEST_SKIP() << "no usable interface on this host";
    }

    Server<10> server;
    AddedEndpoints added;
    const Listed listed[] = { { usable, IPAddressType::kIPv6 } };

    TheseInterfaces first(listed);
    EXPECT_EQ(server.Listen(EndPoints(), &first, 0, &added), CHIP_NO_ERROR);
    EXPECT_EQ(added.Count(), 1u);

    added.Clear();
    TheseInterfaces again(listed);
    EXPECT_EQ(server.Listen(EndPoints(), &again, 0, &added), CHIP_NO_ERROR);
    EXPECT_EQ(added.Count(), 0u);
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
}

TEST_F(TestMinimalMdnsServer, ClosesTheEndpointsOfInterfacesNoLongerListed)
{
    const InterfaceId usable = UsableInterface();
    if (!usable.IsPresent())
    {
        GTEST_SKIP() << "no usable interface on this host";
    }

    Server<10> server;
    const Listed listed[] = { { usable, IPAddressType::kIPv6 } };

    TheseInterfaces first(listed);
    EXPECT_EQ(server.Listen(EndPoints(), &first, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());

    TheseInterfaces none;
    EXPECT_EQ(server.Listen(EndPoints(), &none, 0), CHIP_NO_ERROR);
    EXPECT_FALSE(server.IsListening());
}

TEST_F(TestMinimalMdnsServer, OpensOnlyTheEndpointsOfNewInterfaces)
{
    const InterfaceId usable = UsableInterface();
    if (!usable.IsPresent())
    {
        GTEST_SKIP() << "no usable interface on this host";
    }

    Server<10> server;
    AddedEndpoints added;

    TheseInterfaces none;
    EXPECT_EQ(server.Listen(EndPoints(), &none, 0, &added), CHIP_NO_ERROR);
    EXPECT_EQ(added.Count(), 0u);

    const Listed one[] = { { usable, IPAddressType::kIPv6 } };
    TheseInterfaces first(one);
    EXPECT_EQ(server.Listen(EndPoints(), &first, 0, &added), CHIP_NO_ERROR);
    EXPECT_EQ(added.Count(), 1u);
    EXPECT_TRUE(added.Has(usable, IPAddressType::kIPv6));

    // Whether the IPv4 endpoint can be opened depends on the host; the IPv6 one is not opened again.
    added.Clear();
#if INET_CONFIG_ENABLE_IPV4
    const Listed two[] = { { usable, IPAddressType::kIPv6 }, { usable, IPAddressType::kIPv4 } };
#else
    const Listed two[] = { { usable, IPAddressType::kIPv6 }, { InterfaceId(kNoSuchInterface), IPAddressType::kIPv6 } };
#endif
    TheseInterfaces second(two);
    EXPECT_EQ(server.Listen(EndPoints(), &second, 0, &added), CHIP_NO_ERROR);
    EXPECT_FALSE(added.Has(usable, IPAddressType::kIPv6));
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
}

TEST_F(TestMinimalMdnsServer, KeepsListedEndpointsWhenTheTableIsFull)
{
#if INET_CONFIG_ENABLE_IPV4
    const InterfaceId usable = UsableInterface();
    if (!usable.IsPresent())
    {
        GTEST_SKIP() << "no usable interface on this host";
    }

    // One slot: the open IPv6 endpoint takes it.
    Server<1> server;
    const Listed one[] = { { usable, IPAddressType::kIPv6 } };
    TheseInterfaces first(one);
    EXPECT_EQ(server.Listen(EndPoints(), &first, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());

    // The IPv4 endpoint listed ahead of it finds no slot; the IPv6 one stays open.
    const Listed two[] = { { usable, IPAddressType::kIPv4 }, { usable, IPAddressType::kIPv6 } };
    TheseInterfaces second(two);
    EXPECT_EQ(server.Listen(EndPoints(), &second, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
#else
    GTEST_SKIP() << "needs a second endpoint on the same interface";
#endif
}

} // namespace
