/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <string>
#include <uriparser/Uri.h>

namespace chip {
namespace app {
namespace Clusters {

std::string extractTextRange(const UriTextRangeA & range);
std::string extractPath(const UriPathSegmentA * pathHead);

namespace PushAvStreamTransport {

class TestUriExtractionHelpers : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

// =================================
// Tests for extractTextRange
// =================================

TEST_F(TestUriExtractionHelpers, ExtractTextRange_ValidRange)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + strlen(text);

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "example");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_EmptyRange)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text; // Same pointer = empty range

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_NullFirst)
{
    UriTextRangeA range;
    range.first     = nullptr;
    range.afterLast = reinterpret_cast<const char *>(0x1000);

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_NullAfterLast)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = nullptr;

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_InvalidOrder)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text + 5; // After afterLast
    range.afterLast = text;     // Before first (invalid)

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_SingleChar)
{
    const char * text = "a";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + 1;

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "a");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_WithSpecialChars)
{
    const char * text = "path/to/resource?query=value#fragment";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + strlen(text);

    std::string result = extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "path/to/resource?query=value#fragment");
}

// =================================
// Tests for extractPath
// =================================

TEST_F(TestUriExtractionHelpers, ExtractPath_NullHead)
{
    std::string result = extractPath(nullptr);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SingleSegment)
{
    // Create a single path segment
    char segmentText[] = "api";
    UriPathSegmentA segment;
    segment.text.first     = segmentText;
    segment.text.afterLast = segmentText + strlen(segmentText);
    segment.next           = nullptr;

    std::string result = extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/api");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_MultipleSegments)
{
    // Create multiple path segments: /api/v1/resource
    char seg1Text[] = "api";
    char seg2Text[] = "v1";
    char seg3Text[] = "resource";

    UriPathSegmentA segment3;
    segment3.text.first     = seg3Text;
    segment3.text.afterLast = seg3Text + strlen(seg3Text);
    segment3.next           = nullptr;

    UriPathSegmentA segment2;
    segment2.text.first     = seg2Text;
    segment2.text.afterLast = seg2Text + strlen(seg2Text);
    segment2.next           = &segment3;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/api/v1/resource");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SegmentWithNullText)
{
    // Create a segment with null text pointers
    UriPathSegmentA segment;
    segment.text.first     = nullptr;
    segment.text.afterLast = nullptr;
    segment.next           = nullptr;

    std::string result = extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SegmentWithEmptyText)
{
    // Create a segment with empty text (first == afterLast)
    char emptyText[] = "";
    UriPathSegmentA segment;
    segment.text.first     = emptyText;
    segment.text.afterLast = emptyText;
    segment.next           = nullptr;

    std::string result = extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_MixedValidAndNullSegments)
{
    // Create segments where some have null text
    char seg1Text[] = "api";
    UriPathSegmentA segment2;
    segment2.text.first     = nullptr;
    segment2.text.afterLast = nullptr;
    segment2.next           = nullptr;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/api/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_ComplexPath)
{
    // Create a complex path: /users/123/posts/456/comments
    char seg1Text[] = "users";
    char seg2Text[] = "123";
    char seg3Text[] = "posts";
    char seg4Text[] = "456";
    char seg5Text[] = "comments";

    UriPathSegmentA segment5;
    segment5.text.first     = seg5Text;
    segment5.text.afterLast = seg5Text + strlen(seg5Text);
    segment5.next           = nullptr;

    UriPathSegmentA segment4;
    segment4.text.first     = seg4Text;
    segment4.text.afterLast = seg4Text + strlen(seg4Text);
    segment4.next           = &segment5;

    UriPathSegmentA segment3;
    segment3.text.first     = seg3Text;
    segment3.text.afterLast = seg3Text + strlen(seg3Text);
    segment3.next           = &segment4;

    UriPathSegmentA segment2;
    segment2.text.first     = seg2Text;
    segment2.text.afterLast = seg2Text + strlen(seg2Text);
    segment2.next           = &segment3;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/users/123/posts/456/comments");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_RealUrlWithTrailingSlash)
{
    // Test with a real URL parsed by uriparser
    const char * url = "https://example.com/";
    UriUriA uri;
    const char * errorPos;
    int result = uriParseSingleUriA(&uri, url, &errorPos);

    ASSERT_EQ(result, URI_SUCCESS);

    std::string path = extractPath(uri.pathHead);
    uriFreeUriMembersA(&uri);

    EXPECT_STREQ(path.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_RealUrlWithPath)
{
    // Test with a real URL with a path segment
    const char * url = "https://192.168.1.100:554/stream/";
    UriUriA uri;
    const char * errorPos;
    int result = uriParseSingleUriA(&uri, url, &errorPos);

    ASSERT_EQ(result, URI_SUCCESS);

    std::string path = extractPath(uri.pathHead);
    uriFreeUriMembersA(&uri);

    EXPECT_STREQ(path.c_str(), "/stream/");
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
