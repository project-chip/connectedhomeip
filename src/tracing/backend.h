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

namespace chip {
namespace Tracing {

/// Represents a generic tracing back-end.
///
/// Derived from an intrusive list base as multiple
/// tracing back-ends may exist per application.
///
/// THREAD SAFETY:
///   Implementations of backends are expected to be thread safe as
///   separate threads may call its functions (e.g. BLE and CASE processing
///   may be traced and run on different threads)
class Backend : public ::chip::IntrusiveListNodeBase<>
{
public:
    virtual ~Backend() = default;

    /// Guaranteed to be called before registering
    virtual void Open() {}

    /// Guaranteed to be called after un-registering.
    virtual void Close() {}

    /// Begin a trace for the specified scope.
    ///
    /// Scope WILL be completed by a corresponding TraceEnd call.
    virtual void TraceBegin(const char * label, const char * group) {}

    /// Tracing end assumes completing a previously started scope with TraceBegin
    /// and nesting is assumed.
    ///
    /// Expect scopes like:
    ///    TraceBegin("foo", "A")
    ///      TraceBegin("bar", "A")
    ///
    ///      // NOT VALID HERE: TraceEnd("foo", "A")
    ///
    ///      TraceEnd("bar", "A")  // ends "BAR"
    ///    TraceEnd("foo", "A")    // ends "FOO"
    virtual void TraceEnd(const char * label, const char * group) {}

    /// Trace a zero-sized event
    virtual void TraceInstant(const char * label, const char * group) {}

    virtual void TraceCounter(const char * label) {}
    virtual void LogMessageSend(MessageSendInfo &) { TraceInstant("MessageSent", "Messaging"); }
    virtual void LogMessageReceived(MessageReceivedInfo &) { TraceInstant("MessageReceived", "Messaging"); }

    virtual void LogNodeLookup(NodeLookupInfo &) { TraceInstant("Lookup", "DNSSD"); }
    virtual void LogNodeDiscovered(NodeDiscoveredInfo &) { TraceInstant("Node Discovered", "DNSSD"); }
    virtual void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) { TraceInstant("Discovery Failed", "DNSSD"); }
    virtual void LogMetricEvent(const MetricEvent &) { TraceInstant("Metric Event", "Metric"); }
};

} // namespace Tracing
} // namespace chip
