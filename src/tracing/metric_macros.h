/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>
#include <matter/tracing/build_config.h>

#define __LOG_METRIC_CONCAT_IMPL(a, b) a##b
#define __LOG_METRIC_MACRO_CONCAT(a, b) __LOG_METRIC_CONCAT_IMPL(a, b)

#if MATTER_TRACING_ENABLED

/**
 *  @def ReturnErrorOnFailureWithMetric(kMetricKey, expr)
 *
 *  @brief
 *    This macros emits the specified metric with error code and returns the error code,
 *    if the expression returns an error. For a CHIP_ERROR expression, this means any value
 *    other than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorOnFailureWithMetric(kMetricKey, channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          does not evaluate to CHIP_NO_ERROR. Value of the metric is to the
 *                          result of the expression.
 *  @param[in]  expr        An expression to be tested.
 */
#define ReturnErrorOnFailureWithMetric(kMetricKey, expr)                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, __err);                                                                                  \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnLogErrorOnFailureWithMetric(kMetricKey, expr)
 *
 *  @brief
 *    Returns the error code if the expression returns something different
 *    than CHIP_NO_ERROR. In addition, a metric is emitted with the specified metric key and
 *    error code as the value of the metric.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnLogErrorOnFailureWithMetric(kMetricKey, channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          does not evaluate to CHIP_NO_ERROR. Value of the metric is to the
 *                          result of the expression.
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define ReturnLogErrorOnFailureWithMetric(kMetricKey, expr)                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, __err);                                                                                  \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnOnFailureWithMetric(kMetricKey, expr)
 *
 *  @brief
 *    Returns if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero. If the expression evaluates to
 *    anything but CHIP_NO_ERROR, a metric with the specified key is emitted along with error as the value.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnOnFailureWithMetric(kMetricKey, channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          does not evaluate to CHIP_NO_ERROR. Value of the metric is to the
 *                          result of the expression.
 *  @param[in]  expr        An expression to be tested.
 */
#define ReturnOnFailureWithMetric(kMetricKey, expr)                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, __err);                                                                                  \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrReturnWithMetric(kMetricKey, expr, ...)
 *
 *  @brief
 *    Returns from the void function if expression evaluates to false. If the expression evaluates
 *    to false, a metric with the specified key is emitted.
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnWithMetric(kMetricKey, param != nullptr, LogError("param is nullptr"));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          to false. Value of the metric is set to false.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturnWithMetric(kMetricKey, expr, ...)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, false);                                                                                  \
            __VA_ARGS__;                                                                                                           \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrReturnErrorWithMetric(kMetricKey, expr, code, ...)
 *
 *  @brief
 *    Returns a specified error code if expression evaluates to false. If the expression evaluates
 *    to false, a metric with the specified key is emitted with the value set to the code.
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnErrorWithMetric(kMetricKey, param != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          to false. Value of the metric is to code.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturnErrorWithMetric(kMetricKey, expr, code, ...)                                                                 \
    VerifyOrReturnValueWithMetric(kMetricKey, expr, code, ##__VA_ARGS__)

/**
 *  @def VerifyOrReturnValueWithMetric(kMetricKey, expr, value, ...)
 *
 *  @brief
 *    Returns a specified value if expression evaluates to false. If the expression evaluates
 *    to false, a metric with the specified key is emitted with the value set to value.
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnValueWithMetric(kMetricKey, param != nullptr, Foo());
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          to false. Value of the metric is to value.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  value       A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturnValueWithMetric(kMetricKey, expr, value, ...)                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, value);                                                                                  \
            __VA_ARGS__;                                                                                                           \
            return (value);                                                                                                        \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrReturnLogErrorWithMetric(kMetricKey, expr, code)
 *
 *  @brief
 *    Returns and print a specified error code if expression evaluates to false.
 *    If the expression evaluates to false, a metric with the specified key is emitted
 *    with the value set to code.
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnLogErrorWithMetric(kMetricKey, param != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          to false. Value of the metric is to code.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 */
#if CHIP_CONFIG_ERROR_SOURCE
#define VerifyOrReturnLogErrorWithMetric(kMetricKey, expr, code)                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, code);                                                                                   \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(code), __FILE__, __LINE__);                                         \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)
#else // CHIP_CONFIG_ERROR_SOURCE
#define VerifyOrReturnLogErrorWithMetric(kMetricKey, expr, code)                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, code);                                                                                   \
            ChipLogError(NotSpecified, "%s:%d false: %" CHIP_ERROR_FORMAT, #expr, __LINE__, code.Format());                        \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)
#endif // CHIP_CONFIG_ERROR_SOURCE

/**
 *  @def ReturnErrorCodeWithMetricIf(kMetricKey, expr, code)
 *
 *  @brief
 *    Returns a specified error code if expression evaluates to true
 *    If the expression evaluates to true, a metric with the specified key is emitted
 *    with the value set to code.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorCodeWithMetricIf(kMetricKey, state == kInitialized, CHIP_NO_ERROR);
 *    ReturnErrorCodeWithMetricIf(kMetricKey, state == kInitialized, CHIP_ERROR_INCORRECT_STATE);
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          to true. Value of the metric is to code.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 */
#define ReturnErrorCodeWithMetricIf(kMetricKey, expr, code)                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        if (expr)                                                                                                                  \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, code);                                                                                   \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def SuccessOrExitWithMetric(kMetricKey, error)
 *
 *  @brief
 *    This checks for the specified error, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), and branches to
 *    the local label 'exit' if the error is not success.
 *    If error is not a success, a metric with key kMetricKey is emitted with
 *    the error code as the value of the metric.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR TryHard()
 *  {
 *      CHIP_ERROR err;
 *
 *      err = TrySomething();
 *      SuccessOrExitWithMetric(kMetricKey, err);
 *
 *      err = TrySomethingElse();
 *      SuccessOrExitWithMetric(kMetricKey, err);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted
 *                          if the condition evaluates to false. The value
 *                          for the metric is result of the expression aStatus.
 *  @param[in]  error  A ChipError object to be evaluated against success (CHIP_NO_ERROR).
 *
 */
#define SuccessOrExitWithMetric(kMetricKey, error)                                                                                 \
    nlEXPECT(::chip::Tracing::ErrorHandling::LogMetricIfError((kMetricKey), (error)), exit)

/**
 *  @def VerifyOrExitWithMetric(kMetricKey, aCondition, anAction)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true, and both executes @a anAction and branches to
 *    the local label 'exit' if the condition is false. If the condition
 *    is false a metric event with the specified key is emitted with value
 *    set to the result of the expression anAction.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR MakeBuffer(const uint8_t *& buf)
 *  {
 *      CHIP_ERROR err = CHIP_NO_ERROR;
 *
 *      buf = (uint8_t *)malloc(1024);
 *      VerifyOrExitWithMetric(kMetricKey, buf != NULL, err = CHIP_ERROR_NO_MEMORY);
 *
 *      memset(buf, 0, 1024);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted
 *                          if the aCondition evaluates to false. The value
 *                          for the metric is result of the expression anAction.
 *  @param[in]  aCondition  A Boolean expression to be evaluated.
 *  @param[in]  anAction    An expression or block to execute when the
 *                          assertion fails.
 */
#define VerifyOrExitWithMetric(kMetricKey, aCondition, anAction)                                                                   \
    nlEXPECT_ACTION(aCondition, exit, MATTER_LOG_METRIC((kMetricKey), (anAction)))

/**
 *  @def ExitNowWithMetric(kMetricKey, ...)
 *
 *  @brief
 *    This unconditionally executes @a ... and branches to the local
 *    label 'exit'. In addition a metric is emitted with the specified key.
 *
 *  @note The use of this interface implies neither success nor
 *        failure for the overall exit status of the enclosing function
 *        body.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR ReadAll(Reader& reader)
 *  {
 *      CHIP_ERROR err;
 *
 *      while (true)
 *      {
 *          err = reader.ReadNext();
 *          if (err == CHIP_ERROR_AT_END)
 *              ExitNowWithMetric(kMetricKey, err = CHIP_NO_ERROR);
 *          SuccessOrExit(err);
 *          DoSomething();
 *      }
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted.
 *  @param[in]  ...         Statements to execute. Optional.
 */
#define ExitNowWithMetric(kMetricKey, ...)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        MATTER_LOG_METRIC(kMetricKey);                                                                                             \
        __VA_ARGS__;                                                                                                               \
        goto exit;                                                                                                                 \
    } while (0)

/**
 *  @def LogErrorOnFailureWithMetric(kMetricKey, expr)
 *
 *  @brief
 *    Logs a message if the expression returns something different than CHIP_NO_ERROR.
 *    In addition, a metric is emitted with the specified key and value set to result
 *    of the expression in case it evaluates to anything other than CHIP_NO_ERROR.
 *
 *  Example usage:
 *
 *  @code
 *    LogErrorOnFailureWithMetric(kMetricKey, channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted if the expr evaluates
 *                          does not evaluate to CHIP_NO_ERROR. Value of the metric is to the
 *                          result of the expression.
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define LogErrorOnFailureWithMetric(kMetricKey, expr)                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, __err);                                                                                  \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrDoWithMetric(kMetricKey, expr, ...)
 *
 *  @brief
 *    Do something if expression evaluates to false. If the expression evaluates to false a metric
 *    with the specified key is emitted with value set to false.
 *
 *  Example usage:
 *
 * @code
 *    VerifyOrDoWithMetric(param != nullptr, LogError("param is nullptr"));
 *  @endcode
 *
 *  @param[in]  kMetricKey  Metric key for the metric event to be emitted.
 *                          Value of the metric is set to false.
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        Error code to emit as part of the metric.
 *  @param[in]  ...         Statements to execute.
 */
#define VerifyOrDoWithMetric(kMetricKey, expr, code, ...)                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            MATTER_LOG_METRIC(kMetricKey, code);                                                                                   \
            __VA_ARGS__;                                                                                                           \
        }                                                                                                                          \
    } while (false)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility Macros to support optional arguments for MATTER_LOG_METRIC_XYZ macros
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Utility to always return the 4th argument from macro parameters
#define __GET_4TH_ARG(_a1, _a2, _a3, _a4, ...) _a4

