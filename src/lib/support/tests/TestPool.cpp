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

#include "TestSupport.h"

#include <set>

#include <support/Pool.h>
#include <support/TestUtils.h>

#include <nlunit-test.h>

namespace chip {

template <class T, size_t N>
template <typename F>
void BitMapObjectPool<T, N>::ForEachActiveObject(F f)
{
    for (size_t word = 0; word * kAtomicSize < N; ++word)
    {
        auto & usage   = mUsage[word];
        uint32_t value = usage.load();
        for (size_t offset = 0; offset < kAtomicSize && offset + word * kAtomicSize < N; ++offset)
        {
            if ((value & (1 << offset)) != 0)
            {
                f(GetPoolHead() + (word * kAtomicSize + offset));
            }
        }
    }
}

template <class T, size_t N>
size_t GetNumObjectsInUse(BitMapObjectPool<T, N> & pool)
{
    size_t count = 0;
    pool.ForEachActiveObject([&count](T *) { ++count; });
    return count;
}

} // namespace chip

namespace {

using namespace chip;

void TestFreeNull(nlTestSuite * inSuite, void * inContext)
{
    constexpr const size_t size = 10;
    BitMapObjectPool<uint32_t, size> pool;
    pool.Delete(nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == 0);
}

void TestNewFree(nlTestSuite * inSuite, void * inContext)
{
    constexpr const size_t size = 100;
    BitMapObjectPool<uint32_t, size> pool;
    uint32_t * obj[size];
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        obj[i] = pool.New();
        NL_TEST_ASSERT(inSuite, obj[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
    }

    uint32_t * fail = pool.New();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);

    pool.Delete(obj[55]);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size - 1);
    NL_TEST_ASSERT(inSuite, obj[55] == pool.New());
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);

    fail = pool.New();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size);

    for (size_t i = 0; i < pool.Size(); ++i)
    {
        pool.Delete(obj[i]);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == size - i - 1);
    }
}

void TestNewFreeStruct(nlTestSuite * inSuite, void * inContext)
{
    struct S;

    std::set<S *> objs1;

    struct S
    {
        S(std::set<S *> & set) : set(set) { set.insert(this); }
        ~S() { set.erase(this); }
        std::set<S *> & set;
    };

    constexpr const size_t size = 100;
    BitMapObjectPool<S, size> pool;
    S * objs2[size];
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        objs2[i] = pool.New(objs1);
        NL_TEST_ASSERT(inSuite, objs2[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == objs1.size());
    }
    for (size_t i = 0; i < pool.Size(); ++i)
    {
        pool.Delete(objs2[i]);
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
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestFreeNull), NL_TEST_DEF_FN(TestNewFree), NL_TEST_DEF_FN(TestNewFreeStruct),
                                 NL_TEST_SENTINEL() };

extern "C" int TestPool()
{
    nlTestSuite theSuite = { "CHIP Pool tests", &sTests[0], Setup, Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}
