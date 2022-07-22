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
#include <lib/support/PoolWrapper.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/SystemConfig.h>

#include <nlunit-test.h>

namespace chip {

template <class POOL>
size_t GetNumObjectsInUse(const POOL & pool)
{
    size_t count = 0;
    pool.ForEachActiveObject([&count](const void *) {
        ++count;
        return Loop::Continue;
    });
    return count;
}

} // namespace chip

namespace {

using namespace chip;

template <typename T, size_t N, ObjectPoolMem P>
void TestReleaseNull(nlTestSuite * inSuite, void * inContext)
{
    ObjectPool<T, N, P> pool;
    pool.ReleaseObject(nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == 0);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == 0);
}

void TestReleaseNullStatic(nlTestSuite * inSuite, void * inContext)
{
    TestReleaseNull<uint32_t, 10, ObjectPoolMem::kInline>(inSuite, inContext);
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestReleaseNullDynamic(nlTestSuite * inSuite, void * inContext)
{
    TestReleaseNull<uint32_t, 10, ObjectPoolMem::kHeap>(inSuite, inContext);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <typename T, size_t N, ObjectPoolMem P>
void TestCreateReleaseObject(nlTestSuite * inSuite, void * inContext)
{
    ObjectPool<uint32_t, N, ObjectPoolMem::kInline> pool;
    uint32_t * obj[N];

    NL_TEST_ASSERT(inSuite, pool.Allocated() == 0);
    for (int t = 0; t < 2; ++t)
    {
        pool.ReleaseAll();
        NL_TEST_ASSERT(inSuite, pool.Allocated() == 0);

        for (size_t i = 0; i < N; ++i)
        {
            obj[i] = pool.CreateObject();
            NL_TEST_ASSERT(inSuite, obj[i] != nullptr);
            NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
            NL_TEST_ASSERT(inSuite, pool.Allocated() == i + 1);
        }
    }

    for (size_t i = 0; i < N; ++i)
    {
        pool.ReleaseObject(obj[i]);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == N - i - 1);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == N - i - 1);
    }
}

void TestCreateReleaseObjectStatic(nlTestSuite * inSuite, void * inContext)
{
    constexpr const size_t kSize = 100;
    TestCreateReleaseObject<uint32_t, kSize, ObjectPoolMem::kInline>(inSuite, inContext);

    ObjectPool<uint32_t, kSize, ObjectPoolMem::kInline> pool;
    uint32_t * obj[kSize];

    for (size_t i = 0; i < kSize; ++i)
    {
        obj[i] = pool.CreateObject();
        NL_TEST_ASSERT(inSuite, obj[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == i + 1);
    }

    uint32_t * fail = pool.CreateObject();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    pool.ReleaseObject(obj[55]);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize - 1);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize - 1);
    NL_TEST_ASSERT(inSuite, !pool.Exhausted());
    NL_TEST_ASSERT(inSuite, obj[55] == pool.CreateObject());
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    fail = pool.CreateObject();
    NL_TEST_ASSERT(inSuite, fail == nullptr);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize);
    NL_TEST_ASSERT(inSuite, pool.Exhausted());

    pool.ReleaseAll();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestCreateReleaseObjectDynamic(nlTestSuite * inSuite, void * inContext)
{
    TestCreateReleaseObject<uint32_t, 100, ObjectPoolMem::kHeap>(inSuite, inContext);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestCreateReleaseStruct(nlTestSuite * inSuite, void * inContext)
{
    struct S
    {
        S(std::set<S *> & set) : mSet(set) { mSet.insert(this); }
        ~S() { mSet.erase(this); }
        std::set<S *> & mSet;
    };
    std::set<S *> objs1;

    constexpr const size_t kSize = 100;
    ObjectPool<S, kSize, P> pool;

    S * objs2[kSize];
    for (size_t i = 0; i < kSize; ++i)
    {
        objs2[i] = pool.CreateObject(objs1);
        NL_TEST_ASSERT(inSuite, objs2[i] != nullptr);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == i + 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == i + 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == objs1.size());
    }
    for (size_t i = 0; i < kSize; ++i)
    {
        pool.ReleaseObject(objs2[i]);
        NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize - i - 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize - i - 1);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == objs1.size());
    }

    // Verify that ReleaseAll() calls the destructors.
    for (size_t i = 0; i < kSize; ++i)
    {
        objs2[i] = pool.CreateObject(objs1);
    }
    NL_TEST_ASSERT(inSuite, objs1.size() == kSize);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == kSize);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == kSize);
    printf("allocated = %u\n", static_cast<unsigned int>(pool.Allocated()));
    printf("highwater = %u\n", static_cast<unsigned int>(pool.HighWaterMark()));

    pool.ReleaseAll();
    printf("allocated = %u\n", static_cast<unsigned int>(pool.Allocated()));
    printf("highwater = %u\n", static_cast<unsigned int>(pool.HighWaterMark()));
    NL_TEST_ASSERT(inSuite, objs1.size() == 0);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(pool) == 0);
    NL_TEST_ASSERT(inSuite, pool.Allocated() == 0);
    NL_TEST_ASSERT(inSuite, pool.HighWaterMark() == kSize);
}

