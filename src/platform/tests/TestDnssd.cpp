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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include <pw_unit_test/framework.h>

#include "lib/dnssd/platform/Dnssd.h"
#include "platform/CHIPDeviceLayer.h"
#include "platform/ConnectivityManager.h"
#include "platform/PlatformManager.h"
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Types.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy_DefaultImpl.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/ResponseSender.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/responders/IP.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include "platform/Darwin/dnssd/DnssdImpl.h"
#include <dns_sd.h>
#endif

using chip::Dnssd::DnssdService;
using chip::Dnssd::DnssdServiceProtocol;
using chip::Dnssd::TextEntry;

namespace {

class TestDnssdResolveServerDelegate : public mdns::Minimal::ServerDelegate, public mdns::Minimal::ParserDelegate
{
public:
    TestDnssdResolveServerDelegate(mdns::Minimal::ResponseSender * responder) : mResponder(responder) {}
    virtual ~TestDnssdResolveServerDelegate() = default;

    // Implementation of mdns::Minimal::ServerDelegate

    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override {}
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        mCurSrc = info;
        mdns::Minimal::ParsePacket(data, this);
        mCurSrc = nullptr;
    }

    // Implementation of mdns::Minimal::ParserDelegate

    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override { mMsgId = header.GetMessageId(); }
    void OnQuery(const mdns::Minimal::QueryData & data) override
    {
        TEMPORARY_RETURN_IGNORED mResponder->Respond(mMsgId, data, mCurSrc, mRespConfig);
    }
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data) override {}

private:
    mdns::Minimal::ResponseSender * mResponder;
    mdns::Minimal::ResponseConfiguration mRespConfig;
    const chip::Inet::IPPacketInfo * mCurSrc = nullptr;
    uint16_t mMsgId                          = 0;
};

} // namespace

class TestDnssd : public ::testing::Test
{
public: // protected
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    std::atomic<bool> mTimeoutExpired{ false };

    intptr_t mBrowseIdentifier = 0;

    unsigned int mBrowsedServicesCount  = 0;
    unsigned int mResolvedServicesCount = 0;
    bool mEndOfInput                    = false;
};

static void Timeout(chip::System::Layer * systemLayer, void * context)
{
    auto * ctx = static_cast<TestDnssd *>(context);
    ChipLogError(DeviceLayer, "mDNS test timeout, is avahi daemon running?");
    ctx->mTimeoutExpired = true;
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

static void HandleResolve(void * context, DnssdService * result, const chip::Span<chip::Inet::IPAddress> & addresses,
                          CHIP_ERROR error)
{
    auto * ctx = static_cast<TestDnssd *>(context);
    char addrBuf[100];

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    if (!addresses.empty())
    {
        addresses.data()[0].ToString(addrBuf, sizeof(addrBuf));
        printf("Service[%u] at [%s]:%u\n", ctx->mResolvedServicesCount, addrBuf, result->mPort);
    }

    EXPECT_EQ(result->mTextEntrySize, 1u);
    // must have at least 1 entry to check next key/val expectations.
    ASSERT_GE(result->mTextEntrySize, 1u);
    EXPECT_STREQ(result->mTextEntries[0].mKey, "key");
    constexpr uint8_t kExpectedVal[] = { 'v', 'a', 'l' };
    EXPECT_TRUE(
        chip::ByteSpan(result->mTextEntries[0].mData, result->mTextEntries[0].mDataSize).data_equal(chip::ByteSpan(kExpectedVal)));

    if (ctx->mBrowsedServicesCount == ++ctx->mResolvedServicesCount)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(Timeout, context);
        // StopEventLoopTask can be called from any thread, but when called from
        // non-Matter one it will lock the Matter stack. The same locking rules
        // are required when the resolve callback (this one) is called. In order
        // to avoid deadlocks, we need to call the StopEventLoopTask from inside
        // the Matter event loop by scheduling a lambda.
        TEMPORARY_RETURN_IGNORED chip::DeviceLayer::SystemLayer().ScheduleLambda([]() {
            // After last service is resolved, stop the event loop,
            // so the test case can gracefully exit.
            TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        });
    }
}

static void HandleBrowse(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse, CHIP_ERROR error)
{
    auto * ctx = static_cast<TestDnssd *>(context);

    // Make sure that we will not be called again after end-of-input is set
    EXPECT_FALSE(ctx->mEndOfInput);
    // Cancelled error is expected when the browse is stopped with
    // ChipDnssdStopBrowse(), so we will not assert on it.
    EXPECT_TRUE(error == CHIP_NO_ERROR || error == CHIP_ERROR_CANCELLED);

    ctx->mBrowsedServicesCount += servicesSize;
    ctx->mEndOfInput = finalBrowse;

    if (servicesSize > 0)
    {
        printf("Browse mDNS service size %u\n", static_cast<unsigned int>(servicesSize));
        for (unsigned int i = 0; i < servicesSize; i++)
        {
            printf("Service[%u] name %s\n", i, services[i].mName);
            printf("Service[%u] type %s\n", i, services[i].mType);
            EXPECT_EQ(ChipDnssdResolve(&services[i], services[i].mInterface, HandleResolve, context), CHIP_NO_ERROR);
        }
    }
}

static void DnssdErrorCallback(void * context, CHIP_ERROR error)
{
    EXPECT_EQ(error, CHIP_NO_ERROR);
}

void TestDnssdBrowse_DnssdInitCallback(void * context, CHIP_ERROR error)
{
    auto * ctx = static_cast<TestDnssd *>(context);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    EXPECT_EQ(ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolUdp, chip::Inet::IPAddressType::kAny,
                              chip::Inet::InterfaceId::Null(), HandleBrowse, context, &ctx->mBrowseIdentifier),
              CHIP_NO_ERROR);
}

