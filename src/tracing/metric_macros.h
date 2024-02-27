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
 *  @def SuccessOrExitWithMetric(kMetriKey, error)
 *
 *  @brief
 *    This checks for the specified error, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), and branches to
 *    the local label 'exit' if the error is not success.
 *    If error is not a success, a metric with key kMetriKey is emitted with
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

/*
 * Utility Macros to support optional arguments for MATTER_LOG_METRIC_XYZ macros
 */

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

#define SuccessOrExitWithMetric(kMetricKey, aStatus) SuccessOrExit(aStatus)

#define VerifyOrExitWithMetric(kMetricKey, aCondition, anAction) VerifyOrExit(aCondition, anAction)

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
