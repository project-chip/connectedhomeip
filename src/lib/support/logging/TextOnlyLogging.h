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
 *      This file defines macros, constants, and interfaces for a
 *      platform-independent logging interface for the chip SDK.
 *
 *      chip SDK clients may choose, at compile time, among Android,
 *      C Standard I/O, or external (platform- and integrator-defined)
 *      logging style implementations that will be invoked when any of
 *      the following preprocessor symbols are asserted:
 *
 *         - #CHIP_ERROR_LOGGING
 *         - #CHIP_PROGRESS_LOGGING
 *         - #CHIP_DETAIL_LOGGING
 *         - #CHIP_AUTOMATION_LOGGING
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#include <lib/support/Compiler.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/VerificationMacrosNoLogging.h>
#include <lib/support/logging/Constants.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <typeinfo>

#if CHIP_SYSTEM_CONFIG_PLATFORM_LOG && defined(CHIP_SYSTEM_CONFIG_PLATFORM_LOG_INCLUDE)
#include CHIP_SYSTEM_CONFIG_PLATFORM_LOG_INCLUDE
#endif

#if CHIP_PW_TOKENIZER_LOGGING
#include "pw_tokenizer/tokenize.h"
#endif

/**
 *   @namespace chip::Logging
 *
 *   @brief
 *     This namespace includes all interfaces within chip for shared
 *     logging support.
 *
 *     The interfaces include macros, constants, and functions for a
 *     platform-independent logging interface for the chip SDK.
 *
 *     chip SDK clients may choose, at compile time, among Android,
 *     C Standard I/O, or external (platform- and integrator-defined)
 *     logging style implementations that will be invoked when any of
 *     the following preprocessor symbols are asserted:
 *
 *        - #CHIP_ERROR_LOGGING
 *        - #CHIP_PROGRESS_LOGGING
 *        - #CHIP_DETAIL_LOGGING
 *        - #CHIP_AUTOMATION_LOGGING
 *
 */

namespace chip {

namespace Logging {

// Log redirection
using LogRedirectCallback_t = void (*)(const char * module, uint8_t category, const char * msg, va_list args);

#if CHIP_ERROR_LOGGING
/**
 * @def ChipLogError(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Error'
 *   category.
 *
 */
#define ChipLogError(MOD, MSG, ...) ChipInternalLog(MOD, ERROR, MSG, ##__VA_ARGS__)
#else // CHIP_ERROR_LOGGING
#define ChipLogError(MOD, MSG, ...) ((void) 0)
#endif // CHIP_ERROR_LOGGING

#if CHIP_PROGRESS_LOGGING
/**
 * @def ChipLogProgress(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Progress'
 *   category.
 *
 */
#define ChipLogProgress(MOD, MSG, ...) ChipInternalLog(MOD, PROGRESS, MSG, ##__VA_ARGS__)
#else // CHIP_PROGRESS_LOGGING
#define ChipLogProgress(MOD, MSG, ...) ((void) 0)
#endif // CHIP_PROGRESS_LOGGING

#if CHIP_DETAIL_LOGGING
/**
 * @def ChipLogDetail(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Detail'
 *   category.
 *
 */
#define ChipLogDetail(MOD, MSG, ...) ChipInternalLog(MOD, DETAIL, MSG, ##__VA_ARGS__)

#else // CHP_DETAIL_LOGGING
#define ChipLogDetail(MOD, MSG, ...) ((void) 0)
#endif // CHIP_DETAIL_LOGGING

#if CHIP_AUTOMATION_LOGGING
/**
 * @def ChipLogAutomation(MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Automation'
 *   category.
 *
 */
#define ChipLogAutomation(MSG, ...) ChipInternalLog(Automation, AUTOMATION, MSG, ##__VA_ARGS__)
#else // CHIP_AUTOMATION_LOGGING
#define ChipLogAutomation(MOD, MSG, ...) ((void) 0)
#endif // CHIP_AUTOMATION_LOGGING

/**
 *  @def ChipLogIfFalse(aCondition)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true and emits some log, based on configuration, if
 *    the condition is false.
 *
 *  @note
 *    Evaluation of @a aCondition is always done, but logging is only enabled when
 *    #CHIP_CONFIG_ENABLE_CONDITION_LOGGING is enabled. This can be turned on or
 *    off for each compilation unit by enabling or disabling, as desired,
 *    #CHIP_CONFIG_ENABLE_CONDITION_LOGGING before ChipLogging.h is included by
 *    the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  #define CHIP_CONFIG_ENABLE_CONDITION_LOGGING 1
 *
 *  #include <chip/Support/Logging/ChipLogging.h>
 *
 *  ...
 *
 *  void foo(void)
 *  {
 *      CHIP_ERROR err = CHIP_NO_ERROR;
 *
 *      ...
 *
 *  exit:
 *      ChipLogIfFalse(CHIP_END_OF_TLV == err);
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition     A Boolean expression to be evaluated.
 *
 *  @sa CHIP_CONFIG_ENABLE_TRACE_ON_CHECK_FAILURE
 *
 */

#if CHIP_CONFIG_ENABLE_CONDITION_LOGGING && !defined(ChipLogIfFalse)

#define ChipLogIfFalse(aCondition)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(aCondition))                                                                                                         \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "Condition Failed (%s) at %s:%d", #aCondition, __FILE__, __LINE__);                         \
        }                                                                                                                          \
    } while (0)

