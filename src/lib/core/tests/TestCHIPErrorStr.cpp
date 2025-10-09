/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *      the CHIP Core library error string support interfaces.
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

// Test input data.

// clang-format off
static const CHIP_ERROR kTestElements[] =
{
    CHIP_ERROR_SENDING_BLOCKED,
    CHIP_ERROR_CONNECTION_ABORTED,
    CHIP_ERROR_INCORRECT_STATE,
    CHIP_ERROR_MESSAGE_TOO_LONG,
    CHIP_ERROR_RECURSION_DEPTH_LIMIT,
    CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS,
    CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER,
    CHIP_ERROR_NO_CONNECTION_HANDLER,
    CHIP_ERROR_TOO_MANY_PEER_NODES,
    CHIP_ERROR_SENTINEL,
    CHIP_ERROR_NO_MEMORY,
    CHIP_ERROR_NO_MESSAGE_HANDLER,
    CHIP_ERROR_MESSAGE_INCOMPLETE,
    CHIP_ERROR_DATA_NOT_ALIGNED,
    CHIP_ERROR_UNKNOWN_KEY_TYPE,
    CHIP_ERROR_KEY_NOT_FOUND,
    CHIP_ERROR_WRONG_ENCRYPTION_TYPE,
    CHIP_ERROR_INVALID_UTF8,
    CHIP_ERROR_INTEGRITY_CHECK_FAILED,
    CHIP_ERROR_INVALID_SIGNATURE,
    CHIP_ERROR_INVALID_TLV_CHAR_STRING,
    CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE,
    CHIP_ERROR_INVALID_MESSAGE_LENGTH,
    CHIP_ERROR_BUFFER_TOO_SMALL,
    CHIP_ERROR_DUPLICATE_KEY_ID,
    CHIP_ERROR_WRONG_KEY_TYPE,
    CHIP_ERROR_UNINITIALIZED,
    CHIP_ERROR_INVALID_STRING_LENGTH,
    CHIP_ERROR_INVALID_LIST_LENGTH,
    CHIP_ERROR_FAILED_DEVICE_ATTESTATION,
    CHIP_END_OF_TLV,
    CHIP_ERROR_TLV_UNDERRUN,
    CHIP_ERROR_INVALID_TLV_ELEMENT,
    CHIP_ERROR_INVALID_TLV_TAG,
    CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG,
    CHIP_ERROR_WRONG_TLV_TYPE,
    CHIP_ERROR_TLV_CONTAINER_OPEN,
    CHIP_ERROR_INVALID_MESSAGE_TYPE,
    CHIP_ERROR_UNEXPECTED_TLV_ELEMENT,
    CHIP_ERROR_NOT_IMPLEMENTED,
    CHIP_ERROR_INVALID_ADDRESS,
    CHIP_ERROR_INVALID_ARGUMENT,
    CHIP_ERROR_INVALID_PATH_LIST,
    CHIP_ERROR_INVALID_DATA_LIST,
    CHIP_ERROR_TIMEOUT,
    CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR,
    CHIP_ERROR_INVALID_PASE_PARAMETER,
    CHIP_ERROR_INVALID_USE_OF_SESSION_KEY,
    CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY,
    CHIP_ERROR_MISSING_TLV_ELEMENT,
    CHIP_ERROR_RANDOM_DATA_UNAVAILABLE,
    CHIP_ERROR_HOST_PORT_LIST_EMPTY,
    CHIP_ERROR_FORCED_RESET,
    CHIP_ERROR_NO_ENDPOINT,
    CHIP_ERROR_INVALID_DESTINATION_NODE_ID,
    CHIP_ERROR_NOT_CONNECTED,
    CHIP_ERROR_CA_CERT_NOT_FOUND,
    CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED,
    CHIP_ERROR_CERT_PATH_TOO_LONG,
    CHIP_ERROR_CERT_USAGE_NOT_ALLOWED,
    CHIP_ERROR_CERT_EXPIRED,
    CHIP_ERROR_CERT_NOT_VALID_YET,
    CHIP_ERROR_UNSUPPORTED_CERT_FORMAT,
    CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE,
    CHIP_ERROR_CERT_NOT_FOUND,
    CHIP_ERROR_INVALID_CASE_PARAMETER,
    CHIP_ERROR_CERT_LOAD_FAILED,
    CHIP_ERROR_CERT_NOT_TRUSTED,
    CHIP_ERROR_WRONG_CERT_DN,
    CHIP_ERROR_WRONG_NODE_ID,
    CHIP_ERROR_RETRANS_TABLE_FULL,
    CHIP_ERROR_TRANSACTION_CANCELED,
    CHIP_ERROR_INVALID_SUBSCRIPTION,
    CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
    CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR,
    CHIP_ERROR_INVALID_FABRIC_INDEX,
    CHIP_ERROR_TOO_MANY_CONNECTIONS,
    CHIP_ERROR_SHUT_DOWN,
    CHIP_ERROR_CANCELLED,
    CHIP_ERROR_TLV_TAG_NOT_FOUND,
    CHIP_ERROR_MISSING_SECURE_SESSION,
    CHIP_ERROR_INVALID_ADMIN_SUBJECT,
    CHIP_ERROR_INSUFFICIENT_PRIVILEGE,
    CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED,
    CHIP_ERROR_FABRIC_EXISTS,
    CHIP_ERROR_ENDPOINT_EXISTS,
    CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER,
    CHIP_ERROR_INVALID_KEY_ID,
    CHIP_ERROR_INVALID_TIME,
    CHIP_ERROR_SCHEMA_MISMATCH,
    CHIP_ERROR_INVALID_INTEGER_VALUE,
    CHIP_ERROR_BAD_REQUEST,
    CHIP_ERROR_WRONG_CERT_TYPE,
    CHIP_ERROR_PERSISTED_STORAGE_FAILED,
    CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND,
    CHIP_ERROR_IM_FABRIC_DELETED,
    CHIP_ERROR_ACCESS_DENIED,
    CHIP_ERROR_UNKNOWN_RESOURCE_ID,
    CHIP_ERROR_VERSION_MISMATCH,
    CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL,
    CHIP_EVENT_ID_FOUND,
    CHIP_ERROR_INTERNAL,
    CHIP_ERROR_OPEN_FAILED,
    CHIP_ERROR_READ_FAILED,
    CHIP_ERROR_WRITE_FAILED,
    CHIP_ERROR_DECODE_FAILED,
    CHIP_ERROR_MDNS_COLLISION,
    CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB,
    CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB,
    CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB,
    CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB,
    CHIP_ERROR_PEER_NODE_NOT_FOUND,
    CHIP_ERROR_HSM,
    CHIP_ERROR_IM_STATUS_CODE_RECEIVED,
    CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB,
    CHIP_ERROR_NOT_FOUND,
    CHIP_ERROR_INVALID_FILE_IDENTIFIER,
    CHIP_ERROR_BUSY,
    CHIP_ERROR_MAX_RETRY_EXCEEDED,
    CHIP_ERROR_PROVIDER_LIST_EXHAUSTED,
    CHIP_ERROR_INVALID_SCHEME_PREFIX,
    CHIP_ERROR_MISSING_URI_SEPARATOR,
    CHIP_ERROR_INVALID_FILE_IDENTIFIER,
    CHIP_ERROR_BUSY,
    CHIP_ERROR_HANDLER_NOT_SET,
    CHIP_ERROR_IN_PROGRESS,
};
// clang-format on

