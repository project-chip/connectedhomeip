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

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <stdlib.h>
#include <string.h>

namespace chip {

const size_t kTestSuitesMax = 128;

typedef struct
{
    UnitTestTriggerFunction test_suites[kTestSuitesMax];
    uint32_t num_test_suites;
} test_suites_t;

static test_suites_t gs_test_suites;

#if __ZEPHYR__
inline static bool AlreadyExists(UnitTestTriggerFunction tests)
{
    for (uint32_t i = 0; i < gs_test_suites.num_test_suites; ++i)
        if (gs_test_suites.test_suites[i] == tests)
            return true;
    return false;
}
#endif

CHIP_ERROR RegisterUnitTests(UnitTestTriggerFunction tests)
{
    if (gs_test_suites.num_test_suites >= kTestSuitesMax)
    {
        ChipLogError(Support, "Test suits limit reached");
        return CHIP_ERROR_NO_MEMORY;
    }

#if __ZEPHYR__
    // Not sure yet if it's a Zephyr bug or misconfiguration, but global constructors are called
    // twice on native_posix platform - by libc and by Zephyr's main thread initialization code.
    // This makes sure tests are not run twice for that reason.
    if (AlreadyExists(tests))
        return CHIP_NO_ERROR;
#endif

    gs_test_suites.test_suites[gs_test_suites.num_test_suites] = tests;
    gs_test_suites.num_test_suites++;
    return CHIP_NO_ERROR;
}

int RunRegisteredUnitTests()
{
    int status = 0;
    for (uint32_t i = 0; i < gs_test_suites.num_test_suites; i++)
    {
        status += gs_test_suites.test_suites[i]();
    }
    return status;
}

} // namespace chip
