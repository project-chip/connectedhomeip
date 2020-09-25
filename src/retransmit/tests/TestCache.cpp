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

#include "TestRetransmit.h"

#define UNIT_TESTS

#include <retransmit/Cache.h>
#include <support/TestUtils.h>

#include <bitset>
#include <nlunit-test.h>

// Helpers for simple payload management
namespace {
constexpr int kMaxPayloadValue = 100;

class IntPayloadTracker
{
public:
    void Init(nlTestSuite * suite) { mSuite = suite; }

    void Acquire(int value)
    {
        NL_TEST_ASSERT(mSuite, (value > 0) && value < kMaxPayloadValue);
        mAquired.set(value);
    }

    void Release(int value)
    {
        NL_TEST_ASSERT(mSuite, (value > 0) && value < kMaxPayloadValue);
        NL_TEST_ASSERT(mSuite, mAquired.test(value));
        mAquired.reset(value);
    }

    size_t Count() const { return mAquired.count(); }

    bool IsAquired(int value) const { return mAquired.test(value); }

private:
    nlTestSuite * mSuite;
    std::bitset<kMaxPayloadValue> mAquired;
};

static IntPayloadTracker gPayloadTracker;

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
    // unused address cache should not do any aquire/release at any time
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
    {
        chip::Retransmit::Cache<int, int, 20> test;
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
    }
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void TestDestructorFree(nlTestSuite * inSuite, void * inContext)
{
    {
        chip::Retransmit::Cache<int, int, 20> test;

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

        NL_TEST_ASSERT(inSuite, test.Add(1, 1) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.Add(2, 2) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);
    }

    // destructor should release the items
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void OutOfSpace(nlTestSuite * inSuite, void * inContext)
{
    {
        chip::Retransmit::Cache<int, int, 4> test;

        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

        NL_TEST_ASSERT(inSuite, test.Add(1, 1) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.Add(2, 2) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.Add(3, 4) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, test.Add(4, 6) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

        NL_TEST_ASSERT(inSuite, test.Add(5, 8) == CHIP_ERROR_NO_MEMORY);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

        NL_TEST_ASSERT(inSuite, test.Add(6, 10) == CHIP_ERROR_NO_MEMORY);
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);
    }
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void AddRemove(nlTestSuite * inSuite, void * inContext)
{
    chip::Retransmit::Cache<int, int, 3> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.Add(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.Add(10, 8) == CHIP_ERROR_NO_MEMORY);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 3);

    NL_TEST_ASSERT(inSuite, test.Remove(2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);

    NL_TEST_ASSERT(inSuite, test.Add(10, 8) == CHIP_NO_ERROR);
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
    chip::Retransmit::Cache<int, int, 4> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.Add(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(4, 8) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 4);

    test.RemoveMatching(DivisibleBy(2));
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 2);

    // keys 1 and 3 remain
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAquired(1));
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAquired(4));

    NL_TEST_ASSERT(inSuite, test.Remove(3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPayloadTracker.IsAquired(1));
    NL_TEST_ASSERT(inSuite, !gPayloadTracker.IsAquired(4));
}

void FindMatching(nlTestSuite * inSuite, void * inContext)
{
    chip::Retransmit::Cache<int, int, 4> test;

    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);

    NL_TEST_ASSERT(inSuite, test.Add(1, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(2, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(3, 4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test.Add(4, 8) == CHIP_NO_ERROR);
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

int TestCache(void)
{
    nlTestSuite theSuite = { "Retransmit-Cache", &sTests[0], nullptr, nullptr };
    gPayloadTracker.Init(&theSuite);
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCache)