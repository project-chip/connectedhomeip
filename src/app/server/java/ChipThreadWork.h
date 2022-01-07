/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <functional>

#include <lib/core/CHIPError.h>

namespace chip {
namespace ThreadWork {

using WorkCallback = std::function<void()>;

/// Schedules a task to be run on the CHIP main thread and waits for that
/// task to be complete.
///
/// Returns only once callback has been completely executed (callback
/// will not be running anymore when this returns).
void ChipMainThreadScheduleAndWait(WorkCallback callback);

} // namespace ThreadWork
} // namespace chip