#else // CHIP_CONFIG_ENABLE_CONDITION_LOGGING

#define ChipLogIfFalse(aCondition) IgnoreUnusedVariable(aCondition)

#endif // CHIP_CONFIG_ENABLE_CONDITION_LOGGING

/*
 *  @brief
 *      Macro for use in a string formatter for a 64-bit hex print.
 *      Will split into 2x 32-bit prints to support small printf libraries
 *
 *  Example Usage:
 *
 *  @code
 *  void foo() {
 *      uint64_t value = 0x1122334455667788;
 *      ChipLogProgress(Foo, "A 64-bit value: 0x" ChipLogFormatX64, ChipLogValueX64(value));
 *  }
 *  @endcode
 *
 */
#define ChipLogFormatX64 "%08" PRIX32 "%08" PRIX32

/*
 *  @brief
 *      Macro for use in a printf parameter list for a 64-bit value.
 *      Will split into MSB/LSB 32-bit values to use only 32-bit formatting.
 *
 *  Example Usage:
 *
 *  @code
 *  void foo() {
 *      uint64_t value = 0x1122334455667788;
 *      ChipLogProgress(Foo, "A 64-bit value: 0x" ChipLogFormatX64, ChipLogValueX64(value));
 *  }
 *  @endcode
 *
 *  @param[in]  aValue    64-bit value that will be split in 32-bit MSB/LSB part
 */
#define ChipLogValueX64(aValue) static_cast<uint32_t>(aValue >> 32), static_cast<uint32_t>(aValue)

/*
 *  @brief
 *      Macro for use in a string formatter for a MEI hex print.
 *      Will split into 2x 16-bit prints to display both the MEI prefix/suffix
 *
 *  Example Usage:
 *
 *  @code
 *  void foo() {
 *      chip::CommandId value = 0x12340001;
 *      ChipLogProgress(Foo, "A MEI value: " ChipLogFormatMEI, ChipLogValueMEI(value));
 *  }
 *  @endcode
 *
 */
#define ChipLogFormatMEI "0x%04X_%04X"

/*
 *  @brief
 *      Macro for use in a printf parameter list for MEI value.
 *      Will split into MSB/LSB 16-bit values to separate prefix/suffix.
 *
 *  Example Usage:
 *
 *  @code
 *  void foo() {
 *      chip::CommandId value = 0x12340001;
 *      ChipLogProgress(Foo, "A MEI value: " ChipLogFormatMEI, ChipLogValueMEI(value));
 *  }
 *  @endcode
 *
 *  @param[in]  aValue    "32-bit value that will be split in 16-bit MSB/LSB part
 */