// Utility macro to select the macro with the correct signature based on the invoked MATTER_LOG_METRIC_XYZ macro
#define __SELECT_MACRO_FOR_EVENT_METRIC(...)                                                                                       \
    __GET_4TH_ARG(__VA_ARGS__, __MATTER_LOG_METRIC_3ARGS, __MATTER_LOG_METRIC_2ARGS, __MATTER_LOG_METRIC_1ARGS, )

// Wrapper to capture all arguments and invoke the real wrapper for logging metrics
#define __MATTER_LOG_METRIC(...) __SELECT_MACRO_FOR_EVENT_METRIC(__VA_ARGS__)(__VA_ARGS__)

// Wrapper macro that accepts metric key and logs and instant event
#define __MATTER_LOG_METRIC_1ARGS(key)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        using Type = chip::Tracing::MetricEvent::Type;                                                                             \
        ::chip::Tracing::MetricEvent _metric_event(Type::kInstantEvent, key);                                                      \
        ::chip::Tracing::Internal::LogMetricEvent(_metric_event);                                                                  \
    } while (false)

// Wrapper macro that accepts metric type and key and logs an event corresponding to the type
#define __MATTER_LOG_METRIC_2ARGS(type, key)                                                                                       \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MetricEvent _metric_event(type, key);                                                                     \
        ::chip::Tracing::Internal::LogMetricEvent(_metric_event);                                                                  \
    } while (false)

