/*
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines macros, constants, and interfaces for logging binary
 *      data (represented by ByteSpan).
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#include <lib/support/Span.h>
#include <lib/support/logging/TextOnlyLogging.h>

namespace chip {
namespace Logging {

#if CHIP_DETAIL_LOGGING

/**
 * @def ChipLogByteSpan(MOD, DATA)
 *
 * @brief
 *   Log a byte span for the specified module in the 'Detail' category.
 *
 */
#define ChipLogByteSpan(MOD, DATA) ChipInternalLogByteSpan(MOD, DETAIL, DATA)

#else // CHIP_DETAIL_LOGGING
#define ChipLogByteSpan(MOD, DATA) ((void) 0)
#endif // CHIP_DETAIL_LOGGING

// _CHIP_USE_LOGGING is defined in TextOnlyLogging.h
#if _CHIP_USE_LOGGING

void LogByteSpan(uint8_t module, uint8_t category, const ByteSpan & span);

#if CHIP_SYSTEM_CONFIG_PLATFORM_LOG
#ifndef ChipPlatformLogByteSpan
#error "CHIP_SYSTEM_CONFIG_PLATFORM_LOG is enabled but ChipPlatformLogByteSpan() is not defined"
#endif
#define ChipInternalLogByteSpan(...) ChipPlatformLogByteSpan(__VA_ARGS__)
#else // CHIP_SYSTEM_CONFIG_PLATFORM_LOG
#define ChipInternalLogByteSpan(MOD, CAT, DATA)                                                                                    \
    if (CHIP_CONFIG_LOG_MODULE_##MOD && IsModuleCategoryEnabled(MOD, CAT))                                                         \
    {                                                                                                                              \
        ChipInternalLogByteSpanImpl(MOD, CHIP_LOG_CATEGORY_##CAT, DATA);                                                           \
    }
#endif // CHIP_SYSTEM_CONFIG_PLATFORM_LOG

#define ChipInternalLogByteSpanImpl(MOD, CAT, DATA)                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        if (chip::Logging::IsCategoryEnabled(CAT))                                                                                 \
        {                                                                                                                          \
            chip::Logging::LogByteSpan(chip::Logging::kLogModule_##MOD, CAT, DATA);                                                \
        }                                                                                                                          \
    } while (0)

#endif // _CHIP_USE_LOGGING

} // namespace Logging
} // namespace chip
