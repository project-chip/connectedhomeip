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

#include <matter/tracing/build_config.h>
#include <tracing/backend.h>

namespace chip {
namespace Tracing {

/// Registers a tracing backend to receive trace and logging data
/// Until it is unregistered
///
/// All tracing backends MUST be unregistered before the application
/// exits. Consider using [ScopedRegistration]
///
/// Thread safety:
///    MUST be called with the Matter thread lock held (from the Matter main loop or
///    at application main). This is because data logging and multiplexed tracing
///    iterate over registered backends.
///
///    Even if iteration is thread safe (i.e. main() trace register/unregister),
///    the thread safety of the tracing relies on the thread safety of the macro
///    implementations. We generally require tracing backends to be thread safe.
///
void Register(Backend & backend);

/// Unregister a backend from receiving tracing/logging data
///
/// Thread safety:
///    MUST be called with the Matter thread lock held (from the Matter main loop or
///    at application main). This is because data logging and multiplexed tracing
///    iterate over registered backends.
///
///    Even if iteration is thread safe (i.e. main() trace register/unregister),
///    the thread safety of the tracing relies on the thread safety of the macro
///    implementations. We generally require tracing backends to be thread safe.
void Unregister(Backend & backend);

/// Convenience class to apply Register/Unregister automatically
/// for a backend.
///
/// This ensures the "MUST unregister before application exit"
/// is always met.
///
/// Prefer to use this class instead of direct register/unregister.
class ScopedRegistration
{
public:
    ScopedRegistration(Backend & backend) : mBackend(&backend) { Register(*mBackend); }
    ~ScopedRegistration() { Unregister(*mBackend); }

private:
    Backend * mBackend;
};

#if MATTER_TRACING_ENABLED

// Internal calls, that will delegate to appropriate backends as needed
namespace Internal {

void Begin(const char * label, const char * group);
void End(const char * label, const char * group);
void Instant(const char * label, const char * group);
void Counter(const char * label);

void LogMessageSend(::chip::Tracing::MessageSendInfo & info);
void LogMessageReceived(::chip::Tracing::MessageReceivedInfo & info);
void LogNodeLookup(::chip::Tracing::NodeLookupInfo & info);
void LogNodeDiscovered(::chip::Tracing::NodeDiscoveredInfo & info);
void LogNodeDiscoveryFailed(::chip::Tracing::NodeDiscoveryFailedInfo & info);
void LogMetricEvent(const ::chip::Tracing::MetricEvent & event);

} // namespace Internal

#endif // MATTTER_TRACING_ENABLED

} // namespace Tracing
} // namespace chip
