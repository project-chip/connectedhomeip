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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Advertiser_ImplMinimalMdnsAllocator.h>

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
    void TestAllQNamesAreNull()
    {
        for (size_t i = 0; i < GetMaxAllocatedQNames(); ++i)
        {
            EXPECT_EQ(GetQNamePart(i), nullptr);
        }
    }
    void TestAllRecordRespondersAreNull()
    {
        for (size_t i = 0; i < kMaxRecords; ++i)
        {
            EXPECT_EQ(GetRecordResponder(i), nullptr);
        }
    }
    void TestRecordRespondersMatchQuery()
    {
        mdns::Minimal::QueryResponderRecordFilter noFilter;
        auto queryResponder = GetQueryResponder();
        size_t idx          = 0;
        for (auto it = queryResponder->begin(&noFilter); it != queryResponder->end(); it++, idx++)
        {
            // TODO: Once the responders are exposed in the query responder, check that they match.
            EXPECT_LT(idx, kMaxRecords);
        }
    }
    size_t GetMaxAllocatedQNames() { return QueryResponderAllocator<kMaxRecords>::GetMaxAllocatedQNames(); }
};

class TestMinimalMdnsAllocator : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestMinimalMdnsAllocator, TestQueryAllocatorQName)
{
    TestAllocator test;
#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull();
    test.TestAllQNamesAreNull();

