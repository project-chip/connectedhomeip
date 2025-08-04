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

#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <protocols/bdx/BdxUri.h>

using namespace ::chip;

namespace {

CharSpan ToSpan(const char * str)
{
    return CharSpan::fromCharString(str);
}

TEST(TestBdxUri, TestParseURI)
{
    NodeId nodeId;
    CharSpan file;

    const auto parse = [&](const char * uri) { return bdx::ParseURI(ToSpan(uri), nodeId, file); };

    // Happy path
    EXPECT_EQ(parse("bdx://1234567890ABCDEF/file_name"), CHIP_NO_ERROR);
    EXPECT_EQ(nodeId, 0x1234567890ABCDEFULL);
    EXPECT_TRUE(file.data_equal(ToSpan("file_name")));

    // File designator may contain slashes
    EXPECT_EQ(parse("bdx://1234567890ABCDEF/file/na/me"), CHIP_NO_ERROR);
    EXPECT_EQ(nodeId, 0x1234567890ABCDEFULL);
    EXPECT_TRUE(file.data_equal(ToSpan("file/na/me")));

    // Wrong scheme
    EXPECT_EQ(parse("bdy://1234567890ABCDEF/file_name"), CHIP_ERROR_INVALID_SCHEME_PREFIX);

    // Node ID contains non-hex digit
    EXPECT_EQ(parse("bdx://1234567890ABCDEG/file_name"), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // Node ID too short
    EXPECT_EQ(parse("bdx://1234567890ABCDE/file_name"), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // Node ID too long
    EXPECT_EQ(parse("bdx://1234567890ABCDEFEF/file_name"), CHIP_ERROR_MISSING_URI_SEPARATOR);

    // Node ID to non-operational
    EXPECT_EQ(parse("bdx://FFFFFFFFFFFFFFFF/file_name"), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // No file designator
    EXPECT_EQ(parse("bdx://1234567890ABCDEF/"), CHIP_ERROR_INVALID_STRING_LENGTH);
}

TEST(TestBdxUri, TestMakeURI)
{
    char uriBuffer[128];
    MutableCharSpan uri(uriBuffer);

    // Happy path
    EXPECT_EQ(bdx::MakeURI(0xFEDCBA0987654321ULL, ToSpan("file_name"), uri), CHIP_NO_ERROR);
    EXPECT_TRUE(uri.data_equal(ToSpan("bdx://FEDCBA0987654321/file_name")));

    // File designator with slashes
    uri = MutableCharSpan(uriBuffer);
    EXPECT_EQ(bdx::MakeURI(0xFEDCBA0987654321ULL, ToSpan("file/na/me"), uri), CHIP_NO_ERROR);
    EXPECT_TRUE(uri.data_equal(ToSpan("bdx://FEDCBA0987654321/file/na/me")));

    // Empty file designator
    uri = MutableCharSpan(uriBuffer);
    EXPECT_NE(bdx::MakeURI(0xFEDCBA0987654321ULL, CharSpan(), uri), CHIP_NO_ERROR);

    // Too small buffer
    uri = MutableCharSpan(uriBuffer, 31);
    EXPECT_NE(bdx::MakeURI(0xFEDCBA0987654321ULL, CharSpan(), uri), CHIP_NO_ERROR);
}

} // namespace