void CheckCoreErrorStrHelper(const char * errStr, CHIP_ERROR err)
{
    char expectedText[9];

    // Assert that the error string contains the error number in hex.
    snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, static_cast<uint32_t>(err.AsInteger()));
    EXPECT_TRUE((strstr(errStr, expectedText) != nullptr));

#if !CHIP_CONFIG_SHORT_ERROR_STR
    // Assert that the error string contains a description, which is signaled
    // by a presence of a colon proceeding the description.
    EXPECT_TRUE((strchr(errStr, ':') != nullptr));
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

#if CHIP_CONFIG_ERROR_SOURCE
    // GetFile() should be relative to ${chip_root}
    char const * const file = err.GetFile();
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(strstr(file, "src/lib/core/"), file);

    // File should be included in the error.
    EXPECT_NE(strstr(errStr, file), nullptr);
#endif // CHIP_CONFIG_ERROR_SOURCE
}

TEST(TestCHIPErrorStr, CheckCoreErrorStr)
{
    // Register the layer error formatter

    RegisterCHIPLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        // ErrorStr with static char array.
        CheckCoreErrorStrHelper(ErrorStr(err, /*withSourceLocation=*/true), err);
    }

    // Deregister the layer error formatter
    DeregisterCHIPLayerErrorFormatter();
}

TEST(TestCHIPErrorStr, CheckCoreErrorStrStorage)
{
    // Register the layer error formatter

    RegisterCHIPLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        // ErrorStr with given storage.
        ErrorStrStorage storage;
        CheckCoreErrorStrHelper(ErrorStr(err, /*withSourceLocation=*/true, storage), err);
    }

    // Deregister the layer error formatter
    DeregisterCHIPLayerErrorFormatter();
}

void CheckCoreErrorStrWithoutSourceLocationHelper(const char * errStr, CHIP_ERROR err)
{
    char expectedText[9];

    // Assert that the error string contains the error number in hex.
    snprintf(expectedText, sizeof(expectedText), "%08" PRIX32, static_cast<uint32_t>(err.AsInteger()));
    EXPECT_TRUE((strstr(errStr, expectedText) != nullptr));

#if !CHIP_CONFIG_SHORT_ERROR_STR
    // Assert that the error string contains a description, which is signaled
    // by a presence of a colon proceeding the description.
    EXPECT_TRUE((strchr(errStr, ':') != nullptr));
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

#if CHIP_CONFIG_ERROR_SOURCE
    char const * const file = err.GetFile();
    ASSERT_NE(file, nullptr);
    // File should not be included in the error.
    EXPECT_EQ(strstr(errStr, file), nullptr);
#endif // CHIP_CONFIG_ERROR_SOURCE
}

TEST(TestCHIPErrorStr, CheckCoreErrorStrWithoutSourceLocation)
{
    // Register the layer error formatter

    RegisterCHIPLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        // ErrorStr with static char array.
        CheckCoreErrorStrWithoutSourceLocationHelper(ErrorStr(err, /*withSourceLocation=*/false), err);
    }

    // Deregister the layer error formatter
    DeregisterCHIPLayerErrorFormatter();
}

TEST(TestCHIPErrorStr, CheckCoreErrorStrStorageWithoutSourceLocation)
{
    // Register the layer error formatter

    RegisterCHIPLayerErrorFormatter();

    // For each defined error...
    for (const auto & err : kTestElements)
    {
        // ErrorStr with given storage.
        ErrorStrStorage storage;
        CheckCoreErrorStrWithoutSourceLocationHelper(ErrorStr(err, /*withSourceLocation=*/false, storage), err);
    }

    // Deregister the layer error formatter
    DeregisterCHIPLayerErrorFormatter();
}
