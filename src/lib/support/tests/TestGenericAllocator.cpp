/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for CHIP Memory Management
 *      code functionality.
 *
 */

#include <inttypes.h>

#include <lib/support/GenericAllocator.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Logging;

static void TestGenericAllocator_StaticDynamic(nlTestSuite * inSuite, void * inContext)
{
    GenericAllocator<int, 0> allocator_dynamic;
    GenericAllocator<int, 1> allocator_static;
    NL_TEST_ASSERT(inSuite, allocator_dynamic.IsDynamic());
    NL_TEST_ASSERT(inSuite, !allocator_static.IsDynamic());
}

template <typename T, size_t N>
bool TestCount(nlTestSuite * inSuite, GenericAllocator<T, N> & allocator, int nAllocated)
{
    int count = 0;
    for (auto & a : allocator)
    {
        count++;
        (void) a;
    }
    NL_TEST_ASSERT(inSuite, count == nAllocated);
    bool ranged_for_ok = count == nAllocated;
    if (!ranged_for_ok)
    {
        ChipLogError(Discovery, "Testing ranged for count - expected %d, got %d", nAllocated, count);
    }

    count = 0;
    for (auto it = allocator.begin(); it != allocator.end(); ++it)
    {
        count++;
    }
    NL_TEST_ASSERT(inSuite, count == nAllocated);
    bool iterator_ok = count == nAllocated;
    if (!iterator_ok)
    {
        ChipLogError(Discovery, "Testing ranged for count - expected %d, got %d", nAllocated, count);
    }
    return ranged_for_ok && iterator_ok;
}

template <typename T, size_t N>
void TestGenericAllocator(nlTestSuite * inSuite, const T * const testVals)
{
    // We need at least 4 values for this test.
    static_assert(N == 0 || N > 3, "TestGenericAllocator must be used with an allocator of at least size 3.");
    bool countOk;

    GenericAllocator<T, N> allocator;
    bool expectDynamic = N == 0;
    NL_TEST_ASSERT(inSuite, allocator.IsDynamic() == expectDynamic);
    constexpr int nAllocations = N > 0 ? N : 5;

    // First test that range based for loop and iterator access work with empty allocations
    ChipLogProgress(Discovery, "Testing empty on creation");
    TestCount<T, N>(inSuite, allocator, 0);

    // We should be able to allocate at least nAllocations. Test that the count is correct after each.
    ChipLogProgress(Discovery, "Testing allocations");
    for (int i = 0; i < nAllocations; ++i)
    {
        T * ptr = allocator.Allocate();
        NL_TEST_ASSERT(inSuite, ptr != nullptr);
        if (ptr == nullptr)
        {
            return;
        }
        *ptr    = testVals[i];
        countOk = TestCount<T, N>(inSuite, allocator, i + 1);
        NL_TEST_ASSERT(inSuite, countOk);
    }

    // Check that all the values are as we expect.
    int c = 0;
    for (auto & a : allocator)
    {
        NL_TEST_ASSERT(inSuite, a == testVals[c++]);
    }
    // Ensure nothing changed.
    countOk = TestCount<T, N>(inSuite, allocator, nAllocations);
    NL_TEST_ASSERT(inSuite, countOk);

    // Remove a value at the start, at the end and in the middle and
    // We know N is at least 4 because we checked in the assert above.
    ChipLogProgress(Discovery, "Testing free using iterator");
    int middleVal = nAllocations / 2;
    c             = 0;
    for (auto it = allocator.begin(); it != allocator.end();)
    {
        if (c == 0 || c == middleVal || c == (nAllocations - 1))
        {
            it = allocator.Free(it);
        }
        else
        {
            ++it;
        }
        c++;
    }
    countOk = TestCount<T, N>(inSuite, allocator, nAllocations - 3);
    NL_TEST_ASSERT(inSuite, countOk);

    // Assure the correct values were removed
    c = 0;
    for (auto & a : allocator)
    {
        if (c == 0 || c == middleVal || c == (nAllocations - 1))
        {
            c++;
        }
        NL_TEST_ASSERT(inSuite, a == testVals[c++]);
    }

    // Try adding back a value and assure we have the right count.
    allocator.Allocate();
    countOk = TestCount<T, N>(inSuite, allocator, nAllocations - 2);
    NL_TEST_ASSERT(inSuite, countOk);

    // Free everything and assure we have nothing.
    for (auto it = allocator.begin(); it != allocator.end();)
    {
        it = allocator.Free(it);
    }
    countOk = TestCount<T, N>(inSuite, allocator, 0);
    NL_TEST_ASSERT(inSuite, countOk);

    // Test Free using the data pointer
    ChipLogProgress(Discovery, "Testing free using pointer");
    T * allocations[nAllocations];
    for (int i = 0; i < nAllocations; ++i)
    {
        allocations[i] = allocator.Allocate();
        NL_TEST_ASSERT(inSuite, allocations[i] != nullptr);
        if (allocations[i] == nullptr)
        {
            return;
        }
        *allocations[i] = testVals[i];
        countOk         = TestCount<T, N>(inSuite, allocator, i + 1);
        NL_TEST_ASSERT(inSuite, countOk);
    }

    // Remove the same three values as before
    allocator.Free(allocations[0]);
    allocator.Free(allocations[middleVal]);
    allocator.Free(allocations[nAllocations - 1]);

    // Assure the correct values were removed
    c = 0;
    for (auto & a : allocator)
    {
        if (c == 0 || c == middleVal || c == (nAllocations - 1))
        {
            c++;
        }
        NL_TEST_ASSERT(inSuite, a == testVals[c++]);
    }

    // Try adding back a value and assure we have the right count.
    allocations[0] = allocator.Allocate();
    countOk        = TestCount<T, N>(inSuite, allocator, nAllocations - 2);
    NL_TEST_ASSERT(inSuite, countOk);

    // Free everything and assure we have nothing. This frees the same values we had before and should cause no problems with double
    // frees.
    for (int i = 0; i < nAllocations; i++)
    {
        allocator.Free(allocations[i]);
    }
    countOk = TestCount<T, N>(inSuite, allocator, 0);
    NL_TEST_ASSERT(inSuite, countOk);

    if (N != 0)
    {
        ChipLogProgress(Discovery, "Test overallocations");
        T * ptr;
        for (int i = 0; i < nAllocations; ++i)
        {
            ptr = allocator.Allocate();
            NL_TEST_ASSERT(inSuite, ptr != nullptr);
            countOk = TestCount<T, N>(inSuite, allocator, i + 1);
            NL_TEST_ASSERT(inSuite, countOk);
        }
        // Next one should fail.
        ptr = allocator.Allocate();
        NL_TEST_ASSERT(inSuite, ptr == nullptr);
    }
}

