/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <string>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CharSpanToStdString.h>
#include <lib/support/Span.h>

using namespace chip;

TEST(TestCharSpanToStdString, EmptyDefaultSpanProducesEmptyString)
{
    // Default-constructed CharSpan has data()==nullptr, size()==0. The naive
    // std::string(span.data(), span.size()) on this is undefined behavior; the helper must
    // return an empty std::string instead.
    CharSpan defaultSpan{};
    std::string result = CharSpanToStdString(defaultSpan);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.size(), 0u);
}

TEST(TestCharSpanToStdString, EmptyNonNullSpanProducesEmptyString)
{
    // A CharSpan with a non-null pointer but size()==0 is also valid input; the helper
    // returns an empty string in that case too (the size==0 short-circuit applies regardless
    // of pointer-nullness).
    const char buf[] = "ignored";
    CharSpan empty(buf, 0);
    std::string result = CharSpanToStdString(empty);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.size(), 0u);
}

TEST(TestCharSpanToStdString, NonEmptySpanIsCopiedVerbatim)
{
    const char source[] = "device-supplied-debug";
    CharSpan span(source, sizeof(source) - 1);
    std::string result = CharSpanToStdString(span);
    EXPECT_EQ(result, source);
    EXPECT_EQ(result.size(), sizeof(source) - 1);
}

TEST(TestCharSpanToStdString, EmbeddedNulIsPreserved)
{
    // CharSpan is byte-counted, not nul-terminated. The helper must copy the full size,
    // including any embedded nul bytes, rather than treating them as terminators.
    const char source[] = { 'a', 'b', '\0', 'c', 'd' };
    CharSpan span(source, sizeof(source));
    std::string result = CharSpanToStdString(span);
    EXPECT_EQ(result.size(), sizeof(source));
    EXPECT_EQ(std::memcmp(result.data(), source, sizeof(source)), 0);
}

TEST(TestCharSpanToStdString, ResultIsIndependentOfSourceSpan)
{
    // The helper returns an owning std::string. Mutating the source buffer or letting it go
    // out of scope must not affect the returned string.
    char buf[]         = "original";
    std::string result = CharSpanToStdString(CharSpan(buf, sizeof(buf) - 1));
    buf[0]             = 'X'; // modify source after copy
    EXPECT_EQ(result, "original");
}
