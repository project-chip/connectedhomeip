/*
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 * The declared functions in this header are defined in src/platform/PlatformLockSupport.cpp because of dependency cycle issues.
 * The //src/platform:platform library depends on //src/lib/core:core library but the core library depends on the system library
 * in this directory.
 */

namespace chip {
namespace System {
namespace PlatformLocking {

void LockMatterStack();
void UnlockMatterStack();

} // namespace PlatformLocking
} // namespace System
} // namespace chip
