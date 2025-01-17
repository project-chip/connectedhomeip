/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_ONBOARDING_PAYLOAD
#include <platform/NFCOnboardingPayloadManager.h>

#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include "FunctionLib.h"

namespace chip {
namespace DeviceLayer {

NFCOnboardingPayloadManagerImpl NFCOnboardingPayloadManagerImpl::sInstance;

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StartTagEmulation(const char * payload, size_t payloadLength)
{
    ntagDriverHandleInstance = NTAG_InitDevice((NTAG_ID_T) 0, I2C2);
    assert(ntagDriverHandleInstance);

    CLOCK_EnableClock(kCLOCK_I2c0);
    CLOCK_AttachClk(kOSC32M_to_I2C_CLK);
    HAL_I2C_InitDevice(HAL_I2C_INIT_DEFAULT, kCLOCK_Fro32M, I2C2);

    /* populate the NDEF structure */
    sInstance.ndefUriRecord.recordType    = NDEF_RECORD_TYPE;
    sInstance.ndefUriRecord.recordTypeLen = NDEF_RECORD_TYPE_LEN;
    sInstance.ndefUriRecord.payloadLen    = payloadLength + sizeof(sInstance.ndefUriRecord.uriIdCode);
    sInstance.ndefUriRecord.recordName    = NFC_NDEF_RECORD_NAME;
    sInstance.ndefUriRecord.uriIdCode     = NDEF_URI_ID_CODE;

    if (payloadLength > NDEF_URI_ID_MAX_LENGTH)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else
    {
        memcpy(sInstance.ndefUriRecord.uriIdData, payload, payloadLength);
    }

    /* write the NDEF structure to the NTAG EEPROM */
    if (AppNtagWrite(payload) != E_APP_NTAG_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }
    else
    {
        sInstance.mIsStarted = TRUE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StopTagEmulation()
{
    uint8_t ndefUriRecordSize = AppNdefUriRecordGetSize(sInstance.ndefUriRecord);

    memset(&sInstance.ndefUriRecord, 0, ndefUriRecordSize);

    if (AppNtagEepromUnlockThenWrite(ndefUriRecordSize) != E_APP_NTAG_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    /* Stop I2C */
    HAL_I2C_CloseDevice(I2C2);
    NTAG_CloseDevice(sInstance.ntagDriverHandleInstance);
    sInstance.ntagDriverHandleInstance = NULL;

    sInstance.mIsStarted = FALSE;
    return CHIP_NO_ERROR;
}

bool NFCOnboardingPayloadManagerImpl::IsNtagConfigured(eAppNtagError * pNtagError, const char * payload)
{
    uint32_t addrToRead                                                = NTAG_I2C_BLOCK_SIZE;
    uint8_t eepromDataBuf[NDEF_URI_ID_MAX_LENGTH + TERMINATOR_TLV_LEN] = { 0 };
    uint16_t ndefUriRecordSize                                         = AppNdefUriRecordGetSize(sInstance.ndefUriRecord);
    uint8_t ndefUriLen = sInstance.ndefUriRecord.payloadLen - sizeof(sInstance.ndefUriRecord.uriIdCode);

    if (NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, addrToRead, eepromDataBuf, 2))
    {
        *pNtagError = E_APP_NTAG_READ_ERROR;
        return FALSE;
    }

    /* see also http://apps4android.org/nfc-specifications/NFCForum-TS-Type-2-Tag_1.1.pdf, chapter 2.3 */
    if (eepromDataBuf[0] != 0x03 || eepromDataBuf[1] != ndefUriRecordSize)
    {
        return FALSE;
    }

    /* read the NdefUriRecord from the EEPROM */
    addrToRead += 2;
    if (NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, addrToRead, eepromDataBuf, ndefUriRecordSize))
    {
        *pNtagError = E_APP_NTAG_READ_ERROR;
        return FALSE;
    }

    /* verify if the ndefUriRecord is the same as the one flashed in the the EEPROM:
     * If it is, then the NTAG is already configured.
     */
    return (payload && !memcmp(sInstance.ndefUriRecord.uriIdData, payload, ndefUriLen) &&
            !memcmp((uint8_t *) &(sInstance.ndefUriRecord), eepromDataBuf, ndefUriRecordSize));
}

NFCOnboardingPayloadManagerImpl::eAppNtagError NFCOnboardingPayloadManagerImpl::AppNtagWrite(const char * payload)
{
    eAppNtagError ntagErr = E_APP_NTAG_NO_ERROR;
    uint8_t byte0         = 0;
    uint8_t i             = 0;
    bool_t i2cAddrFound   = FALSE;
    bool_t isConfigured   = FALSE;

    do
    {
        /* Try to access the device at default I2C address */
        if (NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, 0, &byte0, sizeof(byte0)))
        {
            /* Try now with the 0x02 I2C address */
            NTAG_SetNtagI2cAddress(sInstance.ntagDriverHandleInstance, 0x2);
            if (!NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, 0, &byte0, sizeof(byte0)))
            {
                i2cAddrFound = TRUE;
            }
            else
            {
                /* Loop to try to find a valid i2c address */
                for (i = 0; i < 0xFF; i++)
                {
                    if (i == 2 || i == 0x55) /* Skip default and 0x02 I2C address */
                    {
                        continue;
                    }

                    NTAG_SetNtagI2cAddress(sInstance.ntagDriverHandleInstance, i);
                    if (!NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, 0, &byte0, sizeof(byte0)))
                    {
                        i2cAddrFound = TRUE;
                        break;
                    }
                }
            }

            if (!i2cAddrFound)
            {
                ntagErr = E_APP_NTAG_READ_ERROR;
                break;
            }
        }

