/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Unit tests for the Chip Pool API.
 *
 */

#include <set>

#include <lib/support/Pool.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace chip {

template <class T, size_t N>
size_t GetNumObjectsInUse(BitMapObjectPool<T, N> & pool)
{
    size_t count = 0;
    pool.ForEachActiveObject([&count](void *) {
        ++count;
        return true;
    });
    return count;
}

} // namespace chip

namespace {

using namespace chip;

void TestReleaseNull(nlTestSuite * inSuite, void * inContext)
{
    constexpr const size_t size = 10;
    BitMapObjectPool<uint32_t, size> pool;
    pool.ReleaseObject(nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == 0);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == 0);
}

void TestCreateReleaseObject(nlTestSuite * inSuite, void * inContext)
{
    constexpr const size_t size = 100;
    BitMapObjectPool<uint32_t, size> pool;
    uint32_t * obj[size];
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        obj[i] = pool.CreateObject();
        NL_TEST_ASSERT(inSuite, obj[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == i + 1);
    }

    uint32_t * fail = pool.CreateObject();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == size);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    pool.ReleaseObject(obj[55]);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size - 1);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == size - 1);
    NL_TEST_ASSERT(inSuite, !pool.Exhausted());
    NL_TEST_ASSERT(inSuite, obj[55] == pool.CreateObject());
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == size);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    fail = pool.CreateObject();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == size);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    for (size_t i = 0; i < pool.Size(); ++i)
    {
        pool.ReleaseObject(obj[i]);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size - i - 1);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == size - i - 1);
    }
}

void TestCreateReleaseStruct(nlTestSuite * inSuite, void * inContext)
{
    struct S
    {
        S(std::set<S *> & set) : mSet(set) { mSet.insert(this); }
        ~S() { mSet.erase(this); }
        std::set<S *> & mSet;
    };

    std::set<S *> objs1;

    constexpr const size_t size = 100;
    BitMapObjectPool<S, size> pool;
    S * objs2[size];
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        objs2[i] = pool.CreateObject(objs1);
        NL_TEST_ASSERT(inSuite, objs2[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == objs1.size());
    }
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        pool.ReleaseObject(objs2[i]);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size - i - 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == objs1.size());
    }
}

int Setup(void * inContext)
{
    return SUCCESS;
}

int Teardown(void * inContext)
{
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestReleaseNull), NL_TEST_DEF_FN(TestCreateReleaseObject),
                                 NL_TEST_DEF_FN(TestCreateReleaseStruct), NL_TEST_SENTINEL() };

int TestPool()
{
    nlTestSuite theSuite = { "CHIP Pool tests", &sTests[0], Setup, Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPool);
