/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <stdint.h>

#include <app/TestEventTriggerDelegate.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;

namespace {

class TestEventHandler : public TestEventTriggerHandler
{
public:
    TestEventHandler() = delete;

    explicit TestEventHandler(uint64_t supportedEventTriggerValue) : mSupportedEventTriggerValue(supportedEventTriggerValue) {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (eventTrigger == mSupportedEventTriggerValue)
        {
            ++mCount;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int GetCount() const { return mCount; }
    void ClearCount() { mCount = 0; }

private:
    uint64_t mSupportedEventTriggerValue;
    int mCount = 0;
};

class TestEventDelegate : public TestEventTriggerDelegate
{
public:
    explicit TestEventDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override
    {
        return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
    }

private:
    ByteSpan mEnableKey;
};

void TestKeyChecking(nlTestSuite * aSuite, void * aContext)
{
    const uint8_t kTestKey[16]       = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const uint8_t kBadKey[16]        = { 255, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const uint8_t kDiffLenBadKey[17] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    TestEventDelegate delegate{ ByteSpan{ kTestKey } };

    NL_TEST_ASSERT(aSuite, delegate.DoesEnableKeyMatch(ByteSpan{ kTestKey }) == true);
    NL_TEST_ASSERT(aSuite, delegate.DoesEnableKeyMatch(ByteSpan{ kBadKey }) == false);
    NL_TEST_ASSERT(aSuite, delegate.DoesEnableKeyMatch(ByteSpan{ kDiffLenBadKey }) == false);
    NL_TEST_ASSERT(aSuite, delegate.DoesEnableKeyMatch(ByteSpan{}) == false);
}

void TestHandlerManagement(nlTestSuite * aSuite, void * aContext)
{
    const uint8_t kTestKey[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    TestEventDelegate delegate{ ByteSpan{ kTestKey } };

    TestEventHandler event1Handler{ 1 };
    TestEventHandler event2Handler{ 2 };

    // Add 2, check 2 works 1 doesn't.
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event2Handler) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event2Handler) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, event1Handler.GetCount() == 0);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 2);

    event1Handler.ClearCount();
    event2Handler.ClearCount();

    // Add 1, check 1 and 2 work.
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event1Handler) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event1Handler) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, event1Handler.GetCount() == 2);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 1);

    event1Handler.ClearCount();
    event2Handler.ClearCount();

    // Remove 2, check 1 works.
    delegate.RemoveHandler(&event2Handler);

    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, event1Handler.GetCount() == 1);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 0);

    // Remove again, should be NO-OP.
    delegate.RemoveHandler(&event2Handler);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 0);

    event1Handler.ClearCount();
    event2Handler.ClearCount();

    // Add 2 again, check 1 and 2 work.
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event2Handler) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event2Handler) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, event1Handler.GetCount() == 1);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 1);

    event1Handler.ClearCount();
    event2Handler.ClearCount();

    // Remove all handlers, check neither works.
    delegate.ClearAllHandlers();

    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(1) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, delegate.HandleEventTriggers(2) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, event1Handler.GetCount() == 0);
    NL_TEST_ASSERT(aSuite, event2Handler.GetCount() == 0);

    // Add a handler at the end: having it remaining should not cause crashes/leaks.
    NL_TEST_ASSERT(aSuite, delegate.AddHandler(&event2Handler) == CHIP_NO_ERROR);
}

int TestSetup(void * inContext)
{
    return SUCCESS;
}

int TestTeardown(void * inContext)
{
    return SUCCESS;
}

} // namespace

int TestTestEventTriggerDelegate()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestKeyChecking", TestKeyChecking),
                               NL_TEST_DEF("TestHandlerManagement", TestHandlerManagement), NL_TEST_SENTINEL() };

    nlTestSuite theSuite = {
        "TestTestEventTriggerDelegate",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };

    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestTestEventTriggerDelegate)
