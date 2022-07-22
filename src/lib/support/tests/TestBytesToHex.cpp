/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <stdio.h>
#include <string.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip::Encoding;

void TestBytesToHexNotNullTerminated(nlTestSuite * inSuite, void * inContext)
{
    // Uppercase
    {
        uint8_t src[]     = { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char dest2[18]    = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { 'F', 'E', 'D', 'C', 'B', 'A', '9', '8', '7', '6', '5', '4', '3', '2', '1', '0', '!', '@' };
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], sizeof(src), &dest[0], sizeof(src) * 2u, HexFlags::kUppercase) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite, BytesToUppercaseHexBuffer(&src[0], sizeof(src), &dest2[0], sizeof(src) * 2u) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest2[0], &expected[0], sizeof(expected)) == 0);
    }

    // Lowercase
    {
        uint8_t src[]     = { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char dest2[18]    = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { 'f', 'e', 'd', 'c', 'b', 'a', '9', '8', '7', '6', '5', '4', '3', '2', '1', '0', '!', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(&src[0], sizeof(src), &dest[0], sizeof(src) * 2u, HexFlags::kNone) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        // Test Alias
        NL_TEST_ASSERT(inSuite, BytesToLowercaseHexBuffer(&src[0], sizeof(src), &dest2[0], sizeof(src) * 2u) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest2[0], &expected[0], sizeof(expected)) == 0);
    }

    // Trivial: Zero size input
    {
        uint8_t src[]    = { 0x00 };
        char dest[2]     = { '!', '@' };
        char expected[2] = { '!', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(&src[0], 0, &dest[0], sizeof(src) * 2u, HexFlags::kNone) == CHIP_NO_ERROR);
        // Nothing should have been touched.
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

    // Trivial: Zero size input with null buffer
    {
        char dest[2]     = { '!', '@' };
        char expected[2] = { '!', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, &dest[0], sizeof(dest), HexFlags::kNone) == CHIP_NO_ERROR);
        // Nothing should have been touched.
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, nullptr, 0, HexFlags::kNone) == CHIP_NO_ERROR);
        // Nothing should have been touched.
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, nullptr, 1, HexFlags::kNone) == CHIP_ERROR_INVALID_ARGUMENT);
    }
}

void TestBytesToHexNullTerminated(nlTestSuite * inSuite, void * inContext)
{
    // Uppercase
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char dest2[18]    = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0', '@' };
        NL_TEST_ASSERT(inSuite, ((sizeof(src) * 2u) + 1u) <= sizeof(dest));
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], sizeof(src), &dest[0], (sizeof(src) * 2u) + 1, HexFlags::kUppercaseAndNullTerminate) ==
                           CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        // Test Alias
        CHIP_ERROR retval = BytesToUppercaseHexString(&src[0], sizeof(src), &dest2[0], sizeof(dest2));
        printf("retval=%" CHIP_ERROR_FORMAT "\n", retval.Format());
        NL_TEST_ASSERT(inSuite, retval == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest2[0], &expected[0], sizeof(expected)) == 0);
    }

    // Lowercase
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char dest2[18]    = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', '\0', '@' };
        NL_TEST_ASSERT(inSuite, ((sizeof(src) * 2u) + 1u) <= sizeof(dest));
        NL_TEST_ASSERT(
            inSuite, BytesToHex(&src[0], sizeof(src), &dest[0], (sizeof(src) * 2u) + 1, HexFlags::kNullTerminate) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        // Test Alias
        NL_TEST_ASSERT(inSuite, BytesToLowercaseHexString(&src[0], sizeof(src), &dest2[0], sizeof(dest2)) == CHIP_NO_ERROR);
        printf("->%s\n", dest2);
        NL_TEST_ASSERT(inSuite, memcmp(&dest2[0], &expected[0], sizeof(expected)) == 0);
    }

    // Trivial: Zero size input
    {
        uint8_t src[]    = { 0x00 };
        char dest[2]     = { '!', '@' };
        char expected[2] = { '\0', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(&src[0], 0, &dest[0], sizeof(dest), HexFlags::kNullTerminate) == CHIP_NO_ERROR);
        // Expect nul termination
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

    // Trivial: Zero size input with null buffer
    {
        char dest[2]     = { '!', '@' };
        char expected[2] = { '\0', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, &dest[0], sizeof(dest), HexFlags::kNullTerminate) == CHIP_NO_ERROR);
        // Nothing should have been touched.
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, nullptr, 0, HexFlags::kNullTerminate) == CHIP_ERROR_BUFFER_TOO_SMALL);

        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, &dest[0], 1, HexFlags::kNullTerminate) == CHIP_NO_ERROR);
        // Nothing should have been touched.
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite, BytesToHex(nullptr, 0, nullptr, 1, HexFlags::kNullTerminate) == CHIP_ERROR_INVALID_ARGUMENT);
    }
}

