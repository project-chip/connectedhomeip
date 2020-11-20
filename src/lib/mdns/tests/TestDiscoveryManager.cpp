#include <condition_variable>
#include <mutex>
#include <thread>

#include <nlunit-test.h>

#include "lib/mdns/DiscoveryManager.h"
#include "lib/support/CHIPMem.h"
#include "platform/CHIPDeviceLayer.h"

namespace {
constexpr uint64_t kTestNodeId = 12344321;
}

class TestResolveDelegate : public chip::Mdns::ResolveDelegate
{
public:
    TestResolveDelegate(nlTestSuite * suite) : mSuite(suite), mCount(0) {}

    void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const chip::Mdns::MdnsService & address) override
    {
        NL_TEST_ASSERT(mSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(mSuite, nodeId == kTestNodeId);
        NL_TEST_ASSERT(mSuite, address.mAddress.HasValue());
        if (mCount == 0)
        {
            mCount++;
            chip::Mdns::DiscoveryManager::GetInstance().ResolveNodeId(kTestNodeId, 0);
        }
        else
        {
            // Exit the event loop to notify the test has passed
            // If this is not triggered this will mean a resolution failure
            exit(0);
        }
    }

    ~TestResolveDelegate() override {}

private:
    nlTestSuite * mSuite;
    int mCount = 0;
};

void TestDiscoveryManagerTask(nlTestSuite * suite, void * context)
{
    TestResolveDelegate testDelegate(suite);

    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::Mdns::DiscoveryManager::GetInstance().Init();
    chip::Mdns::DiscoveryManager::GetInstance().RegisterResolveDelegate(&testDelegate);
    chip::Mdns::DiscoveryManager::GetInstance().ResolveNodeId(kTestNodeId, 0);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

static const nlTest sTests[] = { NL_TEST_DEF("TestDiscoveryManager", TestDiscoveryManagerTask), NL_TEST_SENTINEL() };

int main()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable done;
    int retVal = EXIT_FAILURE;

    std::thread t([&mtx, &done, &retVal]() {
        {
            std::lock_guard<std::mutex> localLock(mtx);
            nlTestSuite theSuite = { "CHIP discovery manager", &sTests[0], nullptr, nullptr };

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
