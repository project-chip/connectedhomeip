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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/k32w/common/RamStorage.h>

namespace chip::DeviceLayer::Internal {

RamStorage::Buffer RamStorage::sBuffer = nullptr;

CHIP_ERROR RamStorage::Init(uint16_t aNvmId, uint16_t aInitialSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    sBuffer        = getRamBuffer(aNvmId, aInitialSize);
    if (!sBuffer)
    {
        err = CHIP_ERROR_NO_MEMORY;
    }

    return err;
}

void RamStorage::FreeBuffer()
{
    if (sBuffer)
    {
        free(sBuffer);
        sBuffer = nullptr;
    }
}

CHIP_ERROR RamStorage::Read(uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength)
{
    CHIP_ERROR err;
    rsError status;

    status = ramStorageGet(sBuffer, aKey, aIndex, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));

exit:
    return err;
}

CHIP_ERROR RamStorage::Write(uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    CHIP_ERROR err;
    rsError status = RS_ERROR_NONE;

    // Delete all occurrences of "key" and resize buffer if needed
    // before scheduling writing of new value.
    ramStorageDelete(sBuffer, aKey, -1);
    status = ramStorageResize(&sBuffer, aKey, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));
    status = ramStorageSet(sBuffer, aKey, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));

exit:
    return err;
}

CHIP_ERROR RamStorage::Delete(uint16_t aKey, int aIndex)
{
    rsError status = ramStorageDelete(sBuffer, aKey, aIndex);
    return MapStatusToChipError(status);
}

CHIP_ERROR RamStorage::MapStatusToChipError(rsError rsStatus)
{
    CHIP_ERROR err;

    switch (rsStatus)
    {
    case RS_ERROR_NONE:
        err = CHIP_NO_ERROR;
        break;
    case RS_ERROR_NOT_FOUND:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    default:
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    }

    return err;
}

} // namespace chip::DeviceLayer::Internal
