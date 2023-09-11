/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
