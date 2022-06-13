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

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/Constants.h>

#if CHIP_CONFIG_ENABLE_ARG_PARSER

using namespace chip::ArgParser;

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);
static void HandleArgError(const char * msg, ...);
static void ClearCallbackRecords();

#define DEBUG_TESTS 0

#define QuitWithError(MSG)                                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        fprintf(stderr, "%s FAILED (line %d): ", __FUNCTION__, __LINE__);                                                          \
        fputs(MSG, stderr);                                                                                                        \
        fputs("\n", stderr);                                                                                                       \
        exit(EXIT_FAILURE);                                                                                                        \
    } while (0)

#define VerifyOrQuit(TST, MSG)                                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(TST))                                                                                                                \
        {                                                                                                                          \
            QuitWithError(MSG);                                                                                                    \
        }                                                                                                                          \
    } while (0)

#define VerifyHandleOptionCallback(INDEX, EXPECT_PROG_NAME, EXPECT_OPTSET, EXPECT_ID, EXPECT_NAME, EXPECT_ARG)                     \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        const char * arg     = EXPECT_ARG;                                                                                         \
        VerifyOrQuit(rec.Type == CallbackRecord::kHandleOption, "Invalid callback type (expected HandleOption)");                  \
        VerifyOrQuit(strcmp(rec.ProgName, EXPECT_PROG_NAME) == 0, "Invalid value for HandleOption argument: progName");            \
        VerifyOrQuit(rec.OptSet == EXPECT_OPTSET, "Invalid value for HandleOption argument: optSet");                              \
        VerifyOrQuit(rec.Id == EXPECT_ID, "Invalid value for HandleOption argument: id");                                          \
        VerifyOrQuit(strcmp(rec.Name, EXPECT_NAME) == 0, "Invalid value for HandleOption argument: name");                         \
        if (arg != NULL)                                                                                                           \
            VerifyOrQuit(strcmp(rec.Arg, arg) == 0, "Invalid value for HandleOption argument: arg");                               \
        else                                                                                                                       \
            VerifyOrQuit(rec.Arg == NULL, "Invalid value for HandleOption argument: arg");                                         \
    } while (0)

#define VerifyHandleNonOptionArgsCallback(INDEX, EXPECT_PROG_NAME, EXPECT_ARGC)                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        VerifyOrQuit(rec.Type == CallbackRecord::kHandleNonOptionArgs, "Invalid callback type (expected HandleNonOptionArgs)");    \
        VerifyOrQuit(strcmp(rec.ProgName, EXPECT_PROG_NAME) == 0, "Invalid value for HandleNonOptionArgs argument: progName");     \
        VerifyOrQuit(rec.Argc == EXPECT_ARGC, "Invalid value for HandleNonOptionArgs argument: argc");                             \
    } while (0)

#define VerifyNonOptionArg(INDEX, EXPECT_ARG)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        VerifyOrQuit(rec.Type == CallbackRecord::kNonOptionArg, "Invalid callback type (expected NonOptionArg)");                  \
        VerifyOrQuit(strcmp(rec.Arg, EXPECT_ARG) == 0, "Invalid value for NonOptionArg");                                          \
    } while (0)

#define VerifyPrintArgErrorCallback(INDEX)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        VerifyOrQuit(rec.Type == CallbackRecord::kArgError, "Invalid callback type (expected ArgError)");                          \
    } while (0)

#define VerifyArgErrorContains(INDEX, EXPECT_TEXT)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        CallbackRecord & rec = sCallbackRecords[INDEX];                                                                            \
        VerifyOrQuit(strstr(rec.Error, EXPECT_TEXT) != NULL, "Expected text not found in error output");                           \
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
    { "bar",  kNoArgument, 1001 },
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

static void SimpleParseTest_SingleLongOption()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");
    VerifyOrQuit(sCallbackRecordCount == 2, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

static void SimpleParseTest_SingleShortOption()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");
    VerifyOrQuit(sCallbackRecordCount == 2, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetB, 's', "-s", nullptr);
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

static void SimpleParseTest_SingleLongOptionWithValue()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");
    VerifyOrQuit(sCallbackRecordCount == 2, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

static void SimpleParseTest_SingleShortOptionWithValue()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");
    VerifyOrQuit(sCallbackRecordCount == 2, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 0);
}

static void SimpleParseTest_VariousShortAndLongWithArgs()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");
    VerifyOrQuit(sCallbackRecordCount == 12, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleOptionCallback(2, __FUNCTION__, &sOptionSetB, 's', "-s", nullptr);
    VerifyHandleOptionCallback(3, __FUNCTION__, &sOptionSetA, 1001, "--bar", nullptr);
    VerifyHandleOptionCallback(4, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyHandleOptionCallback(5, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleOptionCallback(6, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value-2");
    VerifyHandleNonOptionArgsCallback(7, __FUNCTION__, 4);
    VerifyNonOptionArg(8, "non-opt-arg-1");
    VerifyNonOptionArg(9, "non-opt-arg-2");
    VerifyNonOptionArg(10, "non-opt-arg-3");
    VerifyNonOptionArg(11, "non-opt-arg-4");
}

static void UnknownOptionTest_UnknownShortOption()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 3, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyPrintArgErrorCallback(2);
    VerifyArgErrorContains(2, "Unknown");
    VerifyArgErrorContains(2, "-q");
}

static void UnknownOptionTest_UnknownLongOption()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 3, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyPrintArgErrorCallback(2);
    VerifyArgErrorContains(2, "Unknown option");
    VerifyArgErrorContains(2, "--bad");
}

static void UnknownOptionTest_UnknownShortOptionAfterKnown()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 4, "Invalid value returned for sCallbackRecordCount");
    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "--foo", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetB, 1000, "--run", "run-value");
    VerifyHandleOptionCallback(2, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyPrintArgErrorCallback(3);
    VerifyArgErrorContains(3, "Unknown");
    VerifyArgErrorContains(3, "-Q");
}

