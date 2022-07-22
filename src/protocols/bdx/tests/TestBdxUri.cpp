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
#include <lib/support/UnitTestRegistration.h>
#include <protocols/bdx/BdxUri.h>

#include <cstring>
#include <nlunit-test.h>

using namespace ::chip;

namespace {

CharSpan ToSpan(const char * str)
{
    return CharSpan::fromCharString(str);
}

void TestParseURI(nlTestSuite * inSuite, void * inContext)
{
    NodeId nodeId;
    CharSpan file;

    const auto parse = [&](const char * uri) { return bdx::ParseURI(ToSpan(uri), nodeId, file); };

    // Happy path
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDEF/file_name") == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, nodeId == 0x1234567890ABCDEFULL);
    NL_TEST_ASSERT(inSuite, file.data_equal(ToSpan("file_name")));

    // File designator may contain slashes
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDEF/file/na/me") == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, nodeId == 0x1234567890ABCDEFULL);
    NL_TEST_ASSERT(inSuite, file.data_equal(ToSpan("file/na/me")));

    // Wrong scheme
    NL_TEST_ASSERT(inSuite, parse("bdy://1234567890ABCDEF/file_name") == CHIP_ERROR_INVALID_SCHEME_PREFIX);

    // Node ID contains non-hex digit
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDEG/file_name") == CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // Node ID too short
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDE/file_name") == CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // Node ID too long
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDEFEF/file_name") == CHIP_ERROR_MISSING_URI_SEPARATOR);

    // Node ID to non-operational
    NL_TEST_ASSERT(inSuite, parse("bdx://FFFFFFFFFFFFFFFF/file_name") == CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // No file designator
    NL_TEST_ASSERT(inSuite, parse("bdx://1234567890ABCDEF/") == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestMakeURI(nlTestSuite * inSuite, void * inContext)
{
    char uriBuffer[128];
    MutableCharSpan uri(uriBuffer);

    // Happy path
    NL_TEST_ASSERT(inSuite, bdx::MakeURI(0xFEDCBA0987654321ULL, ToSpan("file_name"), uri) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, uri.data_equal(ToSpan("bdx://FEDCBA0987654321/file_name")));

    // File designator with slashes
    uri = MutableCharSpan(uriBuffer);
    NL_TEST_ASSERT(inSuite, bdx::MakeURI(0xFEDCBA0987654321ULL, ToSpan("file/na/me"), uri) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, uri.data_equal(ToSpan("bdx://FEDCBA0987654321/file/na/me")));

    // Empty file designator
    uri = MutableCharSpan(uriBuffer);
    NL_TEST_ASSERT(inSuite, bdx::MakeURI(0xFEDCBA0987654321ULL, CharSpan(), uri) != CHIP_NO_ERROR);

    // Too small buffer
    uri = MutableCharSpan(uriBuffer, 31);
    NL_TEST_ASSERT(inSuite, bdx::MakeURI(0xFEDCBA0987654321ULL, CharSpan(), uri) != CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestParseURI", TestParseURI),
    NL_TEST_DEF("TestMakeURI", TestMakeURI),
    NL_TEST_SENTINEL()
};
// clang-format on

nlTestSuite sSuite = { "Test BDX URI", &sTests[0], nullptr, nullptr };
} // namespace

int TestBdxUri()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBdxUri)