void TestCreateReleaseStructStatic(nlTestSuite * inSuite, void * inContext)
{
    TestCreateReleaseStruct<ObjectPoolMem::kInline>(inSuite, inContext);
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestCreateReleaseStructDynamic(nlTestSuite * inSuite, void * inContext)
{
    TestCreateReleaseStruct<ObjectPoolMem::kHeap>(inSuite, inContext);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestForEachActiveObject(nlTestSuite * inSuite, void * inContext)
{
    struct S
    {
        S(size_t id) : mId(id) {}
        size_t mId;
    };

    constexpr size_t kSize = 50;
    S * objArray[kSize];
    std::set<size_t> objIds;

    ObjectPool<S, kSize, P> pool;

    for (size_t i = 0; i < kSize; ++i)
    {
        objArray[i] = pool.CreateObject(i);
        NL_TEST_ASSERT(inSuite, objArray[i] != nullptr);
        NL_TEST_ASSERT(inSuite, objArray[i]->mId == i);
        objIds.insert(i);
    }

    // Verify that iteration visits all objects.
    size_t count = 0;
    size_t sum   = 0;
    pool.ForEachActiveObject([&](S * object) {
        NL_TEST_ASSERT(inSuite, object != nullptr);
        NL_TEST_ASSERT(inSuite, objIds.count(object->mId) == 1);
        objIds.erase(object->mId);
        ++count;
        sum += object->mId;
        return Loop::Continue;
    });
    NL_TEST_ASSERT(inSuite, count == kSize);
    NL_TEST_ASSERT(inSuite, sum == kSize * (kSize - 1) / 2);
    NL_TEST_ASSERT(inSuite, objIds.size() == 0);

    // Verify that returning Loop::Break stops iterating.
    count = 0;
    pool.ForEachActiveObject([&](S * object) {
        objIds.insert(object->mId);
        return ++count != kSize / 2 ? Loop::Continue : Loop::Break;
    });
    NL_TEST_ASSERT(inSuite, count == kSize / 2);
    NL_TEST_ASSERT(inSuite, objIds.size() == kSize / 2);

    // Verify that iteration can be nested.
    count = 0;
    pool.ForEachActiveObject([&](S * outer) {
        if (objIds.count(outer->mId) == 1)
        {
            pool.ForEachActiveObject([&](S * inner) {
                if (inner == outer)
                {
                    objIds.erase(inner->mId);
                }
                else
                {
                    ++count;
                }
                return Loop::Continue;
            });
        }
        return Loop::Continue;
    });
    NL_TEST_ASSERT(inSuite, count == (kSize - 1) * kSize / 2);
    NL_TEST_ASSERT(inSuite, objIds.size() == 0);

    count = 0;
    pool.ForEachActiveObject([&](S * object) {
        ++count;
        if ((object->mId % 2) == 0)
        {
            objArray[object->mId] = nullptr;
            pool.ReleaseObject(object);
        }
        else
        {
            objIds.insert(object->mId);
        }
        return Loop::Continue;
    });
    NL_TEST_ASSERT(inSuite, count == kSize);
    NL_TEST_ASSERT(inSuite, objIds.size() == kSize / 2);
    for (size_t i = 0; i < kSize; ++i)
    {
        if ((i % 2) == 0)
        {
            NL_TEST_ASSERT(inSuite, objArray[i] == nullptr);
        }
        else
        {
            NL_TEST_ASSERT(inSuite, objArray[i] != nullptr);
            NL_TEST_ASSERT(inSuite, objArray[i]->mId == i);
        }
    }

    count = 0;
    pool.ForEachActiveObject([&](S * object) {
        ++count;
        if ((object->mId % 2) == 1)
        {
            size_t id = object->mId - 1;
            NL_TEST_ASSERT(inSuite, objArray[id] == nullptr);
            objArray[id] = pool.CreateObject(id);
            NL_TEST_ASSERT(inSuite, objArray[id] != nullptr);
        }
        return Loop::Continue;
    });
    for (size_t i = 0; i < kSize; ++i)
    {
        NL_TEST_ASSERT(inSuite, objArray[i] != nullptr);
        NL_TEST_ASSERT(inSuite, objArray[i]->mId == i);
    }
    NL_TEST_ASSERT(inSuite, count >= kSize / 2);
    NL_TEST_ASSERT(inSuite, count <= kSize);

    pool.ReleaseAll();
}

void TestForEachActiveObjectStatic(nlTestSuite * inSuite, void * inContext)
{
    TestForEachActiveObject<ObjectPoolMem::kInline>(inSuite, inContext);
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestForEachActiveObjectDynamic(nlTestSuite * inSuite, void * inContext)
{
    TestForEachActiveObject<ObjectPoolMem::kHeap>(inSuite, inContext);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestPoolInterface(nlTestSuite * inSuite, void * inContext)
{
    struct TestObject
    {
        TestObject(uint32_t * set, size_t id) : mSet(set), mId(id) { *mSet |= (1 << mId); }
        ~TestObject() { *mSet &= ~(1 << mId); }
        uint32_t * mSet;
        size_t mId;
    };
    using TestObjectPoolType = PoolInterface<TestObject, uint32_t *, size_t>;

    struct PoolHolder
    {
        PoolHolder(TestObjectPoolType & testObjectPool) : mTestObjectPoolInterface(testObjectPool) {}
        TestObjectPoolType & mTestObjectPoolInterface;
    };

    constexpr size_t kSize = 10;
    PoolImpl<TestObject, kSize, P, typename TestObjectPoolType::Interface> testObjectPool;
    PoolHolder poolHolder(testObjectPool);
    uint32_t bits = 0;

    TestObject * objs2[kSize];
    for (size_t i = 0; i < kSize; ++i)
    {
        objs2[i] = poolHolder.mTestObjectPoolInterface.CreateObject(&bits, i);
        NL_TEST_ASSERT(inSuite, objs2[i] != nullptr);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface) == i + 1);
        NL_TEST_ASSERT(inSuite, bits == (1ul << (i + 1)) - 1);
    }
    for (size_t i = 0; i < kSize; ++i)
    {
        poolHolder.mTestObjectPoolInterface.ReleaseObject(objs2[i]);
        NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface) == kSize - i - 1);
    }
    NL_TEST_ASSERT(inSuite, bits == 0);

    // Verify that ReleaseAll() calls the destructors.
    for (size_t i = 0; i < kSize; ++i)
    {
        objs2[i] = poolHolder.mTestObjectPoolInterface.CreateObject(&bits, i);
    }
    NL_TEST_ASSERT(inSuite, bits == (1ul << kSize) - 1);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface) == kSize);

    poolHolder.mTestObjectPoolInterface.ReleaseAll();
    NL_TEST_ASSERT(inSuite, bits == 0);
    NL_TEST_ASSERT(inSuite, GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface) == 0);
}