static void UnknownOptionTest_UnknownShortOptionBeforeKnown()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 1, "Invalid value returned for sCallbackRecordCount");
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "-Q");
}

static void UnknownOptionTest_UnknownShortOptionAfterArgs()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 1, "Invalid value returned for sCallbackRecordCount");
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "-Q");
}

static void UnknownOptionTest_UnknownLongOptionAfterArgs()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 1, "Invalid value returned for sCallbackRecordCount");
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Unknown");
    VerifyArgErrorContains(0, "--barf");
}

static void UnknownOptionTest_IgnoreUnknownLongOption()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");

    VerifyOrQuit(sCallbackRecordCount == 4, "Invalid value returned for sCallbackRecordCount");

    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(1, __FUNCTION__, 2);
    VerifyNonOptionArg(2, "non-opt-arg-1");
    VerifyNonOptionArg(3, "non-opt-arg-2");
}

static void UnknownOptionTest_IgnoreUnknownShortOption()
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
    VerifyOrQuit(res == true, "ParseArgs() returned false");

    VerifyOrQuit(sCallbackRecordCount == 5, "Invalid value returned for sCallbackRecordCount");

    VerifyHandleOptionCallback(0, __FUNCTION__, &sOptionSetA, '1', "-1", nullptr);
    VerifyHandleOptionCallback(1, __FUNCTION__, &sOptionSetA, 'Z', "-Z", "baz-value");
    VerifyHandleNonOptionArgsCallback(2, __FUNCTION__, 2);
    VerifyNonOptionArg(3, "non-opt-arg-1");
    VerifyNonOptionArg(4, "non-opt-arg-2");
}

static void MissingValueTest_MissingShortOptionValue()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 1, "Invalid value returned for sCallbackRecordCount");
    VerifyPrintArgErrorCallback(0);
    VerifyArgErrorContains(0, "Missing");
    VerifyArgErrorContains(0, "-Z");
}

static void MissingValueTest_MissingLongOptionValue()
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
    VerifyOrQuit(res == false, "ParseArgs() returned true");
    VerifyOrQuit(sCallbackRecordCount == 1, "Invalid value returned for sCallbackRecordCount");
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

    VerifyOrQuit(sCallbackRecordCount < kMaxCallbackRecords, "Out of callback records");
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

    VerifyOrQuit(sCallbackRecordCount < kMaxCallbackRecords, "Out of callback records");
    sCallbackRecords[sCallbackRecordCount].Type     = CallbackRecord::kHandleNonOptionArgs;
    sCallbackRecords[sCallbackRecordCount].ProgName = chip::Platform::MemoryAllocString(progName, strlen(progName));
    sCallbackRecords[sCallbackRecordCount].Argc     = argc;
    sCallbackRecordCount++;

    for (int i = 0; i < argc; i++)
    {
        VerifyOrQuit(sCallbackRecordCount < kMaxCallbackRecords, "Out of callback records");
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

    VerifyOrQuit(sCallbackRecordCount < kMaxCallbackRecords, "Out of callback records");

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

int TestCHIPArgParser(void)
{
    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        return EXIT_FAILURE;
    }

    SimpleParseTest_SingleLongOption();
    SimpleParseTest_SingleShortOption();
    SimpleParseTest_SingleLongOptionWithValue();
    SimpleParseTest_SingleShortOptionWithValue();
    SimpleParseTest_VariousShortAndLongWithArgs();

    UnknownOptionTest_UnknownShortOption();
    UnknownOptionTest_UnknownLongOption();
    UnknownOptionTest_UnknownShortOptionAfterArgs();
    UnknownOptionTest_UnknownShortOptionAfterKnown();
    UnknownOptionTest_UnknownShortOptionBeforeKnown();
    UnknownOptionTest_UnknownLongOptionAfterArgs();
    UnknownOptionTest_IgnoreUnknownShortOption();
    UnknownOptionTest_IgnoreUnknownLongOption();

    MissingValueTest_MissingShortOptionValue();
    MissingValueTest_MissingLongOptionValue();

    ClearCallbackRecords();

    printf("All tests succeeded\n");

    chip::Platform::MemoryShutdown();

    return (EXIT_SUCCESS);
}
#else  // CHIP_CONFIG_ENABLE_ARG_PARSER
int TestCHIPArgParser(void)
{
    printf("No tests were run\n");
    return (EXIT_SUCCESS);
}
#endif // CHIP_CONFIG_ENABLE_ARG_PARSER

CHIP_REGISTER_TEST_SUITE(TestCHIPArgParser);
