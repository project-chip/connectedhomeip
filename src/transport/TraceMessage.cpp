/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <transport/TraceMessage.h>

namespace chip {
namespace trace {
namespace {

TransportTraceHandler gTransportTraceHandler = nullptr;

} // namespace

void SetTransportTraceHook(TransportTraceHandler handler)
{
    gTransportTraceHandler = handler;
}

namespace internal {

void HandleTransportTrace(const char * type, const void * data, size_t size)
{
    if (gTransportTraceHandler)
    {
        gTransportTraceHandler(type, data, size);
    }
}

} // namespace internal

} // namespace trace
} // namespace chip
