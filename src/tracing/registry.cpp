/*
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

namespace {
// Contains the registered tracing backends. Modifications of the backend array
// are protected by the Matter stack lock, but iteration can happen from any
// thread at any time.
//
// Depending on the size of the array, we optimize iteration by stopping on
// the first nullptr entry; in this case Register/Unregister enforce the invariant
// that all valid backends come before any nullptr entries. The special value
// kNoBackendMarker is used to indicate a slot that is empty, but is not at the
// end of the array (i.e. there are valid backends after it). The array is typed as
// void * to accommodate the special marker value; all other non-null values are
// valid Backend * pointers.
std::atomic<void *> gTracingBackends[CHIP_CONFIG_MAX_TRACING_BACKENDS]{};

// If we support only one backend the early stopping logic is not needed.
inline constexpr bool kStopOnNullptr = (CHIP_CONFIG_MAX_TRACING_BACKENDS > 1);

// A marker value that differs from any valid Backend *, and from nullptr if we
// want to be able to stop iteration early. Otherwise it is just nullptr.
inline constexpr void * kNoBackendMarker = kStopOnNullptr ? &gTracingBackends : nullptr;

template <typename Fn>
void ForEachBackend(Fn && fn)
{
    for (auto & slot : gTracingBackends)
    {
        void * backend = slot.load(std::memory_order_acquire);
        VerifyOrReturn(!kStopOnNullptr || backend != nullptr);
        if (backend != kNoBackendMarker)
        {
            fn(*static_cast<Backend *>(backend));
        }
    }
}

// Clear trailing kNoBackendMarker entries,
// so iteration can stop as early as possible.
void OptimizeBackends()
{
    VerifyOrReturn(kStopOnNullptr);

    for (int i = CHIP_CONFIG_MAX_TRACING_BACKENDS - 1; i >= 0; --i)
    {
        auto & slot    = gTracingBackends[i];
        void * backend = slot.load(std::memory_order_acquire);
        if (backend == kNoBackendMarker)
        {
            slot.store(nullptr, std::memory_order_release);
            continue;
        }
        if (backend != nullptr)
        {
            break;
        }
    }
}

} // namespace

void Register(Backend & aBackend)
{
    assertChipStackLockedByCurrentThread();
    for (auto & slot : gTracingBackends)
    {
        void * backend = slot.load(std::memory_order_acquire);
        VerifyOrReturn(backend != &aBackend); // already registered
        if (backend == nullptr || backend == kNoBackendMarker)
        {
            aBackend.Open();
            slot.store(&aBackend, std::memory_order_release);
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
        void * backend = slot.load(std::memory_order_acquire);
        if (backend == &aBackend)
        {
            slot.store(kNoBackendMarker, std::memory_order_release);
            aBackend.Close();
            OptimizeBackends();
            return;
        }
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

} // namespace chip::Tracing
