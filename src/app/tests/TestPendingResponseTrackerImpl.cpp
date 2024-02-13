/*
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

#include <app/PendingResponseTrackerImpl.h>
#include <lib/support/UnitTestRegistration.h>

#include <algorithm>
#include <nlunit-test.h>
#include <vector>

namespace {

using namespace chip;

void TestPendingResponseTracker_FillEntireTracker(nlTestSuite * inSuite, void * inContext)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;
    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        NL_TEST_ASSERT(inSuite, false == pendingResponseTracker.IsResponsePending(commandRef));
        NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == pendingResponseTracker.AddPendingResponse(commandRef));
        NL_TEST_ASSERT(inSuite, true == pendingResponseTracker.IsResponsePending(commandRef));
    }

    NL_TEST_ASSERT(inSuite, std::numeric_limits<uint16_t>::max() == pendingResponseTracker.Count());

    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == pendingResponseTracker.ResponseReceived(commandRef));
        NL_TEST_ASSERT(inSuite, false == pendingResponseTracker.IsResponsePending(commandRef));
    }
    NL_TEST_ASSERT(inSuite, 0 == pendingResponseTracker.Count());
}

void TestPendingResponseTracker_FillSingleEntryInTracker(nlTestSuite * inSuite, void * inContext)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // No real reason to choose 40 here. I just choose this value since we need a value to test.
    uint16_t commandRefToSet = 40;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == pendingResponseTracker.AddPendingResponse(commandRefToSet));

    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        bool expectedIsSetResult = (commandRef == commandRefToSet);
        NL_TEST_ASSERT(inSuite, expectedIsSetResult == pendingResponseTracker.IsResponsePending(commandRef));
    }
}

void TestPendingResponseTracker_RemoveNonExistentEntryInTrackerFails(nlTestSuite * inSuite, void * inContext)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // No real reason to choose 40 here. I just choose this value since we need a value to test.
    uint16_t commandRef = 40;
    NL_TEST_ASSERT(inSuite, false == pendingResponseTracker.IsResponsePending(commandRef));
    NL_TEST_ASSERT(inSuite, CHIP_ERROR_KEY_NOT_FOUND == pendingResponseTracker.ResponseReceived(commandRef));
}

void TestPendingResponseTracker_AddingSecondEntryFails(nlTestSuite * inSuite, void * inContext)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // No real reason to choose 40 here. I just choose this value since we need a value to test.
    uint16_t commandRef = 40;
    NL_TEST_ASSERT(inSuite, false == pendingResponseTracker.IsResponsePending(commandRef));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == pendingResponseTracker.AddPendingResponse(commandRef));
    NL_TEST_ASSERT(inSuite, true == pendingResponseTracker.IsResponsePending(commandRef));
    NL_TEST_ASSERT(inSuite, CHIP_ERROR_INVALID_ARGUMENT == pendingResponseTracker.AddPendingResponse(commandRef));
}

void TestPendingResponseTracker_PopFindsAllPendingRequests(nlTestSuite * inSuite, void * inContext)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // No real reason why values in requestsToAdd are choosen. We just need values to use to test.
    std::vector<uint16_t> requestsToAdd = { 0, 2, 50, 2000 };
    for (const uint16_t & commandRef : requestsToAdd)
    {
        NL_TEST_ASSERT(inSuite, false == pendingResponseTracker.IsResponsePending(commandRef));
        NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == pendingResponseTracker.AddPendingResponse(commandRef));
        NL_TEST_ASSERT(inSuite, true == pendingResponseTracker.IsResponsePending(commandRef));
    }

    NL_TEST_ASSERT(inSuite, requestsToAdd.size() == pendingResponseTracker.Count());

    for (size_t i = 0; i < requestsToAdd.size(); i++)
    {
        auto commandRef = pendingResponseTracker.PopPendingResponse();
        NL_TEST_ASSERT(inSuite, true == commandRef.HasValue());
        bool expectedCommandRef = std::find(requestsToAdd.begin(), requestsToAdd.end(), commandRef.Value()) != requestsToAdd.end();
        NL_TEST_ASSERT(inSuite, true == expectedCommandRef);
    }
    NL_TEST_ASSERT(inSuite, 0 == pendingResponseTracker.Count());
    auto commandRef = pendingResponseTracker.PopPendingResponse();
    NL_TEST_ASSERT(inSuite, false == commandRef.HasValue());
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestPendingResponseTracker_FillEntireTracker),
                                 NL_TEST_DEF_FN(TestPendingResponseTracker_FillSingleEntryInTracker),
                                 NL_TEST_DEF_FN(TestPendingResponseTracker_RemoveNonExistentEntryInTrackerFails),
                                 NL_TEST_DEF_FN(TestPendingResponseTracker_AddingSecondEntryFails),
                                 NL_TEST_DEF_FN(TestPendingResponseTracker_PopFindsAllPendingRequests),
                                 NL_TEST_SENTINEL() };

int TestPendingResponseTracker()
{
    nlTestSuite theSuite = { "CHIP PendingResponseTrackerImpl tests", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPendingResponseTracker)
