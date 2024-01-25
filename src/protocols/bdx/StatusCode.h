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

#pragma once

#include <lib/core/CHIPError.h>

namespace chip {
namespace bdx {

enum class StatusCode : uint16_t
{
    kLengthTooLarge             = 0x0012,
    kLengthTooShort             = 0x0013,
    kLengthMismatch             = 0x0014,
    kLengthRequired             = 0x0015,
    kBadMessageContents         = 0x0016,
    kBadBlockCounter            = 0x0017,
    kUnexpectedMessage          = 0x0018,
    kResponderBusy              = 0x0019,
    kTransferFailedUnknownError = 0x001F,
    kTransferMethodNotSupported = 0x0050,
    kFileDesignatorUnknown      = 0x0051,
    kStartOffsetNotSupported    = 0x0052,
    kVersionNotSupported        = 0x0053,
    kUnknown                    = 0x005F,
};

StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR error);

} // namespace bdx
} // namespace chip
