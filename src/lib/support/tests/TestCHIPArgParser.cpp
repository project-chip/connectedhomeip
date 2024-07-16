/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/Constants.h>

using namespace chip::ArgParser;

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);
static void HandleArgError(const char * msg, ...);
static void ClearCallbackRecords();

class TestCHIPArgParser : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite()
    {
        ClearCallbackRecords();
        chip::Platform::MemoryShutdown();
    }
};

#define DEBUG_TESTS 0

#define VerifyHandleOptionCallback(INDEX, EXPECT_PROG_NAME, EXPECT_OPTSET, EXPECT_ID, EXPECT_NAME, EXPECT_ARG)                     \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        const char * arg     = EXPECT_ARG;                                                                                         \
        ASSERT_EQ(rec.Type, CallbackRecord::kHandleOption) << "Invalid callback type (expected HandleOption)";                     \
        ASSERT_STREQ(rec.ProgName, EXPECT_PROG_NAME) << "Invalid value for HandleOption argument: progName";                       \
        ASSERT_EQ(rec.OptSet, EXPECT_OPTSET) << "Invalid value for HandleOption argument: optSet";                                 \
        ASSERT_EQ(rec.Id, EXPECT_ID) << "Invalid value for HandleOption argument: id";                                             \
        ASSERT_STREQ(rec.Name, EXPECT_NAME) << "Invalid value for HandleOption argument: name";                                    \
        if (arg != NULL)                                                                                                           \
        {                                                                                                                          \
            ASSERT_STREQ(rec.Arg, arg) << "Invalid value for HandleOption argument: arg";                                          \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            ASSERT_EQ(rec.Arg, nullptr) << "Invalid value for HandleOption argument: arg";                                         \
        }                                                                                                                          \
    } while (0)

#define VerifyHandleNonOptionArgsCallback(INDEX, EXPECT_PROG_NAME, EXPECT_ARGC)                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        ASSERT_EQ(rec.Type, CallbackRecord::kHandleNonOptionArgs) << "Invalid callback type (expected HandleNonOptionArgs)";       \
        ASSERT_STREQ(rec.ProgName, EXPECT_PROG_NAME) << "Invalid value for HandleNonOptionArgs argument: progName";                \
        ASSERT_EQ(rec.Argc, EXPECT_ARGC) << "Invalid value for HandleNonOptionArgs argument: argc";                                \
    } while (0)

#define VerifyNonOptionArg(INDEX, EXPECT_ARG)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        ASSERT_EQ(rec.Type, CallbackRecord::kNonOptionArg) << "Invalid callback type (expected NonOptionArg)";                     \
        ASSERT_STREQ(rec.Arg, EXPECT_ARG) << "Invalid value for NonOptionArg";                                                     \
    } while (0)

#define VerifyPrintArgErrorCallback(INDEX)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        ASSERT_EQ(rec.Type, CallbackRecord::kArgError) << "Invalid callback type (expected ArgError)";                             \
    } while (0)

#define VerifyArgErrorContains(INDEX, EXPECT_TEXT)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        ASSERT_NE(strstr(rec.Error, EXPECT_TEXT), nullptr) << "Expected text not found in error output";                           \
    } while (0)

struct CallbackRecord
{
    enum
    {
        kHandleOption,
        kHandleNonOptionArgs,
        kNonOptionArg,
        kArgError
    } Type;
    char * ProgName;
    OptionSet * OptSet;
    int Id;
    char * Name;
    char * Arg;
    int Argc;
    char * Error;
};

enum
{
    kMaxCallbackRecords = 100
};

static CallbackRecord sCallbackRecords[kMaxCallbackRecords];
static size_t sCallbackRecordCount = 0;

// clang-format off
static OptionDef sOptionSetA_Defs[] =
{
    { "foo",  kNoArgument, '1'  },
    { "bar",  kNoArgument, 1002 },
    { "baz",  kArgumentRequired, 'Z'  },
    { }
};

static OptionSet sOptionSetA =
{
    HandleOption,
    sOptionSetA_Defs,
    "OPTION SET A",
    "help text for option set A\n"
    "-1 --foo\n"
    "--bar\n"
    "-Z --baz\n"
};

