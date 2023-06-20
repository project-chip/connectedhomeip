/*
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

#include <lib/support/IntrusiveList.h>
#include <tracing/log_declares.h>
#include <tracing/scopes.h>

namespace chip {
namespace Tracing {

/// Represents a generic tracing back-end.
///
/// Derived from an intrusive list base as multiple
/// tracing back-ends may exist per application.
class Backend : public ::chip::IntrusiveListNodeBase<>
{
public:
    virtual ~Backend() = default;

    /// Begin a trace for the specified scope.
    ///
    /// Scopes must be completed by a corresponding
    /// TraceEnd call.
    virtual void TraceBegin(Scope scope) = 0;

    /// Tracing end assumes completing a previously
    /// started scope with TraceBegin and nesting is assumed.
    ///
    /// Expect scopes like:
    ///    TraceBegin(Foo)
    ///      TraceBegin(Bar)
    ///      TraceEnd(Bar)
    ///    TraceEnd(Foo)
    ///
    /// The following is NOT acceptable:
    ///    TraceBegin(Foo)
    ///    TraceBegin(Bar)
    ///    TraceEnd(Foo)
    ///    TraceEnd(Bar)
    virtual void TraceEnd(Scope scope) = 0;

    /// Trace a zero-sized event
    virtual void TraceInstant(Instant instant) = 0;

    virtual void LogMessageSend(MessageSendInfo &) { TraceInstant(Instant::Log_MessageSend); }
    virtual void LogMessageReceived(MessageReceivedInfo &) { TraceInstant(Instant::Log_MessageReceived); }

    virtual void LogNodeLookup(NodeLookupInfo &) { TraceInstant(Instant::Log_NodeLookup); }
    virtual void LogNodeDiscovered(NodeDiscoveredInfo &) { TraceInstant(Instant::Log_NodeDiscovered); }
    virtual void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) { TraceInstant(Instant::Log_NodeDiscoveryFailed); }
};

} // namespace Tracing
} // namespace chip
