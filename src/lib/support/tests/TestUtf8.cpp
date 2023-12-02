
/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#define TEST_VALID_BYTES(...)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        uint8_t _buff[] = { __VA_ARGS__ };                                                                                         \
        CharSpan _span(reinterpret_cast<const char *>(_buff), sizeof(_buff));                                                      \
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(_span));                                                                             \
    } while (0)

#define TEST_INVALID_BYTES(...)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        uint8_t _buff[] = { __VA_ARGS__ };                                                                                         \
        CharSpan _span(reinterpret_cast<const char *>(_buff), sizeof(_buff));                                                      \
        NL_TEST_ASSERT(inSuite, !Utf8::IsValid(_span));                                                                            \
    } while (0)

void TestValidStrings(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan())); // empty span ok

    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("abc")));

    // Various tests from https://www.w3.org/2001/06/utf-8-wrong/UTF-8-test.html

    // Generic UTF8
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("κόσμε")));

    // First possible sequence of a certain length
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("ࠀ")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("𐀀")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("�����")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("������")));

    // Last possible sequence of a certain length
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("߿")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("￿")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("����")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("�����")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("������")));

    // Other boundary conditions
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("퟿")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("�")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("􏿿")));
    NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan::fromCharString("����")));

    // NOTE: UTF8 allows embeded NULLs
    //       even though strings like that are probably not ideal for handling
    //       Test that we allow this, but consider later to disallow them
    //       completely if the spec is updated as such
    {
        char zero[16] = { 0 };
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 0)));
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 1)));
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 2)));
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 3)));
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 4)));
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(zero, 16)));
    }

    {
        char insideZero[] = "test\0zero";
        NL_TEST_ASSERT(inSuite, Utf8::IsValid(CharSpan(insideZero)));
    }

    // Test around forbidden 0xD800..0xDFFF UTF-16 surrogate pairs.
    TEST_VALID_BYTES(0b1110'1101, 0b10'011111, 0b10'111111);
    TEST_VALID_BYTES(0b1110'1110, 0b10'000000, 0b10'000000);
}

void TestInvalidStrings(nlTestSuite * inSuite, void * inContext)
{
    // Overly long sequences
    TEST_INVALID_BYTES(0xc0, 0b10'111111);
    TEST_INVALID_BYTES(0xc1, 0b10'111111);

    TEST_INVALID_BYTES(0xe0, 0b1001'1111, 0x80); // A
    TEST_INVALID_BYTES(0xed, 0b1011'0000, 0x80); // B
    TEST_INVALID_BYTES(0xf0, 0b1000'1111, 0x80); // C

    // Invalid 0xD800 .. 0xDFFF UTF-16 surrogates that should not appear in UTF-8.
    TEST_INVALID_BYTES(0b1110'1101, 0b10'100000, 0b10'000000);
    TEST_INVALID_BYTES(0b1110'1101, 0b10'111111, 0b10'111111);

    // Outside codepoint
    TEST_INVALID_BYTES(0xf4, 0x90, 0x80, 0x80); // D
    TEST_INVALID_BYTES(0xf4, 0x91, 0x82, 0x83);
    TEST_INVALID_BYTES(0xf5, 0x81, 0x82, 0x83);

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
    // 4.1  Examples of an overlong ASCII character (in w3c tests)
    TEST_INVALID_BYTES(0xc0, 0xaf);
    TEST_INVALID_BYTES(0xe0, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xf0, 0x80, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xf8, 0x80, 0x80, 0x80, 0xaf);
    TEST_INVALID_BYTES(0xfc, 0x80, 0x80, 0x80, 0x80, 0xaf);
    // 4.2  Maximum overlong sequences (in w3c tests)
    TEST_INVALID_BYTES(0xc1, 0xbf);
    TEST_INVALID_BYTES(0xe0, 0x9f, 0xbf);
    TEST_INVALID_BYTES(0xf0, 0x8f, 0xbf, 0xbf);
    TEST_INVALID_BYTES(0xf8, 0x87, 0xbf, 0xbf, 0xbf);
    TEST_INVALID_BYTES(0xfc, 0x83, 0xbf, 0xbf, 0xbf, 0xbf);
    // 4.3  Overlong representation of the NUL character (in w3c tests)
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
    nlTestSuite theSuite = { "UTF8 validator tests", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestUtf8);
