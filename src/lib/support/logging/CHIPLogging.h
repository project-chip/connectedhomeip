/*
 *
 *    <COPYRIGHT>
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
 *
 */

#ifndef CHIPLOGGING_H_
#define CHIPLOGGING_H_

#include <stdint.h>

#include <core/CHIPConfig.h>

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
 *
 */

namespace chip {
namespace Logging {

/**
 *  @enum LogModule
 *
 *  @brief
 *    Identifies a logical section of code that is a source of log
 *    messages.
 *
 *  @note If you add modules or rearrange this list you must update the
 *        ModuleNames tables in chipLogging.cpp.
 *
 */
enum LogModule
{
    kLogModule_NotSpecified         = 0,

    kLogModule_Inet,
    kLogModule_Ble,
    kLogModule_MessageLayer,
    kLogModule_SecurityManager,
    kLogModule_ExchangeManager,
    kLogModule_TLV,
    kLogModule_ASN1,
    kLogModule_Crypto,
    kLogModule_DeviceManager,
    kLogModule_Alarm,
    kLogModule_BDX,
    kLogModule_DataManagement,
    kLogModule_DeviceControl,
    kLogModule_DeviceDescription,
    kLogModule_Echo,
    kLogModule_FabricProvisioning,
    kLogModule_NetworkProvisioning,
    kLogModule_ServiceDirectory,
    kLogModule_ServiceProvisioning,
    kLogModule_SoftwareUpdate,
    kLogModule_TokenPairing,
    kLogModule_TimeService,
    kLogModule_chipTunnel,
    kLogModule_Heartbeat,
    kLogModule_chipSystemLayer,
    kLogModule_EventLogging,
    kLogModule_Support,

    kLogModule_Max
};

/**
 *  @enum LogCategory
 *
 *  @brief
 *    Identifies a category to which an particular error message
 *    belongs.
 *
 */
enum LogCategory
{
    /*!<
     *   This log category indicates, when passed to SetLogFilter(),
     *   that no messages should be logged.
     *
     */
    kLogCategory_None           = 0,

    /*!<
     *   Indicates a category of log message that describes an unexpected
     *   or severe failure.
     *
     *   This log category indicates that a logged message describes
     *   an unexpected or severe failure in the code.
     *
     *   It should be used for things such as out-of-resource errors,
     *   internal inconsistencies, API misuse, etc. In general, errors
     *   that are expected to occur as part of normal operation, or
     *   that are largely determined by external factors (e.g. network
     *   errors, user/operator induced errors, etc.) should be logged
     *   as kLogCategory_Progress messages, not as kLogCategory_Error
     *   messages.
     *
     */
    kLogCategory_Error          = 1,

    /*!<
     *   Indicates a category of log message that describes an event
     *   that marks the start or end of a major activity, or a major
     *   change in the state of the overall system.
     *
     *   It should be reserved for high-level events. Such messages
     *   should provide the log reader with a good sense of the
     *   overall activity of the system at any point in time, while
     *   being minimally verbose. Where necessary such messages should
     *   include identifiers or other values that can be used to
     *   correlate messages involving a common actor or subject
     *   (e.g. connection ids, request ids, etc.) and/or to identify
     *   types of actions being taken or handled (e.g.  message types,
     *   requested resource types, error numbers, etc.).
     *
     */
    kLogCategory_Progress       = 2,

    /*!<
     *   Indicates a category of log message that describes detailed
     *   information about an event or the state of the system.
     *
     *   Such messages can be used to provide ancillary information
     *   not suitable for the kLogCategory_Error and
     *   kLogCategory_Progress categories.
     *
     */
    kLogCategory_Detail         = 3,

    /*!<
     *   Indicates a category of log message that describes information
     *   needed by IE and QA teams for automated testing.
     *
     */
    kLogCategory_Retain         = 4,

