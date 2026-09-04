/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "StatusCode.h"

namespace chip {
namespace bdx {

StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR error)
{
    auto status = StatusCode::kUnknown;

    if (error == CHIP_ERROR_INCORRECT_STATE)
    {
        status = StatusCode::kUnexpectedMessage;
    }
    else if (error == CHIP_ERROR_INVALID_ARGUMENT)
    {
        status = StatusCode::kBadMessageContents;
    }
    else if (error == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        status = StatusCode::kTransferMethodNotSupported;
    }
    else if (error == CHIP_ERROR_UNKNOWN_RESOURCE_ID)
    {
        status = StatusCode::kFileDesignatorUnknown;
    }

    return status;
}

CHIP_ERROR GetChipErrorFromBdxStatusCode(StatusCode statusCode)
{
    // Map a BDX StatusCode received from a peer to a CHIP_ERROR. This is the rough inverse of
    // GetBdxStatusCodeFromChipError. Any status code that does not have a more specific mapping is
    // reported as CHIP_ERROR_BAD_REQUEST so that a peer-initiated abort is never indistinguishable
    // from an internal error.
    switch (statusCode)
    {
    case StatusCode::kUnexpectedMessage:
        return CHIP_ERROR_INCORRECT_STATE;
    case StatusCode::kLengthTooLarge:
    case StatusCode::kLengthTooShort:
    case StatusCode::kLengthMismatch:
    case StatusCode::kLengthRequired:
    case StatusCode::kBadMessageContents:
    case StatusCode::kBadBlockCounter:
    case StatusCode::kStartOffsetNotSupported:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case StatusCode::kResponderBusy:
        return CHIP_ERROR_BUSY;
    case StatusCode::kTransferMethodNotSupported:
    case StatusCode::kVersionNotSupported:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    case StatusCode::kFileDesignatorUnknown:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    case StatusCode::kTransferFailedUnknownError:
    case StatusCode::kUnknown:
    default:
        return CHIP_ERROR_BAD_REQUEST;
    }
}

} // namespace bdx
} // namespace chip