void TestBytesToHexErrors(nlTestSuite * inSuite, void * inContext)
{
    // NULL destination
    {
        uint8_t src[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char * dest   = nullptr;
        NL_TEST_ASSERT(inSuite, BytesToHex(&src[0], 0, dest, sizeof(src) * 2u, HexFlags::kNone) == CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Destination buffer too small for non-null-terminated
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        NL_TEST_ASSERT(inSuite, ((sizeof(src) * 2u) + 1u) <= sizeof(dest));
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], sizeof(src), &dest[0], (sizeof(src) * 2u) - 1, HexFlags::kNone) ==
                           CHIP_ERROR_BUFFER_TOO_SMALL);
        // Ensure output not touched
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

    // Destination buffer too small for null-terminated
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        NL_TEST_ASSERT(inSuite, ((sizeof(src) * 2u) + 1u) <= sizeof(dest));
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], sizeof(src), &dest[0], (sizeof(src) * 2u), HexFlags::kNullTerminate) ==
                           CHIP_ERROR_BUFFER_TOO_SMALL);
        // Ensure output not touched
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

    // Writing in a larger buffer is fine, bytes past the nul terminator (when requested) are untouched.
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', '\0', '@' };
        NL_TEST_ASSERT(inSuite, ((sizeof(src) * 2u) + 1u) < sizeof(dest));
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], sizeof(src), &dest[0], sizeof(dest), HexFlags::kNullTerminate) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

    // Source size that would not fit in any output using size_t
    {
        uint8_t src[]     = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        char dest[18]     = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18] = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], SIZE_MAX / 2u, &dest[0], sizeof(dest), HexFlags::kNullTerminate) ==
                           CHIP_ERROR_BUFFER_TOO_SMALL);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);

        NL_TEST_ASSERT(inSuite,
                       BytesToHex(&src[0], SIZE_MAX / 2u, &dest[0], sizeof(dest), HexFlags::kNone) == CHIP_ERROR_BUFFER_TOO_SMALL);
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }
}

void TestBytesToHexUint64(nlTestSuite * inSuite, void * inContext)
{
    // Different values in each byte and each nibble should let us know if the conversion is correct.
    uint64_t test     = 0x0123456789ABCDEF;
    uint32_t test32_0 = 0x01234567;
    uint32_t test32_1 = 0x89ABCDEF;
    uint16_t test16_0 = 0x0123;
    uint16_t test16_1 = 0x4567;
    uint16_t test16_2 = 0x89AB;
    uint16_t test16_3 = 0xCDEF;
    char buf[17];
    char upperExpected[] = "0123456789ABCDEF";
    char lowerExpected[] = "0123456789abcdef";

    // Lower case - uint64_t.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf), HexFlags::kNone) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, lowerExpected, strlen(lowerExpected)) == 0);
    // No null termination.
    NL_TEST_ASSERT(inSuite, buf[16] == 1);

    // Lower case - uint32_t.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint32ToHex(test32_0, buf, sizeof(uint32_t) * 2, HexFlags::kNone) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   Uint32ToHex(test32_1, &buf[sizeof(uint32_t) * 2], sizeof(uint32_t) * 2, HexFlags::kNone) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, lowerExpected, strlen(lowerExpected)) == 0);
    // No null termination.
    NL_TEST_ASSERT(inSuite, buf[16] == 1);

    // Upper case - uint64_t.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf), HexFlags::kUppercase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, upperExpected, strlen(upperExpected)) == 0);
    // No null termination.
    NL_TEST_ASSERT(inSuite, buf[16] == 1);

    // Upper case - uint16_t.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint16ToHex(test16_0, buf, sizeof(uint16_t) * 2, HexFlags::kUppercase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   Uint16ToHex(test16_1, &buf[sizeof(uint16_t) * 2 * 1], sizeof(uint16_t) * 2, HexFlags::kUppercase) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   Uint16ToHex(test16_2, &buf[sizeof(uint16_t) * 2 * 2], sizeof(uint16_t) * 2, HexFlags::kUppercase) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   Uint16ToHex(test16_3, &buf[sizeof(uint16_t) * 2 * 3], sizeof(uint16_t) * 2, HexFlags::kUppercase) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, upperExpected, strlen(upperExpected)) == 0);
    // No null termination.
    NL_TEST_ASSERT(inSuite, buf[16] == 1);

    // Lower case with null termination.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf), HexFlags::kNullTerminate) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, lowerExpected, sizeof(lowerExpected)) == 0);

    // Upper case with null termination.
    memset(buf, 1, sizeof(buf));
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf), HexFlags::kUppercaseAndNullTerminate) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(buf, upperExpected, sizeof(upperExpected)) == 0);

    // Too small buffer
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf) - 2, HexFlags::kNone) == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf) - 2, HexFlags::kUppercase) == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, Uint64ToHex(test, buf, sizeof(buf) - 1, HexFlags::kNullTerminate) == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite,
                   Uint64ToHex(test, buf, sizeof(buf) - 1, HexFlags::kUppercaseAndNullTerminate) == CHIP_ERROR_BUFFER_TOO_SMALL);
}

