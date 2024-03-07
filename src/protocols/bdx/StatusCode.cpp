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

} // namespace bdx
} // namespace chip