    kLogCategory_Max            = kLogCategory_Retain
};

extern void Log(uint8_t module, uint8_t category, const char *msg, ...);
extern uint8_t GetLogFilter(void);
extern void SetLogFilter(uint8_t category);

#ifndef CHIP_ERROR_LOGGING
#define CHIP_ERROR_LOGGING 1
#endif

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 1
#endif

#if CHIP_ERROR_LOGGING
/**
 * @def chipLogError(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Error'
 *   category.
 *
 */
#ifndef chipLogError
#define chipLogError(MOD, MSG, ...) chip::Logging::Log( chip::Logging::kLogModule_##MOD , chip::Logging::kLogCategory_Error, MSG, ## __VA_ARGS__)
#endif
#else
#define chipLogError(MOD, MSG, ...)
#endif

#ifndef CHIP_PROGRESS_LOGGING
#define CHIP_PROGRESS_LOGGING 1
#endif

#if CHIP_PROGRESS_LOGGING
/**
 * @def chipLogProgress(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Progress'
 *   category.
 *
 */
#ifndef chipLogProgress
#define chipLogProgress(MOD, MSG, ...) chip::Logging::Log( chip::Logging::kLogModule_##MOD , chip::Logging::kLogCategory_Progress, MSG, ## __VA_ARGS__)
#endif
#else
#define chipLogProgress(MOD, MSG, ...)
#endif


#ifndef CHIP_DETAIL_LOGGING
#define CHIP_DETAIL_LOGGING 1
#endif

#if CHIP_DETAIL_LOGGING
/**
 * @def chipLogDetail(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Detail'
 *   category.
 *
 */
#ifndef chipLogDetail
#define chipLogDetail(MOD, MSG, ...) chip::Logging::Log( chip::Logging::kLogModule_##MOD , chip::Logging::kLogCategory_Detail, MSG, ## __VA_ARGS__)
#endif
#else
#define chipLogDetail(MOD, MSG, ...)
#endif

#ifndef CHIP_RETAIN_LOGGING
#define CHIP_RETAIN_LOGGING CHIP_PROGRESS_LOGGING
#define chipLogRetain(MOD, MSG, ...) chipLogProgress(MOD, MSG, ## __VA_ARGS__)
#endif

#if CHIP_RETAIN_LOGGING
/**
 * @def chipLogRetain(MOD, MSG, ...)
 *
 * @brief
 *   Log a chip message for the specified module in the 'Retain'
 *   category. This is used for IE testing.
 *   If the product has not defined CHIP_RETAIN_LOGGING, it defaults to the same as chipLogProgress
 *
 */
#ifndef chipLogRetain
#define chipLogRetain(MOD, MSG, ...) chip::Logging::Log( chip::Logging::kLogModule_##MOD , chip::Logging::kLogCategory_Retain, MSG, ## __VA_ARGS__)
#endif

#else // #if CHIP_RETAIN_LOGGING
#ifdef chipLogRetain
// This is to ensure that chipLogRetain is null if
// the product has defined CHIP_RETAIN_LOGGING to 0 itself
#undef chipLogRetain
#endif
#define chipLogRetain(MOD, MSG, ...)
#endif // #if CHIP_RETAIN_LOGGING


#if CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING || CHIP_RETAIN_LOGGING
#define _CHIP_USE_LOGGING 1
#else
#define _CHIP_USE_LOGGING 0
#endif /* CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING || CHIP_RETAIN_LOGGING */

#if _CHIP_USE_LOGGING

#define chipLoggingchipPrefixLen 6
#define chipLoggingModuleNameLen 3
#define chipLoggingMessageSeparatorLen 2
#define chipLoggingMessageTrailerLen 2
#define chipLoggingTotalMessagePadding (chipLoggingchipPrefixLen + \
                                           chipLoggingModuleNameLen + \
                                           chipLoggingMessageSeparatorLen + \
                                           chipLoggingMessageTrailerLen)

extern void GetMessageWithPrefix(char *buf, uint8_t bufSize, uint8_t module, const char *msg);
extern void GetModuleName(char *buf, uint8_t module);
void PrintMessagePrefix(uint8_t module);

#else

static inline void GetMessageWithPrefix(char *buf, uint8_t bufSize, uint8_t module, const char *msg)
{
    return;
}

static inline void GetModuleName(char *buf, uint8_t module)
{
    return;
}

#endif // _CHIP_USE_LOGGING

#if CHIP_LOG_FILTERING

extern uint8_t gLogFilter;

#define IsCategoryEnabled(CAT) ((CAT) <= gLogFilter)

#else // CHIP_LOG_FILTERING

#define IsCategoryEnabled(CAT) (true)

#endif // CHIP_LOG_FILTERING



/**
 *  @def chipLogIfFalse(aCondition)
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
 *    #CHIP_CONFIG_ENABLE_CONDITION_LOGGING before chipLogging.h is included by
 *    the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  #define CHIP_CONFIG_ENABLE_CONDITION_LOGGING 1
 *
 *  #include <chip/Support/Logging/chipLogging.h>
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
 *      chipLogIfFalse(CHIP_END_OF_TLV == err);
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition     A Boolean expression to be evaluated.
 *
 *  @sa CHIP_CONFIG_ENABLE_TRACE_ON_CHECK_FAILURE
 *
 */

#if CHIP_CONFIG_ENABLE_CONDITION_LOGGING && !defined(chipLogIfFalse)

#define chipLogIfFalse(aCondition)                                                     \
do                                                                                      \
{                                                                                       \
    if (!(aCondition))                                                                  \
    {                                                                                   \
        chipLogError(NotSpecified, "Condition Failed (%s) at %s:%d",                   \
            #aCondition, __FILE__, __LINE__);                                           \
    }                                                                                   \
} while (0)

#else // CHIP_CONFIG_ENABLE_CONDITION_LOGGING

#define chipLogIfFalse(aCondition) \
    IgnoreUnusedVariable(aCondition)

#endif // CHIP_CONFIG_ENABLE_CONDITION_LOGGING


/**
 *  @def chipLogFunctError(aErr)
 *
 *  @brief
 *    If the given error value (@a aErr) is not successful (!= CHIP_NO_ERROR),
 *    the method logs the file name, line number, and the error code.
 *
 *  @note
 *    Evaluation of @a aErr is always done, but logging is only enabled when
 *    #CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING is enabled. This can be turned
 *    on or off for each compilation unit by enabling or disabling, as desired,
 *    #CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING before chipLogging.h is included
 *    by the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  #define CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 1
 *
 *  #include <chip/Support/Logging/chipLogging.h>
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
 *      chipLogFunctError(err);
 *  }
 *  @endcode
 *
 *  @param[in]  aErr     A scalar status to be evaluated against CHIP_NO_ERROR.
 *
 *  @sa #CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
 *
 */

#if CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING && !defined(chipLogFunctError)

#define chipLogFunctError(aErr)                                                           \
do                                                                                         \
{                                                                                          \
    if ((aErr) != CHIP_NO_ERROR)                                                          \
    {                                                                                      \
        chipLogError(NotSpecified, "%s at %s:%d", ErrorStr(aErr), __FILE__, __LINE__);\
    }                                                                                      \
} while (0)

#else // CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING

#define chipLogFunctError(aErr) \
    IgnoreUnusedVariable(aErr)

#endif // CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING



} // namespace Logging
} // namespace chip

#endif /* CHIPLOGGING_H_ */
