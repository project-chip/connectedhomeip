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

namespace chip {
namespace Tracing {
namespace LogJson {

/// A Backend that outputs data to chip logging.
///
/// Structured data is formatted as json strings.
class LogJsonBackend : public ::chip::Tracing::Backend
{
public:
    LogJsonBackend() = default;

    void TraceBegin(Scope scope) override;
    void TraceEnd(Scope scope) override;
    void TraceInstant(Instant instant) override;
    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceiveInfo &) override;
    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
};

} // namespace LogJson
} // namespace Tracing
} // namespace chip