#if !CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
// Verify that platform DNS-SD implementation can browse and resolve services.
//
// This test case uses platform-independent mDNS server implementation based on
// minimal mdns library. The server is configured to respond to PTR, SRV, TXT,
// A and AAAA queries without additional records. In order to pass this test,
// the platform DNS-SD client implementation must be able to browse and resolve
// services by querying for all of these records separately.
//
// However, when using Network.framework (on Darwin), we can't bind to port 5353
// because SO_REUSEPORT is not supported. As a result, the minimal server cannot run,
// even if it is compiled.
TEST_F(TestDnssd, TestDnssdBrowse)
{
    mdns::Minimal::SetDefaultAddressPolicy();

    mdns::Minimal::Server<10> server;
    mdns::Minimal::QNamePart serverName[] = { "resolve-tester", "_mock", chip::Dnssd::kCommissionProtocol,
                                              chip::Dnssd::kLocalDomain };
    mdns::Minimal::ResponseSender responseSender(&server);

    mdns::Minimal::QueryResponder<16> queryResponder;
    TEMPORARY_RETURN_IGNORED responseSender.AddQueryResponder(&queryResponder);

    // Respond to PTR queries for _mock._udp.local
    mdns::Minimal::QNamePart serviceName[]       = { "_mock", chip::Dnssd::kCommissionProtocol, chip::Dnssd::kLocalDomain };
    mdns::Minimal::QNamePart serverServiceName[] = { "INSTANCE", chip::Dnssd::kCommissionableServiceName,
                                                     chip::Dnssd::kCommissionProtocol, chip::Dnssd::kLocalDomain };
    mdns::Minimal::PtrResponder ptrUdpResponder(serviceName, serverServiceName);
    queryResponder.AddResponder(&ptrUdpResponder);

    // Respond to SRV queries for INSTANCE._matterc._udp.local
    mdns::Minimal::SrvResponder srvResponder(mdns::Minimal::SrvResourceRecord(serverServiceName, serverName, CHIP_PORT));
    queryResponder.AddResponder(&srvResponder);

    // Respond to TXT queries for INSTANCE._matterc._udp.local
    const char * txtEntries[] = { "key=val" };
    mdns::Minimal::TxtResponder txtResponder(mdns::Minimal::TxtResourceRecord(serverServiceName, txtEntries));
    queryResponder.AddResponder(&txtResponder);

    // Respond to A queries
    mdns::Minimal::IPv4Responder ipv4Responder(serverName);
    queryResponder.AddResponder(&ipv4Responder);

    // Respond to AAAA queries
    mdns::Minimal::IPv6Responder ipv6Responder(serverName);
    queryResponder.AddResponder(&ipv6Responder);

    TestDnssdResolveServerDelegate delegate(&responseSender);
    server.SetDelegate(&delegate);

    auto endpoints = mdns::Minimal::GetAddressPolicy()->GetListenEndpoints();
    EXPECT_EQ(server.Listen(chip::DeviceLayer::UDPEndPointManager(), endpoints.get(), 5353), CHIP_NO_ERROR);

    EXPECT_EQ(chip::Dnssd::ChipDnssdInit(TestDnssdBrowse_DnssdInitCallback, DnssdErrorCallback, this), CHIP_NO_ERROR);
    EXPECT_EQ(chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(5), Timeout, this), CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");

    EXPECT_GT(mResolvedServicesCount, 0u);
    EXPECT_FALSE(mTimeoutExpired);

    // Stop browsing so we can safely shutdown DNS-SD
    TEMPORARY_RETURN_IGNORED chip::Dnssd::ChipDnssdStopBrowse(mBrowseIdentifier);

    chip::Dnssd::ChipDnssdShutdown();
}
#endif // !CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

static void HandlePublish(void * context, const char * type, const char * instanceName, CHIP_ERROR error)
{
    EXPECT_EQ(error, CHIP_NO_ERROR);
}

static void TestDnssdPublishService_DnssdInitCallback(void * context, CHIP_ERROR error)
{
    auto * ctx = static_cast<TestDnssd *>(context);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    DnssdService service{};
    TextEntry entry{ "key", reinterpret_cast<const uint8_t *>("val"), 3 };

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    constexpr chip::Inet::InterfaceId kInterfaceId = chip::Inet::InterfaceId(kDNSServiceInterfaceIndexLocalOnly);
#else
    constexpr chip::Inet::InterfaceId kInterfaceId = chip::Inet::InterfaceId::Null();
#endif

    service.mInterface = kInterfaceId;
    service.mPort      = 80;
    strcpy(service.mHostName, "MatterTest");
    strcpy(service.mName, "test");
    strcpy(service.mType, "_mock");
    service.mAddressType   = chip::Inet::IPAddressType::kAny;
    service.mProtocol      = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mTextEntries   = &entry;
    service.mTextEntrySize = 1;
    service.mSubTypes      = nullptr;
    service.mSubTypeSize   = 0;

    EXPECT_EQ(ChipDnssdPublishService(&service, HandlePublish, nullptr), CHIP_NO_ERROR);

    EXPECT_EQ(ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolTcp, chip::Inet::IPAddressType::kAny, kInterfaceId,
                              HandleBrowse, context, &ctx->mBrowseIdentifier),
              CHIP_NO_ERROR);
}

// Verify that the platform DNS-SD implementation can publish services.
//
// This test uses platform implementation of DNS-SD server and client. Since
// client implementation should be verified by the TestDnssdBrowse test case,
// here we only verify that the server implementation can publish services.
TEST_F(TestDnssd, TestDnssdPublishService)
{
    EXPECT_EQ(chip::Dnssd::ChipDnssdInit(TestDnssdPublishService_DnssdInitCallback, DnssdErrorCallback, this), CHIP_NO_ERROR);
    EXPECT_EQ(chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(5), Timeout, this), CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");

    EXPECT_GT(mResolvedServicesCount, 0u);
    EXPECT_FALSE(mTimeoutExpired);

    // Stop browsing so we can safely shutdown DNS-SD
    TEMPORARY_RETURN_IGNORED chip::Dnssd::ChipDnssdStopBrowse(mBrowseIdentifier);

    chip::Dnssd::ChipDnssdShutdown();
}

#if CHIP_DEVICE_LAYER_TARGET_DARWIN

// Coverage for the deferred mDNS resolve teardown: when
// ChipDnssdResolveNoLongerNeeded drops the consumer count to zero we must NOT
// immediately tear down the DNSServiceRef (DNSServiceRefDeallocate discards
// already-queued socket events, which loses in-flight resolve results).
// Instead we defer the teardown so a still-queued read indicator can fire
// first, and a follow-up ChipDnssdResolve for the same instance name within
// the deferred window must reuse the existing ResolveContext rather than open
// a new DNSServiceCreateConnection. These tests pin the bookkeeping that
// drives that contract; they do not exercise the live mDNSResponder, only the
// platform-side context lifecycle.

namespace {

constexpr char kCoalesceTestInstanceName[] = "00000000-0000_TEST_DARWIN_COALESCE";
constexpr char kCoalesceTestServiceType[]  = "_matter._tcp";

void DummyResolveCallback(void * /* context */, DnssdService * /* result */,
                          const chip::Span<chip::Inet::IPAddress> & /* addresses */, CHIP_ERROR /* error */)
{}

// Dispatches `fn` on the Matter event loop and waits up to 2 seconds for
// it to complete. We use this so test bodies can read MdnsContexts state
// without racing the work queue that drives the dnssd platform layer.
//
// All shared state (the moved-in std::function, the synchronization
// primitives, the done flag) lives in a heap-allocated, refcounted Args
// struct. Either the dispatched lambda or this caller (whichever finishes
// last) deletes it. This guards against the timeout path: if cv.wait_for
// times out and we return, the lambda may still execute later -- but it
// will be operating on the heap struct, not on stack-resident references
// that have already been destroyed.
void RunOnMatterEventLoopSync(std::function<void()> fn)
{
    struct Args
    {
        std::atomic<int> refCount{ 2 };
        std::function<void()> fn;
        std::mutex mutex;
        std::condition_variable cv;
        bool done = false;
    };
    auto * args = new Args();
    args->fn    = std::move(fn);

    auto release = [](Args * a) {
        if (a->refCount.fetch_sub(1) == 1)
        {
            delete a;
        }
    };

    (void) chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            auto * a = reinterpret_cast<Args *>(arg);
            a->fn();
            {
                std::lock_guard<std::mutex> lk(a->mutex);
                a->done = true;
            }
            a->cv.notify_all();
            if (a->refCount.fetch_sub(1) == 1)
            {
                delete a;
            }
        },
        reinterpret_cast<intptr_t>(args));

    std::unique_lock<std::mutex> lk(args->mutex);
    bool finished = args->cv.wait_for(lk, std::chrono::seconds(2), [args] { return args->done; });
    EXPECT_TRUE(finished);
    lk.unlock();
    release(args);
}

