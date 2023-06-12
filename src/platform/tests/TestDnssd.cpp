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

#include <nlunit-test.h>

#include "lib/dnssd/platform/Dnssd.h"
#include "platform/CHIPDeviceLayer.h"
#include "platform/ConnectivityManager.h"
#include "platform/PlatformManager.h"
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
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

using chip::Dnssd::DnssdService;
using chip::Dnssd::DnssdServiceProtocol;
using chip::Dnssd::TextEntry;

namespace {

struct DnssdContext
{
    nlTestSuite * mTestSuite;

    std::atomic<bool> mTimeoutExpired{ false };

    intptr_t mBrowseIdentifier = 0;

    unsigned int mBrowsedServicesCount  = 0;
    unsigned int mResolvedServicesCount = 0;
    bool mEndOfInput                    = false;
};

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
    void OnQuery(const mdns::Minimal::QueryData & data) override { mResponder->Respond(mMsgId, data, mCurSrc, mRespConfig); }
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data) override {}

private:
    mdns::Minimal::ResponseSender * mResponder;
    mdns::Minimal::ResponseConfiguration mRespConfig;
    const chip::Inet::IPPacketInfo * mCurSrc = nullptr;
    uint16_t mMsgId                          = 0;
};

} // namespace

static void Timeout(chip::System::Layer * systemLayer, void * context)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    ChipLogError(DeviceLayer, "mDNS test timeout, is avahi daemon running?");
    ctx->mTimeoutExpired = true;
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

static void HandleResolve(void * context, DnssdService * result, const chip::Span<chip::Inet::IPAddress> & addresses,
                          CHIP_ERROR error)
{
    auto * ctx   = static_cast<DnssdContext *>(context);
    auto * suite = ctx->mTestSuite;
    char addrBuf[100];

    NL_TEST_EXIT_ON_FAILED_ASSERT(suite, result != nullptr);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

    // The NL_TEST_ASSERT above will not abort the test, so we need to
    // explicitly abort it here to avoid dereferencing a null pointer.
    VerifyOrReturn(result != nullptr, );

    if (!addresses.empty())
    {
        addresses.data()[0].ToString(addrBuf, sizeof(addrBuf));
        printf("Service[%u] at [%s]:%u\n", ctx->mResolvedServicesCount, addrBuf, result->mPort);
    }

    NL_TEST_ASSERT(suite, result->mTextEntrySize == 1);
    NL_TEST_ASSERT(suite, strcmp(result->mTextEntries[0].mKey, "key") == 0);
    NL_TEST_ASSERT(suite, strcmp(reinterpret_cast<const char *>(result->mTextEntries[0].mData), "val") == 0);

    if (ctx->mBrowsedServicesCount == ++ctx->mResolvedServicesCount)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(Timeout, context);
        // StopEventLoopTask can be called from any thread, but when called from
        // non-Matter one it will lock the Matter stack. The same locking rules
        // are required when the resolve callback (this one) is called. In order
        // to avoid deadlocks, we need to call the StopEventLoopTask from inside
        // the Matter event loop by scheduling a lambda.
        chip::DeviceLayer::SystemLayer().ScheduleLambda([]() {
            // After last service is resolved, stop the event loop,
            // so the test case can gracefully exit.
            chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        });
    }
}

static void HandleBrowse(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse, CHIP_ERROR error)
{
    auto * ctx   = static_cast<DnssdContext *>(context);
    auto * suite = ctx->mTestSuite;

    // Make sure that we will not be called again after end-of-input is set
    NL_TEST_ASSERT(suite, ctx->mEndOfInput == false);
    // Cancelled error is expected when the browse is stopped with
    // ChipDnssdStopBrowse(), so we will not assert on it.
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR || error == CHIP_ERROR_CANCELLED);

    ctx->mBrowsedServicesCount += servicesSize;
    ctx->mEndOfInput = finalBrowse;

    if (servicesSize > 0)
    {
        printf("Browse mDNS service size %u\n", static_cast<unsigned int>(servicesSize));
        for (unsigned int i = 0; i < servicesSize; i++)
        {
            printf("Service[%u] name %s\n", i, services[i].mName);
            printf("Service[%u] type %s\n", i, services[i].mType);
            NL_TEST_ASSERT(suite, ChipDnssdResolve(&services[i], services[i].mInterface, HandleResolve, context) == CHIP_NO_ERROR);
        }
    }
}

static void DnssdErrorCallback(void * context, CHIP_ERROR error)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    NL_TEST_ASSERT(ctx->mTestSuite, error == CHIP_NO_ERROR);
}

