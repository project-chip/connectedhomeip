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

#ifndef MATTER_TRACING_ENABLED

#define _MATTER_TRACE_DISABLE                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (false)

#define MATTER_TRACE_BEGIN(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_END(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#define MATTER_TRACE_INSTANT(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#define MATTER_LOG_MESSAGE_SEND(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_MESSAGE_RECEIVED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#define MATTER_LOG_NODE_LOOKUP(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_NODE_DISCOVERED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_LOG_NODE_DISCOVERY_FAILED(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#else // MATTER_TRACING_ENABLED

#include <tracing/log_declares.h>
#include <tracing/registry.h>
#include <tracing/scopes.h>

#define MATTER_TRACE_BEGIN(scope) ::chip::Tracing::Internal::Begin(scope)
#define MATTER_TRACE_END(scope) ::chip::Tracing::Internal::End(scope)
#define MATTER_TRACE_INSTANT(scope) ::chip::Tracing::Internal::Instant(scope)

#define MATTER_LOG_MESSAGE_SEND(...)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MessageSendInfo _trace_data(__VA_ARGS__);                                                                 \
        ::chip::Tracing::Internal::LogMessageSend(_trace_data);                                                                    \
    } while (false)

#define MATTER_LOG_MESSAGE_RECEIVED(...)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::MessageReceivedInfo _trace_data(__VA_ARGS__);                                                             \
        ::chip::Tracing::Internal::LogMessageReceived(_trace_data);                                                                \
    } while (false)

#define MATTER_LOG_NODE_LOOKUP(...)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeLookupInfo _trace_data(__VA_ARGS__);                                                                  \
        ::chip::Tracing::Internal::LogNodeLookup(_trace_data);                                                                     \
    } while (false)

#define MATTER_LOG_NODE_DISCOVERED(...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeDiscoveredInfo _trace_data(__VA_ARGS__);                                                              \
        ::chip::Tracing::Internal::LogNodeDiscovered(_trace_data);                                                                 \
    } while (false)

#define MATTER_LOG_NODE_DISCOVERY_FAILED(...)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::Tracing::NodeDiscoveryFailedInfo _trace_data(__VA_ARGS__);                                                         \
        ::chip::Tracing::Internal::LogNodeDiscoveryFailed(_trace_data);                                                            \
    } while (false)

#endif
