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
 *      This file implements unit tests for the CHIP Allocation pool.
 *
 */

#include "TestCore.h"

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <core/AllocationPool.h>

namespace {

void CheckBasicAllocation(nlTestSuite * inSuite, void * context)
{
    CHIP_ERROR err;
    chip::AllocationPool<int, 3> testPool;

    int * v1 = testPool.Allocate(1);

    NL_TEST_ASSERT(inSuite, v1 != nullptr);
    NL_TEST_ASSERT(inSuite, *v1 == 1);

    int * v2 = testPool.Allocate(2);

    NL_TEST_ASSERT(inSuite, v2 != nullptr);
    NL_TEST_ASSERT(inSuite, *v2 == 2);

    int * v3 = testPool.Allocate(3);

    NL_TEST_ASSERT(inSuite, v3 != nullptr);
    NL_TEST_ASSERT(inSuite, *v3 == 3);

    // 3 items allocated. Insufficient space
    NL_TEST_ASSERT(inSuite, testPool.Allocate() == nullptr);

    // Free an item and reallocate
    err = testPool.Free(v2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // cannot free twice
    err = testPool.Free(v2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    int * v2_copy = testPool.Allocate(100);
    NL_TEST_ASSERT(inSuite, v2_copy != nullptr);
    NL_TEST_ASSERT(inSuite, *v2_copy == 100);
    // memory is reused
    NL_TEST_ASSERT(inSuite, v2_copy == v2);

    // Free everything
    err = testPool.Free(v1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = testPool.Free(v2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = testPool.Free(v3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

class ConstructorDestructorTracked
{
public:
    ConstructorDestructorTracked(int index, const char * extra) : mIndex(index), mExtra(extra) { lastCreated = index; }
    ~ConstructorDestructorTracked() { lastDestroyed = mIndex; }

    static int GetLastCreated() { return lastCreated; }

    static int GetLastDestroyed() { return lastDestroyed; }

private:
    static int lastCreated;
    static int lastDestroyed;

    int mIndex;
    const char * mExtra;
};

int ConstructorDestructorTracked::lastCreated   = 0;
int ConstructorDestructorTracked::lastDestroyed = 0;

void ConstructorDestructorTest(nlTestSuite * inSuite, void * context)
{
    {
        chip::AllocationPool<ConstructorDestructorTracked, 2> pool;

        ConstructorDestructorTracked * a = pool.Allocate(123, "first object");
        NL_TEST_ASSERT(inSuite, a != nullptr);
        NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastCreated() == 123);

        ConstructorDestructorTracked * b = pool.Allocate(222, "second object");
        NL_TEST_ASSERT(inSuite, b != nullptr);
        NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastCreated() == 222);

        CHIP_ERROR err = pool.Free(a);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastDestroyed() == 123);

        a = pool.Allocate(321, "anothr object");
        NL_TEST_ASSERT(inSuite, a != nullptr);
        NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastCreated() == 321);

        err = pool.Free(b);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastDestroyed() == 222);
    }

    // Pool destructor clears everything. Only one object was active.
    NL_TEST_ASSERT(inSuite, ConstructorDestructorTracked::GetLastDestroyed() == 321);
}

} // namespace

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicAllocation",    CheckBasicAllocation),
    NL_TEST_DEF("BasicAllocation",    ConstructorDestructorTest),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestAllocationPool(void)
{
    nlTestSuite theSuite = { "chip-allocationpool", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, NULL /* context */);
    return (nlTestRunnerStats(&theSuite));
}
