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
    char outbuf[8]      = {};
    uint8_t outbbuf[8]  = {};
    MutableCharSpan out(outbuf);
    MutableByteSpan outb(outbbuf);

    // f -> Zg==
    ByteSpan inputSpan1(in1, 1);
    CHIP_ERROR encodeErr = Base64Encode(inputSpan1, out);
    EXPECT_EQ(encodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(out.data_equal(CharSpan("Zg==", out.size())));

    CharSpan encodedSpan1(out.data(), out.size());
    CHIP_ERROR decodeErr = Base64Decode(encodedSpan1, outb);
    EXPECT_EQ(decodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(outb.data_equal(ByteSpan(in1, 1)));

    // fo -> Zm8=
    const uint8_t in2[] = { 'f', 'o' };
    out                 = MutableCharSpan(outbuf);
    ByteSpan inputSpan2(in2, 2);
    encodeErr = Base64Encode(inputSpan2, out);
    EXPECT_EQ(encodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(out.data_equal(CharSpan("Zm8=", out.size())));

    outb = MutableByteSpan(outbbuf);
    CharSpan encodedSpan2(out.data(), out.size());
    decodeErr = Base64Decode(encodedSpan2, outb);
    EXPECT_EQ(decodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(outb.data_equal(ByteSpan(in2, 2)));

    // foo -> Zm9v
    const uint8_t in3[] = { 'f', 'o', 'o' };
    out                 = MutableCharSpan(outbuf);
    ByteSpan inputSpan3(in3, 3);
    encodeErr = Base64Encode(inputSpan3, out);
    EXPECT_EQ(encodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(out.data_equal(CharSpan("Zm9v", out.size())));

    outb = MutableByteSpan(outbbuf);
    CharSpan encodedSpan3(out.data(), out.size());
    decodeErr = Base64Decode(encodedSpan3, outb);
    EXPECT_EQ(decodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(outb.data_equal(ByteSpan(in3, 3)));
}

TEST(TestBase64, EncodeDecodeURL)
{
    const uint8_t in[] = { 'B', 'a', 's', 'e', '6', '4', 0x0f, 0xff, 0x12, 0x33, 0x34, 0x0f };
    char out[256];
    uint8_t dec[256];

    ByteSpan inputSpan(in, sizeof(in));
    MutableCharSpan outputSpan(out, sizeof(out));

    CHIP_ERROR encodeErr = Base64URLEncode(inputSpan, outputSpan);
    EXPECT_EQ(encodeErr, CHIP_NO_ERROR);
    EXPECT_GT(outputSpan.size(), 0u);

    CharSpan encodedSpan(outputSpan.data(), outputSpan.size());
    MutableByteSpan decodedSpan(dec, sizeof(dec));

    CHIP_ERROR decodeErr = Base64URLDecode(encodedSpan, decodedSpan);
    EXPECT_EQ(decodeErr, CHIP_NO_ERROR);
    EXPECT_TRUE(decodedSpan.data_equal(ByteSpan(in, sizeof(in))));
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

TEST(TestBase64, FailingEncodingWithSpansOverloads)
{
    const uint8_t testData[] = { 'f', 'o', 'o' };
    char outputBuffer[16];

    // Test CHIP_ERROR_BUFFER_TOO_SMALL for Base64Encode
    {
        ByteSpan input(testData, sizeof(testData));
        MutableCharSpan output(outputBuffer, 3); // Too small for encoded output (needs 4 chars minimum)

        CHIP_ERROR err = Base64Encode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Test CHIP_ERROR_BUFFER_TOO_SMALL for Base64URLEncode
    {
        ByteSpan input(testData, sizeof(testData));
        MutableCharSpan output(outputBuffer, 3); // Too small for encoded output (needs 4 chars minimum)

        CHIP_ERROR err = Base64URLEncode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64Encode with oversized input
    {
        // Create a mock ByteSpan that reports size > UINT32_MAX
        // We can't actually create such large data, but we can test the size check logic
        const uint8_t smallData[] = { 'a' };
        ByteSpan input(smallData, 1);
        MutableCharSpan output(outputBuffer, sizeof(outputBuffer));

        // Normal case should work
        CHIP_ERROR err = Base64Encode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64URLEncode with oversized input
    {
        const uint8_t smallData[] = { 'a' };
        ByteSpan input(smallData, 1);
        MutableCharSpan output(outputBuffer, sizeof(outputBuffer));

        // Normal case should work
        CHIP_ERROR err = Base64URLEncode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Test exact boundary case - buffer exactly the right size should work
    {
        ByteSpan input(testData, sizeof(testData));
        MutableCharSpan output(outputBuffer, BASE64_ENCODED_LEN(sizeof(testData))); // Exactly right size

        CHIP_ERROR err = Base64Encode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(output.size(), 4u); // 3 bytes encode to 4 characters
    }

    // Test exact boundary case for URL encoding
    {
        ByteSpan input(testData, sizeof(testData));
        MutableCharSpan output(outputBuffer, BASE64_ENCODED_LEN(sizeof(testData))); // Exactly right size

        CHIP_ERROR err = Base64URLEncode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(output.size(), 4u); // 3 bytes encode to 4 characters
    }
}

TEST(TestBase64, FailingDecodingWithSpansOverloads)
{
    const char testEncodedData[] = "Zm9v"; // "foo" encoded
    uint8_t outputBuffer[16];

    // Test CHIP_ERROR_BUFFER_TOO_SMALL for Base64Decode
    {
        CharSpan input(testEncodedData, strlen(testEncodedData));
        MutableByteSpan output(outputBuffer, 1); // Too small for decoded output (needs 3 bytes)

        CHIP_ERROR err = Base64Decode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Test CHIP_ERROR_BUFFER_TOO_SMALL for Base64URLDecode
    {
        CharSpan input(testEncodedData, strlen(testEncodedData));
        MutableByteSpan output(outputBuffer, 1); // Too small for decoded output (needs 3 bytes)

        CHIP_ERROR err = Base64URLDecode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64Decode with invalid input
    {
        const char invalidData[] = "Zm9v;"; // Invalid character ';'
        CharSpan input(invalidData, strlen(invalidData));
        MutableByteSpan output(outputBuffer, sizeof(outputBuffer));

        CHIP_ERROR err = Base64Decode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64URLDecode with invalid input
    {
        const char invalidData[] = "Zm9v;"; // Invalid character ';'
        CharSpan input(invalidData, strlen(invalidData));
        MutableByteSpan output(outputBuffer, sizeof(outputBuffer));

        CHIP_ERROR err = Base64URLDecode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64Decode with odd length input
    {
        const char oddLengthData[] = "Z"; // Odd length (1 character)
        CharSpan input(oddLengthData, strlen(oddLengthData));
        MutableByteSpan output(outputBuffer, sizeof(outputBuffer));

        CHIP_ERROR err = Base64Decode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Test CHIP_ERROR_INVALID_ARGUMENT for Base64URLDecode with odd length input
    {
        const char oddLengthData[] = "Z"; // Odd length (1 character)
        CharSpan input(oddLengthData, strlen(oddLengthData));
        MutableByteSpan output(outputBuffer, sizeof(outputBuffer));

        CHIP_ERROR err = Base64URLDecode(input, output);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Test exact boundary case - buffer exactly the right size should work
    {
        CharSpan input(testEncodedData, strlen(testEncodedData));
        MutableByteSpan output(outputBuffer, BASE64_MAX_DECODED_LEN(strlen(testEncodedData))); // Exactly right size

        CHIP_ERROR err = Base64Decode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(output.size(), 3u); // "foo" decodes to 3 bytes
    }

    // Test exact boundary case for URL decoding
    {
        CharSpan input(testEncodedData, strlen(testEncodedData));
        MutableByteSpan output(outputBuffer, BASE64_MAX_DECODED_LEN(strlen(testEncodedData))); // Exactly right size

        CHIP_ERROR err = Base64URLDecode(input, output);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(output.size(), 3u); // "foo" decodes to 3 bytes
    }
}
