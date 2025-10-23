/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      Unit tests for Base64 utility functions in src/lib/support/Base64.cpp
 */

#include <cstring>
#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/support/Base64.h>

using namespace chip;

TEST(TestBase64, EncodeDecodeBasic)
{
    const uint8_t in1[] = { 'f' };
    char out[8]         = {};
    uint8_t outb[8]     = {};

    // f -> Zg==
    uint16_t olen = Base64Encode(in1, 1, out);
    EXPECT_EQ(std::string(out, olen), std::string("Zg=="));

    uint16_t dlen = Base64Decode(out, olen, outb);
    EXPECT_EQ(dlen, 1u);
    EXPECT_EQ(outb[0], 'f');

    // fo -> Zm8=
    const uint8_t in2[] = { 'f', 'o' };
    olen                = Base64Encode(in2, 2, out);
    EXPECT_EQ(std::string(out, olen), std::string("Zm8="));

    dlen = Base64Decode(out, olen, outb);
    EXPECT_EQ(dlen, 2u);
    EXPECT_EQ(std::memcmp(outb, in2, 2), 0);

    // foo -> Zm9v
    const uint8_t in3[] = { 'f', 'o', 'o' };
    olen                = Base64Encode(in3, 3, out);
    EXPECT_EQ(std::string(out, olen), std::string("Zm9v"));

    dlen = Base64Decode(out, olen, outb);
    EXPECT_EQ(dlen, 3u);
    EXPECT_EQ(std::memcmp(outb, in3, 3), 0);
}

TEST(TestBase64, EncodeDecodeURL)
{
    const uint8_t in[] = { 'B', 'a', 's', 'e', '6', '4', 0x0f, 0xff, 0x12, 0x33, 0x34, 0x0f };
    std::vector<char> out(256);
    std::vector<uint8_t> dec(256);

    uint16_t olen = Base64URLEncode(in, sizeof(in), out.data());
    EXPECT_GT(olen, 0u);

    uint16_t dlen = Base64URLDecode(out.data(), olen, dec.data());
    EXPECT_EQ(dlen, static_cast<uint16_t>(sizeof(in)));
    EXPECT_EQ(std::memcmp(dec.data(), in, sizeof(in)), 0);
}

TEST(TestBase64, DecodeErrorCases)
{
    uint8_t outb[16];

    // odd length
    uint16_t dlen = Base64Decode("Z", 1, outb);
    EXPECT_EQ(dlen, UINT16_MAX);

    // invalid character
    dlen = Base64Decode("Zm9vY;", 6, outb);
    EXPECT_EQ(dlen, UINT16_MAX);

    // space inside (isgraph check)
    dlen = Base64Decode("Zm9 vYg", 7, outb);
    EXPECT_EQ(dlen, UINT16_MAX);
}

TEST(TestBase64, Encode32Decode32Chunking)
{
    // Create an input larger than kMaxConvert used in the implementation
    const size_t kMaxConvert = (UINT16_MAX / 4) * 3;
    const size_t bigLen      = kMaxConvert + 10; // force chunking

    std::vector<uint8_t> in(bigLen);
    for (size_t i = 0; i < bigLen; ++i)
        in[i] = static_cast<uint8_t>(i & 0xFF);

    // encode32
    std::vector<char> encoded(((bigLen + 2) / 3) * 4 + 4);
    uint32_t encLen = Base64Encode32(in.data(), static_cast<uint32_t>(bigLen), encoded.data());
    EXPECT_GT(encLen, 0u);

    // decode32
    std::vector<uint8_t> decoded(bigLen + 4);
    uint32_t decLen = Base64Decode32(encoded.data(), encLen, decoded.data());
    EXPECT_EQ(decLen, static_cast<uint32_t>(bigLen));
    EXPECT_EQ(std::memcmp(decoded.data(), in.data(), bigLen), 0);

    // Make an encoded string with an invalid char to force Decode32 to fail
    if (encLen >= 5)
    {
        encoded[2]       = ';'; // invalid
        uint32_t decFail = Base64Decode32(encoded.data(), encLen, decoded.data());
        EXPECT_EQ(decFail, UINT32_MAX);
    }
}
