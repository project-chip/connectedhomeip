/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/IniEscaping.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::IniEscaping;

namespace {

struct TestCase
{
    const char * input;
    const char * expectedOutput;
};

void TestEscaping(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, EscapeKey("") == "");
    NL_TEST_ASSERT(inSuite, EscapeKey("abcd1234,!") == "abcd1234,!");
    NL_TEST_ASSERT(inSuite, EscapeKey("ab\ncd =12\\34\x7f") == "ab\\x0acd\\x20\\x3d12\\x5c34\\x7f");
    NL_TEST_ASSERT(inSuite, EscapeKey(" ") == "\\x20");
    NL_TEST_ASSERT(inSuite, EscapeKey("===") == "\\x3d\\x3d\\x3d");
}

void TestUnescaping(nlTestSuite * inSuite, void * inContext)
{
    // Test valid cases
    NL_TEST_ASSERT(inSuite, UnescapeKey("") == "");
    NL_TEST_ASSERT(inSuite, UnescapeKey("abcd1234,!") == "abcd1234,!");
    NL_TEST_ASSERT(inSuite, UnescapeKey("ab\\x0acd\\x20\\x3d12\\x5c34\\x7f") == "ab\ncd =12\\34\x7f");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x20") == " ");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x3d\\x3d\\x3d") == "===");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x0d") == "\r");

    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x01\\x02\\x03\\x04\\x05\\x06\\x07") == "\x01\x02\x03\x04\x05\x06\x07");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x0e") == "\x08\x09\x0a\x0b\x0c\x0d\x0e");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x0f\\x10\\x11\\x12\\x13\\x14\\x15") == "\x0f\x10\x11\x12\x13\x14\x15");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c") == "\x16\x17\x18\x19\x1a\x1b\x1c");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x1d\\x1e\\x1f\\x20\\x7f\\x3d\\x5c") == "\x1d\x1e\x1f \x7f=\\");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x81\\x82\\xff") == "\x81\x82\xff");

    // Test invalid cases

    // letters should never be escaped
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x5a\55") != "ZU");
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x5a\55") == "");

    // Capitalized hex forbidden
    NL_TEST_ASSERT(inSuite, UnescapeKey("\\x0D") == "");

    // Partial escapes forbidden
    NL_TEST_ASSERT(inSuite, UnescapeKey("1\\x0") == "");
}

void TestRoundTrip(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("")) == "");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("abcd1234,!")) == "abcd1234,!");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("ab\ncd =12\\34\x7f")) == "ab\ncd =12\\34\x7f");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey(" ")) == " ");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("===")) == "===");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\r")) == "\r");

    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x01\x02\x03\x04\x05\x06\x07")) == "\x01\x02\x03\x04\x05\x06\x07");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x08\x09\x0a\x0b\x0c\x0d\x0e")) == "\x08\x09\x0a\x0b\x0c\x0d\x0e");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x0f\x10\x11\x12\x13\x14\x15")) == "\x0f\x10\x11\x12\x13\x14\x15");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x16\x17\x18\x19\x1a\x1b\x1c")) == "\x16\x17\x18\x19\x1a\x1b\x1c");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x1d\x1e\x1f \x7f=\\")) == "\x1d\x1e\x1f \x7f=\\");
    NL_TEST_ASSERT(inSuite, UnescapeKey(EscapeKey("\x81\x82\xff")) == "\x81\x82\xff");

    // Make sure entire range is escapable
    for (int c = 0; c <= 255; c++)
    {
        std::string s(5, static_cast<char>(c));
        NL_TEST_ASSERT_LOOP(inSuite, c, UnescapeKey(EscapeKey(s)) == s);
    }
}

const nlTest sTests[] = { NL_TEST_DEF("Test escaping API", TestEscaping), NL_TEST_DEF("Test unescaping API", TestUnescaping),
                          NL_TEST_DEF("Test escaping API round-tripping with itself", TestRoundTrip), NL_TEST_SENTINEL() };

} // namespace

int TestIniEscaping()
{
    nlTestSuite theSuite = { "IniEscaping tests", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestIniEscaping);
