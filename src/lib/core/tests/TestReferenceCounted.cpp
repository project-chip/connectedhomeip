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
 *      This file implements a test for  CHIP core library reference counted object.
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <lib/core/ReferenceCounted.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

class TestClass : public ReferenceCounted<TestClass>
{
};

static void TestRetainRelease(nlTestSuite * inSuite, void * inContext)
{
    TestClass * testObj = chip::Platform::New<TestClass>();
    NL_TEST_ASSERT(inSuite, testObj->GetReferenceCount() == 1);
    testObj->Retain();
    NL_TEST_ASSERT(inSuite, testObj->GetReferenceCount() == 2);
    testObj->Release();
    NL_TEST_ASSERT(inSuite, testObj->GetReferenceCount() == 1);
    testObj->Release();
}

class TestClassNonHeap;
class Deletor
{
public:
    static void Release(TestClassNonHeap * obj);
};

class TestClassNonHeap : public ReferenceCounted<TestClassNonHeap, Deletor>
{
public:
    bool deleted;
};

void Deletor::Release(TestClassNonHeap * obj)
{
    obj->deleted = true;
}

static void TestRetainReleaseNonHeap(nlTestSuite * inSuite, void * inContext)
{
    TestClassNonHeap testObj;
    testObj.deleted = false;
    NL_TEST_ASSERT(inSuite, testObj.GetReferenceCount() == 1);
    NL_TEST_ASSERT(inSuite, testObj.deleted == false);
    testObj.Retain();
    NL_TEST_ASSERT(inSuite, testObj.GetReferenceCount() == 2);
    NL_TEST_ASSERT(inSuite, testObj.deleted == false);
    testObj.Release();
    NL_TEST_ASSERT(inSuite, testObj.GetReferenceCount() == 1);
    NL_TEST_ASSERT(inSuite, testObj.deleted == false);
    testObj.Release();
    NL_TEST_ASSERT(inSuite, testObj.GetReferenceCount() == 0);
    NL_TEST_ASSERT(inSuite, testObj.deleted == true);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("ReferenceCountedRetain", TestRetainRelease),
    NL_TEST_DEF("ReferenceCountedRetainNonHeap", TestRetainReleaseNonHeap),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestReferenceCounted_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int TestReferenceCounted_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestReferenceCounted(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Reference-Counted",
        &sTests[0],
        TestReferenceCounted_Setup,
        TestReferenceCounted_Teardown
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReferenceCounted)
