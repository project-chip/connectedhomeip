/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
