/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

namespace chip {
void InitializeTracing();
} // namespace chip

#if defined(MATTER_CUSTOM_TRACE) && MATTER_CUSTOM_TRACE

#include "trace/MatterCustomTrace.h"

#else // MATTER_CUSTOM_TRACE

#if defined(PW_TRACE_BACKEND_SET) && PW_TRACE_BACKEND_SET

/* ignore GCC Wconversion warnings for pigweed */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

#include <pw_trace/trace.h>

#pragma GCC diagnostic pop

#define MATTER_TRACE_EVENT_INSTANT(...) PW_TRACE_INSTANT(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_FLAG(...) PW_TRACE_INSTANT_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_DATA(...) PW_TRACE_INSTANT_DATA(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_DATA_FLAG(...) PW_TRACE_INSTANT_DATA_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START(...) PW_TRACE_START(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_FLAG(...) PW_TRACE_START_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_DATA(...) PW_TRACE_START_DATA(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_DATA_FLAG(...) PW_TRACE_START_DATA_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END(...) PW_TRACE_END(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_FLAG(...) PW_TRACE_END_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_DATA(...) PW_TRACE_END_DATA(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_DATA_FLAG(...) PW_TRACE_END_DATA_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_SCOPE(...) PW_TRACE_SCOPE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_SCOPE_FLAG(...) PW_TRACE_SCOPE_FLAG(__VA_ARGS__)
#define MATTER_TRACE_EVENT_FUNCTION(...) PW_TRACE_FUNCTION(__VA_ARGS__)
#define MATTER_TRACE_EVENT_FUNCTION_FLAG(...) PW_TRACE_FUNCTION_FLAG(__VA_ARGS__)

#else // defined(PW_TRACE_BACKEND_SET) && PW_TRACE_BACKEND_SET

#define _MATTER_TRACE_EVENT_DISABLE(...)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (0)

#define MATTER_TRACE_EVENT_INSTANT(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_DATA(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_INSTANT_DATA_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_DATA(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_START_DATA_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_DATA(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_END_DATA_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_SCOPE(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_SCOPE_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_FUNCTION(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_EVENT_FUNCTION_FLAG(...) _MATTER_TRACE_EVENT_DISABLE(__VA_ARGS__)

#endif // defined(PW_TRACE_BACKEND_SET) && PW_TRACE_BACKEND_SET

#endif // defined(MATTER_CUSTOM_TRACE) && MATTER_CUSTOM_TRACE
