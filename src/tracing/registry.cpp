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

#include <tracing/registry.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>
#include <matter/tracing/build_config.h>
#include <platform/LockTracker.h>

#include <atomic>

namespace chip::Tracing {

#if MATTER_TRACING_ENABLED

namespace {
// Modifications of the backend array are protected by the Matter stack lock,
// but iteration can happen from any thread at any time. Slots must be filled
// from the beginning of the array, so iteration can stop at the first nullptr.
std::atomic<Backend *> gTracingBackends[CHIP_CONFIG_MAX_TRACING_BACKENDS]{};

// A marker value to represent an empty slot in the backend array.
// This avoids having to shift backends in the array when unregistering.
alignas(Backend) constexpr char gNullBackendStorage{ 0 };
Backend * NullBackend()
{
    return reinterpret_cast<Backend *>(const_cast<char *>(&gNullBackendStorage));
}

template <typename Fn>
void ForEachBackend(Fn && fn)
{
    for (auto & slot : gTracingBackends)
    {
        Backend * backend = slot.load();
        VerifyOrReturn(backend != nullptr);
        if (backend != NullBackend())
        {
            fn(*backend);
        }
    }
}

} // namespace

void Register(Backend & aBackend)
{
    assertChipStackLockedByCurrentThread();
    for (auto & slot : gTracingBackends)
    {
        Backend * backend = slot.load();
        VerifyOrReturn(backend != &aBackend); // already registered
        if (backend == nullptr || backend == NullBackend())
        {
            aBackend.Open();
            slot.store(&aBackend);
            return;
        }
    }
    VerifyOrDie(false); // CHIP_CONFIG_MAX_TRACING_BACKENDS exceeded
}

void Unregister(Backend & aBackend)
{
    assertChipStackLockedByCurrentThread();
    for (auto & slot : gTracingBackends)
    {
        Backend * backend = slot.load();
        if (backend == &aBackend)
        {
            slot.store(NullBackend());
            aBackend.Close();
            goto vacuum;
        }
    }
    return;
vacuum:
    // Replace trailing NullBackend() entries with nullptr,
    // so iteration can stop as early as possible.
    for (int i = CHIP_CONFIG_MAX_TRACING_BACKENDS - 1; i >= 0; --i)
    {
        auto & slot = gTracingBackends[i];
        VerifyOrReturn(slot.load() == NullBackend());
        slot.store(nullptr);
    }
}

namespace Internal {

void Begin(const char * label, const char * group)
{
    ForEachBackend([&](Backend & backend) { backend.TraceBegin(label, group); });
}

void End(const char * label, const char * group)
{
    ForEachBackend([&](Backend & backend) { backend.TraceEnd(label, group); });
}

void Instant(const char * label, const char * group)
{
    ForEachBackend([&](Backend & backend) { backend.TraceInstant(label, group); });
}

void Counter(const char * label)
{
    ForEachBackend([&](Backend & backend) { backend.TraceCounter(label); });
}

void LogMessageSend(::chip::Tracing::MessageSendInfo & info)
{
    ForEachBackend([&](Backend & backend) { backend.LogMessageSend(info); });
}

void LogMessageReceived(::chip::Tracing::MessageReceivedInfo & info)
{
    ForEachBackend([&](Backend & backend) { backend.LogMessageReceived(info); });
}

void LogNodeLookup(::chip::Tracing::NodeLookupInfo & info)
{
    ForEachBackend([&](Backend & backend) { backend.LogNodeLookup(info); });
}

void LogNodeDiscovered(::chip::Tracing::NodeDiscoveredInfo & info)
{
    ForEachBackend([&](Backend & backend) { backend.LogNodeDiscovered(info); });
}

void LogNodeDiscoveryFailed(::chip::Tracing::NodeDiscoveryFailedInfo & info)
{
    ForEachBackend([&](Backend & backend) { backend.LogNodeDiscoveryFailed(info); });
}

void LogMetricEvent(const ::chip::Tracing::MetricEvent & event)
{
    ForEachBackend([&](Backend & backend) { backend.LogMetricEvent(event); });
}

} // namespace Internal

#else

void Register(Backend & backend)
{
    /* no-op if tracing is disabled */
}

void Unregister(Backend & backend)
{
    /* no-op if tracing is disabled */
}

#endif // MATTER_TRACING_ENABLED

} // namespace chip::Tracing
