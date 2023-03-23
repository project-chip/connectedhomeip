/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/dnssd/Advertiser_ImplMinimalMdnsAllocator.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Dnssd;
using namespace mdns::Minimal;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
#include <dmalloc.h>
#endif // CHIP_CONFIG_MEMORY_DEBUG_DMALLOC

namespace {

constexpr size_t kMaxRecords = 10;
class TestAllocator : public QueryResponderAllocator<kMaxRecords>
{
public:
    TestAllocator()
    {
#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
        // void dmalloc_track(const dmalloc_track_t track_func)
#endif
    }
    void TestAllQNamesAreNull(nlTestSuite * inSuite)
    {
        for (size_t i = 0; i < GetMaxAllocatedQNames(); ++i)
        {
            NL_TEST_ASSERT(inSuite, GetQNamePart(i) == nullptr);
        }
    }
    void TestAllRecordRespondersAreNull(nlTestSuite * inSuite)
    {
        for (size_t i = 0; i < kMaxRecords; ++i)
        {
            NL_TEST_ASSERT(inSuite, GetRecordResponder(i) == nullptr);
        }
    }
    void TestRecordRespondersMatchQuery(nlTestSuite * inSuite)
    {
        mdns::Minimal::QueryResponderRecordFilter noFilter;
        auto queryResponder = GetQueryResponder();
        size_t idx          = 0;
        for (auto it = queryResponder->begin(&noFilter); it != queryResponder->end(); it++, idx++)
        {
            // TODO: Once the responders are exposed in the query responder, check that they match.
            NL_TEST_ASSERT(inSuite, idx < kMaxRecords);
        }
    }
    size_t GetMaxAllocatedQNames() { return QueryResponderAllocator<kMaxRecords>::GetMaxAllocatedQNames(); }
};

void TestQueryAllocatorQName(nlTestSuite * inSuite, void * inContext)
{
    TestAllocator test;
#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull(inSuite);
    test.TestAllQNamesAreNull(inSuite);

    // We should be able to add up to GetMaxAllocatedQNames QNames
    for (size_t i = 0; i < test.GetMaxAllocatedQNames(); ++i)
    {
        NL_TEST_ASSERT(inSuite, test.AllocateQName("test", "testy", "udp") != FullQName());
        test.TestAllRecordRespondersAreNull(inSuite);
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    NL_TEST_ASSERT(inSuite, nAllocated != 0);
#endif

    // Adding one more should fail.
    NL_TEST_ASSERT(inSuite, test.AllocateQName("test", "testy", "udp") == FullQName());
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    NL_TEST_ASSERT(inSuite, nAllocated == dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull(inSuite);
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    NL_TEST_ASSERT(inSuite, dmalloc_count_changed(mark, 1, 0) == 0);
#endif
}

void TestQueryAllocatorQNameArray(nlTestSuite * inSuite, void * inContext)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif

    constexpr size_t kNumParts     = 4;
    const char * kArray[kNumParts] = { "this", "is", "a", "test" };

    // Start empty.
    test.TestAllRecordRespondersAreNull(inSuite);
    test.TestAllQNamesAreNull(inSuite);

    // We should be able to add up to GetMaxAllocatedQNames QNames
    for (size_t i = 0; i < test.GetMaxAllocatedQNames(); ++i)
    {
        NL_TEST_ASSERT(inSuite, test.AllocateQNameFromArray(kArray, kNumParts) != FullQName());
        test.TestAllRecordRespondersAreNull(inSuite);
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    NL_TEST_ASSERT(inSuite, nAllocated != 0);
#endif

    // Adding one more should fail.
    NL_TEST_ASSERT(inSuite, test.AllocateQNameFromArray(kArray, kNumParts) == FullQName());
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    NL_TEST_ASSERT(inSuite, nAllocated == dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull(inSuite);
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    NL_TEST_ASSERT(inSuite, dmalloc_count_changed(mark, 1, 0) == 0);
#endif
}

void TestQueryAllocatorRecordResponder(nlTestSuite * inSuite, void * inContext)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull(inSuite);
    test.TestAllQNamesAreNull(inSuite);

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");

    for (size_t i = 0; i < kMaxRecords; ++i)
    {
        NL_TEST_ASSERT(inSuite, test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    NL_TEST_ASSERT(inSuite, nAllocated != 0);
#endif

    // Adding one more should fail.
    NL_TEST_ASSERT(inSuite, !test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    NL_TEST_ASSERT(inSuite, nAllocated == dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull(inSuite);
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    NL_TEST_ASSERT(inSuite, dmalloc_count_changed(mark, 1, 0) == 0);
#endif
}

void TestQueryAllocatorRecordResponderTypes(nlTestSuite * inSuite, void * inContext)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull(inSuite);
    test.TestAllQNamesAreNull(inSuite);

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");
    FullQName hostName     = test.AllocateQName("test", "host");
    FullQName someTxt      = test.AllocateQName("L1=some text", "L2=some other text");

    NL_TEST_ASSERT(inSuite, serviceName != FullQName());
    NL_TEST_ASSERT(inSuite, instanceName != FullQName());
    NL_TEST_ASSERT(inSuite, hostName != FullQName());
    NL_TEST_ASSERT(inSuite, someTxt != FullQName());

    // Test that we can add all types
    NL_TEST_ASSERT(inSuite, test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<SrvResponder>(SrvResourceRecord(instanceName, hostName, 57)).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<TxtResponder>(TxtResourceRecord(instanceName, someTxt)).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<IPv6Responder>(hostName).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<IPv4Responder>(hostName).IsValid());

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    NL_TEST_ASSERT(inSuite, nAllocated != 0);
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull(inSuite);
    test.TestAllRecordRespondersAreNull(inSuite);

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    NL_TEST_ASSERT(inSuite, dmalloc_count_changed(mark, 1, 0) == 0);
#endif
}

void TestGetResponder(nlTestSuite * inSuite, void * inContext)
{
    TestAllocator test;
    // Start empty.
    test.TestAllRecordRespondersAreNull(inSuite);
    test.TestAllQNamesAreNull(inSuite);

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");
    FullQName hostName     = test.AllocateQName("test", "host");
    FullQName someTxt      = test.AllocateQName("L1=some text", "L2=some other text");
    FullQName notAdded     = test.AllocateQName("not", "added");

    NL_TEST_ASSERT(inSuite, serviceName != FullQName());
    NL_TEST_ASSERT(inSuite, instanceName != FullQName());
    NL_TEST_ASSERT(inSuite, hostName != FullQName());
    NL_TEST_ASSERT(inSuite, someTxt != FullQName());

    NL_TEST_ASSERT(inSuite, test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<SrvResponder>(SrvResourceRecord(instanceName, hostName, 57)).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<TxtResponder>(TxtResourceRecord(instanceName, someTxt)).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<IPv6Responder>(hostName).IsValid());
    NL_TEST_ASSERT(inSuite, test.AddResponder<IPv4Responder>(hostName).IsValid());

    // These should all exist
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::PTR, serviceName) != nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::SRV, instanceName) != nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::TXT, instanceName) != nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::A, hostName) != nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::AAAA, hostName) != nullptr);

