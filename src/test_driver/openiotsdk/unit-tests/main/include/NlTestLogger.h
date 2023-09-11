/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
