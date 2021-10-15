/*
 *
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

namespace chip {
namespace access {

/**
 * @enum AuthMode
 *
 * @brief Access control auth modes. Expressed as bit flags so they can be
 * combined with privileges.
 */
enum AuthMode
{
    kNone  = 0,
    kPase  = 1 << 5,
    kCase  = 1 << 6,
    kGroup = 1 << 7
};

} // namespace access
} // namespace chip
