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

#include <core/CHIPCore.h>

#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>

using namespace chip;

#define CHECK(v) (v) ? true : (fprintf(stderr, "%s:%d: error: CHECK(%s) failed\n", __FUNCTION__, __LINE__, #v), false)

#define CHECK_EQ_STR(a, b)                                                                                                         \
    !strcmp((a), (b))                                                                                                              \
        ? true                                                                                                                     \
        : (fprintf(stderr, "%s:%d: error: CHECK_EQ_STR(\"%s\", \"%s\") failed\n", __FUNCTION__, __LINE__, (a), (b)), false)

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

static bool testRegisterDeregisterErrorFormatter()
{
    static ErrorFormatter falseFormatter  = { falseFormat, nullptr };
    static ErrorFormatter falseFormatter2 = { falseFormat2, nullptr };
    static ErrorFormatter trueFormatter   = { trueFormat, nullptr };

    // assume success
    bool ret = true;

    // simple case
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(1);
    ret &= CHECK(falseFormatCalled == 1);
    // reset
    falseFormatCalled = 0;

    // re-registration should be ignored
    RegisterErrorFormatter(&falseFormatter);
    ErrorStr(1);
    ret &= CHECK(falseFormatCalled == 1);
    // reset
    falseFormatCalled = 0;

    // registration of a new handler, nobody handling anything
    RegisterErrorFormatter(&falseFormatter2);
    ErrorStr(1);
    ret &= CHECK(falseFormatCalled == 1);
    ret &= CHECK(falseFormat2Called == 1);
    // reset
    falseFormatCalled  = 0;
    falseFormat2Called = 0;

    // registration of a true handler, gets first crack
    RegisterErrorFormatter(&trueFormatter);
    ErrorStr(1);
    ret &= CHECK(trueFormatCalled == 1);
    ret &= CHECK(falseFormatCalled == 0);
    ret &= CHECK(falseFormat2Called == 0);
    // reset
    trueFormatCalled = 0;

    // deregister true
    DeregisterErrorFormatter(&trueFormatter);
    ErrorStr(1);
    ret &= CHECK(trueFormatCalled == 0);
    ret &= CHECK(falseFormatCalled == 1);
    ret &= CHECK(falseFormat2Called == 1);

    // verify this doesn't crash
    DeregisterErrorFormatter(&trueFormatter);

    return ret;
}

static bool testNoError()
{
    return CHECK_EQ_STR(ErrorStr(CHIP_NO_ERROR), "No Error");
}

static bool testFormatErr()
{
    // assume success
    bool ret = true;

#if CHIP_CONFIG_SHORT_ERROR_STR

    // TODO tests for this config

#else // CHIP_CONFIG_SHORT_ERROR_STR
    static const size_t kBufSize = 1024;
    static char buf[kBufSize];
    static const char * subsys = "subsys";
    static const char * desc   = "desc";

    strcpy(buf, "hi");
    // shouldn't touch the buffer
    FormatError(buf, 0, subsys, 0, desc);
    ret &= CHECK_EQ_STR(buf, "hi");

    // guarantees null termination, doesn't touch past 1st byte
    strcpy(buf, "hi");
    FormatError(buf, 1, subsys, 0, desc);
    ret &= CHECK_EQ_STR(buf, "");
    ret &= CHECK(buf[1] == 'i');

    // whole shebang
    FormatError(buf, kBufSize, subsys, 1, desc);
    ret &= CHECK_EQ_STR(buf, "subsys Error 1 (0x00000001): desc");

    // skip desc
    FormatError(buf, kBufSize, subsys, 1, nullptr);
    ret &= CHECK_EQ_STR(buf, "subsys Error 1 (0x00000001)");

    // skip subsys
    FormatError(buf, kBufSize, nullptr, 1, desc);
    ret &= CHECK_EQ_STR(buf, "Error 1 (0x00000001): desc");

    // skip both
    FormatError(buf, kBufSize, nullptr, 1, nullptr);
    ret &= CHECK_EQ_STR(buf, "Error 1 (0x00000001)");

    // negative
    FormatError(buf, kBufSize, nullptr, static_cast<CHIP_ERROR>(-1), nullptr);
    ret &= CHECK_EQ_STR(buf, "Error -1 (0xFFFFFFFF)");
#endif

    return ret;
}

int TestErrorStr(void)
{

    if (!testNoError() || !testRegisterDeregisterErrorFormatter() || !testFormatErr())
    {
        return EXIT_FAILURE;
    }

    printf("All tests succeeded\n");

    return EXIT_SUCCESS;
}

CHIP_REGISTER_TEST_SUITE(TestErrorStr);
