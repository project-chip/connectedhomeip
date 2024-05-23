/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 * Enum used to control iteration (e.g. via a callback function that is called
 * for each of a set of elements).
 *
 * When used as the callback return type:
 *   Continue: Continue the iteration.
 *   Break: Stop the iteration.
 *
 * When used as the return type of the entire iteration procedure:
 *   Break: Some callback returned Break.
 *   Finish: All callbacks returned Continue.
 */
enum class Loop : uint8_t
{
    Continue,
    Break,
    Finish,
};

} // namespace chip
