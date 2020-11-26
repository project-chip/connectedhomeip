#include "lib/mdns/ServicePool.h"

#include <nlunit-test.h>

#include "lib/support/CHIPMem.h"
#include "lib/support/TestUtils.h"

using chip::Mdns::ServicePool;

void TestAddDeleteFind(nlTestSuite * suite, void * context)
{

    chip::Mdns::ServicePool pool;
    chip::Mdns::MdnsService testServices[6];
    uint8_t testData[3]                    = { 0, 1, 2 };
    chip::Mdns::TextEntry testTestEntry[2] = {
        { "test0", testData, sizeof(testData) },
        { "test1", testData, sizeof(testData) },
    };
    chip::Mdns::MdnsService * foundService;

    for (size_t i = 0; i < 6; i++)
    {
        testServices[i].mPort          = 100 + i;
        testServices[i].mTextEntrySize = 2;
        testServices[i].mTextEntryies  = testTestEntry;
    }

    chip::Platform::MemoryInit();

    NL_TEST_ASSERT(suite, pool.AddService(96, 0, testServices[0]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, pool.AddService(32, 0, testServices[1]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, pool.AddService(1, 0, testServices[2]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, pool.AddService(7, 0, testServices[3]) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 100);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 101);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 102);
    NL_TEST_ASSERT(suite, pool.FindService(7, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0, &foundService) == false);

    NL_TEST_ASSERT(suite, pool.RemoveService(32, 0) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 100);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 102);
    NL_TEST_ASSERT(suite, pool.FindService(7, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0, &foundService) == false);

    NL_TEST_ASSERT(suite, pool.RemoveService(96, 0) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 102);
    NL_TEST_ASSERT(suite, pool.FindService(7, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0, &foundService) == false);

    NL_TEST_ASSERT(suite, pool.AddService(64, 0, testServices[4]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, pool.AddService(33, 0, testServices[5]) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 104);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 102);
    NL_TEST_ASSERT(suite, pool.FindService(33, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 105);
    NL_TEST_ASSERT(suite, pool.FindService(7, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0, &foundService) == false);

    pool.ReHash();

    NL_TEST_ASSERT(suite, pool.FindService(96, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0, &foundService) == false);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 104);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 102);
    NL_TEST_ASSERT(suite, pool.FindService(33, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 105);
    NL_TEST_ASSERT(suite, pool.FindService(7, 0, &foundService) == true);
    NL_TEST_ASSERT(suite, foundService->mPort == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0, &foundService) == false);
}

void TestReplace(nlTestSuite * suite, void * context)
{
    chip::Mdns::ServicePool pool;
    chip::Mdns::MdnsService testService;
    chip::Mdns::MdnsService * foundService;

    pool.Clear();

    for (size_t i = 0; i < ServicePool::kServicePoolCapacity; i++)
    {
        NL_TEST_ASSERT(suite, pool.AddService(i + 1, 0, testService) == CHIP_NO_ERROR);
    }

    for (size_t i = 0; i < ServicePool::kServicePoolCapacity; i++)
    {
        NL_TEST_ASSERT(suite, pool.FindService(i + 1, 0, &foundService) == true);
    }

    NL_TEST_ASSERT(suite, pool.AddService(ServicePool::kServicePoolCapacity + 1, 0, testService) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0, &foundService) == false);

    for (size_t i = 1; i < ServicePool::kServicePoolCapacity; i++)
    {
        NL_TEST_ASSERT(suite, pool.FindService(i + 1, 0, &foundService) == true);
    }
}

// clang-format off
static const nlTest sTests[] = {
  NL_TEST_DEF("TestAddDeleteFind", TestAddDeleteFind),
  NL_TEST_DEF("TestReplace", TestReplace),
  NL_TEST_SENTINEL()
};
// clang-format on
int TestServicePool()
{
    nlTestSuite theSuite = { "ServicePool", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestServicePool)
