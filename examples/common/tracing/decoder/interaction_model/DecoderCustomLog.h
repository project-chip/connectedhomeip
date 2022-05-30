/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace trace {
namespace interaction_model {

CHIP_ERROR MaybeDecodeNestedReadResponse(const uint8_t * data, size_t dataLen);

CHIP_ERROR MaybeDecodeNestedCommandResponse(const uint8_t * data, size_t dataLen);

CHIP_ERROR MaybeDecodeNestedCommandRequest(const uint8_t * data, size_t dataLen);

} // namespace interaction_model
} // namespace trace
} // namespace chip