// Wrapper macro that accepts metric type, key and value and logs the corresponding event
#define __MATTER_LOG_METRIC_3ARGS(type, key, value)                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MetricEvent _metric_event(type, key, value);                                                              \
        ::chip::Tracing::Internal::LogMetricEvent(_metric_event);                                                                  \
    } while (false)

////////////////////////
// Metric logging macros
////////////////////////

/**
 *  @def MATTER_LOG_METRIC
 *
 *  @brief
 *    When tracing is enabled, this macro generates a metric event and logs it to the tracing backend.
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC(chip::Tracing::kMetricPASESession, err);
 *  @endcode
 *      The above example generates an instant metric event with key kMetricPASESession.
 *      The metric also holds the 32 bit value corresponding to the ChipError object 'err'.
 *
 *  @param[in]  key The key representing the metric name/event.
 *
 *  @param[in]  value An optional value for the metric. This value corresponds to one of the values supported
 *                    in MetricEvent::Value
 */
#define MATTER_LOG_METRIC(key, ...) __MATTER_LOG_METRIC(chip::Tracing::MetricEvent::Type::kInstantEvent, key, ##__VA_ARGS__)

/**
 * @def MATTER_LOG_METRIC_BEGIN
 *
 * @brief
 * Generate a metric with the Begin Type
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC_BEGIN(chip::Tracing::kMetricPASESession);
 *  @endcode
 *      The above example generates a Begin metric event with key kMetricPASESession.
 *
 *  @param[in]  key The key representing the metric name/event.
 */
