/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/CHIPDeviceBuildConfig.h>

/// Defines support for asserting that the chip stack is locked by the current thread via
/// the macro:
///
///   assertChipStackLockedByCurrentThread()
///
/// Makes use of the following preprocessor macros:
///
///   CHIP_STACK_LOCK_TRACKING_ENABLED     - keeps track of who locks/unlocks the chip stack
///   CHIP_STACK_LOCK_TRACKING_ERROR_FATAL - lock tracking errors will cause the chip stack to abort/die

namespace chip {
namespace Platform {

#if CHIP_STACK_LOCK_TRACKING_ENABLED

namespace Internal {

void AssertChipStackLockedByCurrentThread(const char * file, int line);

} // namespace Internal

#define assertChipStackLockedByCurrentThread() ::chip::Platform::Internal::AssertChipStackLockedByCurrentThread(__FILE__, __LINE__)

#else

#define assertChipStackLockedByCurrentThread() (void) 0

#endif

} // namespace Platform
} // namespace chip
