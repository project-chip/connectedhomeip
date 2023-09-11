/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <nlunit-test.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>

// Include entire C++ file to have access to functions-under-test
// such as TokenizeLine despite them being declared within an anonymous namespace.
#include "../MainLoopDefault.cpp"

using namespace chip;
using namespace chip::Shell;
using namespace chip::Logging;

constexpr int TEST_SHELL_MAX_TOKENS = 5;

// =================================
//      Test Vectors
// =================================

struct test_shell_vector
{
    const char * line;
    const char ** argv;
    int argc;
};

static const struct test_shell_vector test_vector_shell_tokenizer[] = {
    {
        .line = "hello how are you?",
        .argv = (const char *[]){ "hello", "how", "are", "you?" },
        .argc = 4,
    },
    {
        .line = "hey yo yo",
        .argv = (const char *[]){ "hey", "yo", "yo" },
        .argc = 3,
    },
    {
        .line = "This  has  double  spaces",
        .argv = (const char *[]){ "This", "has", "double", "spaces" },
        .argc = 4,
    },
    {
        .line = " leading space",
        .argv = (const char *[]){ "leading", "space" },
        .argc = 2,
    },
    {
        .line = "trailing space ",
        .argv = (const char *[]){ "trailing", "space", "" },
        .argc = 3,
    },
    {
        .line = "no_space",
        .argv = (const char *[]){ "no_space" },
        .argc = 1,
    },
    {
        .line = " ",
        .argv = (const char *[]){},
        .argc = 0,
    },
    {
        .line = "",
        .argv = (const char *[]){},
        .argc = 0,
    },
    {
        .line = "hey 1 2 3 4 5 6 7 max out",
        .argv =
            (const char *[]){
                "hey",
                "1",
                "2",
                "3",
            },
        .argc = TEST_SHELL_MAX_TOKENS - 1,
    },
};

// =================================
//      Unit tests
// =================================

static void TestShell_Tokenizer(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(test_vector_shell_tokenizer);
    int numOfTestsRan    = 0;
    const struct test_shell_vector * test_params;

    char line[128];

    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        test_params = &test_vector_shell_tokenizer[vectorIndex];
        strcpy(line, test_params->line);

        char * argv[TEST_SHELL_MAX_TOKENS];
        int argc = TokenizeLine(line, argv, TEST_SHELL_MAX_TOKENS);

        NL_TEST_ASSERT(inSuite, argc == test_params->argc);

        for (int i = 0; i < argc; i++)
        {
            NL_TEST_ASSERT(inSuite, strcmp(argv[i], test_params->argv[i]) == 0);
        }
        numOfTestsRan++;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test Shell: TestShell_Tokenizer", TestShell_Tokenizer),

    NL_TEST_SENTINEL()
};

int TestShellTokenizeLine()
{
    nlTestSuite theSuite = { "Test Shell: MainLoop", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestShellTokenizeLine)
