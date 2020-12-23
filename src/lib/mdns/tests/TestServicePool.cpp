#include "lib/mdns/ServicePool.h"

#include <nlunit-test.h>
#include <string.h>

#include "lib/support/CHIPMem.h"
#include "lib/support/UnitTestRegistration.h"

using chip::Mdns::ServicePool;

bool operator==(chip::Mdns::TextEntry & lhs, chip::Mdns::TextEntry & rhs)
{
    if (strncmp(lhs.mKey, rhs.mKey, chip::Mdns::kMdnsTextMaxSize))
    {
        return false;
    }
    if (lhs.mDataSize != rhs.mDataSize)
    {
        return false;
    }
    if (memcmp(lhs.mData, rhs.mData, lhs.mDataSize))
    {
        return false;
    }
    return true;
}

void TestAddDeleteFind(nlTestSuite * suite, void * context)
{
    chip::Mdns::ServicePool pool;
    chip::Mdns::MdnsService testServices[6];
    const chip::Mdns::ServicePool::Entry * foundEntry;

    chip::Platform::MemoryInit();

    for (size_t i = 0; i < 6; i++)
    {
        testServices[i].mPort = 100 + i;
        testServices[i].mAddress.SetValue(chip::Inet::IPAddress::Any);
    }

    pool.AddService(96, 0, testServices[0]);
    pool.AddService(32, 0, testServices[1]);
    pool.AddService(1, 0, testServices[2]);
    pool.AddService(7, 0, testServices[3]);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(96, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 100);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(32, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 101);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0) == nullptr);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(1, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 102);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(7, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0) == nullptr);

    NL_TEST_ASSERT(suite, pool.RemoveService(32, 0) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(96, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 100);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0) == nullptr);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0) == nullptr);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(1, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 102);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(7, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0) == nullptr);

    NL_TEST_ASSERT(suite, pool.RemoveService(96, 0) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0) == nullptr);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0) == nullptr);
    NL_TEST_ASSERT(suite, pool.FindService(64, 0) == nullptr);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(1, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 102);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(7, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0) == nullptr);

    pool.AddService(64, 0, testServices[4]);
    pool.AddService(33, 0, testServices[5]);

    NL_TEST_ASSERT(suite, pool.FindService(96, 0) == nullptr);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0) == nullptr);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(64, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 104);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(1, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 102);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(33, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 105);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(7, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0) == nullptr);

    pool.ReHash();

    NL_TEST_ASSERT(suite, pool.FindService(96, 0) == nullptr);
    NL_TEST_ASSERT(suite, pool.FindService(32, 0) == nullptr);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(64, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 104);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(1, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 102);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(33, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 105);
    NL_TEST_ASSERT(suite, (foundEntry = pool.FindService(7, 0)) != nullptr);
    NL_TEST_ASSERT(suite, foundEntry->GetPort() == 103);
    NL_TEST_ASSERT(suite, pool.FindService(2, 0) == nullptr);

    pool.Clear();
}

void TestReplace(nlTestSuite * suite, void * context)
{
    chip::Mdns::ServicePool pool;
    chip::Mdns::MdnsService testService;

    pool.Clear();
    testService.mAddress.SetValue(chip::Inet::IPAddress::Any);
    testService.mPort          = 80;
    testService.mTextEntryies  = nullptr;
    testService.mTextEntrySize = 0;

    for (size_t i = 0; i < ServicePool::kServicePoolCapacity; i++)
    {
        pool.AddService(i + 1, 0, testService);
    }

    for (size_t i = 0; i < ServicePool::kServicePoolCapacity; i++)
    {
        NL_TEST_ASSERT(suite, pool.FindService(i + 1, 0) != nullptr);
    }

    pool.AddService(ServicePool::kServicePoolCapacity + 1, 0, testService);
    NL_TEST_ASSERT(suite, pool.FindService(1, 0) == nullptr);

    for (size_t i = 1; i < ServicePool::kServicePoolCapacity; i++)
    {
        NL_TEST_ASSERT(suite, pool.FindService(i + 1, 0) != nullptr);
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
