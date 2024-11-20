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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <inet/InetError.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>

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

TEST(TestInetErrorStr, CheckInetErrorStr)
{
    // Register the layer error formatter
    Inet::RegisterLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        const char * errStr = ErrorStr(err);
        char expectedText[9];

        // Assert that the error string contains the error number in hex.
        snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, err.AsInteger());
        EXPECT_NE(strstr(errStr, expectedText), nullptr);

#if !CHIP_CONFIG_SHORT_ERROR_STR
        // Assert that the error string contains a description, which is signaled
        // by a presence of a colon proceeding the description.
        EXPECT_NE(strchr(errStr, ':'), nullptr);
#endif // !CHIP_CONFIG_SHORT_ERROR_STR
    }
}
