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

#include <pw_unit_test/framework.h>

#include <lib/support/Base85.h>
#include <lib/support/Span.h>

#include <string.h>

using namespace chip;

namespace {

struct TestVector
{
    ByteSpan input;
    const char * encoded;
};

// Helper: create a ByteSpan from a string literal (excluding the null terminator).
// This lets us write test vector inputs using hex escape sequences that
// work in C++ (BYTES("\x01\x02\x03")) and Python (b"\x01\x02\x03").
// Note: All bytes should be hex-escaped since C++ \x can consume more than two digits.
#define BYTES(literal) ByteSpan(reinterpret_cast<const uint8_t *>(literal), sizeof("" literal) - 1)

// To verify: python3 -c 'import base64; print(base64.b85encode(b"<input>").decode())'
const TestVector kTestVectors[] = {
    { BYTES(""), "" },
    { BYTES("\x01"), "0R" },
    { BYTES("\xca\xfe"), "%Ki" },
    { BYTES("\xde\xad\xbe"), "-mSg" },
    { BYTES("\xde\xad\xbe\xef"), "-mSjx" },
    { BYTES("\x00\x00\x00\x00"), "00000" },
    { BYTES("\xff\xff\xff\xff"), "|NsC0" },
    { BYTES("\x01\x02\x03\x04\x05"), "0RjUA1p" },
    { BYTES("\x48\x65\x6c\x6c\x6f\x21\x21\x21"), "NM&qnZy_Ne" },
    { BYTES("\x49\x92\xdb\x53\xe5\xf6\xbf\x40\x9c\xd1"), "Ns`-B<@UcooY4" },
    { BYTES("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13"), "009C61O)~M2nh-c3=Iws5D^j+" },
    { BYTES("\x00\x2f\xdc\x0a\x6e\x3d\x4d\xb0\x02\x4c\x04\xee\xcf\xd5\xa3\xfc\xdf\x93\xa4\xdd\xef\x51\xa5\xbe\xff\x0f\x8a"),
      "059AMZaqz~0!#$%&()*+-;<=>?@^_`{||}" },
};

TEST(TestBase85, EncodeTestVectors)
{
    for (const auto & tv : kTestVectors)
    {
        size_t expectedLen = strlen(tv.encoded);
        EXPECT_EQ(Base85EncodedLength(tv.input.size()), expectedLen);

        char output[40];
        ASSERT_EQ(BytesToBase85(tv.input.data(), tv.input.size(), output, sizeof(output)), CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(output, tv.encoded, expectedLen), 0);
    }
}

TEST(TestBase85, DecodeTestVectors)
{
    for (const auto & tv : kTestVectors)
    {
        size_t encodedLen = strlen(tv.encoded);
        EXPECT_EQ(Base85DecodedLength(encodedLen), tv.input.size());

        // Decode in place
        uint8_t buf[40];
        memcpy(buf, tv.encoded, encodedLen);
        ASSERT_EQ(Base85ToBytes(reinterpret_cast<const char *>(buf), encodedLen, buf, sizeof(buf)), CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(buf, tv.input.data(), tv.input.size()), 0);
    }
}

TEST(TestBase85, EncodeBufferTooSmall)
{
    const uint8_t input[4] = { 1, 2, 3, 4 };
    char output[4]; // needs 5
    EXPECT_EQ(BytesToBase85(input, sizeof(input), output, sizeof(output)), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST(TestBase85, DecodeInvalidCharacters)
{
    uint8_t decoded[4];
    EXPECT_EQ(Base85ToBytes("0 000", 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(Base85ToBytes("00\\00", 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(Base85ToBytes("123\x7F_", 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestBase85, DecodeNonCanonical)
{
    uint8_t decoded[8];
    // A remainder group of 1 "digit" would be pure padding that does not encode a complete byte
    EXPECT_EQ(Base85ToBytes("0", 1, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(Base85ToBytes("AAAAA0", 6, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    // "0S" naively decodes to "\x01", but the canonical encoding is "0R"
    EXPECT_EQ(Base85ToBytes("0S", 2, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestBase85, DecodeOverflow)
{
    uint8_t decoded[4];
    // "~~~~~" decodes to 85^5 - 1 = 4,437,053,124 which exceeds UINT32_MAX
    EXPECT_EQ(Base85ToBytes("~~~~~", 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    // "|NsC1" is one past the maximum valid encoding "|NsC0" (0xFFFFFFFF)
    EXPECT_EQ(Base85ToBytes("|NsC1", 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestBase85, NullArguments)
{
    char encoded[8];
    uint8_t decoded[8];
    // null src with zero size is OK
    EXPECT_EQ(BytesToBase85(nullptr, 0, encoded, sizeof(encoded)), CHIP_NO_ERROR);
    EXPECT_EQ(Base85ToBytes(nullptr, 0, decoded, sizeof(decoded)), CHIP_NO_ERROR);
    // null src with nonzero size is an error
    EXPECT_EQ(BytesToBase85(nullptr, 1, encoded, sizeof(encoded)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(Base85ToBytes(nullptr, 5, decoded, sizeof(decoded)), CHIP_ERROR_INVALID_ARGUMENT);
    // null dest is always an error
    EXPECT_EQ(BytesToBase85(decoded, 0, nullptr, 0), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(Base85ToBytes(encoded, 0, nullptr, 0), CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