void TestHexToBytesAndUint(nlTestSuite * inSuite, void * inContext)
{
    // Different values in each byte and each nibble should let us know if the conversion is correct.
    char hexInLowercase[]      = "0123456789abcdef";
    char hexInUppercase[]      = "0123456789ABCDEF";
    char hexInUppercase32[]    = "6789ABCD";
    char hexInUppercase16[]    = "D7AB";
    uint8_t bytesOutExpected[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    uint64_t test64OutExpected = 0x0123456789ABCDEF;
    uint32_t test32OutExpected = 0x6789ABCD;
    uint16_t test16OutExpected = 0xD7AB;
    uint64_t test64Out;
    uint32_t test32Out;
    uint16_t test16Out;
    uint8_t buf[8];

    // Lower case - bytes.
    memset(buf, 0, sizeof(buf));
    NL_TEST_ASSERT(inSuite, HexToBytes(hexInLowercase, strlen(hexInLowercase), buf, sizeof(buf)) == sizeof(buf));
    NL_TEST_ASSERT(inSuite, memcmp(buf, bytesOutExpected, sizeof(buf)) == 0);

    // Upper case - bytes.
    memset(buf, 0, sizeof(buf));
    NL_TEST_ASSERT(inSuite, HexToBytes(hexInUppercase, strlen(hexInUppercase), buf, sizeof(buf)) == sizeof(buf));
    NL_TEST_ASSERT(inSuite, memcmp(buf, bytesOutExpected, sizeof(buf)) == 0);

    // Lower case - uint64_t.
    test64Out = 0;
    NL_TEST_ASSERT(inSuite, UppercaseHexToUint64(hexInLowercase, strlen(hexInLowercase), test64Out) == 0);

    // Upper case - uint64_t.
    test64Out = 0;
    NL_TEST_ASSERT(inSuite, UppercaseHexToUint64(hexInUppercase, strlen(hexInUppercase), test64Out) == sizeof(uint64_t));
    NL_TEST_ASSERT(inSuite, test64Out == test64OutExpected);

    // Upper case - uint32_t.
    test32Out = 0;
    NL_TEST_ASSERT(inSuite, UppercaseHexToUint32(hexInUppercase32, strlen(hexInUppercase32), test32Out) == sizeof(uint32_t));
    NL_TEST_ASSERT(inSuite, test32Out == test32OutExpected);

    // Upper case - uint16_t.
    test16Out = 0;
    NL_TEST_ASSERT(inSuite, UppercaseHexToUint16(hexInUppercase16, strlen(hexInUppercase16), test16Out) == sizeof(uint16_t));
    NL_TEST_ASSERT(inSuite, test16Out == test16OutExpected);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestBytesToHexNotNullTerminated", TestBytesToHexNotNullTerminated), //
    NL_TEST_DEF("TestBytesToHexNullTerminated", TestBytesToHexNullTerminated),       //
    NL_TEST_DEF("TestBytesToHexErrors", TestBytesToHexErrors),                       //
    NL_TEST_DEF("TestBytesToHexUint64", TestBytesToHexUint64),                       //
    NL_TEST_DEF("TestHexToBytesAndUint", TestHexToBytesAndUint),                     //
    NL_TEST_SENTINEL()                                                               //
};

} // namespace

int TestBytesToHex(void)
{
    nlTestSuite theSuite = { "BytesToHex", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBytesToHex)