static OptionDef sOptionSetB_Defs[] =
{
    { "stop", kNoArgument, 's'  },
    { "run",  kArgumentRequired, 1000  },
    { "walk", kNoArgument, 1001 },
    { }
};

static OptionSet sOptionSetB =
{
    HandleOption,
    sOptionSetB_Defs,
    "OPTION SET B",
    "help text for option set B\n"
    "-s --stop\n"
    "--run\n"
    "--walk\n"
};
// clang-format on

namespace {

struct TestArgv
{
    chip::Platform::ScopedMemoryBuffer<char> strings;
    chip::Platform::ScopedMemoryBuffer<char *> argv;
};

// Duplicate arguments since tests use string constants and argv is not
// defined const.
TestArgv DupeArgs(const char * argv[], int argc_as_int)
{
    size_t argc = static_cast<size_t>(argc_as_int);
    TestArgv ret;
    ret.argv.Alloc(argc + 1);

    size_t len = 0;
    for (size_t i = 0; i < argc; ++i)
    {
        len += strlen(argv[i]) + 1;
    }

    ret.strings.Alloc(len);

    size_t offset = 0;
    for (size_t i = 0; i < argc; ++i)
    {
        ret.argv[i] = &ret.strings[offset];
        memcpy(ret.argv[i], argv[i], strlen(argv[i]) + 1);
        offset += strlen(argv[i]) + 1;
    }

    ret.argv[argc] = nullptr;

    return ret;
}

} // namespace

TEST_F(TestCHIPArgParser, SimpleParseTest_SingleLongOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--foo",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_TRUE(res) << "ParseArgs() returned false";
    ASSERT_EQ(sCallbackRecordCount, 2u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

TEST_F(TestCHIPArgParser, SimpleParseTest_SingleShortOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "-s",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_TRUE(res) << "ParseArgs() returned false";
    ASSERT_EQ(sCallbackRecordCount, 2u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetB, 's', "-s", nullptr);
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

TEST_F(TestCHIPArgParser, SimpleParseTest_SingleLongOptionWithValue)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--run", "run-value",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_TRUE(res) << "ParseArgs() returned false";
    ASSERT_EQ(sCallbackRecordCount, 2u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

TEST_F(TestCHIPArgParser, SimpleParseTest_SingleShortOptionWithValue)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "-Z", "baz-value",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_TRUE(res) << "ParseArgs() returned false";
    ASSERT_EQ(sCallbackRecordCount, 2u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

TEST_F(TestCHIPArgParser, SimpleParseTest_VariousShortAndLongWithArgs)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--foo",
        "--run", "run-value",
        "-s",
        "--bar",
        "-1Z", "baz-value",
        "--run", "run-value-2",
        "non-opt-arg-1",
        "non-opt-arg-2",
        "non-opt-arg-3",
        "non-opt-arg-4",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_TRUE(res) << "ParseArgs() returned false";
    ASSERT_EQ(sCallbackRecordCount, 12u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleOptionCallback(2, __FUNCTION__, &sOptionSetB, 's', "-s", nullptr);
    VerifyHandleOptionCallback(3, __FUNCTION__, &sOptionSetA, 1002, "--bar", nullptr);
    VerifyHandleOptionCallback(4, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyHandleOptionCallback(5, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleOptionCallback(6, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value-2");
    VerifyHandleNonOptionArgsCallback(7, __FUNCTION__, 4);
    VerifyNonOptionArg(8, "non-opt-arg-1");
    VerifyNonOptionArg(9, "non-opt-arg-2");
    VerifyNonOptionArg(10, "non-opt-arg-3");
    VerifyNonOptionArg(11, "non-opt-arg-4");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownShortOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetB, &sOptionSetA, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--foo",
        "--run", "run-value",
        "-q", // <-- unknown option -q
        "--bar",
        "non-opt-arg-1",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 3u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyPrintArgErrorCallback(2);
    VerifyArgErrorContains(2, "Unknown");
    VerifyArgErrorContains(2, "-q");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownLongOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetB, &sOptionSetA, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--foo",
        "--run", "run-value",
        "--bad", // <-- unknown option --bad
        "--bar",
        "non-opt-arg-1",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 3u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyPrintArgErrorCallback(2);
    VerifyArgErrorContains(2, "Unknown option");
    VerifyArgErrorContains(2, "--bad");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownShortOptionAfterKnown)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetB, &sOptionSetA, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--foo",
        "--run", "run-value",
        "-1Q", // <-- unknown option -Q
        "--bar",
        "non-opt-arg-1",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 4u) << "Invalid value returned for sCallbackRecordCount";
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleOptionCallback(2, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyPrintArgErrorCallback(3);
    VerifyArgErrorContains(3, "Unknown");
    VerifyArgErrorContains(3, "-Q");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownShortOptionBeforeKnown)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "-Q1", // <-- unknown option -Q
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 1u) << "Invalid value returned for sCallbackRecordCount";
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "-Q");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownShortOptionAfterArgs)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "non-opt-arg-1",
        "non-opt-arg-2",
        "-Q", // <-- unknown option -Q
        nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 1u) << "Invalid value returned for sCallbackRecordCount";
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "-Q");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_UnknownLongOptionAfterArgs)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "non-opt-arg-1",
        "non-opt-arg-2",
        "--barf", // <-- unknown option --barf
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 1u) << "Invalid value returned for sCallbackRecordCount";
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "--barf");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_IgnoreUnknownLongOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "non-opt-arg-1",
        "non-opt-arg-2",
        "--foob", // <-- unknown option --foob
        "-Zbaz-value",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs, true);
    ASSERT_TRUE(res) << "ParseArgs() returned false";

    ASSERT_EQ(sCallbackRecordCount, 4u) << "Invalid value returned for sCallbackRecordCount";

    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 2);
    VerifyNonOptionArg(2, "non-opt-arg-1");
    VerifyNonOptionArg(3, "non-opt-arg-2");
}

