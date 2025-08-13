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
#include <stdint.h>

#pragma once

/**
 * Enum and
 */
namespace chip {
namespace app {
    namespace Clusters {

        enum class TestEnum : uint8_t {
            kValue1 = 0,
            kValue2 = 1,
            kValue3 = 2,
            kUnknownEnumValue = 3
        };

        static auto __attribute__((unused)) EnsureKnownEnumValue(TestEnum val)
        {
            using EnumType = TestEnum;
            switch (val) {
            case EnumType::kValue1:
            case EnumType::kValue2:
            case EnumType::kValue3:
                return val;
            default:
                return EnumType::kUnknownEnumValue;
            }
        }
    } // namespace Clusters
} // namespace app
} // namespace chip
