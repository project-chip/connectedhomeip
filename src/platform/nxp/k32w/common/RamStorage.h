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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "pdm_ram_storage_glue.h"
#include "ram_storage.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *
 * All ram storage operations should be managed by this class.
 */
class RamStorage
{
public:
    using Buffer = ramBufferDescriptor *;

    static CHIP_ERROR Init(uint16_t aNvmId, uint16_t aInitialSize);
    static void FreeBuffer();
    static Buffer GetBuffer() { return sBuffer; }

    static CHIP_ERROR Read(uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength);
    static CHIP_ERROR Write(uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength);
    static CHIP_ERROR Delete(uint16_t aKey, int aIndex);

private:
    static CHIP_ERROR MapStatusToChipError(rsError rsStatus);

    static Buffer sBuffer;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
