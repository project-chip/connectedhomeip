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
