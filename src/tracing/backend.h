/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    virtual void LogMessageSend(MessageSendInfo &) { TraceInstant("MessageSent", "Messaging"); }
    virtual void LogMessageReceived(MessageReceivedInfo &) { TraceInstant("MessageReceived", "Messaging"); }

    virtual void LogNodeLookup(NodeLookupInfo &) { TraceInstant("Lookup", "DNSSD"); }
    virtual void LogNodeDiscovered(NodeDiscoveredInfo &) { TraceInstant("Node Discovered", "DNSSD"); }
    virtual void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) { TraceInstant("Discovery Failed", "DNSSD"); }
};

} // namespace Tracing
} // namespace chip
