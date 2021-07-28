/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <support/BytesToHex.h>
#include <support/UnitTestRegistration.h>

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
        printf("retval=%" CHIP_ERROR_FORMAT "\n", chip::ChipError::FormatError(retval));
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
}

void TestBytesToHexErrors(nlTestSuite * inSuite, void * inContext)
{
    // NULL source
    {
        const uint8_t * src = nullptr;
        char dest[18]       = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        char expected[18]   = { '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '!', '@' };
        NL_TEST_ASSERT(inSuite, BytesToHex(&src[0], 0, &dest[0], sizeof(dest), HexFlags::kNone) == CHIP_ERROR_INVALID_ARGUMENT);

        // Buffers should not have been touched
        NL_TEST_ASSERT(inSuite, memcmp(&dest[0], &expected[0], sizeof(expected)) == 0);
    }

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

const nlTest sTests[] = {
    NL_TEST_DEF("TestBytesToHexNotNullTerminated", TestBytesToHexNotNullTerminated), //
    NL_TEST_DEF("TestBytesToHexNullTerminated", TestBytesToHexNullTerminated),       //
    NL_TEST_DEF("TestBytesToHexErrors", TestBytesToHexErrors),                       //
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
