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
