/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/silabs/BootloaderError.h>

#include <lib/support/logging/CHIPLogging.h>

#if CHIP_ERROR_LOGGING
extern "C" {
#include "btl_errorcode.h"
}

namespace {

const char * BootloaderErrorCodeName(int32_t errorCode)
{
    switch (errorCode)
    {
    case BOOTLOADER_ERROR_STORAGE_INVALID_SLOT:
        return "BOOTLOADER_ERROR_STORAGE_INVALID_SLOT";
    case BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS:
        return "BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS";
    case BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN:
        return "BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN";
    case BOOTLOADER_ERROR_PARSER_UNEXPECTED:
        return "BOOTLOADER_ERROR_PARSER_UNEXPECTED";
    case BOOTLOADER_ERROR_PARSER_UNKNOWN_TAG:
        return "BOOTLOADER_ERROR_PARSER_UNKNOWN_TAG";
    case BOOTLOADER_ERROR_PARSER_VERSION:
        return "BOOTLOADER_ERROR_PARSER_VERSION";
    case BOOTLOADER_ERROR_PARSER_FILETYPE:
        return "BOOTLOADER_ERROR_PARSER_FILETYPE";
    default:
        return nullptr;
    }
}

} // namespace
#endif // CHIP_ERROR_LOGGING

namespace chip {
namespace DeviceLayer {
namespace Silabs {

void LogBootloaderApiError([[maybe_unused]] const char * apiName, [[maybe_unused]] int32_t errorCode)
{
#if CHIP_ERROR_LOGGING
    const char * errorName = BootloaderErrorCodeName(errorCode);
    if (errorName != nullptr)
    {
        ChipLogError(SoftwareUpdate, "%s() error: %ld (%s)", apiName, errorCode, errorName);
    }
    else
    {
        ChipLogError(SoftwareUpdate, "%s() error: %ld", apiName, errorCode);
    }
#endif // CHIP_ERROR_LOGGING
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
