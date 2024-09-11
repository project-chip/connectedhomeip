/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataDriverImpl.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

using namespace chip::DeviceLayer::PersistedStorage;

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
    mFactoryData = Nv_GetAppFactoryData();
    VerifyOrReturnError(mFactoryData != nullptr, CHIP_ERROR_INTERNAL);

    mSize    = mFactoryData->extendedDataLength;
    mMaxSize = FactoryDataProvider::kFactoryDataMaxSize;

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
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mFactoryData != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mFactoryDataRamBuff == nullptr, CHIP_ERROR_INTERNAL);

    mFactoryDataRamBuff = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(mMaxSize));
    ReturnErrorCodeIf(mFactoryDataRamBuff == nullptr, CHIP_ERROR_INTERNAL);

    memset(mFactoryDataRamBuff, 0, mMaxSize);
    memcpy(mFactoryDataRamBuff, (void *) &mFactoryData->app_factory_data[0], mSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::ClearRamBackup()
{
    VerifyOrReturnError(mFactoryDataRamBuff != nullptr, CHIP_ERROR_NO_MEMORY);
    memset(mFactoryDataRamBuff, 0, mMaxSize);
    chip::Platform::MemoryFree(mFactoryDataRamBuff);

    mFactoryDataRamBuff = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::ReadBackupInRam()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t bytesRead = 0;

    if (mFactoryDataRamBuff == nullptr)
    {
        mFactoryDataRamBuff = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(mMaxSize));
        ReturnErrorCodeIf(mFactoryDataRamBuff == nullptr, CHIP_ERROR_NO_MEMORY);
        memset(mFactoryDataRamBuff, 0, mMaxSize);
    }

    error =
        KeyValueStoreMgr().Get(FactoryDataDriverImpl::GetFactoryBackupKey().KeyName(), mFactoryDataRamBuff, mMaxSize, &bytesRead);
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::BackupFactoryData()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    ReturnErrorCodeIf(mFactoryData == nullptr, CHIP_ERROR_INTERNAL);

    error = KeyValueStoreMgr().Put(FactoryDataDriverImpl::GetFactoryBackupKey().KeyName(), &mFactoryData->app_factory_data[0],
                                   mMaxSize);
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDriverImpl::UpdateFactoryData(void)
{
    FactoryDataProvider::Header * header;
    extendedAppFactoryData_t * data;
    VerifyOrReturnError(mFactoryDataRamBuff != nullptr, CHIP_ERROR_INTERNAL);

    header      = (FactoryDataProvider::Header *) mFactoryDataRamBuff;
    size_t size = offsetof(extendedAppFactoryData_t, app_factory_data) + sizeof(FactoryDataProvider::Header) + header->size;
    data        = static_cast<extendedAppFactoryData_t *>(chip::Platform::MemoryAlloc(size));

    memcpy(data->identificationWord, "APP_FACT_DATA:  ", APP_FACTORY_DATA_ID_STRING_SZ);

    data->extendedDataLength = sizeof(FactoryDataProvider::Header) + header->size;
    mSize                    = data->extendedDataLength;
    memcpy(&data->app_factory_data[0], mFactoryDataRamBuff, mSize);
    uint8_t status = Nv_WriteAppFactoryData(data, mSize);
    VerifyOrReturnError(status == 0, CHIP_ERROR_INTERNAL);

    chip::Platform::MemoryFree(data);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