    // incorrect types
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::SRV, notAdded) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::AAAA, instanceName) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::A, instanceName) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::PTR, hostName) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::TXT, hostName) == nullptr);

    // incorrect names
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::PTR, notAdded) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::SRV, notAdded) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::TXT, notAdded) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::A, notAdded) == nullptr);
    NL_TEST_ASSERT(inSuite, test.GetResponder(QType::AAAA, notAdded) == nullptr);
    test.Clear();
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestQueryAllocatorQName", TestQueryAllocatorQName),                               //
    NL_TEST_DEF("TestQueryAllocatorQNameArray", TestQueryAllocatorQNameArray),                     //
    NL_TEST_DEF("TestQueryAllocatorRecordResponder", TestQueryAllocatorRecordResponder),           //
    NL_TEST_DEF("TestQueryAllocatorRecordResponderTypes", TestQueryAllocatorRecordResponderTypes), //
    NL_TEST_DEF("TestGetResponder", TestGetResponder),                                             //

    NL_TEST_SENTINEL() //
};

int TestSetup(void * inContext)
{
    return chip::Platform::MemoryInit() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

int TestMinimalMdnsAllocator()
{
    nlTestSuite theSuite = { "MinimalMdnsAllocator", &sTests[0], &TestSetup, &TestTeardown };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMinimalMdnsAllocator);
