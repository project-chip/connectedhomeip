/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <functional>

namespace chip {
namespace MainLoopWork {

/**
 * Executes the given function in the CHIP main loop if one exists.
 *
 * Several implementations exist, however generally:
 *
 *   - if already in the chip main loop (or main loop is not running),
 *     `f` gets executed right away
 *   - otherwise:
 *     - if chip stack locking is available, `f` is executed within the lock
 *     - if chip stack locking not available, this will schedule and WAIT
 *       for `f` to execute
 */
void ExecuteInMainLoop(std::function<void()> f);

} // namespace MainLoopWork
} // namespace chip
