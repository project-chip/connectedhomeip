/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/OwnerOf.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

class Member
{
};

class Base
{
public:
    uint32_t Offset0;
    uint32_t Offset4;
    Member member;
};

static void TestMemberOwner(nlTestSuite * inSuite, void * inContext)
{
    Base base;
    Member * member = &base.member;
    NL_TEST_ASSERT(inSuite, OwnerOf(member, &Base::member) == &base);
    NL_TEST_ASSERT(inSuite, &OwnerOf(member, &Base::member)->member == member);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestMemberOwner), NL_TEST_SENTINEL() };

int TestOwnerOf(void)
{
    nlTestSuite theSuite = { "CHIP OwnerOf tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOwnerOf)
