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

#include <openthread/platform/memory.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/k32w/common/RamStorage.h>

#include "pdm_ram_storage_glue.h"

#if PDM_SAVE_IDLE
#include "fsl_os_abstraction.h"
#define mutex_lock(descr, timeout) OSA_MutexLock(descr->header.mutexHandle, timeout)
#define mutex_unlock(descr) OSA_MutexUnlock(descr->header.mutexHandle)
#define mutex_destroy(descr) OSA_MutexDestroy(descr->header.mutexHandle)
#else
#define mutex_lock(...)
#define mutex_unlock(...)
#define mutex_destroy(...)
#endif

#if PDM_USE_DYNAMIC_MEMORY
#define ot_free otPlatFree
#else
#define ot_free(...)
#endif

namespace chip::DeviceLayer::Internal {

CHIP_ERROR RamStorage::Init(uint16_t aInitialSize)
{
    CHIP_ERROR err;

    mBuffer = getRamBuffer(mPdmId, aInitialSize);

    return mBuffer ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

void RamStorage::FreeBuffer()
{
    if (mBuffer)
    {
        mutex_lock(mBuffer, osaWaitForever_c);
        if (mBuffer->buffer)
        {
            ot_free(mBuffer->buffer);
            mBuffer->buffer = nullptr;
        }
        mutex_unlock(mBuffer);
        mutex_destroy(mBuffer);
        ot_free(mBuffer);
        mBuffer = nullptr;
    }
}

CHIP_ERROR RamStorage::Read(uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength) const
{
    CHIP_ERROR err;
    rsError status;

    mutex_lock(mBuffer, osaWaitForever_c);
    status = ramStorageGet(mBuffer, aKey, aIndex, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));

exit:
    mutex_unlock(mBuffer);
    return err;
}

CHIP_ERROR RamStorage::Write(uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    CHIP_ERROR err;
    rsError status = RS_ERROR_NONE;
    PDM_teStatus pdmStatus;

    mutex_lock(mBuffer, osaWaitForever_c);
    // Delete all occurrences of "key" and resize buffer if needed
    // before scheduling writing of new value.
    ramStorageDelete(mBuffer, aKey, -1);
#if PDM_USE_DYNAMIC_MEMORY
    status = ramStorageResize(mBuffer, aKey, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));
#endif
    status = ramStorageSet(mBuffer, aKey, aValue, aValueLength);
    SuccessOrExit(err = MapStatusToChipError(status));
    pdmStatus = PDM_SaveRecord(mPdmId, mBuffer);
    SuccessOrExit(err = MapPdmStatusToChipError(pdmStatus));

exit:
    mutex_unlock(mBuffer);
    return err;
}

CHIP_ERROR RamStorage::Delete(uint16_t aKey, int aIndex)
{
    CHIP_ERROR err;
    rsError status = RS_ERROR_NONE;
    PDM_teStatus pdmStatus;

    mutex_lock(mBuffer, osaWaitForever_c);
    status = ramStorageDelete(mBuffer, aKey, aIndex);
    SuccessOrExit(err = MapStatusToChipError(status));
    pdmStatus = PDM_SaveRecord(mPdmId, mBuffer);
    SuccessOrExit(err = MapPdmStatusToChipError(pdmStatus));

exit:
    mutex_unlock(mBuffer);
    return err;
}

void RamStorage::OnFactoryReset()
{
    mutex_lock(mBuffer, osaWaitForever_c);
    PDM_vDeleteDataRecord(mPdmId);
    mutex_unlock(mBuffer);
    FreeBuffer();
}

CHIP_ERROR RamStorage::MapStatusToChipError(rsError rsStatus) const
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

CHIP_ERROR RamStorage::MapPdmStatusToChipError(PDM_teStatus status) const
{
    CHIP_ERROR err;

    switch (status)
    {
    case PDM_E_STATUS_OK:
        err = CHIP_NO_ERROR;
        break;
    default:
        err = CHIP_ERROR(ChipError::Range::kPlatform, status);
        break;
    }

    return err;
}

} // namespace chip::DeviceLayer::Internal
