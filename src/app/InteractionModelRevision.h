/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inttypes.h>
#include <stddef.h>

/**
 * CHIP_DEVICE_INTERACTION_MODEL_REVISION
 *
 * A monothonic number identifying the interaction model revision.
 */
#ifndef CHIP_DEVICE_INTERACTION_MODEL_REVISION
#define CHIP_DEVICE_INTERACTION_MODEL_REVISION 1
#endif

constexpr uint8_t kInteractionModelRevisionTag = 0xFF;
