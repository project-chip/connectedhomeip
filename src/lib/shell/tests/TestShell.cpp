/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "TestShell.h"

#include <shell/shell.h>
#include <support/CodeUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Logging;

// =================================
//      Test Vectors
// =================================

struct test_shell_vector
{
    const char * line;
    const char ** argv;
};

static const struct test_shell_vector test_vector_shell_tokenizer[] = {
    { .line = "hello how are you?", .argv = (const char *[]){ "hello", "how", "are", "you?" } },
    { .line = "hey yo yo", .argv = (const char *[]){ "hey", "yo", "yo" } },
    { .line = "This  has  double  spaces", .argv = (const char *[]){ "This", "has", "double", "spaces" } },
    { .line = " leading space", .argv = (const char *[]){ "leading", "space" } },
    { .line = "trailing space ", .argv = (const char *[]){ "trailing", "space", "" } },
    { .line = "no_space", .argv = (const char *[]){ "no_space" } },
    { .line = "escaped\\ space", .argv = (const char *[]){ "escaped space" } },
    { .line = "escape\\\\", .argv = (const char *[]){ "escape\\" } },
    { .line = "extended\\ escaped\\ space and\\ more", .argv = (const char *[]){ "extended escaped space", "and more" } },
    { .line = " ", .argv = (const char *[]){ "" } },
    { .line = "", .argv = (const char *[]){} },
};

// =================================
//      Unit tests
// =================================

#define TEST_SHELL_MAX_TOKENS 5

static void TestShell_Tokenizer(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(test_vector_shell_tokenizer);
    int numOfTestsRan    = 0;
    const struct test_shell_vector * test_params;
    char * argv[TEST_SHELL_MAX_TOKENS];
    int argc = TEST_SHELL_MAX_TOKENS;
    int count;

    char line[128];

    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        test_params = &test_vector_shell_tokenizer[vectorIndex];
        strcpy(line, test_params->line);

        count = shell_line_tokenize(line, argv, argc);

        for (int i = 0; i < count; i++)
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

int TestShell(void)
{
    nlTestSuite theSuite = { "CHIP Shell tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
