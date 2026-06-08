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
#include <lib/shell/Engine.h>
#include <lib/support/CodeUtils.h>

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

struct test_streamer_vector
{
    const char * output;
};

static const struct test_streamer_vector test_vector_streamer_out[] = {
    { .output = "prompt>\n" }, { .output = "T123 " }, { .output = "T456\n" }, { .output = "T789 " }, { .output = "T10\n" },
};

// =================================
//      Unit tests
// =================================

TEST(TestShellStreamerStdio, TestStreamer_Output)
{
    int numOfTestVectors = MATTER_ARRAY_SIZE(test_vector_streamer_out);
    int numOfTestsRan    = 0;
    const struct test_streamer_vector * test_params;

    const char * output;
    ssize_t num_chars;

    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        test_params = &test_vector_streamer_out[vectorIndex];
        output      = test_params->output;

        num_chars = streamer_write(streamer_get(), output, strlen(output));
        // Let's assume that all our output lengths fit in ssize_t.
        EXPECT_EQ(num_chars, static_cast<ssize_t>(strlen(output)));
        numOfTestsRan++;
    }
    EXPECT_GT(numOfTestsRan, 0);
}
