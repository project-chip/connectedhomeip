/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <lib/support/IntrusiveList.h>

#include <matter/tracing/build_config.h>
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
    if (!backend.IsInList())
    {
        backend.Open();
        gTracingBackends.PushBack(&backend);
    }
}

void Unregister(Backend & backend)
{
    assertChipStackLockedByCurrentThread();
    if (backend.IsInList())
    {
        gTracingBackends.Remove(&backend);
        backend.Close();
    }
}

#if MATTER_TRACING_ENABLED

namespace Internal {

void Begin(const char * label, const char * group)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceBegin(label, group);
    }
}

void End(const char * label, const char * group)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceEnd(label, group);
    }
}

void Instant(const char * label, const char * group)
{
    for (auto & backend : gTracingBackends)
    {
        backend.TraceInstant(label, group);
    }
}

void LogMessageSend(::chip::Tracing::MessageSendInfo & info)
{
    for (auto & backend : gTracingBackends)
    {
        backend.LogMessageSend(info);
    }
}

void LogMessageReceived(::chip::Tracing::MessageReceivedInfo & info)
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
