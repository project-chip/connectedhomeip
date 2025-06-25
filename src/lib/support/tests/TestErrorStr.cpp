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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

#if CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

const char * CheckAndSkipSource(const char * s, const char * file, unsigned int line)
{
    size_t fileLength = strlen(file);
    EXPECT_EQ(strncmp(s, file, fileLength), 0);
    EXPECT_EQ(s[fileLength], ':');
    char * end;
    EXPECT_EQ(strtoul(&s[fileLength + 1], &end, 10), line);
    EXPECT_EQ(strncmp(end, ": ", 2), 0);
    return end + 2;
}

#define CHECK_AND_SKIP_SOURCE(s) CheckAndSkipSource((s), __FILE__, __LINE__)

#else // CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

#define CHECK_AND_SKIP_SOURCE(s) (s)

#endif // CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

static int falseFormatCalled = 0;
static bool falseFormat(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    falseFormatCalled += 1;
    return false; // means keep going
}
static int falseFormat2Called = 0;
static bool falseFormat2(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    falseFormat2Called += 1;
    return false; // means keep going
}
static int trueFormatCalled = 0;
static bool trueFormat(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    trueFormatCalled += 1;
    return true; // means I handled it
}

TEST(TestErrorStr, CheckRegisterDeregisterSingleErrorFormatter)
{
    static ErrorFormatter falseFormatter = { falseFormat, nullptr };

    RegisterErrorFormatter(&falseFormatter);
    DeregisterErrorFormatter(&falseFormatter);
}

TEST(TestErrorStr, CheckRegisterDeregisterErrorFormatter)
{
    static ErrorFormatter falseFormatter  = { falseFormat, nullptr };
    static ErrorFormatter falseFormatter2 = { falseFormat2, nullptr };
    static ErrorFormatter trueFormatter   = { trueFormat, nullptr };

    // simple case
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(falseFormatCalled, 1);
    // reset
    falseFormatCalled = 0;

    // re-registration should be ignored
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(falseFormatCalled, 1);
    // reset
    falseFormatCalled = 0;

    // registration of a new handler, nobody handling anything
    RegisterErrorFormatter(&falseFormatter2);
    ErrorStr(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(falseFormatCalled, 1);
    EXPECT_EQ(falseFormat2Called, 1);
    // reset
    falseFormatCalled  = 0;
    falseFormat2Called = 0;

    // registration of a true handler, gets first crack
    RegisterErrorFormatter(&trueFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(trueFormatCalled, 1);
    EXPECT_EQ(falseFormatCalled, 0);
    EXPECT_EQ(falseFormat2Called, 0);
    // reset
    trueFormatCalled = 0;

    // deregister true
    DeregisterErrorFormatter(&trueFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(trueFormatCalled, 0);
    EXPECT_EQ(falseFormatCalled, 1);
    EXPECT_EQ(falseFormat2Called, 1);

    // verify this doesn't crash
    DeregisterErrorFormatter(&trueFormatter);
    DeregisterErrorFormatter(&falseFormatter);
    DeregisterErrorFormatter(&falseFormatter2);
}

TEST(TestErrorStr, CheckNoError)
{
    EXPECT_STREQ(CHECK_AND_SKIP_SOURCE(ErrorStr(CHIP_NO_ERROR)), CHIP_NO_ERROR_STRING);
}

TEST(TestErrorStr, CheckErrorWithProvidedStorage)
{
    ErrorStrStorage storage;
    EXPECT_STREQ(CHECK_AND_SKIP_SOURCE(ErrorStr(CHIP_NO_ERROR, true, storage)), CHIP_NO_ERROR_STRING);
    EXPECT_STREQ(CHECK_AND_SKIP_SOURCE(ErrorStr(CHIP_ERROR_INTERNAL, true, storage)), "Error 0x000000AC");
}

TEST(TestErrorStr, CheckFormatErr)
{
#if CHIP_CONFIG_SHORT_ERROR_STR

    // TODO tests for this config

#else // CHIP_CONFIG_SHORT_ERROR_STR
    static const size_t kBufSize = 1024;
    static char buf[kBufSize];
    static const char subsys[] = "subsys";
    static const char desc[]   = "desc";

    strcpy(buf, "hi");
    // shouldn't touch the buffer
    FormatError(buf, 0, subsys, CHIP_ERROR_INTERNAL, desc);
    EXPECT_STREQ(buf, "hi");

    // guarantees null termination, doesn't touch past 1st byte
    strcpy(buf, "hi");
    FormatError(buf, 1, subsys, CHIP_ERROR_INTERNAL, desc);
    EXPECT_STREQ(buf, "");
    EXPECT_EQ(buf[1], 'i');

    // whole shebang
    FormatError(buf, kBufSize, subsys, CHIP_CORE_ERROR(1), desc);
    EXPECT_STREQ(buf, "subsys Error 0x00000001: desc");

    // skip desc
    FormatError(buf, kBufSize, subsys, CHIP_CORE_ERROR(1), nullptr);
    EXPECT_STREQ(buf, "subsys Error 0x00000001");

    // skip subsys
    FormatError(buf, kBufSize, nullptr, CHIP_CORE_ERROR(1), desc);
    EXPECT_STREQ(buf, "Error 0x00000001: desc");

    // skip both
    FormatError(buf, kBufSize, nullptr, CHIP_CORE_ERROR(1), nullptr);
    EXPECT_STREQ(buf, "Error 0x00000001");
#endif
}
