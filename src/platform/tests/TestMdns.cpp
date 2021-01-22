#include <condition_variable>
#include <mutex>
#include <thread>

#include <nlunit-test.h>

#include "lib/mdns/platform/Mdns.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CHIPMem.h"
#include "support/UnitTestRegistration.h"

using chip::Mdns::MdnsService;
using chip::Mdns::MdnsServiceProtocol;
using chip::Mdns::TextEntry;

static void HandleResolve(void * context, MdnsService * result, CHIP_ERROR error)
{
    char addrBuf[100];
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, result != nullptr);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    result->mAddress.Value().ToString(addrBuf, sizeof(addrBuf));
    printf("Service at [%s]:%u\n", addrBuf, result->mPort);
    NL_TEST_ASSERT(suite, result->mTextEntrySize == 1);
    NL_TEST_ASSERT(suite, strcmp(result->mTextEntryies[0].mKey, "key") == 0);
    NL_TEST_ASSERT(suite, strcmp(reinterpret_cast<const char *>(result->mTextEntryies[0].mData), "val") == 0);

    exit(0);
}

static void HandleBrowse(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error)
{
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    if (services)
    {
        printf("Mdns service size %zu\n", servicesSize);
        printf("Service name %s\n", services->mName);
        printf("Service type %s\n", services->mType);
        NL_TEST_ASSERT(suite, ChipMdnsResolve(services, INET_NULL_INTERFACEID, HandleResolve, suite) == CHIP_NO_ERROR);
    }
}

static void InitCallback(void * context, CHIP_ERROR error)
{
    MdnsService service;
    TextEntry entry;
    char key[]          = "key";
    char val[]          = "val";
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

    service.mInterface = INET_NULL_INTERFACEID;
    service.mPort      = 80;
    strcpy(service.mName, "test");
    strcpy(service.mType, "_mock");
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    entry.mKey             = key;
    entry.mData            = reinterpret_cast<const uint8_t *>(val);
    entry.mDataSize        = strlen(reinterpret_cast<const char *>(entry.mData));
    service.mTextEntryies  = &entry;
    service.mTextEntrySize = 1;
    service.mSubTypes      = nullptr;
    service.mSubTypeSize   = 0;

    NL_TEST_ASSERT(suite, ChipMdnsPublishService(&service) == CHIP_NO_ERROR);
    ChipMdnsBrowse("_mock", MdnsServiceProtocol::kMdnsProtocolTcp, chip::Inet::kIPAddressType_Any, INET_NULL_INTERFACEID,
                   HandleBrowse, suite);
}

static void ErrorCallback(void * context, CHIP_ERROR error)
{
    if (error != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Mdns error: %d\n", static_cast<int>(error));
        abort();
    }
}

void TestMdnsPubSub(nlTestSuite * inSuite, void * inContext)
{
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, chip::Mdns::ChipMdnsInit(InitCallback, ErrorCallback, inSuite) == CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

static const nlTest sTests[] = { NL_TEST_DEF("Test Mdns::PubSub", TestMdnsPubSub), NL_TEST_SENTINEL() };

int TestMdns()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable done;
    int retVal = EXIT_FAILURE;

    std::thread t([&mtx, &done, &retVal]() {
        {
            std::lock_guard<std::mutex> localLock(mtx);
            nlTestSuite theSuite = { "CHIP DeviceLayer mdns tests", &sTests[0], nullptr, nullptr };

            nlTestRunner(&theSuite, nullptr);
            retVal = nlTestRunnerStats(&theSuite);
        }
        done.notify_all();
    });

    if (done.wait_for(lock, std::chrono::seconds(5)) == std::cv_status::timeout)
    {
        fprintf(stderr, "mDNS test timeout, is avahi daemon running?");
        retVal = EXIT_FAILURE;
    }
    return retVal;
}

CHIP_REGISTER_TEST_SUITE(TestMdns);
