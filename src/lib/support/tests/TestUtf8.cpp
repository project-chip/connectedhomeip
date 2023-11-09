
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

    // Various tests from https://www.w3.org/2001/06/utf-8-wrong/UTF-8-test.html

    // Generic UTF8
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

    // Other boundary conditions
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("퟿"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8(""));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("�"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("􏿿"));
    NL_TEST_ASSERT(inSuite, IsValidCStringAsUtf8("����"));
}

#define TEST_INVALID_BYTES(...)                                                                                                    \
    {                                                                                                                              \
        uint8_t _buff[] = { __VA_ARGS__ };                                                                                         \
        CharSpan _span(reinterpret_cast<const char *>(_buff), sizeof(_buff));                                                      \
        NL_TEST_ASSERT(inSuite, !Utf8::IsValid(_span));                                                                            \
    }                                                                                                                              \
    (void) 0

void TestInvalidStrings(nlTestSuite * inSuite, void * inContext)
{
    // cannot embed zeroes
    TEST_INVALID_BYTES(0x00);

    // Missing continuation
    TEST_INVALID_BYTES(0xC2);
    TEST_INVALID_BYTES(0xE0);
    TEST_INVALID_BYTES(0xE1);
    TEST_INVALID_BYTES(0xE1, 0x9F);
    TEST_INVALID_BYTES(0xED, 0x9F);
    TEST_INVALID_BYTES(0xEE, 0x9F);
    TEST_INVALID_BYTES(0xF0);
    TEST_INVALID_BYTES(0xF0, 0x9F);
    TEST_INVALID_BYTES(0xF0, 0x9F, 0x9F);
    TEST_INVALID_BYTES(0xF1);
    TEST_INVALID_BYTES(0xF1, 0x9F);
    TEST_INVALID_BYTES(0xF1, 0x9F, 0x9F);
    TEST_INVALID_BYTES(0xF4);
    TEST_INVALID_BYTES(0xF4, 0x9F);
    TEST_INVALID_BYTES(0xF4, 0x9F, 0x9F);

    // More tests from  https://www.w3.org/2001/06/utf-8-wrong/UTF-8-test.html
    TEST_INVALID_BYTES(0x80); // First continuation byte
    TEST_INVALID_BYTES(0xBF); // Last continuation byte

    // Impossible bytes
    TEST_INVALID_BYTES(0xFE);
    TEST_INVALID_BYTES(0xFF);
    TEST_INVALID_BYTES(0xFE, 0xFE, 0xFF, 0xFF);

    // Overlong sequences
    TEST_INVALID_BYTES(0xc0, 0xaf);
    TEST_INVALID_BYTES(0xe0, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xf0, 0x80, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xf8, 0x80, 0x80, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xfc, 0x80, 0x80, 0x80, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xc0, 0x80);
    TEST_INVALID_BYTES(0xe0, 0x80, 0x80);
    TEST_INVALID_BYTES(0xf0, 0x80, 0x80, 0x80);
    TEST_INVALID_BYTES(0xf8, 0x80, 0x80, 0x80, 0x80);
    TEST_INVALID_BYTES(0xfc, 0x80, 0x80, 0x80, 0x80, 0x80);
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
