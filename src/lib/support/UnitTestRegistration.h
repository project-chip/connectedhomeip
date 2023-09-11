/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

/**
 * @def CHIP_REGISTER_TEST_SUITE(FUNCTION)
 *
 * @brief
 *   Registers a unit test suite runner
 *
 * Adds a function of the signature int(*)() to the list of test runners which
 * should be invoked when chip::RunRegisteredUnitTests() is called.
 *
 * Example:
 *
 * @code
 * int MyTestSuiteRunner()
 * {
 *     if (!RunSomeTests())
 *         return -1; // test failure
 *     return 0; // test success
 * }
 *
 * CHIP_REGISTER_TEST_SUITE(MyTestSuiteRunner)
 * @endcode
 */
#define CHIP_REGISTER_TEST_SUITE(FUNCTION)                                                                                         \
    static void __attribute__((constructor)) Register##FUNCTION(void)                                                              \
    {                                                                                                                              \
        VerifyOrDie(chip::RegisterUnitTests(&FUNCTION) == CHIP_NO_ERROR);                                                          \
    }

namespace chip {

typedef int (*UnitTestTriggerFunction)();

CHIP_ERROR RegisterUnitTests(UnitTestTriggerFunction tests);

int RunRegisteredUnitTests();

} // namespace chip