DnssdService MakeCoalesceTestService()
{
    DnssdService service{};
    chip::Platform::CopyString(service.mName, kCoalesceTestInstanceName);
    chip::Platform::CopyString(service.mType, kCoalesceTestServiceType);
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mAddressType = chip::Inet::IPAddressType::kAny;
    service.mInterface   = chip::Inet::InterfaceId::Null();
    service.mPort        = 0;
    return service;
}

} // namespace

class TestDarwinDnssdResolveCoalesce : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().StartEventLoopTask(), CHIP_NO_ERROR);

        // Probe whether the underlying mDNSResponder is reachable on this host
        // by exercising the same ChipDnssdResolve path the tests rely on.
        // GitHub-hosted macOS (simulated) runners can pass a bare
        // DNSServiceCreateConnection probe yet still fail subsequent
        // DNSServiceResolve calls (returning a DNSServiceErrorType that maps
        // to CHIP_ERROR_INTERNAL). A bare connection probe therefore isn't a
        // reliable proxy for "tests can run here" -- do the actual round trip
        // through the platform shim and only enable the suite if it succeeds.
        //
        // Probe state is heap-allocated and refcounted: if our wait_for times
        // out and we return, the dispatched lambda may still run later and
        // would otherwise dereference stack memory that has gone out of scope.
        sBonjourAvailable = false;
        struct ProbeArgs
        {
            std::atomic<int> refCount{ 2 };
            DnssdService service{};
            std::mutex mutex;
            std::condition_variable cv;
            bool done        = false;
            CHIP_ERROR error = CHIP_NO_ERROR;
        };
        auto * probe   = new ProbeArgs();
        probe->service = MakeCoalesceTestService();

        auto release = [](ProbeArgs * a) {
            if (a->refCount.fetch_sub(1) == 1)
            {
                delete a;
            }
        };

        (void) chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto * a       = reinterpret_cast<ProbeArgs *>(arg);
                CHIP_ERROR err = chip::Dnssd::ChipDnssdResolve(&a->service, a->service.mInterface, DummyResolveCallback, nullptr);
                if (err == CHIP_NO_ERROR)
                {
                    chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
                }
                {
                    std::lock_guard<std::mutex> lk(a->mutex);
                    a->error = err;
                    a->done  = true;
                }
                a->cv.notify_all();
                if (a->refCount.fetch_sub(1) == 1)
                {
                    delete a;
                }
            },
            reinterpret_cast<intptr_t>(probe));

        std::unique_lock<std::mutex> lk(probe->mutex);
        bool finished         = probe->cv.wait_for(lk, std::chrono::seconds(2), [probe] { return probe->done; });
        CHIP_ERROR probeError = probe->error;
        lk.unlock();
        release(probe);
        if (finished && probeError == CHIP_NO_ERROR)
        {
            sBonjourAvailable = true;
            // Wait long enough for the (default 500ms) deferred-teardown timer
            // we just armed to fire, so the probe leaves no residual state for
            // the actual tests to trip over.
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
        }
    }
    static void TearDownTestSuite()
    {
        TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        if (!sBonjourAvailable)
        {
            GTEST_SKIP() << "mDNSResponder not usable in this CI environment; "
                         << "ChipDnssdResolve probe failed. Skipping ResolveCoalesce tests.";
        }
        // Use a short deferred-teardown delay so the "fires if no new resolve"
        // path is observable in test time. The default is 500ms.
        chip::Dnssd::SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32(50));
    }

    void TearDown() override
    {
        if (!sBonjourAvailable)
        {
            return;
        }
        // Restore default so other tests aren't affected.
        chip::Dnssd::SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32(500));
    }

protected:
    static bool sBonjourAvailable;
};

bool TestDarwinDnssdResolveCoalesce::sBonjourAvailable = false;

// Resolve -> ResolveNoLongerNeeded -> Resolve within the deferred-teardown
// window must reuse the existing ResolveContext: counter goes 1 -> 0 -> 1, the
// pending-teardown flag is cleared, and the same context pointer is observed
// throughout (no fresh allocation).
TEST_F(TestDarwinDnssdResolveCoalesce, ReusesContextWithinDeferredWindow)
{
    auto service = MakeCoalesceTestService();

    chip::Dnssd::ResolveContext * ctxPtrAfterFirstResolve = nullptr;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        ctxPtrAfterFirstResolve =
            chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxPtrAfterFirstResolve, nullptr);
        EXPECT_EQ(*ctxPtrAfterFirstResolve->consumerCounter, 1u);
        EXPECT_FALSE(ctxPtrAfterFirstResolve->deferredTeardownScheduled);
    });

    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        // Context is still alive (deferred teardown), counter is 0, timer
        // armed.
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, ctxPtrAfterFirstResolve);
        EXPECT_EQ(*ctx->consumerCounter, 0u);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    RunOnMatterEventLoopSync([&]() {
        // New resolve arrives within the window -- must reuse, not realloc.
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, ctxPtrAfterFirstResolve);
        EXPECT_EQ(*ctx->consumerCounter, 1u);
        EXPECT_FALSE(ctx->deferredTeardownScheduled);
    });

    // Clean up.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    // Wait long enough for the deferred teardown to fire.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Resolve -> ResolveNoLongerNeeded -> wait > teardown delay: the deferred
// teardown must fire and remove the context exactly once.
TEST_F(TestDarwinDnssdResolveCoalesce, DeferredTeardownFiresIfNoNewResolve)
{
    auto service = MakeCoalesceTestService();

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        ASSERT_NE(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });

    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // Wait > teardown delay (50ms set in SetUp; allow generous margin for
    // CI scheduling).
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

namespace {
struct CancelObserver
{
    std::atomic<int> failureDispatchCount{ 0 };
    CHIP_ERROR lastError = CHIP_NO_ERROR;
};

void CancelObservingCallback(void * context, DnssdService * /* result */, const chip::Span<chip::Inet::IPAddress> & /* addresses */,
                             CHIP_ERROR error)
{
    auto * obs = static_cast<CancelObserver *>(context);
    if (error != CHIP_NO_ERROR)
    {
        obs->failureDispatchCount.fetch_add(1);
        obs->lastError = error;
    }
}
} // namespace

// Even with the deferred-teardown window, when no new Resolve comes in and no
// in-flight result arrives, callers must still observe a single
// CHIP_ERROR_CANCELLED dispatch -- preserving the existing failure-path
// contract upper-layer state machines (MTRDevice CASE retry, Discovery) rely
// on.
TEST_F(TestDarwinDnssdResolveCoalesce, CancelStillPropagatesIfNoInFlightResult)
{
    auto service = MakeCoalesceTestService();
    CancelObserver obs;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CancelObservingCallback, &obs), CHIP_NO_ERROR);
    });
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });

    // Wait > teardown delay so the timer fires.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
    EXPECT_EQ(obs.failureDispatchCount.load(), 1);
    EXPECT_EQ(obs.lastError, CHIP_ERROR_CANCELLED);
}

