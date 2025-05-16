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

#include <cstdint>

namespace chip {
namespace Access {

// Using bitfield values so privilege set and auth mode can be stored together.
// Privilege set can have more than one value expressed (e.g. View,
// ProxyView, and Operate).
// NOTE: Every time this enum class changes, we need to update the function
// "kPrivilegeMaskValidation" below.
// In particular, we need to keep the array "privilegeValues[]" up to date ALWAYS.
enum class Privilege : uint8_t
{
    kView       = 1 << 0,
    kProxyView  = 1 << 1,
    kOperate    = 1 << 2,
    kManage     = 1 << 3,
    kAdminister = 1 << 4
};

// Function used to validate that the parameter 'mask' contains all the values
// defined inside the enum class "Privilege", and only those values.
constexpr auto kPrivilegeMaskValidation(uint8_t mask)
{

    // Array of all current values defined inside the enum class "Privilege".
    // For this validation to work, this array ALWAYS must match the listed contents
    // of our enum class "Privilege".
    Privilege privilegeValues[] = { Access::Privilege::kView, Access::Privilege::kProxyView, Access::Privilege::kOperate,
                                    Access::Privilege::kManage, Access::Privilege::kAdminister };

    for (const auto & value : privilegeValues)
    {
        mask ^= static_cast<uint8_t>(value);
    }

    return (mask == 0);
};

} // namespace Access
} // namespace chip
