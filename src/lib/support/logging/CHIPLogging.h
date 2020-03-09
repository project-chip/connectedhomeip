/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines macros, constants, and interfaces for a
 *      platform-independent logging interface for the Weave SDK.
 *
 *      Weave SDK clients may choose, at compile time, among Android,
 *      C Standard I/O, or external (platform- and integrator-defined)
 *      logging style implementations that will be invoked when any of
 *      the following preprocessor symbols are asserted:
 *
 *         - #WEAVE_ERROR_LOGGING
 *         - #WEAVE_PROGRESS_LOGGING
 *         - #WEAVE_DETAIL_LOGGING
 *
 */

#ifndef WEAVELOGGING_H_
#define WEAVELOGGING_H_

#include <stdint.h>

#include <Weave/Core/WeaveConfig.h>

/**
 *   @namespace nl::Weave::Logging
 *
 *   @brief
 *     This namespace includes all interfaces within Weave for shared
 *     logging support.
 *
 *     The interfaces include macros, constants, and functions for a
 *     platform-independent logging interface for the Weave SDK.
 *
 *     Weave SDK clients may choose, at compile time, among Android,
 *     C Standard I/O, or external (platform- and integrator-defined)
 *     logging style implementations that will be invoked when any of
 *     the following preprocessor symbols are asserted:
 *
 *        - #WEAVE_ERROR_LOGGING
 *        - #WEAVE_PROGRESS_LOGGING
 *        - #WEAVE_DETAIL_LOGGING
 *
 */

