/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <system/SystemClock.h>
#include <transport/Session.h>
#include <transport/SessionHandle.h>

namespace chip {
namespace app {

// This is an expected upper bound of time-to-first-byte for IM transactions, the actual processing time should never exceed this
// value unless specified elsewhere (e.g. async command handling for some clusters).
static constexpr System::Clock::Timeout kExpectedIMProcessingTime = System::Clock::Seconds16(2);

} // namespace app
} // namespace chip
