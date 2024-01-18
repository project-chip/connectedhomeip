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
#include "tracing.h"

#include <lib/support/logging/CHIPLogging.h>

#include <tracing/perfetto/event_storage.h>
#include <tracing/perfetto/simple_initialize.h>

namespace chip {
namespace Android {

// TODO:
//   we likely want to be able to register a data logging backend
//   of type chip::Tracing::Perfetto::PerfettoBackend
//
//   For that to work, we need to first fix data type dependencies as
//   `platform` is a very low layer compared to data logging backends which
//   need access to dnssd and messaging.
void InitializeTracing()
{
    ChipLogProgress(DeviceLayer, "Initializing tracing for android");
    chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
    chip::Tracing::Perfetto::RegisterEventTrackingStorage();
}

void ShutdownTracing()
{
    ChipLogProgress(DeviceLayer, "Cleaning up tracing for android");
    chip::Tracing::Perfetto::FlushEventTrackingStorage();
}

} // namespace Android
} // namespace chip
