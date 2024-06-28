/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

struct GenericStorage
{
    virtual ~GenericStorage() = default;

    virtual CHIP_ERROR Set(uint16_t id, const uint8_t * value)                           = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint8_t & value)                                 = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint16_t * value)                          = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint16_t & value)                                = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint32_t * value)                          = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint32_t & value)                                = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint64_t * value)                          = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint64_t & value)                                = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size) = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint8_t * value, size_t size)              = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
