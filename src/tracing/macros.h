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

#include <matter/tracing/build_config.h>

#if MATTER_TRACING_ENABLED

// Expected macros provided by implementations:
//    MATTER_TRACE_BEGIN(label, group)
//    MATTER_TRACE_END(label, group)
//    MATTER_TRACE_INSTANT(label, group)
//    MATTER_TRACE_SCOPE(label, group)

// Tracing macro to trace monotonically increasing counter values.
//  MATTER_TRACE_COUNTER(label)

// Tracing macro to represent historical metric data i.e the data points which represent different
// values at different point of time.
//  MATTER_TRACE_METRIC(label, value)

#include <matter/tracing/macros_impl.h>
#include <tracing/log_declares.h>
#include <tracing/registry.h>

////////////////////// DATA LOGGING

#define MATTER_LOG_MESSAGE_SEND(...)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MessageSendInfo _trace_data{ __VA_ARGS__ };                                                               \
        ::chip::Tracing::Internal::LogMessageSend(_trace_data);                                                                    \
    } while (false)

#define MATTER_LOG_MESSAGE_RECEIVED(...)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MessageReceivedInfo _trace_data{ __VA_ARGS__ };                                                           \
        ::chip::Tracing::Internal::LogMessageReceived(_trace_data);                                                                \
    } while (false)

#define MATTER_LOG_NODE_LOOKUP(...)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeLookupInfo _trace_data{ __VA_ARGS__ };                                                                \
        ::chip::Tracing::Internal::LogNodeLookup(_trace_data);                                                                     \
    } while (false)

#define MATTER_LOG_NODE_DISCOVERED(...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeDiscoveredInfo _trace_data{ __VA_ARGS__ };                                                            \
        ::chip::Tracing::Internal::LogNodeDiscovered(_trace_data);                                                                 \
    } while (false)

#define MATTER_LOG_NODE_DISCOVERY_FAILED(...)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeDiscoveryFailedInfo _trace_data{ __VA_ARGS__ };                                                       \
        ::chip::Tracing::Internal::LogNodeDiscoveryFailed(_trace_data);                                                            \
    } while (false)

////////////////////// Metric LOGGING

#define __GET_4TH_ARG(_a1,_a2,_a3,_a4,...) _a4

#define __SELECT_MACRO_FOR_EVENT_METRIC(...) __GET_4TH_ARG(__VA_ARGS__,                                                            \
        __MATTER_LOG_METRIC_3ARGS,                                                                                                 \
        __MATTER_LOG_METRIC_2ARGS,                                                                                                 \
        __MATTER_LOG_METRIC_1ARGS, )

#define __MATTER_LOG_METRIC(...) __SELECT_MACRO_FOR_EVENT_METRIC(__VA_ARGS__)(__VA_ARGS__)

#define __MATTER_LOG_METRIC_1ARGS(key)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(Tag::Instant, chip::Tracing::kMetric##key);                                     \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)

#define __MATTER_LOG_METRIC_2ARGS(tag,key)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(tag, chip::Tracing::kMetric##key);                                              \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)

#define __MATTER_LOG_METRIC_3ARGS(tag,key,value)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        using Tag = chip::Tracing::MetricEvent::Tag;                                                                               \
        ::chip::Tracing::MetricEvent _metric_event(tag, chip::Tracing::kMetric##key,value);                                        \
        ::chip::Tracing::Internal::LogEvent(_metric_event);                                                                        \
    } while (false)


// Format for the args the metric macros: MATTER_LOG_METRIC_XYZ(key, [optional value])
#define MATTER_LOG_METRIC(...) __MATTER_LOG_METRIC(Tag::Instant, __VA_ARGS__)
#define MATTER_LOG_METRIC_BEGIN(...) __MATTER_LOG_METRIC(Tag::Begin, __VA_ARGS__)
#define MATTER_LOG_METRIC_END(...) __MATTER_LOG_METRIC(Tag::End, __VA_ARGS__)

#else // MATTER_TRACING_ENABLED

#define _MATTER_TRACE_DISABLE(...)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (false)

#define MATTER_TRACE_BEGIN(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_END(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_INSTANT(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_SCOPE(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_COUNTER(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

// Metric is not enabled, however allow the value to be evaluated
#define MATTER_TRACE_METRIC(label,value)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        _Pragma("GCC diagnostic push")                                                                                             \
        _Pragma("GCC diagnostic ignored \"-Wunused-value\"")                                                                       \
        value;                                                                                                                     \
        _Pragma("GCC diagnostic pop")                                                                                              \
    } while (0)

#define MATTER_LOG_MESSAGE_SEND(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_MESSAGE_RECEIVED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#define MATTER_LOG_NODE_LOOKUP(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_NODE_DISCOVERED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_NODE_DISCOVERY_FAILED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_METRIC_EVENT(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#endif // MATTER_TRACING_ENABLED
