/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the NXP K32W SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/k32w/k32w0/K32W0Config.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

osaMutexId_t K32WConfig::pdmMutexHandle = NULL;

#if CHIP_DEVICE_LAYER_ENABLE_PDM_LOGS
static void PDM_SystemCallback(uint32_t number, PDM_eSystemEventCode code)
{
    uint8_t capacity  = PDM_u8GetSegmentCapacity();
    uint8_t occupancy = PDM_u8GetSegmentOccupancy();
    ChipLogProgress(DeviceLayer, "[PDM]Event (number, code): (%lu, %d)", number, code);
    ChipLogProgress(DeviceLayer, "[PDM]Capacity: %hhu", capacity);
    ChipLogProgress(DeviceLayer, "[PDM]Occupancy: %hhu", occupancy);
}
#endif

CHIP_ERROR K32WConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int status;

    /* Initialise the Persistent Data Manager */
    pdmMutexHandle = OSA_MutexCreate();
    VerifyOrExit((NULL != pdmMutexHandle), err = CHIP_ERROR_NO_MEMORY);
    status = PDM_Init();
    SuccessOrExit(err = MapPdmInitStatusToChipError(status));
#if CHIP_DEVICE_LAYER_ENABLE_PDM_LOGS
    PDM_vRegisterSystemCallback(PDM_SystemCallback);
#endif

    err = RamStorage::Init(kNvmIdChipConfigData, kRamBufferInitialSize);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (pdmMutexHandle)
        {
            OSA_MutexDestroy(pdmMutexHandle);
        }

        RamStorage::FreeBuffer();
    }
    return err;
}

void K32WConfig::MutexLock(osaMutexId_t mutexId, uint32_t millisec)
{
    osaStatus_t status = OSA_MutexLock(mutexId, millisec);
    if (osaStatus_Success != status)
    {
        ChipLogProgress(DeviceLayer, "OSA mutex lock failed.");
    }
}

void K32WConfig::MutexUnlock(osaMutexId_t mutexId)
{
    osaStatus_t status = OSA_MutexUnlock(mutexId);
    if (osaStatus_Success != status)
    {
        ChipLogProgress(DeviceLayer, "OSA mutex unlock failed.");
    }
}

CHIP_ERROR K32WConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint16_t sizeToRead = bufSize;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    err = RamStorage::Read(key, 0, (uint8_t *) buf, &sizeToRead);
    SuccessOrExit(err);

    outLen = sizeToRead;

exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueStr(key, (char *) buf, bufSize, outLen);
}

CHIP_ERROR K32WConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return ReadConfigValue(key, val);
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    PDM_teStatus status;
    RamStorage::Buffer buffer;

    MutexLock(pdmMutexHandle, osaWaitForever_c);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if (!str)
    {
        err = RamStorage::Delete(key, -1);
    }
    else
    {
        err = RamStorage::Write(key, (uint8_t *) str, strLen);
        SuccessOrExit(err);
        buffer = RamStorage::GetBuffer();
        status = PDM_eSaveRecordDataInIdleTask(kNvmIdChipConfigData, buffer, buffer->ramBufferLen + kRamDescHeaderSize);
    }

exit:
    MutexUnlock(pdmMutexHandle);
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueStr(key, (char *) data, dataLen);
}

CHIP_ERROR K32WConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return WriteConfigValue(key, val);
}

CHIP_ERROR K32WConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PDM_teStatus status;
    RamStorage::Buffer buffer;

    MutexLock(pdmMutexHandle, osaWaitForever_c);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    err = RamStorage::Delete(key, -1);
    SuccessOrExit(err);

    buffer = RamStorage::GetBuffer();
    status = PDM_eSaveRecordDataInIdleTask(kNvmIdChipConfigData, buffer, buffer->ramBufferLen + kRamDescHeaderSize);
    SuccessOrExit(err = MapPdmStatusToChipError(status));

exit:
    MutexUnlock(pdmMutexHandle);
    return err;
}

bool K32WConfig::ConfigValueExists(Key key)
{
    CHIP_ERROR err;
    uint16_t sizeToRead;
    bool found = false;

    if (ValidConfigKey(key))
    {
        err   = RamStorage::Read(key, 0, NULL, &sizeToRead);
        found = (err == CHIP_NO_ERROR && sizeToRead != 0);
    }

    return found;
}

CHIP_ERROR K32WConfig::FactoryResetConfig(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PDM_teStatus status;
    RamStorage::Buffer buffer;

    MutexLock(pdmMutexHandle, osaWaitForever_c);
    FactoryResetConfigInternal(kMinConfigKey_ChipCounter, kMaxConfigKey_ChipCounter);
    FactoryResetConfigInternal(kMinConfigKey_ChipConfig, kMaxConfigKey_ChipConfig);
    FactoryResetConfigInternal(kMinConfigKey_KVSKey, kMaxConfigKey_KVSKey);
    FactoryResetConfigInternal(kMinConfigKey_KVSValue, kMaxConfigKey_KVSValue);

    buffer = RamStorage::GetBuffer();
    status = PDM_eSaveRecordData(kNvmIdChipConfigData, buffer, buffer->ramBufferLen + kRamDescHeaderSize);
    SuccessOrExit(err = MapPdmStatusToChipError(status));

exit:
    RamStorage::FreeBuffer();
    MutexUnlock(pdmMutexHandle);
    return err;
}

void K32WConfig::FactoryResetConfigInternal(Key firstKey, Key lastKey)
{
    for (Key key = firstKey; key <= lastKey; key++)
    {
        RamStorage::Delete(key, -1);
    }
}

CHIP_ERROR K32WConfig::MapPdmStatusToChipError(PDM_teStatus status)
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

CHIP_ERROR K32WConfig::MapPdmInitStatusToChipError(int status)
{
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPlatform, status);
}

bool K32WConfig::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid CHIP Config PDM key range.
    if ((key >= kMinConfigKey_ChipFactory) && (key <= kMaxConfigKey_KVSValue))
    {
        return true;
    }

    return false;
}

void K32WConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
