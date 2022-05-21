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

static void HandleResolve(void * context, DnssdService * result, const chip::Span<chip::Inet::IPAddress> & addresses,
                          CHIP_ERROR error)
{
    char addrBuf[100];
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, result != nullptr);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    if (!addresses.empty())
    {
        addresses.data()[0].ToString(addrBuf, sizeof(addrBuf));
        printf("Service at [%s]:%u\n", addrBuf, result->mPort);
    }
    NL_TEST_ASSERT(suite, result->mTextEntrySize == 1);
    NL_TEST_ASSERT(suite, strcmp(result->mTextEntries[0].mKey, "key") == 0);
    NL_TEST_ASSERT(suite, strcmp(reinterpret_cast<const char *>(result->mTextEntries[0].mData), "val") == 0);

    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    chip::DeviceLayer::PlatformMgr().Shutdown();
    exit(0);
}

static void HandleBrowse(void * context, DnssdService * services, size_t servicesSize, CHIP_ERROR error)
{
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    if (services)
    {
        printf("Mdns service size %u\n", static_cast<unsigned int>(servicesSize));
        printf("Service name %s\n", services->mName);
        printf("Service type %s\n", services->mType);
        NL_TEST_ASSERT(suite, ChipDnssdResolve(services, chip::Inet::InterfaceId::Null(), HandleResolve, suite) == CHIP_NO_ERROR);
    }
}

static void HandlePublish(void * context, const char * type, CHIP_ERROR error) {}

static void InitCallback(void * context, CHIP_ERROR error)
{
    DnssdService service;
    TextEntry entry;
    char key[]          = "key";
    char val[]          = "val";
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

    service.mInterface = chip::Inet::InterfaceId::Null();
    service.mPort      = 80;
    strcpy(service.mName, "test");
    strcpy(service.mType, "_mock");
    service.mAddressType   = chip::Inet::IPAddressType::kAny;
    service.mProtocol      = DnssdServiceProtocol::kDnssdProtocolTcp;
    entry.mKey             = key;
    entry.mData            = reinterpret_cast<const uint8_t *>(val);
    entry.mDataSize        = strlen(reinterpret_cast<const char *>(entry.mData));
    service.mTextEntries   = &entry;
    service.mTextEntrySize = 1;
    service.mSubTypes      = nullptr;
    service.mSubTypeSize   = 0;

    NL_TEST_ASSERT(suite, ChipDnssdPublishService(&service, HandlePublish) == CHIP_NO_ERROR);
    ChipDnssdBrowse("_mock", DnssdServiceProtocol::kDnssdProtocolTcp, chip::Inet::IPAddressType::kAny,
                    chip::Inet::InterfaceId::Null(), HandleBrowse, suite);
}

static void ErrorCallback(void * context, CHIP_ERROR error)
{
    VerifyOrDieWithMsg(error == CHIP_NO_ERROR, DeviceLayer, "Mdns error: %" CHIP_ERROR_FORMAT "\n", error.Format());
}

void TestDnssdPubSub(nlTestSuite * inSuite, void * inContext)
{
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, chip::Dnssd::ChipDnssdInit(InitCallback, ErrorCallback, inSuite) == CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "End EventLoop");
}

static const nlTest sTests[] = { NL_TEST_DEF("Test Dnssd::PubSub", TestDnssdPubSub), NL_TEST_SENTINEL() };

int TestDnssd()
{
    std::mutex mtx;

    std::condition_variable readyCondition;
    bool ready = false;

    std::condition_variable doneCondition;
    bool done     = false;
    bool shutdown = false;

    int retVal = EXIT_FAILURE;

    std::thread t([&]() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
            readyCondition.notify_one();
        }

        nlTestSuite theSuite = { "CHIP DeviceLayer mdns tests", &sTests[0], nullptr, nullptr };

        nlTestRunner(&theSuite, nullptr);
        retVal = nlTestRunnerStats(&theSuite);

        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
            doneCondition.notify_all();
        }
    });

    {
        std::unique_lock<std::mutex> lock(mtx);
        readyCondition.wait(lock, [&] { return ready; });

        doneCondition.wait_for(lock, std::chrono::seconds(5));
        if (!done)
        {
            fprintf(stderr, "mDNS test timeout, is avahi daemon running?\n");

            //
            // This will stop the event loop above, and wait till it has actually stopped
            // (i.e exited RunEventLoop()).
            //
            chip::Dnssd::ChipDnssdShutdown();
            chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
            chip::DeviceLayer::PlatformMgr().Shutdown();
            shutdown = true;

            doneCondition.wait_for(lock, std::chrono::seconds(1));
            if (!done)
            {
                fprintf(stderr, "Orderly shutdown of the platform main loop failed as well.\n");
            }
            retVal = EXIT_FAILURE;
        }
    }
    t.join();

    if (!shutdown)
    {
        chip::Dnssd::ChipDnssdShutdown();
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }
    chip::Platform::MemoryShutdown();

    return retVal;
}

CHIP_REGISTER_TEST_SUITE(TestDnssd);
