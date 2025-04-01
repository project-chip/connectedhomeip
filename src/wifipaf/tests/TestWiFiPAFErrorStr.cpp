/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    Copyright (c) 2025 NXP Inc.
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
 *      the CHIP WiFiPAF layer library error string support interfaces.
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/ErrorStr.h>

#include <wifipaf/WiFiPAFError.h>

using namespace chip;

// Test input data.

static const CHIP_ERROR kTestElements[] = {
    WIFIPAF_ERROR_NO_CONNECTION_RECEIVED_CALLBACK,
    WIFIPAF_ERROR_CHIPPAF_PROTOCOL_ABORT,
    WIFIPAF_ERROR_REMOTE_DEVICE_DISCONNECTED,
    WIFIPAF_ERROR_APP_CLOSED_CONNECTION,
    WIFIPAF_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS,
    WIFIPAF_ERROR_INVALID_FRAGMENT_SIZE,
    WIFIPAF_ERROR_START_TIMER_FAILED,
    WIFIPAF_ERROR_CONNECT_TIMED_OUT,
    WIFIPAF_ERROR_RECEIVE_TIMED_OUT,
    WIFIPAF_ERROR_INVALID_MESSAGE,
    WIFIPAF_ERROR_FRAGMENT_ACK_TIMED_OUT,
    WIFIPAF_ERROR_KEEP_ALIVE_TIMED_OUT,
    WIFIPAF_ERROR_NO_CONNECT_COMPLETE_CALLBACK,
    WIFIPAF_ERROR_INVALID_ACK,
    WIFIPAF_ERROR_REASSEMBLER_MISSING_DATA,
    WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS,
    WIFIPAF_ERROR_INVALID_PAFTP_SEQUENCE_NUMBER,
};

TEST(TestWiFiPAFErrorStr, CheckWiFiPAFErrorStr)
{
    // Register the layer error formatter
    WiFiPAF::RegisterLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        const char * errStr = ErrorStr(err);
        char expectedText[9];

        // Assert that the error string contains the error number in hex.
        snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, err.AsInteger());
        EXPECT_NE(strstr(errStr, expectedText), nullptr);
    }
}