void TestPoolInterfaceStatic(nlTestSuite * inSuite, void * inContext)
{
    TestPoolInterface<ObjectPoolMem::kInline>(inSuite, inContext);
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestPoolInterfaceDynamic(nlTestSuite * inSuite, void * inContext)
{
    TestPoolInterface<ObjectPoolMem::kHeap>(inSuite, inContext);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

int Setup(void * inContext)
{
    return ::chip::Platform::MemoryInit() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int Teardown(void * inContext)
{
    ::chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    // clang-format off
    NL_TEST_DEF_FN(TestReleaseNullStatic),
    NL_TEST_DEF_FN(TestCreateReleaseObjectStatic),
    NL_TEST_DEF_FN(TestCreateReleaseStructStatic),
    NL_TEST_DEF_FN(TestForEachActiveObjectStatic),
    NL_TEST_DEF_FN(TestPoolInterfaceStatic),
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    NL_TEST_DEF_FN(TestReleaseNullDynamic),
    NL_TEST_DEF_FN(TestCreateReleaseObjectDynamic),
    NL_TEST_DEF_FN(TestCreateReleaseStructDynamic),
    NL_TEST_DEF_FN(TestForEachActiveObjectDynamic),
    NL_TEST_DEF_FN(TestPoolInterfaceDynamic),
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    NL_TEST_SENTINEL()
    // clang-format on
};

int TestPool()
{
    nlTestSuite theSuite = { "CHIP Pool tests", &sTests[0], Setup, Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPool);
