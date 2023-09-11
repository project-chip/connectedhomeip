/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "simple_initialize.h"

#include <perfetto.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

void Initialize(uint32_t backends)
{
    perfetto::TracingInitArgs args;
    args.backends = backends;
    perfetto::Tracing::Initialize(args);
}

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
