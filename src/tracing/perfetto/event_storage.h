/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

namespace chip {
namespace Tracing {
namespace Perfetto {

/// Registers static event storage.
///
/// Registers static event storage for default chip perfetto events.
void RegisterEventTrackingStorage();

/// Trigger a flush on the registered event tracking storage
void FlushEventTrackingStorage();

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
