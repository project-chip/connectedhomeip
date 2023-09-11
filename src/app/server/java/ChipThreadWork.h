/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
