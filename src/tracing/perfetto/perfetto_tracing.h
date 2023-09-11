/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <tracing/backend.h>

#include <memory>
#include <perfetto.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

/// A Backend that outputs data to chip logging.
///
/// Structured data is formatted as json strings.
class PerfettoBackend : public ::chip::Tracing::Backend
{
public:
    PerfettoBackend() = default;

    // TraceBegin/End/Instant are EXPLICITLY not provided
    // as they would be slower than expected. Perfetto trace macros
    // are expected to be set exclusively (via matter_trace_config)

    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;

    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
};

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
