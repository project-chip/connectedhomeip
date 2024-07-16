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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Pool.h>
#include <lib/support/PoolWrapper.h>
#include <system/SystemConfig.h>

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

class TestPool : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

template <typename T, size_t N, ObjectPoolMem P>
void TestReleaseNull()
{
    ObjectPool<T, N, P> pool;
    pool.ReleaseObject(nullptr);
    EXPECT_EQ(GetNumObjectsInUse(pool), 0u);
    EXPECT_EQ(pool.Allocated(), 0u);
}

TEST_F(TestPool, TestReleaseNullStatic)
{
    TestReleaseNull<uint32_t, 10, ObjectPoolMem::kInline>();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
TEST_F(TestPool, TestReleaseNullDynamic)
{
    TestReleaseNull<uint32_t, 10, ObjectPoolMem::kHeap>();
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <typename T, size_t N, ObjectPoolMem P>
void TestCreateReleaseObject()
{
    ObjectPool<uint32_t, N, ObjectPoolMem::kInline> pool;
    uint32_t * obj[N];

    EXPECT_EQ(pool.Allocated(), 0u);
    for (int t = 0; t < 2; ++t)
    {
        pool.ReleaseAll();
        EXPECT_EQ(pool.Allocated(), 0u);

        for (size_t i = 0; i < N; ++i)
        {
            obj[i] = pool.CreateObject();
            ASSERT_NE(obj[i], nullptr);
            EXPECT_EQ(GetNumObjectsInUse(pool), i + 1);
            EXPECT_EQ(pool.Allocated(), i + 1);
        }
    }

    for (size_t i = 0; i < N; ++i)
    {
        pool.ReleaseObject(obj[i]);
        EXPECT_EQ(GetNumObjectsInUse(pool), N - i - 1);
        EXPECT_EQ(pool.Allocated(), N - i - 1);
    }
}

TEST_F(TestPool, TestCreateReleaseObjectStatic)
{
    constexpr const size_t kSize = 100;
    TestCreateReleaseObject<uint32_t, kSize, ObjectPoolMem::kInline>();

    ObjectPool<uint32_t, kSize, ObjectPoolMem::kInline> pool;
    uint32_t * obj[kSize];

    for (size_t i = 0; i < kSize; ++i)
    {
        obj[i] = pool.CreateObject();
        ASSERT_NE(obj[i], nullptr);
        EXPECT_EQ(GetNumObjectsInUse(pool), i + 1);
        EXPECT_EQ(pool.Allocated(), i + 1);
    }

    uint32_t * fail = pool.CreateObject();
    EXPECT_EQ(fail, nullptr);
    EXPECT_EQ(GetNumObjectsInUse(pool), kSize);
    EXPECT_EQ(pool.Allocated(), kSize);
    EXPECT_TRUE(pool.Exhausted());

    pool.ReleaseObject(obj[55]);
    EXPECT_EQ(GetNumObjectsInUse(pool), kSize - 1);
    EXPECT_EQ(pool.Allocated(), kSize - 1);
    EXPECT_FALSE(pool.Exhausted());
    EXPECT_EQ(obj[55], pool.CreateObject());
    EXPECT_EQ(GetNumObjectsInUse(pool), kSize);
    EXPECT_EQ(pool.Allocated(), kSize);
    EXPECT_TRUE(pool.Exhausted());

    fail = pool.CreateObject();
    ASSERT_EQ(fail, nullptr);
    EXPECT_EQ(GetNumObjectsInUse(pool), kSize);
    EXPECT_EQ(pool.Allocated(), kSize);
    EXPECT_TRUE(pool.Exhausted());

    pool.ReleaseAll();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
TEST_F(TestPool, TestCreateReleaseObjectDynamic)
{
    TestCreateReleaseObject<uint32_t, 100, ObjectPoolMem::kHeap>();
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestCreateReleaseStruct()
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
        ASSERT_NE(objs2[i], nullptr);
        EXPECT_EQ(pool.Allocated(), i + 1);
        EXPECT_EQ(GetNumObjectsInUse(pool), i + 1);
        EXPECT_EQ(GetNumObjectsInUse(pool), objs1.size());
    }
    for (size_t i = 0; i < kSize; ++i)
    {
        pool.ReleaseObject(objs2[i]);
        EXPECT_EQ(pool.Allocated(), kSize - i - 1);
        EXPECT_EQ(GetNumObjectsInUse(pool), kSize - i - 1);
        EXPECT_EQ(GetNumObjectsInUse(pool), objs1.size());
    }

    // Verify that ReleaseAll() calls the destructors.
    for (auto & obj : objs2)
    {
        obj = pool.CreateObject(objs1);
    }
    EXPECT_EQ(objs1.size(), kSize);
    EXPECT_EQ(pool.Allocated(), kSize);
    EXPECT_EQ(GetNumObjectsInUse(pool), kSize);
    printf("allocated = %u\n", static_cast<unsigned int>(pool.Allocated()));
    printf("highwater = %u\n", static_cast<unsigned int>(pool.HighWaterMark()));

    pool.ReleaseAll();
    printf("allocated = %u\n", static_cast<unsigned int>(pool.Allocated()));
    printf("highwater = %u\n", static_cast<unsigned int>(pool.HighWaterMark()));
    EXPECT_EQ(objs1.size(), 0u);
    EXPECT_EQ(GetNumObjectsInUse(pool), 0u);
    EXPECT_EQ(pool.Allocated(), 0u);
    EXPECT_EQ(pool.HighWaterMark(), kSize);
}

TEST_F(TestPool, TestCreateReleaseStructStatic)
{
    TestCreateReleaseStruct<ObjectPoolMem::kInline>();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
TEST_F(TestPool, TestCreateReleaseStructDynamic)
{
    TestCreateReleaseStruct<ObjectPoolMem::kHeap>();
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestForEachActiveObject()
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
        ASSERT_NE(objArray[i], nullptr);
        EXPECT_EQ(objArray[i]->mId, i);
        objIds.insert(i);
    }

    // Default constructor of an iterator should be pointing to the pool end.
    {
        typename ObjectPoolIterator<S, P>::Type defaultIterator;
        EXPECT_EQ(defaultIterator, pool.end());
    }

    // Verify that iteration visits all objects.
    size_t count = 0;
    {
        size_t sum = 0;
        pool.ForEachActiveObject([&](S * object) -> Loop {
            EXPECT_NE(object, nullptr);
            if (object == nullptr)
            {
                // Using EXPECT_NE instead of ASSERT_NE due to compilation errors when using ASSERT_NE
                return Loop::Continue;
            }
            EXPECT_EQ(objIds.count(object->mId), 1u);
            objIds.erase(object->mId);
            ++count;
            sum += object->mId;
            return Loop::Continue;
        });
        EXPECT_EQ(count, kSize);
        EXPECT_EQ(sum, kSize * (kSize - 1) / 2);
        EXPECT_EQ(objIds.size(), 0u);
    }

    // Test begin/end iteration
    {
        // re-create the above test environment, this time using iterators
        for (size_t i = 0; i < kSize; ++i)
        {
            objIds.insert(i);
        }
        count      = 0;
        size_t sum = 0;
        for (auto v = pool.begin(); v != pool.end(); ++v)
        {
            EXPECT_EQ(objIds.count((*v)->mId), 1u);
            objIds.erase((*v)->mId);
            ++count;
            sum += (*v)->mId;
        }
        EXPECT_EQ(count, kSize);
        EXPECT_EQ(sum, kSize * (kSize - 1) / 2);
        EXPECT_EQ(objIds.size(), 0u);
    }

    // Verify that returning Loop::Break stops iterating.
    count = 0;
    pool.ForEachActiveObject([&](S * object) {
        objIds.insert(object->mId);
        return ++count != kSize / 2 ? Loop::Continue : Loop::Break;
    });
    EXPECT_EQ(count, kSize / 2);
    EXPECT_EQ(objIds.size(), kSize / 2);

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
    EXPECT_EQ(count, (kSize - 1) * kSize / 2);
    EXPECT_EQ(objIds.size(), 0u);

    // Verify that iteration can be nested for iterator types
    {
        count = 0;
        for (auto v : pool)
        {
            objIds.insert(v->mId);
            if (++count == kSize / 2)
            {
                break;
            }
        }

        count = 0;
        for (auto outer : pool)
        {
            if (objIds.count(outer->mId) != 1)
            {
                continue;
            }

            for (auto inner : pool)
            {
                if (inner == outer)
                {
                    objIds.erase(inner->mId);
                }
                else
                {
                    ++count;
                }
            }
        }
        EXPECT_EQ(count, (kSize - 1) * kSize / 2);
        EXPECT_EQ(objIds.size(), 0u);
    }

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
    EXPECT_EQ(count, kSize);
    EXPECT_EQ(objIds.size(), kSize / 2);
    for (size_t i = 0; i < kSize; ++i)
    {
        if ((i % 2) == 0)
        {
            EXPECT_EQ(objArray[i], nullptr);
        }
        else
        {
            ASSERT_NE(objArray[i], nullptr);
            EXPECT_EQ(objArray[i]->mId, i);
        }
    }

    count = 0;
    pool.ForEachActiveObject([&](S * object) {
        ++count;
        if ((object->mId % 2) == 1)
        {
            size_t id = object->mId - 1;
            EXPECT_EQ(objArray[id], nullptr);
            objArray[id] = pool.CreateObject(id);
            EXPECT_NE(objArray[id], nullptr);
        }
        return Loop::Continue;
    });
    for (size_t i = 0; i < kSize; ++i)
    {
        ASSERT_NE(objArray[i], nullptr);
        EXPECT_EQ(objArray[i]->mId, i);
    }
    EXPECT_GE(count, kSize / 2);
    EXPECT_LE(count, kSize);

    // Test begin/end iteration
    {
        count = 0;
        for (auto object : pool)
        {
            ++count;
            if ((object->mId % 2) == 0)
            {
                objArray[object->mId] = nullptr;
                // NOTE: this explicitly tests if pool supports releasing while iterating
                //       this MUST be supported by contract of Pool iterators
                pool.ReleaseObject(object);
            }
            else
            {
                objIds.insert(object->mId);
            }
        }
        EXPECT_EQ(count, kSize);
        EXPECT_EQ(objIds.size(), kSize / 2);

        // validate we iterate only over active objects
        for (auto object : pool)
        {
            EXPECT_EQ((object->mId % 2), 1u);
        }

        for (size_t i = 0; i < kSize; ++i)
        {
            if ((i % 2) == 0)
            {
                EXPECT_EQ(objArray[i], nullptr);
            }
            else
            {
                ASSERT_NE(objArray[i], nullptr);
                EXPECT_EQ(objArray[i]->mId, i);
            }
        }

        count = 0;
        for (auto object : pool)
        {
            ++count;
            if ((object->mId % 2) != 1)
            {
                continue;
            }
            size_t id = object->mId - 1;
            EXPECT_EQ(objArray[id], nullptr);
            objArray[id] = pool.CreateObject(id);
            EXPECT_NE(objArray[id], nullptr);
        }
        for (size_t i = 0; i < kSize; ++i)
        {
            ASSERT_NE(objArray[i], nullptr);
            EXPECT_EQ(objArray[i]->mId, i);
        }
        EXPECT_GE(count, kSize / 2);
        EXPECT_LE(count, kSize);
    }

    pool.ReleaseAll();
}

TEST_F(TestPool, TestForEachActiveObjectStatic)
{
    TestForEachActiveObject<ObjectPoolMem::kInline>();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
TEST_F(TestPool, TestForEachActiveObjectDynamic)
{
    TestForEachActiveObject<ObjectPoolMem::kHeap>();
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <ObjectPoolMem P>
void TestPoolInterface()
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
        ASSERT_NE(objs2[i], nullptr);
        EXPECT_EQ(GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface), i + 1);
        EXPECT_EQ(bits, (1ul << (i + 1)) - 1);
    }
    for (size_t i = 0; i < kSize; ++i)
    {
        poolHolder.mTestObjectPoolInterface.ReleaseObject(objs2[i]);
        EXPECT_EQ(GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface), kSize - i - 1);
    }
    EXPECT_EQ(bits, 0u);

    // Verify that ReleaseAll() calls the destructors.
    for (size_t i = 0; i < kSize; ++i)
    {
        objs2[i] = poolHolder.mTestObjectPoolInterface.CreateObject(&bits, i);
    }
    EXPECT_EQ(bits, (1ul << kSize) - 1);
    EXPECT_EQ(GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface), kSize);

    poolHolder.mTestObjectPoolInterface.ReleaseAll();
    EXPECT_EQ(bits, 0u);
    EXPECT_EQ(GetNumObjectsInUse(poolHolder.mTestObjectPoolInterface), 0u);
}

TEST_F(TestPool, TestPoolInterfaceStatic)
{
    TestPoolInterface<ObjectPoolMem::kInline>();
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
TEST_F(TestPool, TestPoolInterfaceDynamic)
{
    TestPoolInterface<ObjectPoolMem::kHeap>();
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace
