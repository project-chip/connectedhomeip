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

#include <platform/K32W/K32WConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR K32WConfig::Init()
{
    CHIP_ERROR err;
    int pdmStatus;

    /* Initialise the Persistent Data Manager */
    pdmStatus = PDM_Init();
    SuccessOrExit(err = MapPdmInitStatus(pdmStatus));

exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    bool tempVal;
    uint16_t bytesRead;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, &tempVal, sizeof(bool), &bytesRead);
    SuccessOrExit(err = MapPdmStatus(pdmStatus));
    val = tempVal;

exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t tempVal;
    uint16_t bytesRead;
    uint16_t recordSize;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    if (PDM_bDoesDataExist((uint16_t) key, &recordSize))
    {
        pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, &tempVal, sizeof(uint32_t), &bytesRead);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));
        val = tempVal;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    uint64_t tempVal;
    uint16_t bytesRead;
    uint16_t recordSize;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (PDM_bDoesDataExist((uint16_t) key, &recordSize))
    {
        pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, &tempVal, sizeof(uint64_t), &bytesRead);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));
        val = tempVal;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    const uint8_t * strEnd;
    char * pData = NULL;
    uint16_t bytesRead;
    uint16_t recordSize;
    PDM_teStatus pdmStatus;

    outLen = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (PDM_bDoesDataExist((uint16_t) key, &recordSize))
    {
        pData = (char *) pvPortMalloc(recordSize);
        VerifyOrExit((pData != NULL), err = CHIP_ERROR_NO_MEMORY);

        pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, pData, recordSize, &bytesRead);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));

        strEnd = (const uint8_t *) memchr(pData, 0, bytesRead);
        VerifyOrExit(strEnd != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

        outLen = strEnd - (const uint8_t *) pData;

        // NOTE: the caller is allowed to pass NULL for buf to query the length of the stored value.

        if (buf != NULL)
        {
            VerifyOrExit(bufSize > outLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            memcpy(buf, pData, outLen + 1);
        }
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

exit:
    if (pData != NULL)
    {
        vPortFree((void *) pData);
    }
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint8_t * pData = NULL;
    uint16_t bytesRead;
    uint16_t recordSize;
    PDM_teStatus pdmStatus;

    outLen = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (PDM_bDoesDataExist((uint16_t) key, &recordSize))
    {
        pData = (uint8_t *) pvPortMalloc(recordSize);
        VerifyOrExit((pData != NULL), err = CHIP_ERROR_NO_MEMORY);

        pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, pData, recordSize, &bytesRead);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));

        if (buf != NULL)
        {
            VerifyOrExit((bufSize >= bytesRead), err = CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(buf, pData, bytesRead);
        }
        outLen = bytesRead;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

exit:
    if (pData != NULL)
    {
        vPortFree((void *) pData);
    }

    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t tempVal;
    uint16_t bytesRead;
    uint16_t recordSize;
    PDM_teStatus pdmStatus;

    Key key = kMinConfigKey_ChipCounter + counterIdx;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (PDM_bDoesDataExist((uint16_t) key, &recordSize))
    {
        pdmStatus = PDM_eReadDataFromRecord((uint16_t) key, &tempVal, sizeof(uint32_t), &bytesRead);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));
        val = tempVal;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    pdmStatus = PDM_eSaveRecordData((uint16_t) key, &val, sizeof(bool));
    SuccessOrExit(err = MapPdmStatus(pdmStatus));

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    pdmStatus = PDM_eSaveRecordData((uint16_t) key, &val, sizeof(uint32_t));
    SuccessOrExit(err = MapPdmStatus(pdmStatus));

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    pdmStatus = PDM_eSaveRecordData((uint16_t) key, &val, sizeof(uint64_t));
    SuccessOrExit(err = MapPdmStatus(pdmStatus));

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (str != NULL)
    {
        pdmStatus = PDM_eSaveRecordData((uint16_t) key, (void *) str, strLen);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));
    }
    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if ((data != NULL) && (dataLen > 0))
    {
        pdmStatus = PDM_eSaveRecordData((uint16_t) key, (void *) data, dataLen);
        SuccessOrExit(err = MapPdmStatus(pdmStatus));
    }
    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR K32WConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    CHIP_ERROR err;
    PDM_teStatus pdmStatus;

    Key key = kMinConfigKey_ChipCounter + counterIdx;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    pdmStatus = PDM_eSaveRecordData((uint16_t) key, &val, sizeof(uint32_t));
    SuccessOrExit(err = MapPdmStatus(pdmStatus));

exit:
    return err;
}

CHIP_ERROR K32WConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    PDM_vDeleteDataRecord((uint16_t) key);

    SuccessOrExit(err);

exit:
    return err;
}

bool K32WConfig::ConfigValueExists(Key key)
{
    CHIP_ERROR err;
    uint16_t size = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    VerifyOrExit(PDM_bDoesDataExist((uint16_t) key, &size), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

exit:
    // Return true if the record was found.
    return (err == CHIP_NO_ERROR);
}

CHIP_ERROR K32WConfig::FactoryResetConfig(void)
{
    CHIP_ERROR err;

    err = FactoryResetConfigInternal(kMinConfigKey_ChipConfig, kMaxConfigKey_ChipConfig);

    if (err == CHIP_NO_ERROR)
    {
        err = FactoryResetConfigInternal(kMinConfigKey_KVS, kMaxConfigKey_KVS);
    }

    return err;
}

CHIP_ERROR K32WConfig::FactoryResetConfigInternal(Key firstKey, Key lastKey)
{
    CHIP_ERROR err;

    // Iterate over all the CHIP Config PDM ID records and delete each one
    err = ForEachRecord(firstKey, lastKey, false, [](const Key & pdmKey, const size_t & length) -> CHIP_ERROR {
        CHIP_ERROR err2;

        err2 = ClearConfigValue(pdmKey);
        SuccessOrExit(err2);

    exit:
        return err2;
    });

    // Return success at end of iterations.
    if (err == CHIP_END_OF_INPUT)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR K32WConfig::MapPdmStatus(PDM_teStatus pdmStatus)
{
    CHIP_ERROR err;

    switch (pdmStatus)
    {
    case PDM_E_STATUS_OK:
        err = CHIP_NO_ERROR;
        break;
    default:
        err = CHIP_ERROR(ChipError::Range::kPlatform, pdmStatus);
        break;
    }

    return err;
}

CHIP_ERROR K32WConfig::MapPdmInitStatus(int pdmStatus)
{
    return (pdmStatus == 0) ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPlatform, pdmStatus);
}

bool K32WConfig::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid CHIP Config PDM key range.

    if ((key >= kMinConfigKey_ChipFactory) && (key <= kMaxConfigKey_KVS))
    {
        return true;
    }

    return false;
}

CHIP_ERROR K32WConfig::ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (Key pdmKey = firstKey; pdmKey <= lastKey; pdmKey++)
    {
        uint16_t dataLen;

        if (PDM_bDoesDataExist((uint16_t) pdmKey, &dataLen))
        {
            if (!addNewRecord)
            {
                // Invoke the caller's function
                // (for retrieve,store,delete,enumerate GroupKey operations).
                err = funct(pdmKey, dataLen);
            }
        }
        else
        {
            if (addNewRecord)
            {
                // Invoke caller's function
                // (for add GroupKey operation).
                err = funct(pdmKey, dataLen);
            }
        }

        SuccessOrExit(err);
    }

exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
