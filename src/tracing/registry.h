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

#include <tracing/backend.h>

namespace chip {
namespace Tracing {

void Register(Backend & backend);
void Unregister(Backend & backend);

class ScopedRegistration
{
public:
    ScopedRegistration(Backend & backend) : mBackend(&backend) { Register(*mBackend); }
    ~ScopedRegistration() { Unregister(*mBackend); }

private:
    Backend * mBackend;
};

#ifdef MATTER_TRACING_ENABLED

// Internal calls, that will delegate to appropriate backends as needed
namespace Internal {

void Begin(::chip::Tracing::Scope scope);
void End(::chip::Tracing::Scope scope);
void Instant(::chip::Tracing::Instant instant);

void LogMessageSend(::chip::Tracing::MessageSendInfo & info);
void LogMessageReceived(::chip::Tracing::MessageReceiveInfo & info);
void LogNodeLookup(::chip::Tracing::NodeLookupInfo & info);
void LogNodeDiscovered(::chip::Tracing::NodeDiscoveredInfo & info);
void LogNodeDiscoveryFailed(::chip::Tracing::NodeDiscoveryFailedInfo & info);

} // namespace Internal

#endif // MATTTER_TRACING_ENABLED

} // namespace Tracing
} // namespace chip
