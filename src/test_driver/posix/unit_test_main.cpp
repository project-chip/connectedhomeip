/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <string_view>

#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/logging/CHIPLogging.h>

#include "lib/support/logging/Constants.h"
#include "lib/support/logging/TextOnlyLogging.h"
#include "pw_unit_test/event_handler.h"
#include "pw_unit_test/framework.h"

using namespace chip;
using namespace pw::unit_test;

namespace {

bool gQuiet = false;

#define kOptQuiet 'q'

ArgParser::OptionDef gProgramCustomOptionDefs[] = {
    { "quiet", ArgParser::kNoArgument, kOptQuiet },
    {},
};

const char gProgramCustomOptionHelp[] = "  --quiet\n"
                                        "       Output only failures and assertion errors.\n"
                                        "\n";

bool HandleCustomOption(const char * aProgram, ArgParser::OptionSet * aOptions, int aIdentifier, const char * aName,
                        const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptQuiet:
        gQuiet = true;
        break;
    default:
        ArgParser::PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        return false;
    }

    return true;
}

ArgParser::OptionSet gProgramCustomOptions = { HandleCustomOption, gProgramCustomOptionDefs, "GENERAL OPTIONS",
                                               gProgramCustomOptionHelp };

class ChipLogHandler : public pw::unit_test::EventHandler
{
public:
    ChipLogHandler(bool quiet) : mQuiet(quiet) {}

    void TestProgramStart(const ProgramSummary & program_summary) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[==========] Running %d tests from %d test suite%s.\n", program_summary.tests_to_run, program_summary.test_suites,
               program_summary.test_suites > 1 ? "s" : "");
    }

    void EnvironmentsSetUpEnd() override
    {
        VerifyOrReturn(!mQuiet);
        printf("[----------] Global test environments setup.\n");
    }

    void TestSuiteStart(const TestSuite & test_suite) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[----------] %d tests from %s.\n", test_suite.test_to_run_count, test_suite.name);
    }

    void TestSuiteEnd(const TestSuite & test_suite) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[----------] %d tests from %s.\n", test_suite.test_to_run_count, test_suite.name);
    }

    /// Called after environment teardown for each iteration of tests ends.
    void EnvironmentsTearDownEnd() override
    {
        VerifyOrReturn(!mQuiet);
        printf("[----------] Global test environments tear-down.\n");
    }

    /// Called after all test activities have ended.
    void TestProgramEnd(const ProgramSummary & program_summary) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[==========] %d / %d tests from %d test suite%s ran.\n",
               program_summary.tests_to_run - program_summary.tests_summary.skipped_tests -
                   program_summary.tests_summary.disabled_tests,
               program_summary.tests_to_run, program_summary.test_suites, program_summary.test_suites > 1 ? "s" : "");
        printf("[  PASSED  ] %d test(s).\n", program_summary.tests_summary.passed_tests);

        if (program_summary.tests_summary.skipped_tests || program_summary.tests_summary.disabled_tests)
        {
            printf("[ DISABLED ] %d test(s).\n",
                   program_summary.tests_summary.skipped_tests + program_summary.tests_summary.disabled_tests);
        }

        if (program_summary.tests_summary.failed_tests)
        {
            printf("[  FAILED  ] %d test(s).\n", program_summary.tests_summary.failed_tests);
        }
    }

    /// Called before all tests are run.
    void RunAllTestsStart() override
    {
        VerifyOrReturn(!mQuiet);
        printf("[==========] Running all tests.\n");
    }

    /// Called after all tests are run.
    void RunAllTestsEnd(const RunTestsSummary & run_tests_summary) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[==========] Done running all tests.\n");
        printf("[  PASSED  ] %d test(s).\n", run_tests_summary.passed_tests);

        if (run_tests_summary.skipped_tests)
        {
            printf("[ DISABLED ] %d test(s).\n", run_tests_summary.skipped_tests);
        }
        if (run_tests_summary.failed_tests)
        {
            printf("[  FAILED  ] %d test(s).\n", run_tests_summary.failed_tests);
        }
    }

    void TestCaseStart(const TestCase & test_case) override
    {
        VerifyOrReturn(!mQuiet);
        printf("[ RUN      ] %s.%s\n", test_case.suite_name, test_case.test_name);
    }

    void TestCaseEnd(const TestCase & test_case, TestResult result) override
    {
        switch (result)
        {
        case TestResult::kSuccess:
            if (!mQuiet)
            {
                printf("[       OK ] %s.%s\n", test_case.suite_name, test_case.test_name);
            }
            break;
        case TestResult::kFailure:
            printf("[  FAILED  ] %s.%s\n", test_case.suite_name, test_case.test_name);
            break;
        case TestResult::kSkipped:
            if (!mQuiet)
            {
                printf("[ DISABLED ] %s.%s\n", test_case.suite_name, test_case.test_name);
            }
            break;
        }
    }

    /// Called when a disabled test case is encountered.
    void TestCaseDisabled(const TestCase & test) override
    {
        VerifyOrReturn(!mQuiet);
        printf("Skipping disabled test %s.%s\n", test.suite_name, test.test_name);
    }

    /// Called after each expect or assert statement within a test case with the
    /// result.
    void TestCaseExpect(const TestCase & test_case, const TestExpectation & expectation) override
    {
        VerifyOrReturn(!expectation.success);

        printf("%s:%d: Failure\n", expectation.file_name, expectation.line_number);
        printf("      Expected: %s\n", expectation.expression);
        printf("      Actual:   %s\n", expectation.evaluated_expression);
    }

private:
    bool mQuiet;
};

} // namespace

int main(int argc, char ** argv)
{
    SuccessOrDie(Platform::MemoryInit());

    ArgParser::HelpOptions helpOptions(argv[0], "Usage: unit_test_main [options]", "1.0");
    ArgParser::OptionSet * allOptions[] = { &helpOptions, &gProgramCustomOptions, nullptr };

    if (!ArgParser::ParseArgs(argv[0], argc, argv, allOptions))
    {
        return 1;
    }

    // Make the binary compatible with pw_unit_test:googletest. Has no effect
    // when using pw_unit_test:light.
    testing::InitGoogleTest(&argc, argv);

    if (gQuiet)
    {
        // Extra quiet - we keep errors since those likely help debug failures, however
        // the rest are hidden.
        Logging::SetLogFilter(Logging::kLogCategory_Error);
    }

    ChipLogHandler handler(gQuiet);

    pw::unit_test::RegisterEventHandler(&handler);
    return RUN_ALL_TESTS();
}