#define MATTER_LOG_METRIC_BEGIN(key) __MATTER_LOG_METRIC(chip::Tracing::MetricEvent::Type::kBeginEvent, key)

/**
 * @def MATTER_LOG_METRIC_END
 *
 * @brief
 * Generate a metric with the End Type
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC_END(chip::Tracing::kMetricPASESession);
 *  @endcode
 *      The above example generates an End metric event with key kMetricPASESession.
 *
 *  @param[in]  key The key representing the metric name/event.
 *
 *  @param[in]  value An optional value for the metric. This value corresponds to one of the values supported
 *                    in MetricEvent::Value
 */
#define MATTER_LOG_METRIC_END(key, ...) __MATTER_LOG_METRIC(chip::Tracing::MetricEvent::Type::kEndEvent, key, ##__VA_ARGS__)

/**
 * @def MATTER_LOG_METRIC_SCOPE
 *
 * @brief
 * Generate a scoped metric tracking Begin and End within a given scope.
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC_SCOPE(chip::Tracing::kMetricPASESession);
 *  @endcode
 *      The above example generates an Begin and the End metric using RAII.
 *
 *  @param[in]  key The key representing the metric name/event.
 *  @param[in]  error Reference to a ChipError object that is used as the value of the End event.
 */
#define MATTER_LOG_METRIC_SCOPE(key, error)                                                                                        \
    ::chip::Tracing::ScopedMetricEvent __LOG_METRIC_MACRO_CONCAT(_metric_scope, __COUNTER__)(key, error)

#else // Tracing is disabled

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remap Success, Return, and Verify macros to the ones without metrics
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ReturnErrorOnFailureWithMetric(kMetricKey, expr) ReturnErrorOnFailure(expr)

#define ReturnLogErrorOnFailureWithMetric(kMetricKey, expr) ReturnLogErrorOnFailure(expr)

#define ReturnOnFailureWithMetric(kMetricKey, expr) ReturnOnFailure(expr)

#define VerifyOrReturnWithMetric(kMetricKey, expr, ...) VerifyOrReturn(expr, ##__VA_ARGS__)

#define VerifyOrReturnErrorWithMetric(kMetricKey, expr, code, ...) VerifyOrReturnValue(expr, code, ##__VA_ARGS__)

#define VerifyOrReturnValueWithMetric(kMetricKey, expr, value, ...) VerifyOrReturnValue(expr, value, ##__VA_ARGS__)

#define VerifyOrReturnLogErrorWithMetric(kMetricKey, expr, code) VerifyOrReturnLogError(expr, code)

#define ReturnErrorCodeWithMetricIf(kMetricKey, expr, code) ReturnErrorCodeIf(expr, code)

#define SuccessOrExitWithMetric(kMetricKey, aStatus) SuccessOrExit(aStatus)

#define VerifyOrExitWithMetric(kMetricKey, aCondition, anAction) VerifyOrExit(aCondition, anAction)

#define ExitNowWithMetric(kMetricKey, ...) ExitNow(##__VA_ARGS__)

#define LogErrorOnFailureWithMetric(kMetricKey, expr) LogErrorOnFailure(expr)

#define VerifyOrDoWithMetric(kMetricKey, expr, code, ...) VerifyOrDo(expr, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Map all MATTER_LOG_METRIC_XYZ macros to noops
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __MATTER_LOG_METRIC_DISABLE(...)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (false)

#define MATTER_LOG_METRIC(...) __MATTER_LOG_METRIC_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_BEGIN(...) __MATTER_LOG_METRIC_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_END(...) __MATTER_LOG_METRIC_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_SCOPE(...) __MATTER_LOG_METRIC_DISABLE(__VA_ARGS__)

#endif // MATTER_TRACING_ENABLED