    // We should be able to add up to GetMaxAllocatedQNames QNames
    for (size_t i = 0; i < test.GetMaxAllocatedQNames(); ++i)
    {
        EXPECT_NE(test.AllocateQName("test", "testy", "udp"), FullQName());
        test.TestAllRecordRespondersAreNull();
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    EXPECT_NE(nAllocated, 0);
#endif

    // Adding one more should fail.
    EXPECT_EQ(test.AllocateQName("test", "testy", "udp"), FullQName());
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    EXPECT_EQ(nAllocated, dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull();
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    EXPECT_EQ(dmalloc_count_changed(mark, 1, 0), 0);
#endif
}

TEST_F(TestMinimalMdnsAllocator, TestQueryAllocatorQNameArray)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif

    constexpr size_t kNumParts     = 4;
    const char * kArray[kNumParts] = { "this", "is", "a", "test" };

    // Start empty.
    test.TestAllRecordRespondersAreNull();
    test.TestAllQNamesAreNull();

    // We should be able to add up to GetMaxAllocatedQNames QNames
    for (size_t i = 0; i < test.GetMaxAllocatedQNames(); ++i)
    {
        EXPECT_NE(test.AllocateQNameFromArray(kArray, kNumParts), FullQName());
        test.TestAllRecordRespondersAreNull();
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    EXPECT_NE(nAllocated, 0);
#endif

    // Adding one more should fail.
    EXPECT_EQ(test.AllocateQNameFromArray(kArray, kNumParts), FullQName());
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    EXPECT_EQ(nAllocated, dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull();
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    EXPECT_EQ(dmalloc_count_changed(mark, 1, 0), 0);
#endif
}

TEST_F(TestMinimalMdnsAllocator, TestQueryAllocatorRecordResponder)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull();
    test.TestAllQNamesAreNull();

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");

    for (size_t i = 0; i < kMaxRecords; ++i)
    {
        EXPECT_TRUE(test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    }

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    EXPECT_NE(nAllocated, 0);
#endif

    // Adding one more should fail.
    EXPECT_FALSE(test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // We should not have allocated any more memory
    EXPECT_EQ(nAllocated, dmalloc_count_changed(mark, 1, 0));
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull();
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    EXPECT_EQ(dmalloc_count_changed(mark, 1, 0), 0);
#endif
}

TEST_F(TestMinimalMdnsAllocator, TestQueryAllocatorRecordResponderTypes)
{
    TestAllocator test;

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    unsigned long mark = dmalloc_mark();
#endif
    // Start empty.
    test.TestAllRecordRespondersAreNull();
    test.TestAllQNamesAreNull();

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");
    FullQName hostName     = test.AllocateQName("test", "host");
    FullQName someTxt      = test.AllocateQName("L1=some text", "L2=some other text");

    EXPECT_NE(serviceName, FullQName());
    EXPECT_NE(instanceName, FullQName());
    EXPECT_NE(hostName, FullQName());
    EXPECT_NE(someTxt, FullQName());

    // Test that we can add all types
    EXPECT_TRUE(test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    EXPECT_TRUE(test.AddResponder<SrvResponder>(SrvResourceRecord(instanceName, hostName, 57)).IsValid());
    EXPECT_TRUE(test.AddResponder<TxtResponder>(TxtResourceRecord(instanceName, someTxt)).IsValid());
    EXPECT_TRUE(test.AddResponder<IPv6Responder>(hostName).IsValid());
    EXPECT_TRUE(test.AddResponder<IPv4Responder>(hostName).IsValid());

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // Count the memory that has not been freed at this point (since mark)
    unsigned long nAllocated = dmalloc_count_changed(mark, 1, 0);
    EXPECT_NE(nAllocated, 0);
#endif

    // Clear should take us back to all empty.
    test.Clear();
    test.TestAllQNamesAreNull();
    test.TestAllRecordRespondersAreNull();

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
    // The amount of unfreed pointers should be 0.
    EXPECT_EQ(dmalloc_count_changed(mark, 1, 0), 0);
#endif
}

TEST_F(TestMinimalMdnsAllocator, TestGetResponder)
{
    TestAllocator test;
    // Start empty.
    test.TestAllRecordRespondersAreNull();
    test.TestAllQNamesAreNull();

    FullQName serviceName  = test.AllocateQName("test", "service");
    FullQName instanceName = test.AllocateQName("test", "instance");
    FullQName hostName     = test.AllocateQName("test", "host");
    FullQName someTxt      = test.AllocateQName("L1=some text", "L2=some other text");
    FullQName notAdded     = test.AllocateQName("not", "added");

    EXPECT_NE(serviceName, FullQName());
    EXPECT_NE(instanceName, FullQName());
    EXPECT_NE(hostName, FullQName());
    EXPECT_NE(someTxt, FullQName());

    EXPECT_TRUE(test.AddResponder<PtrResponder>(serviceName, instanceName).IsValid());
    EXPECT_TRUE(test.AddResponder<SrvResponder>(SrvResourceRecord(instanceName, hostName, 57)).IsValid());
    EXPECT_TRUE(test.AddResponder<TxtResponder>(TxtResourceRecord(instanceName, someTxt)).IsValid());
    EXPECT_TRUE(test.AddResponder<IPv6Responder>(hostName).IsValid());
    EXPECT_TRUE(test.AddResponder<IPv4Responder>(hostName).IsValid());

    // These should all exist
    ASSERT_NE(test.GetResponder(QType::PTR, serviceName), nullptr);
    ASSERT_NE(test.GetResponder(QType::SRV, instanceName), nullptr);
    ASSERT_NE(test.GetResponder(QType::TXT, instanceName), nullptr);
    ASSERT_NE(test.GetResponder(QType::A, hostName), nullptr);
    ASSERT_NE(test.GetResponder(QType::AAAA, hostName), nullptr);

    // incorrect types
    EXPECT_EQ(test.GetResponder(QType::SRV, notAdded), nullptr);
    EXPECT_EQ(test.GetResponder(QType::AAAA, instanceName), nullptr);
    EXPECT_EQ(test.GetResponder(QType::A, instanceName), nullptr);
    EXPECT_EQ(test.GetResponder(QType::PTR, hostName), nullptr);
    EXPECT_EQ(test.GetResponder(QType::TXT, hostName), nullptr);

    // incorrect names
    EXPECT_EQ(test.GetResponder(QType::PTR, notAdded), nullptr);
    EXPECT_EQ(test.GetResponder(QType::SRV, notAdded), nullptr);
    EXPECT_EQ(test.GetResponder(QType::TXT, notAdded), nullptr);
    EXPECT_EQ(test.GetResponder(QType::A, notAdded), nullptr);
    EXPECT_EQ(test.GetResponder(QType::AAAA, notAdded), nullptr);
    test.Clear();
}

} // namespace

;
