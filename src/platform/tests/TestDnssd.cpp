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

/**
 *    @file
 *      This file implements a unit test suite for the mDNS code functionality.
 *
 */

#include <condition_variable>
#include <mutex>
#include <thread>

#include <nlunit-test.h>

#include "lib/dnssd/platform/Dnssd.h"
#include "platform/CHIPDeviceLayer.h"
#include "platform/PlatformManager.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

using chip::Dnssd::DnssdService;
using chip::Dnssd::DnssdServiceProtocol;
using chip::Dnssd::TextEntry;

namespace {

struct DnssdContext
{
    nlTestSuite * mTestSuite;

    unsigned int mBrowsedServicesCount  = 0;
    unsigned int mResolvedServicesCount = 0;
    bool mEndOfInput                    = false;
};

} // namespace

static void HandleResolve(void * context, DnssdService * result, const chip::Span<chip::Inet::IPAddress> & addresses,
                          CHIP_ERROR error)
{
    auto * ctx   = static_cast<DnssdContext *>(context);
    auto * suite = ctx->mTestSuite;
    char addrBuf[100];

    NL_TEST_ASSERT(suite, result != nullptr);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

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
        // After last service is resolved, stop the event loop,
        // so the test case can gracefully exit.
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

static void HandleBrowse(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse, CHIP_ERROR error)
{
    auto * ctx   = static_cast<DnssdContext *>(context);
    auto * suite = ctx->mTestSuite;

    // Make sure that we will not be called again after end-of-input is set
    NL_TEST_ASSERT(suite, ctx->mEndOfInput == false);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

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

static void HandlePublish(void * context, const char * type, const char * instanceName, CHIP_ERROR error)
{
    auto * ctx = static_cast<DnssdContext *>(context);
    NL_TEST_ASSERT(ctx->mTestSuite, error == CHIP_NO_ERROR);
}

static void TestDnssdPubSub_DnssdInitCallback(void * context, CHIP_ERROR error)
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

    intptr_t browseIdentifier;
    NL_TEST_ASSERT(suite,
                   ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolTcp, chip::Inet::IPAddressType::kAny,
                                   chip::Inet::InterfaceId::Null(), HandleBrowse, context, &browseIdentifier) == CHIP_NO_ERROR);
}

void TestDnssdPubSub(nlTestSuite * inSuite, void * inContext)
{
    DnssdContext context;
    context.mTestSuite = inSuite;

    NL_TEST_ASSERT(inSuite,
                   chip::Dnssd::ChipDnssdInit(TestDnssdPubSub_DnssdInitCallback, DnssdErrorCallback, &context) == CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");

    chip::Dnssd::ChipDnssdShutdown();
}

static const nlTest sTests[] = { NL_TEST_DEF("Test Dnssd::PubSub", TestDnssdPubSub), NL_TEST_SENTINEL() };

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

    // Run test suit against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDnssd);
