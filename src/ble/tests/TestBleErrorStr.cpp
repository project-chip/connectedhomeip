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
 *      the CHIP BLE layer library error string support interfaces.
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

#include <ble/BleError.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

// Test input data.

// clang-format off
static const CHIP_ERROR kTestElements[] =
{
    BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK,
    BLE_ERROR_CENTRAL_UNSUBSCRIBED,
    BLE_ERROR_GATT_SUBSCRIBE_FAILED,
    BLE_ERROR_GATT_UNSUBSCRIBE_FAILED,
    BLE_ERROR_GATT_WRITE_FAILED,
    BLE_ERROR_GATT_INDICATE_FAILED,
    BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT,
    BLE_ERROR_REMOTE_DEVICE_DISCONNECTED,
    BLE_ERROR_APP_CLOSED_CONNECTION,
    BLE_ERROR_NOT_CHIP_DEVICE,
    BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS,
    BLE_ERROR_INVALID_FRAGMENT_SIZE,
    BLE_ERROR_START_TIMER_FAILED,
    BLE_ERROR_CONNECT_TIMED_OUT,
    BLE_ERROR_RECEIVE_TIMED_OUT,
    BLE_ERROR_INVALID_MESSAGE,
    BLE_ERROR_FRAGMENT_ACK_TIMED_OUT,
    BLE_ERROR_KEEP_ALIVE_TIMED_OUT,
    BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK,
    BLE_ERROR_INVALID_ACK,
    BLE_ERROR_REASSEMBLER_MISSING_DATA,
    BLE_ERROR_INVALID_BTP_HEADER_FLAGS,
    BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER,
    BLE_ERROR_REASSEMBLER_INCORRECT_STATE,
};
// clang-format on

static void CheckBleErrorStr(nlTestSuite * inSuite, void * inContext)
{
    // Register the layer error formatter

    Ble::RegisterLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        const char * errStr = ErrorStr(err);
        char expectedText[9];

        // Assert that the error string contains the error number in hex.
        snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, err.AsInteger());
        NL_TEST_ASSERT(inSuite, (strstr(errStr, expectedText) != NULL));

#if !CHIP_CONFIG_SHORT_ERROR_STR
        // Assert that the error string contains a description, which is signaled
        // by a presence of a colon proceeding the description.
        NL_TEST_ASSERT(inSuite, (strchr(errStr, ':') != NULL));
#endif // !CHIP_CONFIG_SHORT_ERROR_STR
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BleErrorStr", CheckBleErrorStr),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestBleErrorStr(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "Ble-Error-Strings",
        &sTests[0],
        NULL,
        NULL
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBleErrorStr)