TEST_F(TestCHIPArgParser, UnknownOptionTest_IgnoreUnknownShortOption)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "non-opt-arg-1",
        "non-opt-arg-2",
        "-Q1", // <-- unknown option -Q
        "-Zbaz-value",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs, true);
    ASSERT_TRUE(res) << "ParseArgs() returned false";

    ASSERT_EQ(sCallbackRecordCount, 5u) << "Invalid value returned for sCallbackRecordCount";

    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(2, __FUNCTION__, 2);
    VerifyNonOptionArg(3, "non-opt-arg-1");
    VerifyNonOptionArg(4, "non-opt-arg-2");
}

TEST_F(TestCHIPArgParser, MissingValueTest_MissingShortOptionValue)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "-Z",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs, true);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 1u) << "Invalid value returned for sCallbackRecordCount";
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Missing");
    VerifyArgErrorContains(0, "-Z");
}

TEST_F(TestCHIPArgParser, MissingValueTest_MissingLongOptionValue)
{
    bool res;

    static OptionSet * optionSets[] = { &sOptionSetA, &sOptionSetB, nullptr };
    // clang-format off
    static const char *argv[] =
    {
        "",
        "--run",
		nullptr
    };
    // clang-format on
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClearCallbackRecords();
    PrintArgError = HandleArgError;

    TestArgv argsDup = DupeArgs(argv, argc);
    res              = ParseArgs(__FUNCTION__, argc, argsDup.argv.Get(), optionSets, HandleNonOptionArgs, true);
    ASSERT_FALSE(res) << "ParseArgs() returned true";
    ASSERT_EQ(sCallbackRecordCount, 1u) << "Invalid value returned for sCallbackRecordCount";
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Missing");
    VerifyArgErrorContains(0, "--run");
}

