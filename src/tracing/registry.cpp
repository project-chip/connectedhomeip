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
#include <lib/support/IntrusiveList.h>

#include <platform/LockTracker.h>
#include <tracing/registry.h>

namespace chip {
namespace Tracing {
namespace {

IntrusiveList<Backend> gTracingBackends;

} // namespace

void Register(Backend & backend)
{
    assertChipStackLockedByCurrentThread();
    gTracingBackends.PushBack(&backend);
}

void Unregister(Backend & backend)
{
    assertChipStackLockedByCurrentThread();
    gTracingBackends.Remove(&backend);
}

#ifdef MATTER_TRACING_ENABLED

namespace Internal {

void Begin(::chip::Tracing::Scope scope)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceBegin(scope);
    }
}

void End(::chip::Tracing::Scope scope)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceEnd(scope);
    }
}
void Instant(::chip::Tracing::Instant instant)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceInstant(instant);
    }
}

void LogMessageSend(::chip::Tracing::MessageSendInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogMessageSend(info);
    }
}

void LogMessageReceived(::chip::Tracing::MessageReceiveInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogMessageReceived(info);
    }
}

void LogNodeLookup(::chip::Tracing::NodeLookupInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogNodeLookup(info);
    }
}

void LogNodeDiscovered(::chip::Tracing::NodeDiscoveredInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogNodeDiscovered(info);
    }
}

void LogNodeDiscoveryFailed(::chip::Tracing::NodeDiscoveryFailedInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogNodeDiscoveryFailed(info);
    }
}

} // namespace Internal

#endif // MATTTER_TRACING_ENABLED

} // namespace Tracing
} // namespace chip
