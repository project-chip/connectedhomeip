/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <perfetto.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

/// Simplified register, equivalent of setting up
/// `perfetto::Tracing::Initialize` with the single given backend(s).
///
/// typically called with perfetto::kSystemBackend or perfetto::kInProcessBackend or both.
void Initialize(uint32_t backends);

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
