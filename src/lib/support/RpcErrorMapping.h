/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>
#include <pw_status/status.h>

namespace chip {
namespace rpc {

/**
 * @brief Maps a pw::Status RPC error code to a CHIP_ERROR.
 *
 * Provides a 1:1 mapping between RPC error codes and Matter stack error codes.
 *
 * @param status The pw::Status error code from an RPC call.
 * @return Closest semantic corresponding CHIP_ERROR value (default to CHIP_ERROR_INTERNAL for unrecognized codes).
 */
static inline CHIP_ERROR MapRpcStatusToChipError(const ::pw::Status & status)
{
    switch (status.code())
    {
    case pw_Status::PW_STATUS_OK:
        return CHIP_NO_ERROR;
    case pw_Status::PW_STATUS_CANCELLED:
        return CHIP_ERROR_CANCELLED;
    case pw_Status::PW_STATUS_UNKNOWN:
        return CHIP_ERROR_INTERNAL; // No CHIP_ERROR_UNKNOWN, use INTERNAL
    case pw_Status::PW_STATUS_INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case pw_Status::PW_STATUS_DEADLINE_EXCEEDED:
        return CHIP_ERROR_TIMEOUT;
    case pw_Status::PW_STATUS_NOT_FOUND:
        return CHIP_ERROR_NOT_FOUND;
    case pw_Status::PW_STATUS_ALREADY_EXISTS:
        return CHIP_ERROR_ALREADY_INITIALIZED; // No CHIP_ERROR_EXISTS, use ALREADY_INITIALIZED
    case pw_Status::PW_STATUS_PERMISSION_DENIED:
        return CHIP_ERROR_ACCESS_DENIED;
    case pw_Status::PW_STATUS_RESOURCE_EXHAUSTED:
        return CHIP_ERROR_NO_MEMORY; // No CHIP_ERROR_RESOURCE_EXHAUSTED, use NO_MEMORY
    case pw_Status::PW_STATUS_FAILED_PRECONDITION:
        return CHIP_ERROR_INCORRECT_STATE; // No CHIP_ERROR_PRECONDITION_NOT_MET, use INCORRECT_STATE
    case pw_Status::PW_STATUS_ABORTED:
        return CHIP_ERROR_TRANSACTION_CANCELED; // No CHIP_ERROR_ABORTED, use TRANSACTION_CANCELED
    case pw_Status::PW_STATUS_OUT_OF_RANGE:
        return CHIP_ERROR_INVALID_ARGUMENT; // No CHIP_ERROR_OUT_OF_RANGE, use INVALID_ARGUMENT
    case pw_Status::PW_STATUS_UNIMPLEMENTED:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case pw_Status::PW_STATUS_INTERNAL:
        return CHIP_ERROR_INTERNAL;
    case pw_Status::PW_STATUS_UNAVAILABLE:
        return CHIP_ERROR_NOT_CONNECTED; // No CHIP_ERROR_UNAVAILABLE, use NOT_CONNECTED
    case pw_Status::PW_STATUS_DATA_LOSS:
        return CHIP_ERROR_DECODE_FAILED; // No CHIP_ERROR_DATA_LOSS, use DECODE_FAILED
    case pw_Status::PW_STATUS_UNAUTHENTICATED:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED; // No CHIP_ERROR_UNAUTHENTICATED, use INTEGRITY_CHECK_FAILED
    default:
        // Generic error for unrecognized status codes, i.e. pw_Status enum is expanded.
        return CHIP_ERROR_INTERNAL;
    }
}

} // namespace rpc
} // namespace chip
