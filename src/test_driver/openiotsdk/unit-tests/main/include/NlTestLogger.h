/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Custom NL test logger implementation
 *
 */

#include <nlunit-test.h>
#include <stdio.h>

class NlTestLogger
{
    static void def_log_name(struct _nlTestSuite * inSuite) { printf("[ %s ]\r\n", inSuite->name); }

    static void def_log_initialize(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        printf("[ %s : %-*s ] : %s\r\n", inSuite->name, inWidth, "Initialize", inResult == FAILURE ? "FAILED" : "PASSED");
    }
    static void def_log_terminate(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        printf("[ %s : %-*s ] : %s\r\n", inSuite->name, inWidth, "Terminate", inResult == FAILURE ? "FAILED" : "PASSED");
    }

    static void def_log_setup(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        printf("[ %s : %-*s ] : %s\r\n", inSuite->name, inWidth, "Setup", inResult == FAILURE ? "FAILED" : "PASSED");
    }

    static void def_log_test(struct _nlTestSuite * inSuite, int inWidth, int inIndex)
    {
        printf("[ %s : %-*s ] : %s\r\n", inSuite->name, inWidth, inSuite->tests[inIndex].name,
               inSuite->flagError ? "FAILED" : "PASSED");
    }

    static void def_log_teardown(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        printf("[ %s : %-*s ] : %s\r\n", inSuite->name, inWidth, "TearDown", inResult == FAILURE ? "FAILED" : "PASSED");
    }

    static void def_log_statTest(struct _nlTestSuite * inSuite)
    {
        printf("Failed Tests:   %d / %d\r\n", inSuite->failedTests, inSuite->runTests);
    }

    static void def_log_statAssert(struct _nlTestSuite * inSuite)
    {
        printf("Failed Asserts: %d / %d\r\n", inSuite->failedAssertions, inSuite->performedAssertions);
    }

public:
    static constexpr nl_test_output_logger_t nl_test_logger = {
        def_log_name, def_log_initialize, def_log_terminate, def_log_setup,
        def_log_test, def_log_teardown,   def_log_statTest,  def_log_statAssert,
    };
};
