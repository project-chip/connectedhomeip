/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      Utility header to encode an input into a Base38 String
 */

#pragma once

#include <core/CHIPError.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace chip {
// returns CHIP_NO_ERROR on successful decode
CHIP_ERROR base38Decode(std::string base38, std::vector<uint8_t> & out);
std::string base38Encode(const uint8_t * buf, size_t buf_len);

} // namespace chip