namespace nl {
namespace Weave {
namespace Logging {

/**
 *  @enum LogModule
 *
 *  @brief
 *    Identifies a logical section of code that is a source of log
 *    messages.
 *
 *  @note If you add modules or rearrange this list you must update the
 *        ModuleNames tables in WeaveLogging.cpp.
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
    kLogModule_HeatLink,
    kLogModule_TimeService,
    kLogModule_WeaveTunnel,
    kLogModule_Heartbeat,
    kLogModule_WeaveSystemLayer,
    kLogModule_DropcamLegacyPairing,
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

#ifndef WEAVE_ERROR_LOGGING
#define WEAVE_ERROR_LOGGING 1
#endif

#ifndef WEAVE_LOG_FILTERING
#define WEAVE_LOG_FILTERING 1
#endif

#if WEAVE_ERROR_LOGGING
/**
 * @def WeaveLogError(MOD, MSG, ...)
 *
 * @brief
 *   Log a Weave message for the specified module in the 'Error'
 *   category.
 *
 */
#ifndef WeaveLogError
#define WeaveLogError(MOD, MSG, ...) nl::Weave::Logging::Log( nl::Weave::Logging::kLogModule_##MOD , nl::Weave::Logging::kLogCategory_Error, MSG, ## __VA_ARGS__)
#endif
#else
#define WeaveLogError(MOD, MSG, ...)
#endif

#ifndef WEAVE_PROGRESS_LOGGING
#define WEAVE_PROGRESS_LOGGING 1
#endif

#if WEAVE_PROGRESS_LOGGING
/**
 * @def WeaveLogProgress(MOD, MSG, ...)
 *
 * @brief
 *   Log a Weave message for the specified module in the 'Progress'
 *   category.
 *
 */
#ifndef WeaveLogProgress
#define WeaveLogProgress(MOD, MSG, ...) nl::Weave::Logging::Log( nl::Weave::Logging::kLogModule_##MOD , nl::Weave::Logging::kLogCategory_Progress, MSG, ## __VA_ARGS__)
#endif
#else
#define WeaveLogProgress(MOD, MSG, ...)
#endif


#ifndef WEAVE_DETAIL_LOGGING
#define WEAVE_DETAIL_LOGGING 1
#endif

#if WEAVE_DETAIL_LOGGING
/**
 * @def WeaveLogDetail(MOD, MSG, ...)
 *
 * @brief
 *   Log a Weave message for the specified module in the 'Detail'
 *   category.
 *
 */
#ifndef WeaveLogDetail
#define WeaveLogDetail(MOD, MSG, ...) nl::Weave::Logging::Log( nl::Weave::Logging::kLogModule_##MOD , nl::Weave::Logging::kLogCategory_Detail, MSG, ## __VA_ARGS__)
#endif
#else
#define WeaveLogDetail(MOD, MSG, ...)
#endif

#ifndef WEAVE_RETAIN_LOGGING
#define WEAVE_RETAIN_LOGGING WEAVE_PROGRESS_LOGGING
#define WeaveLogRetain(MOD, MSG, ...) WeaveLogProgress(MOD, MSG, ## __VA_ARGS__)
#endif

#if WEAVE_RETAIN_LOGGING
/**
 * @def WeaveLogRetain(MOD, MSG, ...)
 *
 * @brief
 *   Log a Weave message for the specified module in the 'Retain'
 *   category. This is used for IE testing.
 *   If the product has not defined WEAVE_RETAIN_LOGGING, it defaults to the same as WeaveLogProgress
 *
 */
#ifndef WeaveLogRetain
#define WeaveLogRetain(MOD, MSG, ...) nl::Weave::Logging::Log( nl::Weave::Logging::kLogModule_##MOD , nl::Weave::Logging::kLogCategory_Retain, MSG, ## __VA_ARGS__)
#endif

#else // #if WEAVE_RETAIN_LOGGING
#ifdef WeaveLogRetain
// This is to ensure that WeaveLogRetain is null if
// the product has defined WEAVE_RETAIN_LOGGING to 0 itself
#undef WeaveLogRetain
#endif
#define WeaveLogRetain(MOD, MSG, ...)
#endif // #if WEAVE_RETAIN_LOGGING


#if WEAVE_ERROR_LOGGING || WEAVE_PROGRESS_LOGGING || WEAVE_DETAIL_LOGGING || WEAVE_RETAIN_LOGGING
#define _WEAVE_USE_LOGGING 1
#else
#define _WEAVE_USE_LOGGING 0
#endif /* WEAVE_ERROR_LOGGING || WEAVE_PROGRESS_LOGGING || WEAVE_DETAIL_LOGGING || WEAVE_RETAIN_LOGGING */

#if _WEAVE_USE_LOGGING

#define nlWeaveLoggingWeavePrefixLen 6
#define nlWeaveLoggingModuleNameLen 3
#define nlWeaveLoggingMessageSeparatorLen 2
#define nlWeaveLoggingMessageTrailerLen 2
#define nlWeaveLoggingTotalMessagePadding (nlWeaveLoggingWeavePrefixLen + \
                                           nlWeaveLoggingModuleNameLen + \
                                           nlWeaveLoggingMessageSeparatorLen + \
                                           nlWeaveLoggingMessageTrailerLen)

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

#endif // _WEAVE_USE_LOGGING

#if WEAVE_LOG_FILTERING

extern uint8_t gLogFilter;

#define IsCategoryEnabled(CAT) ((CAT) <= gLogFilter)

#else // WEAVE_LOG_FILTERING

#define IsCategoryEnabled(CAT) (true)

#endif // WEAVE_LOG_FILTERING



/**
 *  @def WeaveLogIfFalse(aCondition)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true and emits some log, based on configuration, if
 *    the condition is false.
 *
 *  @note
 *    Evaluation of @a aCondition is always done, but logging is only enabled when
 *    #WEAVE_CONFIG_ENABLE_CONDITION_LOGGING is enabled. This can be turned on or
 *    off for each compilation unit by enabling or disabling, as desired,
 *    #WEAVE_CONFIG_ENABLE_CONDITION_LOGGING before WeaveLogging.h is included by
 *    the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  #define WEAVE_CONFIG_ENABLE_CONDITION_LOGGING 1
 *
 *  #include <Weave/Support/Logging/WeaveLogging.h>
 *
 *  ...
 *
 *  void foo(void)
 *  {
 *      WEAVE_ERROR err = WEAVE_NO_ERROR;
 *
 *      ...
 *
 *  exit:
 *      WeaveLogIfFalse(WEAVE_END_OF_TLV == err);
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition     A Boolean expression to be evaluated.
 *
 *  @sa WEAVE_CONFIG_ENABLE_TRACE_ON_CHECK_FAILURE
 *
 */

#if WEAVE_CONFIG_ENABLE_CONDITION_LOGGING && !defined(WeaveLogIfFalse)

#define WeaveLogIfFalse(aCondition)                                                     \
do                                                                                      \
{                                                                                       \
    if (!(aCondition))                                                                  \
    {                                                                                   \
        WeaveLogError(NotSpecified, "Condition Failed (%s) at %s:%d",                   \
            #aCondition, __FILE__, __LINE__);                                           \
    }                                                                                   \
} while (0)

#else // WEAVE_CONFIG_ENABLE_CONDITION_LOGGING

#define WeaveLogIfFalse(aCondition) \
    IgnoreUnusedVariable(aCondition)

#endif // WEAVE_CONFIG_ENABLE_CONDITION_LOGGING


/**
 *  @def WeaveLogFunctError(aErr)
 *
 *  @brief
 *    If the given error value (@a aErr) is not successful (!= WEAVE_NO_ERROR),
 *    the method logs the file name, line number, and the error code.
 *
 *  @note
 *    Evaluation of @a aErr is always done, but logging is only enabled when
 *    #WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING is enabled. This can be turned
 *    on or off for each compilation unit by enabling or disabling, as desired,
 *    #WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING before WeaveLogging.h is included
 *    by the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  #define WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 1
 *
 *  #include <Weave/Support/Logging/WeaveLogging.h>
 *
 *  ...
 *
 *  void foo(void)
 *  {
 *      WEAVE_ERROR err = WEAVE_NO_ERROR;
 *
 *      ...
 *
 *  exit:
 *      WeaveLogFunctError(err);
 *  }
 *  @endcode
 *
 *  @param[in]  aErr     A scalar status to be evaluated against WEAVE_NO_ERROR.
 *
 *  @sa #WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
 *
 */

#if WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING && !defined(WeaveLogFunctError)

#define WeaveLogFunctError(aErr)                                                           \
do                                                                                         \
{                                                                                          \
    if ((aErr) != WEAVE_NO_ERROR)                                                          \
    {                                                                                      \
        WeaveLogError(NotSpecified, "%s at %s:%d", nl::ErrorStr(aErr), __FILE__, __LINE__);\
    }                                                                                      \
} while (0)

#else // WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING

#define WeaveLogFunctError(aErr) \
    IgnoreUnusedVariable(aErr)

#endif // WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING



} // namespace Logging
} // namespace Weave
} // namespace nl

#endif /* WEAVELOGGING_H_ */
