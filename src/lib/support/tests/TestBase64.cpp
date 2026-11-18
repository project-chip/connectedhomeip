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

#include <lib/support/Base64.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <cstdint>
#include <cstring>

using namespace chip;

namespace {

struct TestVector
{
    ByteSpan raw;
    const char * encoded;
    bool isUnpadded = false;
};

#define BYTES(literal) ByteSpan(reinterpret_cast<const uint8_t *>(literal), sizeof("" literal) - 1)

const TestVector kValidVectors[] = {
    { BYTES(""), "" },
    { BYTES("f"), "Zg==" },
    { BYTES("fo"), "Zm8=" },
    { BYTES("foo"), "Zm9v" },
    { BYTES("foob"), "Zm9vYg==" },
    { BYTES("fooba"), "Zm9vYmE=" },
    { BYTES("foobar"), "Zm9vYmFy" },
    { BYTES("M"), "TQ", true },
    { BYTES("Ma"), "TWE", true },
    { BYTES("f"), "Zg", true },
    { BYTES("fo"), "Zm8", true },
    { BYTES("foob"), "Zm9vYg", true },
    { BYTES("fooba"), "Zm9vYmE", true },
    { BYTES("Base64\x0f\xef\x31\x32\x33\x34\x0f\xff"), "QmFzZTY0D+8xMjM0D/8=" },
};

const char * kInvalidInputs[] = {
    "Z",
    "A",
    "Zm9vY",
    "AA!A",
    "AB!A",
    "Zm9vAA!A",
    "AB!",
    "!AAA",
    "A!AA",
    "AAA!",
    "=",
    "==",
    "===",
    "====",
    "A===",
    "AA=A",
    "AA==Zg",
    "Zg=",
    "Zm9v=",
    "Z\x019vYmFy",
    "Zm9vY;",
    "Zm9 vYg",
    "Zm9v ",
    "QmFzZTY0D-8xMjM0D_8=",
};

TEST(TestBase64, EncodeValidVectors)
{
    for (const auto & tv : kValidVectors)
    {
        if (tv.isUnpadded)
        {
            continue;
        }

        const size_t expectedLen = strlen(tv.encoded);
        EXPECT_EQ(BASE64_ENCODED_LEN(tv.raw.size()), expectedLen);

        char out16[64];
        ASSERT_GE(sizeof(out16), expectedLen);
        uint16_t len16 = Base64Encode(tv.raw.data(), static_cast<uint16_t>(tv.raw.size()), out16);
        ASSERT_EQ(len16, expectedLen);
        EXPECT_EQ(memcmp(out16, tv.encoded, expectedLen), 0);

        char out32[64];
        ASSERT_GE(sizeof(out32), expectedLen);
        uint32_t len32 = Base64Encode32(tv.raw.data(), static_cast<uint32_t>(tv.raw.size()), out32);
        ASSERT_EQ(len32, expectedLen);
        EXPECT_EQ(memcmp(out32, tv.encoded, expectedLen), 0);
    }
}

TEST(TestBase64, DecodeValidVectors)
{
    for (const auto & tv : kValidVectors)
    {
        const size_t encodedLen = strlen(tv.encoded);
        EXPECT_GE(BASE64_MAX_DECODED_LEN(encodedLen), tv.raw.size());

        uint8_t out16[64];
        uint16_t len16 = Base64Decode(tv.encoded, static_cast<uint16_t>(encodedLen), out16);
        ASSERT_EQ(len16, tv.raw.size());
        if (len16 > 0)
        {
            EXPECT_EQ(memcmp(out16, tv.raw.data(), len16), 0);
        }

        uint8_t out32[64];
        uint32_t len32 = Base64Decode32(tv.encoded, static_cast<uint32_t>(encodedLen), out32);
        ASSERT_EQ(len32, tv.raw.size());
        if (len32 > 0)
        {
            EXPECT_EQ(memcmp(out32, tv.raw.data(), len32), 0);
        }

        // Verify in-place decoding (out == in).
        uint8_t inPlace16[64];
        ASSERT_GE(sizeof(inPlace16), encodedLen);
        memcpy(inPlace16, tv.encoded, encodedLen);
        uint16_t inPlaceLen16 =
            Base64Decode(reinterpret_cast<const char *>(inPlace16), static_cast<uint16_t>(encodedLen), inPlace16);
        ASSERT_EQ(inPlaceLen16, tv.raw.size());
        if (inPlaceLen16 > 0)
        {
            EXPECT_EQ(memcmp(inPlace16, tv.raw.data(), inPlaceLen16), 0);
        }

        uint8_t inPlace32[64];
        ASSERT_GE(sizeof(inPlace32), encodedLen);
        memcpy(inPlace32, tv.encoded, encodedLen);
        uint32_t inPlaceLen32 =
            Base64Decode32(reinterpret_cast<const char *>(inPlace32), static_cast<uint32_t>(encodedLen), inPlace32);
        ASSERT_EQ(inPlaceLen32, tv.raw.size());
        if (inPlaceLen32 > 0)
        {
            EXPECT_EQ(memcmp(inPlace32, tv.raw.data(), inPlaceLen32), 0);
        }
    }
}

TEST(TestBase64, DecodeInvalidInputs)
{
    for (const char * input : kInvalidInputs)
    {
        const size_t inputLen = strlen(input);
        uint8_t out16[64];
        EXPECT_EQ(Base64Decode(input, static_cast<uint16_t>(inputLen), out16), UINT16_MAX);

        uint8_t out32[64];
        EXPECT_EQ(Base64Decode32(input, static_cast<uint32_t>(inputLen), out32), UINT32_MAX);
    }
}

TEST(TestBase64, UrlSafeRoundTrip)
{
    const ByteSpan raw       = BYTES("Base64\x0f\xef\x31\x32\x33\x34\x0f\xff");
    const char expectedUrl[] = "QmFzZTY0D-8xMjM0D_8=";
    const size_t expectedLen = strlen(expectedUrl);

    char encoded[64];
    uint16_t encodedLen = Base64URLEncode(raw.data(), static_cast<uint16_t>(raw.size()), encoded);
    ASSERT_EQ(encodedLen, expectedLen);
    EXPECT_EQ(memcmp(encoded, expectedUrl, expectedLen), 0);

    uint8_t decoded[64];
    uint16_t decodedLen = Base64URLDecode(encoded, encodedLen, decoded);
    ASSERT_EQ(decodedLen, raw.size());
    EXPECT_EQ(memcmp(decoded, raw.data(), decodedLen), 0);
}

} // namespace
