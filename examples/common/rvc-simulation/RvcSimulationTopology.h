/*
 *
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

namespace chip {
namespace examples {
namespace rvc_simulation {

// Topology identifiers shared by rvc-app and all-devices-app so the same
// certification tests can drive either application via named-pipe commands.
namespace Topology {

constexpr uint32_t kMapIdXX = 3;
constexpr uint32_t kMapIdYY = 245;

constexpr uint32_t kAreaIdA = 7;
constexpr uint32_t kAreaIdB = 1234567;
constexpr uint32_t kAreaIdC = 10050;
constexpr uint32_t kAreaIdD = 0x88888888;

constexpr uint8_t kRunModeIdle     = 0;
constexpr uint8_t kRunModeCleaning = 1;
constexpr uint8_t kRunModeMapping  = 2;

constexpr uint8_t kCleanModeQuick = 0;

} // namespace Topology

} // namespace rvc_simulation
} // namespace examples
} // namespace chip
