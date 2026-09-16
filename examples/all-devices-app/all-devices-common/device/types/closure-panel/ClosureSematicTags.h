/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <cstdint>

namespace chip::app {

inline constexpr uint8_t kClosureNamespaceId = 0x44;
enum class ClosureTag : uint8_t
{
    kCovering   = 0x00,
    kWindow     = 0x01,
    kBarrier    = 0x02,
    kCabinet    = 0x03,
    kGate       = 0x04,
    kGarageDoor = 0x05,
    kDoor       = 0x06,
};

inline constexpr uint8_t kClosurePanelNamespaceId = 0x45;
enum class ClosurePanelTag : uint8_t
{
    kLift    = 0x00,
    kTilt    = 0x01,
    kSliding = 0x02,
    kRotate  = 0x03,
};

} // namespace chip::app