#define ChipLogValueMEI(aValue) static_cast<uint16_t>(aValue >> 16), static_cast<uint16_t>(aValue)

/**
 * Logging helpers for exchanges.  Log the exchange id, whether
 * it's an initiator or responder and the scoped node.  Some callsites only
 * have the exchange id and initiator/responder boolean, not an actual exchange,
 * so we want to have a helper for that case too.
 */
#define ChipLogFormatExchangeId "%u%c"
#define ChipLogValueExchangeId(id, isInitiator) id, ((isInitiator) ? 'i' : 'r')

#if CHIP_EXCHANGE_NODE_ID_LOGGING
#define ChipLogFormatExchange ChipLogFormatExchangeId " with Node: " ChipLogFormatScopedNodeId
#define ChipLogValueExchange(ec)                                                                                                   \
    ChipLogValueExchangeId((ec)->GetExchangeId(), (ec)->IsInitiator()),                                                            \
        ChipLogValueScopedNodeId((ec)->HasSessionHandle() ? (ec)->GetSessionHandle()->GetPeer() : ScopedNodeId())
#else // CHIP_EXCHANGE_NODE_ID_LOGGING
#define ChipLogFormatExchange ChipLogFormatExchangeId
#define ChipLogValueExchange(ec) ChipLogValueExchangeId((ec)->GetExchangeId(), (ec)->IsInitiator())
#endif // CHIP_EXCHANGE_NODE_ID_LOGGING

#define ChipLogValueExchangeIdFromSentHeader(payloadHeader)                                                                        \
    ChipLogValueExchangeId((payloadHeader).GetExchangeID(), (payloadHeader).IsInitiator())
// A received header's initiator boolean is the inverse of the exchange's.
#define ChipLogValueExchangeIdFromReceivedHeader(payloadHeader)                                                                    \
    ChipLogValueExchangeId((payloadHeader).GetExchangeID(), !(payloadHeader).IsInitiator())

/**
 * Logging helpers for logging the dynamic type of an object, if possible.
 *
 * Primarily useful when logging the type of delegates or similar objects when
 * performing logging for a fatal error in DumpToLog().
 *
 * Example:
 * @code
 * ChipLogError(Foo, "Delegate=" ChipLogFormatRtti, ChipLogValueRtti(mDelegate));
 * @endcode
 */
#define ChipLogFormatRtti "%s"
#if CHIP_HAVE_RTTI
#define ChipLogValueRtti(ptr) ((ptr) != nullptr ? typeid(*(ptr)).name() : "null")
#else
#define ChipLogValueRtti(ptr) ((ptr) != nullptr ? "?" : "null")
#endif

/**
 * Logging helpers for protocol ids.  A protocol id is a (vendor-id,
 * protocol-id) pair.
 */
#define ChipLogFormatProtocolId "(%u, %u)"
#define ChipLogValueProtocolId(id) (id).GetVendorId(), (id).GetProtocolId()

/**
 * Logging helpers for message counters, so we format them consistently.
 */
#define ChipLogFormatMessageCounter "%" PRIu32

/**
 * Logging helpers for message types, so we format them consistently.
 */
#define ChipLogFormatMessageType "0x%x"

/** Logging helpers for scoped node ids, which is a tuple of <NodeId, FabricIndex> */
#define ChipLogFormatScopedNodeId "<" ChipLogFormatX64 ", %d>"
#define ChipLogValueScopedNodeId(id) ChipLogValueX64((id).GetNodeId()), (id).GetFabricIndex()

/** Logging helpers for PeerId, which is a tuple of <Compressed Fabric Id, NodeId>
 *
 * This gets logged in the form that's used for the DNS-SD instance name for the
 * peer.
 */
#define ChipLogFormatPeerId ChipLogFormatX64 "-" ChipLogFormatX64
#define ChipLogValuePeerId(id) ChipLogValueX64((id).GetCompressedFabricId()), ChipLogValueX64((id).GetNodeId())

