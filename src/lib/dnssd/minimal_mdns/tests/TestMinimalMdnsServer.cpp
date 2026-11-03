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
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/support/CHIPMem.h>
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

    // Leave a single UDP endpoint: the first usable interface takes it, the next finds none.
    UDPEndPointHandle held[INET_CONFIG_NUM_UDP_ENDPOINTS - 1];
    for (auto & endpoint : held)
    {
        ASSERT_EQ(EndPoints()->NewEndPoint(endpoint), CHIP_NO_ERROR);
    }

    Server<10> server;
    SomeInterfaces interfaces(/* vanished = */ false, /* real = */ true);

    EXPECT_EQ(server.Listen(EndPoints(), &interfaces, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(server.IsListening());
    server.Shutdown();
}

} // namespace