namespace {
class NoopDiscoverNodeDelegate : public chip::Dnssd::DiscoverNodeDelegate
{
public:
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & /* nodeData */) override {}
};
} // namespace

// Mirror of ReusesContextWithinDeferredWindow but driving the
// DiscoverNodeDelegate overload of ChipDnssdResolve. Delegate-based resolves
// (used by MTRCommissionableBrowser's rapid OnBrowseAdd/OnBrowseRemove churn)
// are deliberately NOT subject to deferred teardown: holding the underlying
// DNSServiceRef alive across browse cancel/restart cycles starves
// DNSServiceGetAddrInfo of a chance to deliver before the next remove fires,
// which manifested as MTRCommissionableBrowserTests/test005 timing out under
// TSAN. This test pins that contract: ChipDnssdResolveNoLongerNeeded on a
// delegate-based context tears it down synchronously.
TEST_F(TestDarwinDnssdResolveCoalesce, DelegateBasedResolveIsNotDeferred)
{
    auto service = MakeCoalesceTestService();
    NoopDiscoverNodeDelegate delegate;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, &delegate), CHIP_NO_ERROR);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(*ctx->consumerCounter, 1u);
        EXPECT_FALSE(ctx->deferredTeardownScheduled);
    });

    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        // Delegate-based context is torn down synchronously -- no
        // deferred-teardown window, no opportunity to coalesce.
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

namespace {
// Walks MdnsContexts and returns every ResolveContext whose Matches() agrees
// with `name`. Used by the multi-sibling rescue tests below.
std::vector<chip::Dnssd::ResolveContext *> AllResolveSiblings(const char * name)
{
    std::vector<chip::Dnssd::GenericContext *> generic;
    chip::Dnssd::MdnsContexts::GetInstance().FindAllMatchingPredicate(
        [name](chip::Dnssd::GenericContext * item) {
            return item->type == chip::Dnssd::ContextType::Resolve &&
                static_cast<chip::Dnssd::ResolveContext *>(item)->Matches(name);
        },
        generic);
    std::vector<chip::Dnssd::ResolveContext *> out;
    out.reserve(generic.size());
    for (auto * g : generic)
    {
        out.push_back(static_cast<chip::Dnssd::ResolveContext *>(g));
    }
    return out;
}
} // namespace

// Regression pin for the multi-sibling rescue path. When N sibling
// ResolveContexts share the same instance name (each with deferred-teardown
// armed), a single follow-up ChipDnssdResolve must clear deferredTeardownScheduled
// on EVERY sibling. Pre-fix the loop incremented the shared counter on the
// first iteration, flipping the *consumerCounter == 0 gate to false, so
// subsequent siblings were never rescued -- their timers would still fire and
// OnResolveDeferredTeardown would observe a non-zero counter, leaving them
// alive. This test fails (siblings 2..N still have deferredTeardownScheduled=true)
// without the fix and passes with it.
TEST_F(TestDarwinDnssdResolveCoalesce, RescuesAllSiblingContextsWithinDeferredWindow)
{
    auto service = MakeCoalesceTestService();

    // Build two sibling ResolveContexts by issuing two back-to-back
    // ChipDnssdResolve calls. The second call won't coalesce (no deferred
    // window armed yet) so a fresh ResolveContext is created sharing the
    // same consumerCounter shared_ptr.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        // All siblings share the same counter holder.
        EXPECT_EQ(*siblings[0]->consumerCounter, 2u);
        EXPECT_EQ(siblings[0]->consumerCounter.get(), siblings[1]->consumerCounter.get());
    });

    // Drop both consumers; both siblings should arm deferred-teardown.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 0u);
        for (auto * s : siblings)
        {
            EXPECT_TRUE(s->deferredTeardownScheduled);
        }
    });

    // Single follow-up resolve must rescue EVERY sibling (clear the flag),
    // regardless of how many there are.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        for (auto * s : siblings)
        {
            EXPECT_FALSE(s->deferredTeardownScheduled);
        }
    });

    // Clean up: drop the rescue counter and let teardown finalize.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Regression pin for the BLOCKER. Siblings share the same shared_ptr<uint32_t>
// consumerCounter; the rescue path must increment it EXACTLY ONCE for the
// new logical Resolve, not once per sibling walked. Pre-fix code did the latter,
// leaving the counter permanently elevated and preventing eventual teardown.
TEST_F(TestDarwinDnssdResolveCoalesce, SiblingRescueDoesNotOverIncrementSharedCounter)
{
    auto service = MakeCoalesceTestService();

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
    });

    // Drop both consumers. After this the shared counter is 0 and both
    // siblings are armed for deferred teardown.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 0u);
    });

    // Single Resolve. Counter MUST go to exactly 1, not 2 (or N).
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 1u);
    });

    // The rescue should restore the system to a state where a single
    // ChipDnssdResolveNoLongerNeeded actually frees everything.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Pins context identity and shared_ptr stability across rapid
// Resolve/NoLongerNeeded toggle cycles inside the deferred window. The
// ResolveContext pointer and the consumerCounter shared_ptr must not change.
TEST_F(TestDarwinDnssdResolveCoalesce, RepeatedToggleWithinDeferredWindowKeepsSameContext)
{
    auto service                           = MakeCoalesceTestService();
    chip::Dnssd::ResolveContext * firstCtx = nullptr;
    std::shared_ptr<uint32_t> firstCounter;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        firstCtx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(firstCtx, nullptr);
        firstCounter = firstCtx->consumerCounter;
    });

    for (int i = 0; i < 3; ++i)
    {
        RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
        RunOnMatterEventLoopSync([&]() {
            EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
            auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
            ASSERT_NE(ctx, nullptr);
            EXPECT_EQ(ctx, firstCtx);
            EXPECT_EQ(ctx->consumerCounter.get(), firstCounter.get());
            EXPECT_EQ(*ctx->consumerCounter, 1u);
        });
    }

    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Pins late-firing deferred-teardown timer vs rescue: the timer callback
// already guards on deferredTeardownScheduled and consumerCounter, so a stale
// timer firing after the rescue must be a no-op (it sees the flag cleared and
// returns). Simulate by calling OnResolveDeferredTeardown directly on a rescued
// context.
TEST_F(TestDarwinDnssdResolveCoalesce, LateDeferredTeardownTimerDoesNotTearDownReusedContext)
{
    auto service                             = MakeCoalesceTestService();
    chip::Dnssd::ResolveContext * rescuedCtx = nullptr;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
    });
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        rescuedCtx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(rescuedCtx, nullptr);
        EXPECT_FALSE(rescuedCtx->deferredTeardownScheduled);
        EXPECT_EQ(*rescuedCtx->consumerCounter, 1u);
    });

    // Synthesize a late timer fire. The implementation must observe
    // deferredTeardownScheduled == false (rescue cleared it) and return
    // without finalizing.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::OnResolveDeferredTeardown(&chip::DeviceLayer::SystemLayer(), rescuedCtx);
        // Context must still be alive after the late-timer no-op.
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, rescuedCtx);
        EXPECT_EQ(*ctx->consumerCounter, 1u);
    });

    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