void TestDnssdBrowse_DnssdInitCallback(void * context, CHIP_ERROR error)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    NL_TEST_ASSERT(ctx->mTestSuite, error == CHIP_NO_ERROR);
    auto * suite = ctx->mTestSuite;

    NL_TEST_ASSERT(suite,
                   ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolUdp, chip::Inet::IPAddressType::kAny,
                                   chip::Inet::InterfaceId::Null(), HandleBrowse, context,
                                   &ctx->mBrowseIdentifier) == CHIP_NO_ERROR);
}

// Verify that platform DNS-SD implementation can browse and resolve services.
//
// This test case uses platform-independent mDNS server implementation based on
// minimal mdns library. The server is configured to respond to PTR, SRV, TXT,
// A and AAAA queries without additional records. In order to pass this test,
// the platform DNS-SD client implementation must be able to browse and resolve
// services by querying for all of these records separately.
void TestDnssdBrowse(nlTestSuite * inSuite, void * inContext)
{
    DnssdContext context;
    context.mTestSuite = inSuite;

    mdns::Minimal::SetDefaultAddressPolicy();

    mdns::Minimal::Server<10> server;
    mdns::Minimal::QNamePart serverName[] = { "resolve-tester", "_mock", chip::Dnssd::kCommissionProtocol,
                                              chip::Dnssd::kLocalDomain };
    mdns::Minimal::ResponseSender responseSender(&server);

    mdns::Minimal::QueryResponder<16> queryResponder;
    responseSender.AddQueryResponder(&queryResponder);

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
    NL_TEST_ASSERT(inSuite, server.Listen(chip::DeviceLayer::UDPEndPointManager(), endpoints.get(), 5353) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   chip::Dnssd::ChipDnssdInit(TestDnssdBrowse_DnssdInitCallback, DnssdErrorCallback, &context) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(5), Timeout, &context) ==
                       CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");

    NL_TEST_ASSERT(inSuite, context.mResolvedServicesCount > 0);
    NL_TEST_ASSERT(inSuite, !context.mTimeoutExpired);

    // Stop browsing so we can safely shutdown DNS-SD
    chip::Dnssd::ChipDnssdStopBrowse(context.mBrowseIdentifier);

    chip::Dnssd::ChipDnssdShutdown();
}

static void HandlePublish(void * context, const char * type, const char * instanceName, CHIP_ERROR error)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    NL_TEST_ASSERT(ctx->mTestSuite, error == CHIP_NO_ERROR);
}

static void TestDnssdPublishService_DnssdInitCallback(void * context, CHIP_ERROR error)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    NL_TEST_ASSERT(ctx->mTestSuite, error == CHIP_NO_ERROR);
    auto * suite = ctx->mTestSuite;

    DnssdService service{};
    TextEntry entry{ "key", reinterpret_cast<const uint8_t *>("val"), 3 };

    service.mInterface = chip::Inet::InterfaceId::Null();
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

    NL_TEST_ASSERT(suite, ChipDnssdPublishService(&service, HandlePublish, context) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite,
                   ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolTcp, chip::Inet::IPAddressType::kAny,
                                   chip::Inet::InterfaceId::Null(), HandleBrowse, context,
                                   &ctx->mBrowseIdentifier) == CHIP_NO_ERROR);
}

// Verify that the platform DNS-SD implementation can publish services.
//
// This test uses platform implementation of DNS-SD server and client. Since
// client implementation should be verified by the TestDnssdBrowse test case,
// here we only verify that the server implementation can publish services.
void TestDnssdPublishService(nlTestSuite * inSuite, void * inContext)
{
    DnssdContext context;
    context.mTestSuite = inSuite;

    NL_TEST_ASSERT(inSuite,
                   chip::Dnssd::ChipDnssdInit(TestDnssdPublishService_DnssdInitCallback, DnssdErrorCallback, &context) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(5), Timeout, &context) ==
                       CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");

    NL_TEST_ASSERT(inSuite, context.mResolvedServicesCount > 0);
    NL_TEST_ASSERT(inSuite, !context.mTimeoutExpired);

    // Stop browsing so we can safely shutdown DNS-SD
    chip::Dnssd::ChipDnssdStopBrowse(context.mBrowseIdentifier);

    chip::Dnssd::ChipDnssdShutdown();
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test ChipDnssdBrowse", TestDnssdBrowse),
    NL_TEST_DEF("Test ChipDnssdPublishService", TestDnssdPublishService),
    NL_TEST_SENTINEL(),
};

int TestDnssd_Setup(void * inContext)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(chip::DeviceLayer::PlatformMgr().InitChipStack() == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int TestDnssd_Teardown(void * inContext)
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestDnssd()
{
    nlTestSuite theSuite = { "CHIP DeviceLayer mDNS tests", &sTests[0], TestDnssd_Setup, TestDnssd_Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDnssd);
