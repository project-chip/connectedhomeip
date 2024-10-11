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
    void LogMetricEvent(const MetricEvent &) override;
};

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