namespace {
struct CallbackCounter
{
    std::atomic<int> count{ 0 };
};
void CountingResolveCallback(void * context, DnssdService * /* result */, const chip::Span<chip::Inet::IPAddress> & /* addresses */,
                             CHIP_ERROR /* error */)
{
    static_cast<CallbackCounter *>(context)->count.fetch_add(1);
}
} // namespace

// Pins the boundary-input case the bot reviewers flagged: when a second
// Resolve in the deferred window comes from a different caller (different
// callback / context), the rescued ResolveContext's dispatch target MUST be
// rebound to the new caller. The previous consumer's counter dropped to zero,
// so its callback / context pointers may now point to freed memory; reusing
// them would be a UAF and the new caller would never receive results.
TEST_F(TestDarwinDnssdResolveCoalesce, MismatchedCallbackInDeferredWindowRebindsToNewCaller)
{
    auto service = MakeCoalesceTestService();
    CallbackCounter callerA;
    CallbackCounter callerB;
    chip::Dnssd::ResolveContext * ctxAfterRescue = nullptr;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &callerA), CHIP_NO_ERROR);
    });
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });

    // Different caller comes in during the deferred window.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &callerB), CHIP_NO_ERROR);
        ctxAfterRescue = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxAfterRescue, nullptr);
        // Dispatch target must point at the NEW caller (callerB), not the
        // possibly-freed original.
        EXPECT_EQ(reinterpret_cast<void *>(ctxAfterRescue->callback), reinterpret_cast<void *>(&CountingResolveCallback));
        EXPECT_EQ(ctxAfterRescue->context, static_cast<void *>(&callerB));
    });

    // Cancel and let teardown fire CHIP_ERROR_CANCELLED. callerB receives
    // it, callerA does not.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(callerA.count.load(), 0);
    EXPECT_EQ(callerB.count.load(), 1);
}

// Endurance / sustained-cycle pin: 50 Resolve/NoLongerNeeded toggles inside the
// deferred window must keep ResolveContext pointer, consumerCounter, and the
// MdnsContexts vector size stable.
TEST_F(TestDarwinDnssdResolveCoalesce, SustainedResolveCancelCyclesReuseSameContext)
{
    auto service                           = MakeCoalesceTestService();
    chip::Dnssd::ResolveContext * firstCtx = nullptr;
    std::shared_ptr<uint32_t> firstCounter;
    size_t initialContextCount = 0;

    RunOnMatterEventLoopSync([&]() {
        initialContextCount = AllResolveSiblings(kCoalesceTestInstanceName).size();
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        firstCtx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(firstCtx, nullptr);
        firstCounter = firstCtx->consumerCounter;
    });

    for (int i = 0; i < 50; ++i)
    {
        RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
        RunOnMatterEventLoopSync([&]() {
            EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        });
    }

    RunOnMatterEventLoopSync([&]() {
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, firstCtx);
        EXPECT_EQ(ctx->consumerCounter.get(), firstCounter.get());
        EXPECT_EQ(*ctx->consumerCounter, 1u);
        // Should be exactly the same number of sibling contexts as we had at
        // the end of the very first Resolve -- no leakage.
        EXPECT_EQ(AllResolveSiblings(kCoalesceTestInstanceName).size(), initialContextCount + 1);
    });

    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Focused counter-sequence pin: a Resolve -> ResolveNoLongerNeeded -> Resolve
// pair, where no callback dispatch / in-flight result occurs in the middle,
// must drive the shared consumerCounter through exactly 1 -> 0 -> 1 with the
// rescued context still alive. This pins the state-machine contract
// independent of the other tests' broader assertions (sibling fan-out,
// dispatch rebinding, etc.). Pre-fix the second Resolve would Finalize the
// old context and allocate a new one, breaking pointer identity AND firing a
// CHIP_ERROR_CANCELLED dispatch into the (already-departed) original caller.
TEST_F(TestDarwinDnssdResolveCoalesce, CancelThenRestartWithNoResultsKeepsCounterCorrect)
{
    auto service = MakeCoalesceTestService();
    CallbackCounter dispatches;
    chip::Dnssd::ResolveContext * ctxAfterFirstResolve = nullptr;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &dispatches), CHIP_NO_ERROR);
        ctxAfterFirstResolve =
            chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxAfterFirstResolve, nullptr);
        EXPECT_EQ(*ctxAfterFirstResolve->consumerCounter, 1u);
    });

    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        // Same context, counter dropped to zero, deferred teardown armed.
        EXPECT_EQ(ctx, ctxAfterFirstResolve);
        EXPECT_EQ(*ctx->consumerCounter, 0u);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // No results, no dispatched callbacks should have fired in between.
    EXPECT_EQ(dispatches.count.load(), 0);

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &dispatches), CHIP_NO_ERROR);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        // Same context pointer (rescue, not realloc), counter back to 1,
        // deferred teardown flag cleared.
        EXPECT_EQ(ctx, ctxAfterFirstResolve);
        EXPECT_EQ(*ctx->consumerCounter, 1u);
        EXPECT_FALSE(ctx->deferredTeardownScheduled);
    });

    // Still no callback dispatches: the cancel-then-restart cycle must not
    // synthesize a CHIP_ERROR_CANCELLED into either the old or new caller
    // when the rescue path was taken.
    EXPECT_EQ(dispatches.count.load(), 0);

    // Clean up.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Two concurrent ChipDnssdResolve calls for the SAME instance name from two
// distinct consumers (different callback/context pairs), with NO cancel in
// between, must produce coalesced bookkeeping: separate ResolveContext
// siblings, each tracking its own caller's callback/context, but sharing a
// single std::shared_ptr<uint32_t> consumerCounter that accounts for both
// outstanding logical resolves (counter == 2). Dropping one consumer must
// leave the other intact (counter -> 1, no deferred-teardown armed); dropping
// the second triggers deferred teardown of all siblings.
TEST_F(TestDarwinDnssdResolveCoalesce, ConcurrentResolvesFromDifferentConsumersShareCounter)
{
    auto service = MakeCoalesceTestService();
    CallbackCounter consumerA;
    CallbackCounter consumerB;

    RunOnMatterEventLoopSync([&]() {
        // First consumer. After this we have a single sibling with counter=1.
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &consumerA), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_EQ(siblings.size(), 1u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 1u);
        EXPECT_FALSE(siblings[0]->deferredTeardownScheduled);

        // Second concurrent consumer for the SAME name -- no cancel between.
        // GetCounterHolder returns the existing shared counter, so both
        // siblings track outstanding-consumer count via the same shared_ptr.
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &consumerB), CHIP_NO_ERROR);
        siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_EQ(siblings.size(), 2u);
        // Counter is shared and now reflects BOTH outstanding consumers.
        EXPECT_EQ(siblings[0]->consumerCounter.get(), siblings[1]->consumerCounter.get());
        EXPECT_EQ(*siblings[0]->consumerCounter, 2u);
        // Each sibling has its own dispatch target pointing at the
        // distinct caller that registered it.
        bool sawA = false, sawB = false;
        for (auto * s : siblings)
        {
            if (s->context == static_cast<void *>(&consumerA))
            {
                sawA = true;
            }
            else if (s->context == static_cast<void *>(&consumerB))
            {
                sawB = true;
            }
            // No deferred-teardown armed -- both consumers are still live.
            EXPECT_FALSE(s->deferredTeardownScheduled);
        }
        EXPECT_TRUE(sawA);
        EXPECT_TRUE(sawB);
    });

    // First consumer drops out. Counter goes 2 -> 1. No deferred teardown
    // armed, no CHIP_ERROR_CANCELLED dispatched (still one live consumer).
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_EQ(siblings.size(), 2u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 1u);
        for (auto * s : siblings)
        {
            EXPECT_FALSE(s->deferredTeardownScheduled);
        }
    });
    EXPECT_EQ(consumerA.count.load(), 0);
    EXPECT_EQ(consumerB.count.load(), 0);

    // Second consumer drops out. Counter -> 0; deferred teardown armed on
    // every sibling.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_EQ(siblings.size(), 2u);
        EXPECT_EQ(*siblings[0]->consumerCounter, 0u);
        for (auto * s : siblings)
        {
            EXPECT_TRUE(s->deferredTeardownScheduled);
        }
    });

    // Let the deferred teardown fire. Each sibling dispatches CHIP_ERROR_CANCELLED
    // once into its own caller; total observed callbacks == number of siblings,
    // split evenly between the two consumers.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
    EXPECT_EQ(consumerA.count.load(), 1);
    EXPECT_EQ(consumerB.count.load(), 1);
}

