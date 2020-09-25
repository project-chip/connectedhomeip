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

#include <retransmit/MessageCache.h>
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

private:
    nlTestSuite * mSuite;
    std::bitset<kMaxPayloadValue> mAquired;
};

static IntPayloadTracker gPayloadTracker;

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
        chip::Retransmit::PeerAddressedCache<int, 20> test;
        NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
    }
    NL_TEST_ASSERT(inSuite, gPayloadTracker.Count() == 0);
}

void TestDestructorFree(nlTestSuite * inSuite, void * inContext)
{
    chip::Retransmit::PeerAddressedCache<int, 20> test;

    // FIXME: set some values, then reset
    // FIXME: implement
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("NoOp", TestNoOp),
    NL_TEST_DEF("DestructorFree", TestDestructorFree),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestMessageCache(void)
{
    nlTestSuite theSuite = { "Retransmit-MessageCache", &sTests[0], nullptr, nullptr };
    gPayloadTracker.Init(&theSuite);
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMessageCache)