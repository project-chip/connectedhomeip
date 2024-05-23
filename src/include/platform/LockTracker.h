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

#include <platform/CHIPDeviceConfig.h>

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