// Test values.
constexpr int intTestVals[5] = { 1, 2, 3, 4, 5 };

class TestClass
{
public:
    constexpr TestClass(int intVal, bool boolVal, uint32_t u32Val) : mIntVal(intVal), mBoolVal(boolVal), mU32Val(u32Val) {}
    TestClass() {}
    bool operator==(const TestClass & other) const
    {
        return mIntVal == other.mIntVal && other.mBoolVal == mBoolVal && mU32Val == other.mU32Val;
    }

public:
    int mIntVal      = 0;
    bool mBoolVal    = false;
    uint32_t mU32Val = 0;
};

constexpr TestClass classTestVals[5] = {
    { -11, true, 58 }, { -2, false, 78 }, { 0, false, 97 }, { 5, true, 10098 }, { 7, true, 98 }
};

void TestGenericAllocator_Dynamic(nlTestSuite * inSuite, void * inContext)
{
    TestGenericAllocator<int, 0>(inSuite, intTestVals);
    TestGenericAllocator<TestClass, 0>(inSuite, classTestVals);
}

void TestGenericAllocator_Static(nlTestSuite * inSuite, void * inContext)
{
    TestGenericAllocator<int, 5>(inSuite, intTestVals);
    TestGenericAllocator<TestClass, 5>(inSuite, classTestVals);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Test GenericAllocator static/dynamic", TestGenericAllocator_StaticDynamic), //
    NL_TEST_DEF("Test GenericAllocator::Allocate dynamic", TestGenericAllocator_Dynamic),    //
    NL_TEST_DEF("Test GenericAllocator::Allocate static", TestGenericAllocator_Static),      //
    NL_TEST_SENTINEL()                                                                       //
};

int TestGenericAllocator()
{
    nlTestSuite theSuite = { "CHIP Generic allocator", &sTests[0], nullptr, nullptr };
    chip::Platform::MemoryInit();
    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGenericAllocator)