static void ClearCallbackRecords()
{
    for (size_t i = 0; i < sCallbackRecordCount; i++)
    {
        if (sCallbackRecords[i].ProgName != nullptr)
            chip::Platform::MemoryFree(sCallbackRecords[i].ProgName);
        if (sCallbackRecords[i].Name != nullptr)
            chip::Platform::MemoryFree(sCallbackRecords[i].Name);
        if (sCallbackRecords[i].Arg != nullptr)
            chip::Platform::MemoryFree(sCallbackRecords[i].Arg);
        if (sCallbackRecords[i].Error != nullptr)
            chip::Platform::MemoryFree(sCallbackRecords[i].Error);
    }
    memset(sCallbackRecords, 0, sizeof(sCallbackRecords));
    sCallbackRecordCount = 0;
}

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
#if DEBUG_TESTS
    printf("HandleOption called: progName:%s optSet:%08lX id:%d name:%s arg:%s\n", progName, (intptr_t) optSet, id, name, arg);
#endif

    EXPECT_LT(sCallbackRecordCount, kMaxCallbackRecords) << "Out of callback records";
    sCallbackRecords[sCallbackRecordCount].Type     = CallbackRecord::kHandleOption;
    sCallbackRecords[sCallbackRecordCount].ProgName = chip::Platform::MemoryAllocString(progName, strlen(progName));
    sCallbackRecords[sCallbackRecordCount].OptSet   = optSet;
    sCallbackRecords[sCallbackRecordCount].Id       = id;
    sCallbackRecords[sCallbackRecordCount].Name     = chip::Platform::MemoryAllocString(name, strlen(name));
    sCallbackRecords[sCallbackRecordCount].Arg = (arg != nullptr) ? chip::Platform::MemoryAllocString(arg, strlen(arg)) : nullptr;
    sCallbackRecordCount++;
    return true;
}

static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[])
{
#if DEBUG_TESTS
    // clang-format off
    printf("HandleNonOptionArgs called: progName:%s argc:%d argv[0]:%s argv[1]:%s argv[2]:%s argv[3]:%s \n",
            progName, argc,
            (argc > 0) ? argv[0] : "(n/a)",
            (argc > 1) ? argv[1] : "(n/a)",
            (argc > 2) ? argv[2] : "(n/a)",
            (argc > 3) ? argv[3] : "(n/a)"
    );
    // clang-format on
#endif

    EXPECT_LT(sCallbackRecordCount, kMaxCallbackRecords) << "Out of callback records";
    sCallbackRecords[sCallbackRecordCount].Type     = CallbackRecord::kHandleNonOptionArgs;
    sCallbackRecords[sCallbackRecordCount].ProgName = chip::Platform::MemoryAllocString(progName, strlen(progName));
    sCallbackRecords[sCallbackRecordCount].Argc     = argc;
    sCallbackRecordCount++;

    for (int i = 0; i < argc; i++)
    {
        EXPECT_LT(sCallbackRecordCount, kMaxCallbackRecords) << "Out of callback records";
        sCallbackRecords[sCallbackRecordCount].Type = CallbackRecord::kNonOptionArg;
        sCallbackRecords[sCallbackRecordCount].Arg  = chip::Platform::MemoryAllocString(argv[i], strlen(argv[i]));
        sCallbackRecordCount++;
    }

    return true;
}

static void ENFORCE_FORMAT(1, 2) HandleArgError(const char * msg, ...)
{
    size_t msgLen;
    int status;
    va_list ap;

    ASSERT_LT(sCallbackRecordCount, kMaxCallbackRecords) << "Out of callback records";

    sCallbackRecords[sCallbackRecordCount].Type = CallbackRecord::kArgError;

    va_start(ap, msg);
    msgLen = static_cast<size_t>(vsnprintf(nullptr, 0, msg, ap));
    va_end(ap);

    va_start(ap, msg);
    sCallbackRecords[sCallbackRecordCount].Error = static_cast<char *>(chip::Platform::MemoryAlloc(msgLen + 1));
    status                                       = vsnprintf(sCallbackRecords[sCallbackRecordCount].Error, msgLen + 1, msg, ap);
    (void) status;
    va_end(ap);

#if DEBUG_TESTS
    printf("HandleArgError called: error: %s", sCallbackRecords[sCallbackRecordCount].Error);
#endif

    sCallbackRecordCount++;
}
