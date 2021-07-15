/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a process to effect a functional test for
 *      the CHIP Inet layer library error string support interfaces.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <inet/InetError.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

// Test input data.

// clang-format off
static const CHIP_ERROR kTestElements[] =
{
    INET_ERROR_WRONG_ADDRESS_TYPE,
    INET_ERROR_PEER_DISCONNECTED,
    INET_ERROR_HOST_NOT_FOUND,
    INET_ERROR_DNS_TRY_AGAIN,
    INET_ERROR_DNS_NO_RECOVERY,
    INET_ERROR_WRONG_PROTOCOL_TYPE,
    INET_ERROR_UNKNOWN_INTERFACE,
    INET_ERROR_ADDRESS_NOT_FOUND,
    INET_ERROR_HOST_NAME_TOO_LONG,
    INET_ERROR_INVALID_HOST_NAME,
    INET_ERROR_IDLE_TIMEOUT,
    INET_ERROR_INVALID_IPV6_PKT,
    INET_ERROR_INTERFACE_INIT_FAILURE,
    INET_ERROR_TCP_USER_TIMEOUT,
    INET_ERROR_TCP_CONNECT_TIMEOUT,
    INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE
};
// clang-format on

static void CheckInetErrorStr(nlTestSuite * inSuite, void * inContext)
{
    // Register the layer error formatter
    Inet::RegisterLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        const char * errStr = ErrorStr(err);
        char expectedText[9];

        // Assert that the error string contains the error number in hex.
        snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, ChipError::AsInteger(err));
        NL_TEST_ASSERT(inSuite, (strstr(errStr, expectedText) != nullptr));

#if !CHIP_CONFIG_SHORT_ERROR_STR
        // Assert that the error string contains a description, which is signaled
        // by a presence of a colon proceeding the description.
        NL_TEST_ASSERT(inSuite, (strchr(errStr, ':') != nullptr));
#endif // !CHIP_CONFIG_SHORT_ERROR_STR
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("InetErrorStr", CheckInetErrorStr),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestInetErrorStr(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "Inet-Error-Strings",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInetErrorStr)
