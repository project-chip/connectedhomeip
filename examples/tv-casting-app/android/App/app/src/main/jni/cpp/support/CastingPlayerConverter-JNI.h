/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include "core/CastingPlayer.h"

#include <lib/core/CHIPError.h>

#include <jni.h>

namespace matter {
namespace casting {
namespace support {

/**
 * @brief Convertes a native CastingPlayer into a MatterCastingPlayer jobject
 *
 * @param CastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 *
 * @return pointer to the CastingPlayer jobject if created successfully, nullptr otherwise.
 */
jobject createJCastingPlayer(matter::casting::memory::Strong<core::CastingPlayer> player);

}; // namespace support
}; // namespace casting
}; // namespace matter