// A ChipDnssdResolve for a DIFFERENT instance name must NOT rescue a deferred
// ResolveContext belonging to some other instance name. The rescue path keys
// on Matches(name); a regression that loosened the predicate (e.g. matching
// by service type only) would silently revive an unrelated context, leak the
// fresh DNSServiceRef the second resolve should have opened, and rebind the
// stale context's callback to the wrong caller. This test pins that scoping.
TEST_F(TestDarwinDnssdResolveCoalesce, DifferentInstanceNameDoesNotRescueDeferredContext)
{
    auto serviceA = MakeCoalesceTestService();

    // Build a second service struct for a distinct instance name but the
    // same service type so the only meaningful difference is the name the
    // coalescing predicate matches on.
    DnssdService serviceB{};
    constexpr char kOtherInstanceName[] = "00000000-0000_TEST_DARWIN_COALESCE_OTHER";
    chip::Platform::CopyString(serviceB.mName, kOtherInstanceName);
    chip::Platform::CopyString(serviceB.mType, kCoalesceTestServiceType);
    serviceB.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    serviceB.mAddressType = chip::Inet::IPAddressType::kAny;
    serviceB.mInterface   = chip::Inet::InterfaceId::Null();
    serviceB.mPort        = 0;

    CallbackCounter consumerA;
    CallbackCounter consumerB;
    chip::Dnssd::ResolveContext * ctxA = nullptr;

    // Resolve A, cancel A -> A is in the deferred-teardown window.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceA, serviceA.mInterface, CountingResolveCallback, &consumerA),
                  CHIP_NO_ERROR);
        ctxA = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxA, nullptr);
    });
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, ctxA);
        EXPECT_EQ(*ctx->consumerCounter, 0u);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // Now resolve a DIFFERENT instance name. Must NOT rescue A: A must
    // remain in deferred-teardown, callback must remain bound to consumerA,
    // and a fresh ResolveContext must be created for B.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceB, serviceB.mInterface, CountingResolveCallback, &consumerB),
                  CHIP_NO_ERROR);
        auto * ctxBNew = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kOtherInstanceName);
        ASSERT_NE(ctxBNew, nullptr);
        EXPECT_NE(ctxBNew, ctxA);
        EXPECT_EQ(*ctxBNew->consumerCounter, 1u);
        EXPECT_FALSE(ctxBNew->deferredTeardownScheduled);

        // A's state is untouched by B's resolve.
        auto * ctxAStill = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxAStill, nullptr);
        EXPECT_EQ(ctxAStill, ctxA);
        EXPECT_EQ(*ctxAStill->consumerCounter, 0u);
        EXPECT_TRUE(ctxAStill->deferredTeardownScheduled);
        EXPECT_EQ(ctxAStill->context, static_cast<void *>(&consumerA));
    });

    // Let A's deferred teardown fire and clean up B.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kOtherInstanceName);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kOtherInstanceName), nullptr);
    });

    // Each caller saw exactly one CHIP_ERROR_CANCELLED dispatch, scoped to
    // its own resolve -- no cross-rescue side effects.
    EXPECT_EQ(consumerA.count.load(), 1);
    EXPECT_EQ(consumerB.count.load(), 1);
}

// While a ResolveContext is in the deferred-teardown window (counter==0,
// timer armed), an explicit Finalize() with a non-cancelled error must
// cancel the pending deferred-teardown timer before destroying the context.
// Otherwise the timer would fire later against freed memory -- a use-after-
// free that the DispatchFailure-side timer-cancel block was specifically
// added to prevent. We also assert the failure-path callback fires exactly
// once (from Finalize), not twice (Finalize + later timer).
TEST_F(TestDarwinDnssdResolveCoalesce, FinalizeInDeferredWindowCancelsPendingTimer)
{
    auto service = MakeCoalesceTestService();
    CancelObserver obs;

    // Use a slightly longer teardown delay so we have headroom to call
    // Finalize before the timer would have fired on its own.
    chip::Dnssd::SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32(200));

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CancelObservingCallback, &obs), CHIP_NO_ERROR);
    });
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(*ctx->consumerCounter, 0u);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // Drive Finalize directly while the timer is still pending. The
    // failure-dispatch path must cancel the deferred-teardown timer
    // (otherwise the timer would later fire OnResolveDeferredTeardown on a
    // freed pointer).
    RunOnMatterEventLoopSync([&]() {
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        TEMPORARY_RETURN_IGNORED ctx->Finalize(CHIP_ERROR_INTERNAL);
        // Context must be gone from the tracker after Finalize.
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
    // The caller observed exactly one failure dispatch from Finalize.
    EXPECT_EQ(obs.failureDispatchCount.load(), 1);

    // Wait well past when the (now-cancelled) deferred-teardown timer would
    // have fired. If the cancel were missing, OnResolveDeferredTeardown
    // would either crash on freed memory or (best case) re-dispatch a
    // second CHIP_ERROR_CANCELLED. Either way the observer count would
    // change.
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_EQ(obs.failureDispatchCount.load(), 1);

    // Tracker still shows no context for this name.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });

    // Restore the short delay TearDown expects.
    chip::Dnssd::SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32(50));
}

