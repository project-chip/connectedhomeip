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

class Channel
{
public:
    Channel()  = default;
    ~Channel() = default;

    CHIP_ERROR Init();
    CHIP_ERROR Read(uint8_t * buffer, size_t buffer_length, size_t & bytes_read);
    CHIP_ERROR Write(const uint8_t * buffer, size_t buffer_length);

    static CHIP_ERROR Update(uint16_t handle);
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
