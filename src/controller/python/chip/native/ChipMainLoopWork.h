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

#include <functional>

namespace chip {
namespace MainLoopWork {

/**
 * Executes the given function in the CHIP main loop if one exists.
 *
 * If chip stack locking is available, this will lock the chip stack
 * and execute `f`.
 *
 * If chip stack locking is not available:
 *   - if already in the chip main loop, this will execute `f` right away
 *   - if main loop running, this will schedule and WAIT for `f` to execute
 *   - if main loop is stopped, this will execute `f` right away
 */
void ExecuteInMainLoop(std::function<void()> f);

} // namespace MainLoopWork
} // namespace chip
