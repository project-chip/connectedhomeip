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

#include <matter/tracing/build_config.h>

#if MATTER_TRACING_ENABLED


// Always return the 3rd argument from macro parameters
#define __GET_3RD_ARGUMENT(_a1,_a2,_a3,...) _a3

// Macro to select the macro with the correct signature based on the invoked macro
#define __SELECT_MACRO_FOR_VERIFY_ACTION(...) __GET_3RD_ARGUMENT(__VA_ARGS__,                                                  \
        __VERIFY_ACTION_2ARGS,                                                                                                 \
        __VERIFY_ACTION_1ARGS, )

// Wrapper to capture all arguments and invoke the real wrapper
#define __LOG_METRIC_FOR_VERIFY_ACTION(...) __SELECT_MACRO_FOR_VERIFY_ACTION(__VA_ARGS__)(__VA_ARGS__)

// Macro that takes the action wraps it to emit the metric in a Verify macro, when a metric key is specified
#define __VERIFY_ACTION_2ARGS(anAction, metricKey) MATTER_LOG_METRIC(metricKey, anAction)

// Macro that takes the action in a Verify macro when a metric key is not specified
#define __VERIFY_ACTION_1ARGS(anAction)  anAction

// Macro used by VerifyOrExit macro to handle a third optional metric key and emit an event, if specified
#define LOG_METRIC_FOR_VERIFY_ACTION(anAction,...) __LOG_METRIC_FOR_VERIFY_ACTION(anAction,##__VA_ARGS__)

////////////////////// Metric LOGGING

// Always return the 4th argument from macro parameters
#define __GET_4TH_ARG(_a1,_a2,_a3,_a4,...) _a4

// Macro to select the macro with the correct signature based on the invoked MATTER_LOG_METRIC_XYZ macro
#define __SELECT_MACRO_FOR_EVENT_METRIC(...) __GET_4TH_ARG(__VA_ARGS__,                                                            \
        __MATTER_LOG_METRIC_3ARGS,                                                                                                 \
        __MATTER_LOG_METRIC_2ARGS,                                                                                                 \
        __MATTER_LOG_METRIC_1ARGS, )

// Wrapper to capture all arguments and invoke the real wrapper for logging metrics
#define __MATTER_LOG_METRIC(...) __SELECT_MACRO_FOR_EVENT_METRIC(__VA_ARGS__)(__VA_ARGS__)

// Wrapper macro that accepts metric key and logs and instant event
#define __MATTER_LOG_METRIC_1ARGS(key)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(Tag::Instant, chip::Tracing::kMetric##key);                                     \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)

// Wrapper macro that accepts metric tag and key and logs an event corresponding to the tag
#define __MATTER_LOG_METRIC_2ARGS(tag,key)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(tag, chip::Tracing::kMetric##key);                                              \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)

// Wrapper macro that accepts metric tag, key and value and logs the corresponding event
#define __MATTER_LOG_METRIC_3ARGS(tag,key,value)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(tag, chip::Tracing::kMetric##key,value);                                        \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)


/**
 *  @def MATTER_LOG_METRIC
 *
 *  @brief
 *    When tracing is enabled, this macro generates a metric event and logs it to the tracing backend.
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC(PASESession, err);
 *  @endcode
 *      The above example generates an instant metric event with key kMetricPASESession.
 *      The metric also holds the 32 bit value corresponding to the ChipError object 'err'.
 *
 *  @param[in]  key The key representing the metric name/event. The name is one the kMetricXYZ string literal
 *                  constants as defined in metric_event.h.
 *
 *  @param[in]  value An optional value for the metric. This value corresponds to one of the values supported
 *                    in MetricEvent::Value
 */
#define MATTER_LOG_METRIC(key, ...) __MATTER_LOG_METRIC(Tag::Instant, key, ##__VA_ARGS__)

/**
 * @def MATTER_TRACE_BEGIN
 *
 * @brief
 * Generate a metric with the Begin tag
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC_BEGIN(PASESession);
 *  @endcode
 *      The above example generates a Begin metric event with key kMetricPASESession.
 *
 *  @param[in]  key The key representing the metric name/event. The name is one the kMetricXYZ string literal
 *                  constants as defined in metric_event.h.
 *
 *  @param[in]  value An optional value for the metric. This value corresponds to one of the values supported
 *                    in MetricEvent::Value
 */
#define MATTER_LOG_METRIC_BEGIN(key, ...) __MATTER_LOG_METRIC(Tag::Begin, key, ##__VA_ARGS__)

/**
 * @def MATTER_TRACE_END
 *
 * @brief
 * Generate a metric with the Eng tag
 *
 *  Example usage:
 *  @code
 *      MATTER_LOG_METRIC_END(PASESession);
 *  @endcode
 *      The above example generates an End metric event with key kMetricPASESession.
 *
 *  @param[in]  key The key representing the metric name/event. The name is one the kMetricXYZ string literal
 *                  constants as defined in metric_event.h.
 *
 *  @param[in]  value An optional value for the metric. This value corresponds to one of the values supported
 *                    in MetricEvent::Value
 */
#define MATTER_LOG_METRIC_END(key, ...) __MATTER_LOG_METRIC(Tag::End, key, ##__VA_ARGS__)

#else

// Noop if tracing is disabled
#define MATTER_LOG_METRIC(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_BEGIN(key, ...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_END(key, ...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

// Default behavior when tracing is disabled is to just execute the action
#define LOG_METRIC_FOR_VERIFY_ACTION(anAction,...) anAction

#endif // MATTER_TRACING_ENABLED