/**
 * CHIP Logging Implementation internals.
 */

#if CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING || CHIP_AUTOMATION_LOGGING
#define _CHIP_USE_LOGGING 1
#else
#define _CHIP_USE_LOGGING 0
#endif // CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING || CHIP_AUTOMATION_LOGGING

// Log filtering (no-op unless CHIP_LOG_FILTERING is enabled)
#if _CHIP_USE_LOGGING && CHIP_LOG_FILTERING
DLL_EXPORT uint8_t GetLogFilter();
DLL_EXPORT void SetLogFilter(uint8_t category);
bool IsCategoryEnabled(uint8_t category);
#else  // _CHIP_USE_LOGGING && CHIP_LOG_FILTERING
inline uint8_t GetLogFilter()
{
    return kLogCategory_Max;
}

inline void SetLogFilter(uint8_t category) {}

inline bool IsCategoryEnabled(uint8_t category)
{
    return true;
}
#endif // _CHIP_USE_LOGGING && CHIP_LOG_FILTERING

#if _CHIP_USE_LOGGING

// Get the module name associated with a LogModule, or "-" on invalid value.
const char * GetModuleName(LogModule module);

// Log redirection
DLL_EXPORT void SetLogRedirectCallback(LogRedirectCallback_t callback);

static constexpr uint16_t kMaxModuleNameLen = 3;

/* Internal macros mapping upper case definitions to camel case category constants*/
#define CHIP_LOG_CATEGORY_DETAIL chip::Logging::kLogCategory_Detail
#define CHIP_LOG_CATEGORY_PROGRESS chip::Logging::kLogCategory_Progress
#define CHIP_LOG_CATEGORY_ERROR chip::Logging::kLogCategory_Error
#define CHIP_LOG_CATEGORY_AUTOMATION chip::Logging::kLogCategory_Automation

/*
 * CHIP Logging Modules Categories filtering implementation.
 *
 * @brief
 *     Macro for use to check if given category is enabled for given log module.
 *
 * Example Usage:
 *     Let's assume PROGRESS category control for DeviceLayer log module.
 *
 *     Default behavior - category is not modified, so macro returns global category value:
 *     IsModuleCategoryEnabled(DeviceLayer, PROGRESS) returns CHIP_PROGRESS_LOGGING
 *
 *     Enabling category - category is enabled for module, the category value is ignored and the global value is used:
 *     #define CHIP_CONFIG_LOG_MODULE_DeviceLayer_PROGRESS 1
 *     IsModuleCategoryEnabled(DeviceLayer, PROGRESS) returns CHIP_PROGRESS_LOGGING
 *
 *     Disabling category - category is disabled for module, ignoring global category value:
 *     #define CHIP_CONFIG_LOG_MODULE_DeviceLayer_PROGRESS 0
 *     IsModuleCategoryEnabled(DeviceLayer, PROGRESS) returns 0
 *
 * Algorithm flow:
 *     1. IsModuleCategoryEnabled(MOD, CAT) uses MOD and CAT to create strings for category module macro and global category macro,
 * and invokes _IsModuleCategoryEnabled1().
 *     2. _IsModuleCategoryEnabled1(MOD_CAT, GLOB_CAT) invokes _IsModuleCategoryEnabled2(MOD_CAT, GLOB_CAT) to extract macros
 * values.
 *     3. _IsModuleCategoryEnabled2(MOD_CAT, GLOB_CAT) uses MOD_CAT to create string for helper macro.
 *        - If MOD_CAT is 0 the helper macro containing dummy arg is used.
 *        - If MOD_CAT doesn't exist, empty is used.
 *     4. _IsModuleCategoryEnabled3(DUMMY_ARG, GLOB_CAT) invokes _IsModuleCategoryEnabled4() using different number of arguments
 * depending on DUMMY_ARG.
 *     5. _IsModuleCategoryEnabled4 output:
 *        - If category for module was not defined or define 1, the DUMMY_ARG was empty, so returning ARG2 is GLOB_CAT.
 *        - If category for module was defined 0, the DUMMY_ARG had one argument, so returning ARG2 is 0.
 *
 */
