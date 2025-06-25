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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>

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

TEST(TestShellTokenizeLine, TestShell_Tokenizer)
{
    int numOfTestVectors = MATTER_ARRAY_SIZE(test_vector_shell_tokenizer);
    int numOfTestsRan    = 0;
    const struct test_shell_vector * test_params;

    char line[128];

    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        test_params = &test_vector_shell_tokenizer[vectorIndex];
        strcpy(line, test_params->line);

        char * argv[TEST_SHELL_MAX_TOKENS];
        int argc = TokenizeLine(line, argv, TEST_SHELL_MAX_TOKENS);

        EXPECT_EQ(argc, test_params->argc);

        for (int i = 0; i < argc; i++)
        {
            EXPECT_EQ(strcmp(argv[i], test_params->argv[i]), 0);
        }
        numOfTestsRan++;
    }
    EXPECT_GT(numOfTestsRan, 0);
}
