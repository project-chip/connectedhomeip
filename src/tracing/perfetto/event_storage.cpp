/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "event_storage.h"

#include <matter/tracing/macros_impl.h>

#include <perfetto.h>

// this registers based on categories defined.
// Since only macros_impl.h is provided, only categories defined there are registered
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace chip {
namespace Tracing {
namespace Perfetto {

void RegisterEventTrackingStorage()
{
    perfetto::TrackEvent::Register();
}

void FlushEventTrackingStorage()
{
    perfetto::TrackEvent::Flush();
}

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
