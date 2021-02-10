/* See Project CHIP LICENSE file for licensing information. */


#include "TestShell.h"

#include <shell/shell.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

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

static void TestStreamer_Output(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(test_vector_streamer_out);
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
        NL_TEST_ASSERT(inSuite, num_chars == static_cast<ssize_t>(strlen(output)));
        numOfTestsRan++;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test Streamer: TestStreamer_Output", TestStreamer_Output),

    NL_TEST_SENTINEL()
};

int TestStreamerStdio(void)
{
    nlTestSuite theSuite = { "CHIP Streamer tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStreamerStdio)
