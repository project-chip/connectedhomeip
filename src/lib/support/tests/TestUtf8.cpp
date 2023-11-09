
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

#include <functional>

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/utf8.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

bool IsValidCStringAsUtf8(const char * s)
{
    return Utf8::IsValid(CharSpan(s, strlen(s)));
}

void TestValidStrings(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan())); // empty span ok

    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8(""));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("abc"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("κόσμε"));

    // First possible sequence of a certain length
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8(""));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("ࠀ"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("𐀀"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("�����"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("������"));

    // Last possible sequence of a certain length
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8(""));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("߿"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("￿"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("����"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("�����"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("������"));
}

void TestInvalidStrings(nlTestSuite * inSuite, void * inContext)
{
    {
        // cannot embed zeroes
        char buff[] = { 0 };
        NL_TEST_ASSERT(inSuite, !Utf8::IsValid(CharSpan(buff)));
    }
}

} // namespace

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestValidStrings", TestValidStrings),
    NL_TEST_DEF("TestInvalidStrings", TestInvalidStrings),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestUtf8()
{
    nlTestSuite theSuite = { "CHIP Variant tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestUtf8);