#define IsModuleCategoryEnabled(MOD, CAT) _IsModuleCategoryEnabled1(CHIP_CONFIG_LOG_MODULE_##MOD##_##CAT, CHIP_##CAT##_LOGGING)
#define _IsModuleCategoryEnabled1(MOD_CAT, GLOB_CAT) _IsModuleCategoryEnabled2(MOD_CAT, GLOB_CAT)
#define _IsModuleCategoryEnabled2(MOD_CAT, GLOB_CAT)                                                                               \
    _IsModuleCategoryEnabled3(_IsModuleCategoryEnabled3_DummyArg##MOD_CAT, GLOB_CAT)
#define _IsModuleCategoryEnabled3_DummyArg0 dummyArg,
#define _IsModuleCategoryEnabled3(DUMMY_ARG, GLOB_CAT) _IsModuleCategoryEnabled4(DUMMY_ARG 0, GLOB_CAT)
#define _IsModuleCategoryEnabled4(ARG1, ARG2, ...) (ARG2)

void Log(uint8_t module, uint8_t category, const char * msg, ...) ENFORCE_FORMAT(3, 4);
void LogV(uint8_t module, uint8_t category, const char * msg, va_list args) ENFORCE_FORMAT(3, 0);

#if CHIP_SYSTEM_CONFIG_PLATFORM_LOG
#ifndef ChipPlatformLog
#error "CHIP_SYSTEM_CONFIG_PLATFORM_LOG is enabled but ChipPlatformLog() is not defined"
#endif
#define ChipInternalLog(...) ChipPlatformLog(__VA_ARGS__)
#else // CHIP_SYSTEM_CONFIG_PLATFORM_LOG
#define ChipInternalLog(MOD, CAT, MSG, ...)                                                                                        \
    if (CHIP_CONFIG_LOG_MODULE_##MOD && IsModuleCategoryEnabled(MOD, CAT))                                                         \
    {                                                                                                                              \
        ChipInternalLogImpl(MOD, CHIP_LOG_CATEGORY_##CAT, MSG, ##__VA_ARGS__);                                                     \
    }

#endif // CHIP_SYSTEM_CONFIG_PLATFORM_LOG

#if CHIP_PW_TOKENIZER_LOGGING

void HandleTokenizedLog(uint32_t levels, pw_tokenizer_Token token, pw_tokenizer_ArgTypes, ...);

#define ChipInternalLogImpl(MOD, CAT, MSG, ...)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        if (chip::Logging::IsCategoryEnabled(CAT))                                                                                 \
        {                                                                                                                          \
            PW_TOKENIZE_FORMAT_STRING(PW_TOKENIZER_DEFAULT_DOMAIN, UINT32_MAX, MSG, __VA_ARGS__);                                  \
            ::chip::Logging::HandleTokenizedLog((uint32_t) ((CAT << 8) | chip::Logging::kLogModule_##MOD), _pw_tokenizer_token,    \
                                                PW_TOKENIZER_ARG_TYPES(__VA_ARGS__) PW_COMMA_ARGS(__VA_ARGS__));                   \
        }                                                                                                                          \
    } while (0)
#else // CHIP_PW_TOKENIZER_LOGGING
#define ChipInternalLogImpl(MOD, CAT, MSG, ...)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        if (chip::Logging::IsCategoryEnabled(CAT))                                                                                 \
        {                                                                                                                          \
            chip::Logging::Log(chip::Logging::kLogModule_##MOD, CAT, MSG, ##__VA_ARGS__);                                          \
        }                                                                                                                          \
    } while (0)
#endif // CHIP_PW_TOKENIZER_LOGGING

#else // _CHIP_USE_LOGGING

inline void SetLogRedirectCallback(LogRedirectCallback_t callback) {}

#endif // _CHIP_USE_LOGGING

} // namespace Logging
} // namespace chip