// Cancelling instance A while instance B is also live and in flight must not
// arm a deferred-teardown timer against B. The cancel path is per-instance;
// a regression that armed timers across siblings of unrelated instance names
// would corrupt B's state machine (e.g. cause B to mis-fire
// CHIP_ERROR_CANCELLED while a consumer is still live).
TEST_F(TestDarwinDnssdResolveCoalesce, CancelOneInstanceDoesNotArmTimerOnAnother)
{
    auto serviceA = MakeCoalesceTestService();

    DnssdService serviceB{};
    constexpr char kIsolatedInstanceName[] = "00000000-0000_TEST_DARWIN_COALESCE_ISOLATED";
    chip::Platform::CopyString(serviceB.mName, kIsolatedInstanceName);
    chip::Platform::CopyString(serviceB.mType, kCoalesceTestServiceType);
    serviceB.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    serviceB.mAddressType = chip::Inet::IPAddressType::kAny;
    serviceB.mInterface   = chip::Inet::InterfaceId::Null();
    serviceB.mPort        = 0;

    CallbackCounter consumerA;
    CallbackCounter consumerB;

    // Both resolves live concurrently.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceA, serviceA.mInterface, CountingResolveCallback, &consumerA),
                  CHIP_NO_ERROR);
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceB, serviceB.mInterface, CountingResolveCallback, &consumerB),
                  CHIP_NO_ERROR);
        auto * ctxA = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        auto * ctxB = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kIsolatedInstanceName);
        ASSERT_NE(ctxA, nullptr);
        ASSERT_NE(ctxB, nullptr);
        EXPECT_NE(ctxA, ctxB);
        // Distinct shared_ptr<uint32_t> -- counters are per-instance.
        EXPECT_NE(ctxA->consumerCounter.get(), ctxB->consumerCounter.get());
        EXPECT_EQ(*ctxA->consumerCounter, 1u);
        EXPECT_EQ(*ctxB->consumerCounter, 1u);
        EXPECT_FALSE(ctxA->deferredTeardownScheduled);
        EXPECT_FALSE(ctxB->deferredTeardownScheduled);
    });

    // Cancel only A. B must be untouched: same context pointer, counter
    // still 1, NO deferred-teardown timer armed.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctxA = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        auto * ctxB = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kIsolatedInstanceName);
        ASSERT_NE(ctxA, nullptr);
        ASSERT_NE(ctxB, nullptr);
        EXPECT_EQ(*ctxA->consumerCounter, 0u);
        EXPECT_TRUE(ctxA->deferredTeardownScheduled);
        EXPECT_EQ(*ctxB->consumerCounter, 1u);
        EXPECT_FALSE(ctxB->deferredTeardownScheduled);
    });

    // Let A's teardown fire. B must still be alive and untouched, and B
    // must NOT have received any CHIP_ERROR_CANCELLED dispatch.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
        auto * ctxB = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kIsolatedInstanceName);
        ASSERT_NE(ctxB, nullptr);
        EXPECT_EQ(*ctxB->consumerCounter, 1u);
        EXPECT_FALSE(ctxB->deferredTeardownScheduled);
    });
    EXPECT_EQ(consumerA.count.load(), 1);
    EXPECT_EQ(consumerB.count.load(), 0);

    // Clean up B.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kIsolatedInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kIsolatedInstanceName), nullptr);
    });
    EXPECT_EQ(consumerB.count.load(), 1);
}

// Over-cancel safety: ChipDnssdResolveNoLongerNeeded called more times than
// there are outstanding consumers must be a no-op once the counter reaches
// zero. The implementation guards on `*consumerCounter != 0` before
// decrementing; without that guard a second cancel would underflow the
// uint32_t counter, making the deferred-teardown timer never fire (counter
// huge, never observed as 0) and leaking the ResolveContext + its
// DNSServiceRef. Pin: a second cancel must NOT re-arm the timer, must NOT
// underflow, and the originally-armed teardown must still fire on schedule.
TEST_F(TestDarwinDnssdResolveCoalesce, RedundantResolveNoLongerNeededIsNoOp)
{
    auto service = MakeCoalesceTestService();

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
    });

    chip::Dnssd::ResolveContext * armedCtx = nullptr;
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        armedCtx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(armedCtx, nullptr);
        EXPECT_EQ(*armedCtx->consumerCounter, 0u);
        EXPECT_TRUE(armedCtx->deferredTeardownScheduled);
    });

    // Second (redundant) cancel against a counter that's already zero. Must
    // not underflow, must not re-arm.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, armedCtx);
        // Counter still zero (NOT 0xFFFFFFFF from underflow).
        EXPECT_EQ(*ctx->consumerCounter, 0u);
        // Flag still set; no spurious second timer.
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // Original timer must still fire and remove the context.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Defense-in-depth pin for the OnResolveDeferredTeardown guards. If a stale
// timer somehow fires after the context has already been finalized (e.g. a
// successful resolve completed and removed the context, but the timer cancel
// missed a sibling), the callback's MdnsContexts::Has() guard must kick in
// and the call must be a no-op rather than a UAF on freed memory. Drive this
// directly by finalizing a context, then synthesizing a late-fire of the
// timer with the (now-stale) pointer. The callback should observe Has() !=
// CHIP_NO_ERROR and return without dereferencing sdCtx beyond the tracker
// check.
//
// This test deliberately does NOT pass the freed pointer back to the
// callback -- a real late timer fire would, but observing UAF is impossible
// from a unit test. We instead pass a pointer that's known not to be tracked
// (a fresh ResolveContext that has never been registered with MdnsContexts)
// to exercise the Has()-guard path.
TEST_F(TestDarwinDnssdResolveCoalesce, OnResolveDeferredTeardownIgnoresUntrackedContext)
{
    // Drive the callback with a heap-allocated context that was never
    // registered. The Has() check should short-circuit before any field
    // access on sdCtx that depends on it being tracked. We allocate via
    // chip::Platform::New so the destructor / memory ownership story matches
    // a real ResolveContext lifecycle; we delete after the call returns.
    auto counterHolder = std::make_shared<uint32_t>(0u);

    RunOnMatterEventLoopSync([&]() {
        auto counterCopy = counterHolder;
        auto * untracked = chip::Platform::New<chip::Dnssd::ResolveContext>(
            /* cbContext */ nullptr, DummyResolveCallback, chip::Inet::IPAddressType::kAny, kCoalesceTestInstanceName,
            /* browseCausingResolve */ static_cast<chip::Dnssd::BrowseContext *>(nullptr), std::move(counterCopy));
        ASSERT_NE(untracked, nullptr);
        // Do NOT add to MdnsContexts. Has() must report not-found.
        EXPECT_NE(chip::Dnssd::MdnsContexts::GetInstance().Has(untracked), CHIP_NO_ERROR);

        // Synthesize a stale timer fire. Must not crash, must not finalize
        // anything, must not touch any other tracked context.
        chip::Dnssd::OnResolveDeferredTeardown(&chip::DeviceLayer::SystemLayer(), untracked);

        chip::Platform::Delete(untracked);
    });

    // Tracker remains empty for our test instance.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

// Defense-in-depth pin for the second OnResolveDeferredTeardown guard: if the
// timer fires while the context is still tracked but the
// `deferredTeardownScheduled` flag has been cleared (e.g. a rescue happened
// but the timer cancel was racy and missed), the callback must observe the
// flag false and return WITHOUT finalizing -- the ResolveContext belongs to
// a live consumer. A regression that finalizes regardless of the flag would
// fire a spurious CHIP_ERROR_CANCELLED into the new caller and tear down
// their query mid-flight.
TEST_F(TestDarwinDnssdResolveCoalesce, OnResolveDeferredTeardownIgnoresClearedFlag)
{
    auto service = MakeCoalesceTestService();
    CallbackCounter consumer;
    chip::Dnssd::ResolveContext * rescuedCtx = nullptr;

    // Drive Resolve -> NoLongerNeeded -> Resolve to land in the rescued
    // state: deferredTeardownScheduled cleared, consumerCounter == 1.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &consumer), CHIP_NO_ERROR);
    });
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &consumer), CHIP_NO_ERROR);
        rescuedCtx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(rescuedCtx, nullptr);
        EXPECT_FALSE(rescuedCtx->deferredTeardownScheduled);
        EXPECT_EQ(*rescuedCtx->consumerCounter, 1u);
    });

    // Synthesize a stale timer firing on the rescued (still-tracked) context.
    // Guard hits: deferredTeardownScheduled == false -> early return.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::OnResolveDeferredTeardown(&chip::DeviceLayer::SystemLayer(), rescuedCtx);
        // Context survived the late timer fire.
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_EQ(ctx, rescuedCtx);
        EXPECT_EQ(*ctx->consumerCounter, 1u);
    });
    // No CHIP_ERROR_CANCELLED dispatched into the live consumer.
    EXPECT_EQ(consumer.count.load(), 0);

    // Clean up: cancel and let teardown fire. Now (and only now) the live
    // consumer receives its single CHIP_ERROR_CANCELLED.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
    EXPECT_EQ(consumer.count.load(), 1);
}

