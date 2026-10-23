/*
 *
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

#include "FactoryDataDriverImpl.h"
#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

#if defined(__cplusplus)
extern "C" {
#endif
#include "fsl_adapter_flash.h"
#include "mflash_drv.h"
#if defined(__cplusplus)
}
#endif /* __cplusplus */

using namespace chip::DeviceLayer::PersistedStorage;

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START_OFFSET[];
extern uint32_t __FACTORY_DATA_SIZE[];

namespace chip {
namespace DeviceLayer {

FactoryDataDriverImpl FactoryDataDriverImpl::sInstance;

FactoryDataDriver & FactoryDataDrv()
{
    return FactoryDataDriverImpl::sInstance;
}

FactoryDataDriverImpl & FactoryDataDrvImpl()
{
    return FactoryDataDriverImpl::sInstance;
}

CHIP_ERROR FactoryDataDriverImpl::Init(void)
{
    return CHIP_NO_ERROR;
}

bool FactoryDataDriverImpl::DoesBackupExist(uint16_t * size)
{
    return false;
}

CHIP_ERROR FactoryDataDriverImpl::DeleteBackup()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    error = KeyValueStoreMgr().Delete(FactoryDataDriverImpl::GetFactoryBackupKey().KeyName());
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::InitRamBackup(void)
{
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;

    mFactoryDataRamBuff = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(factoryDataSize));
    VerifyOrReturnError(mFactoryDataRamBuff != nullptr, CHIP_ERROR_INTERNAL);
    if (mflash_drv_read(factoryDataAddress, (uint32_t *) mFactoryDataRamBuff, factoryDataSize) != kStatus_Success)
    {
        return CHIP_ERROR_INTERNAL;
    }

    FactoryDataPrvdImpl().DecryptFactoryData(mFactoryDataRamBuff);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::ClearRamBackup()
{
    memset(mFactoryDataRamBuff, 0, (uint32_t) __FACTORY_DATA_SIZE);
    IsRamBackupFilled = false;
    chip::Platform::MemoryFree(mFactoryDataRamBuff);
    mFactoryDataRamBuff = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::ReadBackupInRam()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t bytesRead = 0;
    /* if ReadBackupInRam if executed during the OTA process, the buffer is already allocated
    but if it is executed after a board reboot, it is not */
    if (mFactoryDataRamBuff == nullptr)
    {
        mFactoryDataRamBuff = static_cast<uint8_t *>(chip::Platform::MemoryAlloc((uint32_t) __FACTORY_DATA_SIZE));
        VerifyOrReturnError(mFactoryDataRamBuff != nullptr, CHIP_ERROR_NO_MEMORY);
    }

    memset(mFactoryDataRamBuff, 0, (uint32_t) __FACTORY_DATA_SIZE);

    error = KeyValueStoreMgr().Get(FactoryDataDriverImpl::GetFactoryBackupKey().KeyName(), mFactoryDataRamBuff,
                                   (uint32_t) __FACTORY_DATA_SIZE, &bytesRead);
    ReturnErrorOnFailure(error);
    // to know that factory data ram buffer is encrypted when executing UpdateFactoryData function
    IsRamBackupFilled = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::BackupFactoryData()
{
    CHIP_ERROR error            = CHIP_NO_ERROR;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint8_t * data              = nullptr;
    data                        = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(factoryDataSize));
    /* Load the factory data into RAM buffer */
    if (mflash_drv_read(factoryDataAddress, (uint32_t *) data, factoryDataSize) != kStatus_Success)
    {
        chip::Platform::MemoryFree(data);
        return CHIP_ERROR_INTERNAL;
    }

    /* Save current encrypted factory data into the file system to be able to restore them if an orror
    occured during the OTA process */
    error = KeyValueStoreMgr().Put(FactoryDataDriverImpl::GetFactoryBackupKey().KeyName(), data, (uint32_t) __FACTORY_DATA_SIZE);
    chip::Platform::MemoryFree(data);
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::UpdateFactoryData(void)
{
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;

    /* mFactoryDataRamBuff is containing uncyphered factory data that has been updated during OTA download,
    else the buffer is containing a copy of the previous (encrypted) factory data */
    if (!IsRamBackupFilled)
        FactoryDataPrvdImpl().EncryptFactoryData(mFactoryDataRamBuff);

    /* Erase flash factory data sectors */
    hal_flash_status_t status = HAL_FlashEraseSector(factoryDataAddress + MFLASH_BASE_ADDRESS, factoryDataSize);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    /* Write new factory data into flash */
    status = HAL_FlashProgramUnaligned(factoryDataAddress + MFLASH_BASE_ADDRESS, factoryDataSize, mFactoryDataRamBuff);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
