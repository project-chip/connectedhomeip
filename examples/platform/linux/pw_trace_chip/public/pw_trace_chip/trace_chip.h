/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Enable these trace types
#define PW_TRACE_TYPE_INSTANT ::chip::trace::PW_TRACE_EVENT_TYPE_INSTANT
#define PW_TRACE_TYPE_INSTANT_GROUP ::chip::trace::PW_TRACE_EVENT_TYPE_INSTANT_GROUP

#define PW_TRACE_TYPE_DURATION_START ::chip::trace::PW_TRACE_EVENT_TYPE_DURATION_START
#define PW_TRACE_TYPE_DURATION_END ::chip::trace::PW_TRACE_EVENT_TYPE_DURATION_END

#define PW_TRACE_TYPE_DURATION_GROUP_START ::chip::trace::PW_TRACE_EVENT_TYPE_DURATION_GROUP_START
#define PW_TRACE_TYPE_DURATION_GROUP_END ::chip::trace::PW_TRACE_EVENT_TYPE_DURATION_GROUP_END

#define PW_TRACE_TYPE_ASYNC_START ::chip::trace::PW_TRACE_EVENT_TYPE_ASYNC_START
#define PW_TRACE_TYPE_ASYNC_INSTANT ::chip::trace::PW_TRACE_EVENT_TYPE_ASYNC_STEP
#define PW_TRACE_TYPE_ASYNC_END ::chip::trace::PW_TRACE_EVENT_TYPE_ASYNC_END

namespace chip {
namespace trace {

typedef enum
{
    PW_TRACE_EVENT_TYPE_INVALID              = 0,
    PW_TRACE_EVENT_TYPE_INSTANT              = 1,
    PW_TRACE_EVENT_TYPE_INSTANT_GROUP        = 2,
    PW_TRACE_EVENT_TYPE_ASYNC_START          = 3,
    PW_TRACE_EVENT_TYPE_ASYNC_STEP           = 4,
    PW_TRACE_EVENT_TYPE_ASYNC_END            = 5,
    PW_TRACE_EVENT_TYPE_DURATION_START       = 6,
    PW_TRACE_EVENT_TYPE_DURATION_END         = 7,
    PW_TRACE_EVENT_TYPE_DURATION_GROUP_START = 8,
    PW_TRACE_EVENT_TYPE_DURATION_GROUP_END   = 9,
} TraceEventType;

// This should not be called directly, instead use the PW_TRACE_* macros.
void TraceEvent(const char * module, const char * label, TraceEventType eventType, const char * group, uint32_t traceId,
                uint8_t flags, const char * dataFormat, const void * dataBuffer, size_t dataSize);

struct TraceEventFields
{
    const char * module;
    const char * label;
    TraceEventType eventType;
    const char * group;
    uint32_t traceId;
    uint8_t flags;
    const char * dataFormat;
    const void * dataBuffer;
    size_t dataSize;
};

typedef bool (*TraceHandlerCallback)(const TraceEventFields & eventFields, void * context);

// Register an additional trace handler which gets called for all data trace events with the given context
void RegisterTraceHandler(TraceHandlerCallback callback, void * context);
void UnregisterAllTraceHandlers();

// These are what the facade actually calls.
#define PW_TRACE(eventType, flags, label, group, traceId)                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::trace::TraceEvent(PW_TRACE_MODULE_NAME, label, eventType, group, traceId, flags, NULL, NULL, 0);                   \
    } while (0)

#define PW_TRACE_DATA(eventType, flags, label, group, traceId, dataFormat, data, size)                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::trace::TraceEvent(PW_TRACE_MODULE_NAME, label, eventType, group, traceId, flags, dataFormat, data, size);          \
    } while (0)

} // namespace trace
} // namespace chip