namespace {
// RAII guard for the test-only deferred-teardown delay knob. Captures the
// current delay on construction and restores it on destruction so the value
// doesn't leak across tests even when an ASSERT_* longjmp short-circuits the
// test body.
struct ScopedResolveDeferredTeardownDelay
{
    chip::System::Clock::Milliseconds32 previous;
    explicit ScopedResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32 next) :
        previous(chip::Dnssd::GetResolveDeferredTeardownDelay())
    {
        chip::Dnssd::SetResolveDeferredTeardownDelay(next);
    }
    ~ScopedResolveDeferredTeardownDelay() { chip::Dnssd::SetResolveDeferredTeardownDelay(previous); }
};
} // namespace

// Multi-sibling rescue UAF regression: when N>1 sibling ResolveContexts share
// a deferred-teardown window and a new caller comes in, only one (the
// "primary") sibling can hold the new caller's callback/context. The others
// must be Finalize()d in the rescue path -- otherwise they retain stale
// callback/context pointers from the departed consumer, and the next dispatch
// (success result OR a future deferred-teardown timer fire) is a UAF on freed
// memory. This pin verifies that after the rescue exactly one ResolveContext
// remains for the instance name and the previous caller's callback is NOT
// invoked.
TEST_F(TestDarwinDnssdResolveCoalesce, RescuingMultipleSiblingsDoesNotInvokeStaleCallback)
{
    auto service = MakeCoalesceTestService();
    CallbackCounter callerA;
    CallbackCounter callerB;

    // Build two sibling ResolveContexts via two back-to-back resolves. The
    // second won't coalesce (no deferred-teardown window armed yet), so a
    // fresh sibling is created sharing the consumerCounter shared_ptr.
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &callerA), CHIP_NO_ERROR);
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &callerA), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
    });

    // Drop both consumers; both siblings arm deferred-teardown.
    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        ASSERT_GE(siblings.size(), 2u);
        for (auto * s : siblings)
        {
            EXPECT_TRUE(s->deferredTeardownScheduled);
        }
    });

    // New caller (callerB) arrives. Rescue: primary keeps callerB callback;
    // non-primary siblings must be Finalize()d (Finalize fires
    // CHIP_ERROR_CANCELLED into their stale callerA callback exactly once
    // each, then removes them from MdnsContexts).
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&service, service.mInterface, CountingResolveCallback, &callerB), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        EXPECT_EQ(siblings.size(), 1u);
        ASSERT_EQ(siblings.size(), 1u);
        EXPECT_EQ(siblings[0]->context, static_cast<void *>(&callerB));
    });

    // Clean up: drop callerB's resolve and let teardown fire. callerB
    // observes its single CHIP_ERROR_CANCELLED.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
    // callerA observed at least one CHIP_ERROR_CANCELLED (from the
    // non-primary-sibling Finalize). callerB observed exactly one (from the
    // final teardown).
    EXPECT_GE(callerA.count.load(), 1);
    EXPECT_EQ(callerB.count.load(), 1);
}

// The rescue path must refuse to coalesce a new caller's resolve onto an
// existing deferred-teardown context whose persisted interfaceId differs.
// Without this guard the new caller silently inherits the previous caller's
// interface scope.
TEST_F(TestDarwinDnssdResolveCoalesce, RescueRefusesCoalesceWhenInterfaceMismatch)
{
    auto serviceA                      = MakeCoalesceTestService();
    serviceA.mInterface                = chip::Inet::InterfaceId(7);
    chip::Dnssd::ResolveContext * ctxA = nullptr;

    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceA, serviceA.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        ctxA = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctxA, nullptr);
        EXPECT_EQ(ctxA->requestedInterfaceId, 7u);
    });

    RunOnMatterEventLoopSync([&]() {
        chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName);
        auto * ctx = chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName);
        ASSERT_NE(ctx, nullptr);
        EXPECT_TRUE(ctx->deferredTeardownScheduled);
    });

    // New caller asks for the SAME instance name on a DIFFERENT interface.
    // Must NOT rescue ctxA: ctxA must remain in deferred-teardown, a fresh
    // sibling for the new interface must be created.
    auto serviceB       = MakeCoalesceTestService();
    serviceB.mInterface = chip::Inet::InterfaceId(9);
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::ChipDnssdResolve(&serviceB, serviceB.mInterface, DummyResolveCallback, nullptr), CHIP_NO_ERROR);
        auto siblings = AllResolveSiblings(kCoalesceTestInstanceName);
        // Two distinct contexts: ctxA still in deferred-teardown, plus the
        // fresh one for interface 9.
        EXPECT_EQ(siblings.size(), 2u);
        bool sawCtxA = false;
        bool sawNew  = false;
        for (auto * s : siblings)
        {
            if (s == ctxA)
            {
                sawCtxA = true;
                EXPECT_TRUE(s->deferredTeardownScheduled);
                EXPECT_EQ(s->requestedInterfaceId, 7u);
            }
            else
            {
                sawNew = true;
                EXPECT_FALSE(s->deferredTeardownScheduled);
                EXPECT_EQ(s->requestedInterfaceId, 9u);
            }
        }
        EXPECT_TRUE(sawCtxA);
        EXPECT_TRUE(sawNew);
    });

    // Clean up: cancel the new resolve and let both teardowns fire.
    RunOnMatterEventLoopSync([&]() { chip::Dnssd::ChipDnssdResolveNoLongerNeeded(kCoalesceTestInstanceName); });
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    RunOnMatterEventLoopSync([&]() {
        EXPECT_EQ(chip::Dnssd::MdnsContexts::GetInstance().GetExistingResolveForInstanceName(kCoalesceTestInstanceName), nullptr);
    });
}

#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
