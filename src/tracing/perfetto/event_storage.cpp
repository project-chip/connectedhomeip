/*
 *
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
