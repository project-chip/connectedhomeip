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

#include <lib/core/CHIPCore.h>

#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

#if CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

const char * CheckAndSkipSource(nlTestSuite * inSuite, const char * s, const char * file, unsigned int line)
{
    size_t fileLength = strlen(file);
    NL_TEST_ASSERT(inSuite, strncmp(s, file, fileLength) == 0);
    NL_TEST_ASSERT(inSuite, s[fileLength] == ':');
    char * end;
    NL_TEST_ASSERT(inSuite, strtoul(&s[fileLength + 1], &end, 10) == line);
    NL_TEST_ASSERT(inSuite, strncmp(end, ": ", 2) == 0);
    return end + 2;
}

#define CHECK_AND_SKIP_SOURCE(suite, s) CheckAndSkipSource((suite), (s), __FILE__, __LINE__)

#else // CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

#define CHECK_AND_SKIP_SOURCE(suite, s) (s)

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

static void CheckRegisterDeregisterErrorFormatter(nlTestSuite * inSuite, void * inContext)
{
    static ErrorFormatter falseFormatter  = { falseFormat, nullptr };
    static ErrorFormatter falseFormatter2 = { falseFormat2, nullptr };
    static ErrorFormatter trueFormatter   = { trueFormat, nullptr };

    // simple case
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(inSuite, falseFormatCalled == 1);
    // reset
    falseFormatCalled = 0;

    // re-registration should be ignored
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(inSuite, falseFormatCalled == 1);
    // reset
    falseFormatCalled = 0;

    // registration of a new handler, nobody handling anything
    RegisterErrorFormatter(&falseFormatter2);
    ErrorStr(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(inSuite, falseFormatCalled == 1);
    NL_TEST_ASSERT(inSuite, falseFormat2Called == 1);
    // reset
    falseFormatCalled  = 0;
    falseFormat2Called = 0;

    // registration of a true handler, gets first crack
    RegisterErrorFormatter(&trueFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(inSuite, trueFormatCalled == 1);
    NL_TEST_ASSERT(inSuite, falseFormatCalled == 0);
    NL_TEST_ASSERT(inSuite, falseFormat2Called == 0);
    // reset
    trueFormatCalled = 0;

    // deregister true
    DeregisterErrorFormatter(&trueFormatter);
    ErrorStr(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(inSuite, trueFormatCalled == 0);
    NL_TEST_ASSERT(inSuite, falseFormatCalled == 1);
    NL_TEST_ASSERT(inSuite, falseFormat2Called == 1);

    // verify this doesn't crash
    DeregisterErrorFormatter(&trueFormatter);
}

static void CheckNoError(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, strcmp(CHECK_AND_SKIP_SOURCE(inSuite, ErrorStr(CHIP_NO_ERROR)), CHIP_NO_ERROR_STRING) == 0);
}

static void CheckFormatErr(nlTestSuite * inSuite, void * inContext)
{
#if CHIP_CONFIG_SHORT_ERROR_STR

    // TODO tests for this config

#else // CHIP_CONFIG_SHORT_ERROR_STR
    static const size_t kBufSize = 1024;
    static char buf[kBufSize];
    static const char * subsys = "subsys";
    static const char * desc   = "desc";

    strcpy(buf, "hi");
    // shouldn't touch the buffer
    FormatError(buf, 0, subsys, CHIP_ERROR_INTERNAL, desc);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "hi") == 0);

    // guarantees null termination, doesn't touch past 1st byte
    strcpy(buf, "hi");
    FormatError(buf, 1, subsys, CHIP_ERROR_INTERNAL, desc);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "") == 0);
    NL_TEST_ASSERT(inSuite, buf[1] == 'i');

    // whole shebang
    FormatError(buf, kBufSize, subsys, CHIP_CORE_ERROR(1), desc);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "subsys Error 0x00000001: desc") == 0);

    // skip desc
    FormatError(buf, kBufSize, subsys, CHIP_CORE_ERROR(1), nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "subsys Error 0x00000001") == 0);

    // skip subsys
    FormatError(buf, kBufSize, nullptr, CHIP_CORE_ERROR(1), desc);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "Error 0x00000001: desc") == 0);

    // skip both
    FormatError(buf, kBufSize, nullptr, CHIP_CORE_ERROR(1), nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "Error 0x00000001") == 0);
#endif
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("NoError",                          CheckNoError),
    NL_TEST_DEF("RegisterDeregisterErrorFormatter", CheckRegisterDeregisterErrorFormatter),
    NL_TEST_DEF("FormatErr",                        CheckFormatErr),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestErrorStr(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "-Error-Strings",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestErrorStr)
