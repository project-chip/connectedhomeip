/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/support/UnitTestRegistration.h>
#include <transport/retransmit/Cache.h>

#include <bitset>
#include <nlunit-test.h>

// Helpers for simple payload management
namespace {
constexpr int kMaxPayloadValue = 100;

/**
 * Derived cache class containing some test helper methods.
 */
template <typename KeyType, typename PayloadType, size_t N>
class TestableCache : public chip::Retransmit::Cache<KeyType, PayloadType, N>
{
public:
    /**
     * Convenience add when types are trivially copyable, so no actual
     * reference needs to be created.
     */
    template <std::enable_if_t<std::is_trivially_copyable<PayloadType>::value, int> = 0>
    CHIP_ERROR AddValue(const KeyType & key, PayloadType payload)
    {
        return chip::Retransmit::Cache<KeyType, PayloadType, N>::Add(key, payload);
    }
};

class IntPayloadTracker
{
public:
    void Init(nlTestSuite * suite) { mSuite = suite; }

    void Acquire(int value)
    {
        NL_TEST_ASSERT(mSuite, (value > 0) && value < kMaxPayloadValue);
        mAcquired.set(static_cast<size_t>(value));
    }

    void Release(int value)
    {
        NL_TEST_ASSERT(mSuite, (value > 0) && value < kMaxPayloadValue);
        NL_TEST_ASSERT(mSuite, mAcquired.test(static_cast<size_t>(value)));
        mAcquired.reset(static_cast<size_t>(value));
    }

    size_t Count() const { return mAcquired.count(); }

    bool IsAcquired(int value) const { return mAcquired.test(static_cast<size_t>(value)); }

private:
    nlTestSuite * mSuite;
    std::bitset<kMaxPayloadValue> mAcquired;
};

IntPayloadTracker gPayloadTracker;

/**
 * Helper class defining a matches method for things divisible by a
 * specific value.
 */
class DivisibleBy
{
public:
    DivisibleBy(int value) : mValue(value) {}

    bool Matches(int x) const { return (x % mValue) == 0; }

private:
    const int mValue;
};

} // namespace

template <>
int chip::Retransmit::Lifetime<int>::Acquire(int & value)
{
    gPayloadTracker.Acquire(value);
    return value;
}

template <>
void chip::Retransmit::Lifetime<int>::Release(int & value)
{
    gPayloadTracker.Release(value);
    value = 0; // make sure it is not used anymore
}

namespace {

void TestNoOp(nlTestSuite * inSuite, void * inContext)
{
    // unused address cache should not do any Acquire/release at any time
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
    {
        TestableCache<int, int, 20> test;
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
    }
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void TestDestructorFree(nlTestSuite * inSuite, void * inContext)
{
    {
        TestableCache<int, int, 20> test;

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

        NL_TEST_ASSERT(inSuite, test.AddValue(1, 1) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.AddValue(2, 2) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);
    }

    // destructor should release the items
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void OutOfSpace(nlTestSuite * inSuite, void * inContext)
{
    {
        TestableCache<int, int, 4> test;

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

        NL_TEST_ASSERT(inSuite, test.AddValue(1, 1) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.AddValue(2, 2) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.AddValue(3, 4) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.AddValue(4, 6) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

        NL_TEST_ASSERT(inSuite, test.AddValue(5, 8) == CHIP_ERROR_NO_MEMORY);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

        NL_TEST_ASSERT(inSuite, test.AddValue(6, 10) == CHIP_ERROR_NO_MEMORY);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);
    }
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void AddRemove(nlTestSuite * inSuite, void * inContext)
{
    TestableCache<int, int, 3> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.AddValue(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.AddValue(10, 8) == CHIP_ERROR_NO_MEMORY);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.Remove(2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);

    NL_TEST_ASSERT(inSuite, test.AddValue(10, 8) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.Remove(14) == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.Remove(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);

    NL_TEST_ASSERT(inSuite, test.Remove(3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 1);

    NL_TEST_ASSERT(inSuite, test.Remove(3) == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 1);

    NL_TEST_ASSERT(inSuite, test.Remove(10) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.Remove(10) == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void RemoveMatching(nlTestSuite * inSuite, void * inContext)
{
    TestableCache<int, int, 4> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.AddValue(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(4, 8) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

    test.RemoveMatching(DivisibleBy(2));
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);

    // keys 1 and 3 remain
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAcquired(1));
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAcquired(4));

    NL_TEST_ASSERT(inSuite, test.Remove(3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAcquired(1));
    NL_TEST_ASSERT(inSuite, !gPayloadTracker.IsAcquired(4));
}

void FindMatching(nlTestSuite * inSuite, void * inContext)
{
    TestableCache<int, int, 4> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.AddValue(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.AddValue(4, 8) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

    const int * key;
    const int * value;

    NL_TEST_ASSERT(inSuite, test.Find(DivisibleBy(20), &key, &value) == false);
    NL_TEST_ASSERT(inSuite, key == nullptr);
    NL_TEST_ASSERT(inSuite, value == nullptr);

    // This relies on linear add. May need changing if implementation changes
    NL_TEST_ASSERT(inSuite, test.Find(DivisibleBy(2), &key, &value) == true);
    NL_TEST_ASSERT(inSuite, *key == 2);
    NL_TEST_ASSERT(inSuite, *value == 2);

    NL_TEST_ASSERT(inSuite, test.Remove(*key) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, test.Find(DivisibleBy(2), &key, &value) == true);
    NL_TEST_ASSERT(inSuite, *key == 4);
    NL_TEST_ASSERT(inSuite, *value == 8);

    NL_TEST_ASSERT(inSuite, test.Remove(*key) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Find(DivisibleBy(2), &key, &value) == false);
    NL_TEST_ASSERT(inSuite, key == nullptr);
    NL_TEST_ASSERT(inSuite, value == nullptr);
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("NoOp", TestNoOp),
    NL_TEST_DEF("DestructorFree", TestDestructorFree),
    NL_TEST_DEF("OutOfSpace", OutOfSpace),
    NL_TEST_DEF("AddRemove", AddRemove),
    NL_TEST_DEF("RemoveMatching", RemoveMatching),
    NL_TEST_DEF("FindMatching", FindMatching),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestCache()
{
    nlTestSuite theSuite = { "Retransmit-Cache", &sTests[0], nullptr, nullptr };
    gPayloadTracker.Init(&theSuite);
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCache)
