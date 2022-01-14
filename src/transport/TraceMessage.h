/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <core/CHIPBuildConfig.h>
#include <transport/raw/MessageHeader.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "pw_trace/trace.h"

#define CHIP_TRACE_MESSAGE(payloadHeader, packetHeader, data, dataLen)                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ::chip::trace::TraceSecureMessageData _trace_data{ &payloadHeader, &packetHeader, data, dataLen };                         \
        PW_TRACE_INSTANT_DATA("SecureMsg", ::chip::trace::kTraceSecureMessageDataFormat,                                           \
                              reinterpret_cast<const char *>(&_trace_data), sizeof(_trace_data));                                  \
    } while (0)
#else
#define CHIP_TRACE_MESSAGE(payloadHeader, packetHeader, data, dataLen)                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (0)

#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

namespace chip {
namespace trace {

constexpr const char * kTraceSecureMessageDataFormat = "SecMsg";

struct TraceSecureMessageData
{
    PayloadHeader * payloadHeader;
    PacketHeader * packetHeader;
    uint8_t * packetPayload;
    size_t packetSize;
};

} // namespace trace
} // namespace chip