        isConfigured = IsNtagConfigured(&ntagErr, payload);
        if (ntagErr != E_APP_NTAG_NO_ERROR)
        {
            break;
        }

        if (!isConfigured)
        {
            ntagErr = AppNtagEepromUnlockThenWrite(0);
        }

    } while (0);

    return ntagErr;
}

bool NFCOnboardingPayloadManagerImpl::AppNtagEepromWrite(uint8_t originalSize)
{
    bool wasWritten      = FALSE;
    uint32_t ndefSize    = AppNdefUriRecordGetSize(sInstance.ndefUriRecord);
    uint32_t addrToWrite = NTAG_I2C_BLOCK_SIZE;
    uint8_t buf[4];
    uint8_t terminatorTLV = 0xFE;

    do
    {
        if (!sInstance.ndefUriRecord.payloadLen)
        {
            if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, addrToWrite, (uint8_t *) &(sInstance.ndefUriRecord),
                                originalSize + sizeof(terminatorTLV)))
            {
                break;
            }
        }
        else
        {
            buf[0] = 0x3;
            buf[1] = ndefSize;
            if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, addrToWrite, buf, 2))
            {
                break;
            }

            addrToWrite += 2;

            if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, addrToWrite, (uint8_t *) &(sInstance.ndefUriRecord), ndefSize))
            {
                break;
            }

            addrToWrite += ndefSize;

            if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, addrToWrite, &terminatorTLV, 1))
            {
                break;
            }
        }

        wasWritten = TRUE;
    } while (0);

    return wasWritten;
}

NFCOnboardingPayloadManagerImpl::eAppNtagError NFCOnboardingPayloadManagerImpl::AppNtagEepromUnlockThenWrite(uint8_t originalSize)
{
    eAppNtagError ntagErr = E_APP_NTAG_NO_ERROR;

    do
    {
        /* Unlock write access */
        ntagErr = AppNtagUnlockWriteAccess();
        if (ntagErr != E_APP_NTAG_NO_ERROR)
        {
            break;
        }

        /* Write the NDEF URI */
        if (!AppNtagEepromWrite(originalSize))
        {
            ntagErr = E_APP_NTAG_WRITE_ERROR;
        }

        /* Lock write access */
        AppNtagLockWriteAccess();
    } while (0);

    return ntagErr;
}

uint8_t NFCOnboardingPayloadManagerImpl::AppNdefUriRecordGetSize(NdefUriRecord_t ndefUriRecord)
{
    return sizeof(sInstance.ndefUriRecord.recordType) + sizeof(sInstance.ndefUriRecord.recordTypeLen) +
        sizeof(sInstance.ndefUriRecord.payloadLen) + sInstance.ndefUriRecord.payloadLen +
        sizeof(sInstance.ndefUriRecord.recordName);
}

NFCOnboardingPayloadManagerImpl::eAppNtagError NFCOnboardingPayloadManagerImpl::AppNtagLockWriteAccess(void)
{
    eAppNtagError ntagErr = E_APP_NTAG_NO_ERROR;

    uint8_t bytes[NTAG_I2C_BLOCK_SIZE];
    FLib_MemSet(bytes, 0x0, sizeof(bytes));

    do
    {
        /* Try to read the block 0 */
        if (NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, 0, bytes, sizeof(bytes)))
        {
            ntagErr = E_APP_NTAG_READ_ERROR;
            break;
        }

        /* Set the Capability Container (CC) */
        bytes[3] = 0xE1; /* Indicates that NDEF data is present inside the tag */
        bytes[4] = 0x10; /* Indicates to support the version 1.0 */
        bytes[5] = 0xE9; /* Indicates 1864 bytes of memory size assigned to the data area */
        bytes[6] = 0x0F; /* Indicates read only access granted */

        if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, 0, bytes, sizeof(bytes)))
        {
            ntagErr = E_APP_NTAG_WRITE_ERROR;
        }
    } while (0);

    return ntagErr;
}

NFCOnboardingPayloadManagerImpl::eAppNtagError NFCOnboardingPayloadManagerImpl::AppNtagUnlockWriteAccess(void)
{
    eAppNtagError ntagErr = E_APP_NTAG_NO_ERROR;

    uint8_t bytes[NTAG_I2C_BLOCK_SIZE];
    FLib_MemSet(bytes, 0x0, sizeof(bytes));

    do
    {
        /* Try to read the block 0 */
        if (NTAG_ReadBytes(sInstance.ntagDriverHandleInstance, 0, bytes, sizeof(bytes)))
        {
            ntagErr = E_APP_NTAG_READ_ERROR;
            break;
        }

        /* Set the Capability Container (CC) */
        bytes[12] = 0xE1; /* Indicates that NDEF data is present inside the tag */
        bytes[13] = 0x10; /* Indicates to support the version 1.0 */
        bytes[14] = 0xE9; /* Indicates 1864 bytes of memory size assigned to the data area */
        bytes[15] = 0x00; /* Indicates read and write access granted without any security */

        if (NTAG_WriteBytes(sInstance.ntagDriverHandleInstance, 0, bytes, sizeof(bytes)))
        {
            ntagErr = E_APP_NTAG_WRITE_ERROR;
        }

    } while (0);

    return ntagErr;
}

} // namespace DeviceLayer
} // namespace chip
#endif
