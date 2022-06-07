/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *        NVM Writes are time-consuming so instead of blocking the main tasks (e.g.: Matter)
 *        buffer the writes in a RAM buffer that gets to be written in the Idle Task
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "FunctionLib.h"
#include "RamStorage.h"

#ifndef RAM_STORAGE_LOG
#define RAM_STORAGE_LOG 0
#endif

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if RAM_STORAGE_LOG
#include "fsl_debug_console.h"
#define RAM_STORAGE_PRINTF(...)                                                                                                    \
    PRINTF("[%s] ", __FUNCTION__);                                                                                                 \
    PRINTF(__VA_ARGS__);                                                                                                           \
    PRINTF("\n\r");
#else
#define RAM_STORAGE_PRINTF(...)
#endif

/* increment size for dynamic memory re-allocation in case the
 * initial RAM buffer size gets insufficient
 */
constexpr size_t kRamBufferReallocSize  = 512;
constexpr size_t kRamBufferMaxAllocSize = 10240;

ramBufferDescriptor * getRamBuffer(uint16_t nvmId, uint16_t initialSize)
{
    ramBufferDescriptor * ramDescr = NULL;
    bool bLoadDataFromNvm          = false;
    uint16_t bytesRead             = 0;
    uint16_t recordSize            = 0;
    uint16_t allocSize             = initialSize;

    /* Check if dataset is present and get its size */
    if (PDM_bDoesDataExist(nvmId, &recordSize))
    {
        bLoadDataFromNvm = true;
        while (recordSize > allocSize)
        {
            // increase size until NVM data fits
            allocSize += kRamBufferReallocSize;
        }
    }

    if (allocSize <= kRamBufferMaxAllocSize)
    {
        ramDescr = (ramBufferDescriptor *) malloc(allocSize);
        if (ramDescr)
        {
            ramDescr->ramBufferLen    = 0;
            ramDescr->ramBufferMaxLen = allocSize - kRamDescHeaderSize;

            if (bLoadDataFromNvm)
            {
                /* Try to load the dataset in RAM */
                if (PDM_E_STATUS_OK != PDM_eReadDataFromRecord(nvmId, ramDescr, recordSize, &bytesRead))
                {
                    memset(ramDescr, 0, allocSize);
                }
            }
        }
    }

    return ramDescr;
}

static rsError ramStorageAdd(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    rsError error                     = RS_ERROR_NONE;
    struct settingsBlock currentBlock = { 0 };
    const uint16_t newBlockLength     = sizeof(settingsBlock) + aValueLength;

    if (pBuffer->ramBufferLen + newBlockLength <= pBuffer->ramBufferMaxLen)
    {
        currentBlock.key    = aKey;
        currentBlock.length = aValueLength;

        memcpy(&pBuffer->pRamBuffer[pBuffer->ramBufferLen], &currentBlock, sizeof(settingsBlock));
        memcpy(&pBuffer->pRamBuffer[pBuffer->ramBufferLen + sizeof(settingsBlock)], aValue, aValueLength);
        pBuffer->ramBufferLen += newBlockLength;

        error = RS_ERROR_NONE;
    }
    else
    {
        error = RS_ERROR_NO_BUFS;
    }

    RAM_STORAGE_PRINTF("key = %d lengthWriten = %d err = %d", aKey, aValueLength, error);

    return error;
}

rsError ramStorageGet(const ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength)
{
    uint16_t i                        = 0;
    uint16_t valueLength              = 0;
    uint16_t readLength               = 0;
    int currentIndex                  = 0;
    struct settingsBlock currentBlock = { 0 };
    rsError error                     = RS_ERROR_NOT_FOUND;

    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(settingsBlock));

        if (aKey == currentBlock.key)
        {
            if (currentIndex == aIndex)
            {
                readLength = currentBlock.length;

                // Perform read only if an input buffer was passed in
                if (aValue != NULL && aValueLength != NULL)
                {
                    // Adjust read length if input buffer size is smaller
                    if (readLength > *aValueLength)
                    {
                        readLength = *aValueLength;
                    }

                    memcpy(aValue, &pBuffer->pRamBuffer[i + sizeof(settingsBlock)], readLength);
                }

                valueLength = currentBlock.length;
                error       = RS_ERROR_NONE;
                break;
            }

            currentIndex++;
        }

        i += sizeof(settingsBlock) + currentBlock.length;
    }

    if (aValueLength != NULL)
    {
        *aValueLength = valueLength;
    }

    RAM_STORAGE_PRINTF("key = %d err = %d", aKey, error);

    return error;
}

static rsError ramStorageSetInternal(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    uint16_t i                        = 0;
    uint16_t currentBlockLength       = 0;
    uint16_t nextBlockStart           = 0;
    struct settingsBlock currentBlock = { 0 };

    // Delete all entries of aKey
    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(settingsBlock));
        currentBlockLength = sizeof(settingsBlock) + currentBlock.length;

        if (aKey == currentBlock.key)
        {
            nextBlockStart = i + currentBlockLength;

            if (nextBlockStart < pBuffer->ramBufferLen)
            {
                memmove(&pBuffer->pRamBuffer[i], &pBuffer->pRamBuffer[nextBlockStart], pBuffer->ramBufferLen - nextBlockStart);
            }

            VerifyOrDie(pBuffer->ramBufferLen >= currentBlockLength);
            pBuffer->ramBufferLen -= currentBlockLength;
        }
        else
        {
            i += currentBlockLength;
        }
    }

    return ramStorageAdd(pBuffer, aKey, aValue, aValueLength);
}

rsError ramStorageSet(ramBufferDescriptor ** pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    rsError err               = RS_ERROR_NONE;
    uint16_t allocSize        = (*pBuffer)->ramBufferMaxLen;
    ramBufferDescriptor * ptr = NULL;

    if (allocSize <= (*pBuffer)->ramBufferLen + aValueLength)
    {
        while ((allocSize < (*pBuffer)->ramBufferLen + aValueLength))
        {
            /* Need to realocate the memory buffer, increase size by kRamBufferReallocSize until NVM data fits */
            allocSize += kRamBufferReallocSize;
        }

        allocSize += kRamDescHeaderSize;

        if (kRamBufferMaxAllocSize <= kRamBufferMaxAllocSize)
        {
            ptr = (ramBufferDescriptor *) realloc((void *) (*pBuffer), allocSize);
            VerifyOrExit((NULL != ptr), err = RS_ERROR_NO_BUFS);
            *pBuffer                    = ptr;
            (*pBuffer)->ramBufferMaxLen = allocSize;
        }
        else
        {
            err = RS_ERROR_NO_BUFS;
        }
    }

    err = ramStorageSetInternal(*pBuffer, aKey, aValue, aValueLength);

exit:
    return err;
}

rsError ramStorageDelete(ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex)
{
    uint16_t i                        = 0;
    int currentIndex                  = 0;
    uint16_t nextBlockStart           = 0;
    uint16_t currentBlockLength       = 0;
    struct settingsBlock currentBlock = { 0 };
    rsError error                     = RS_ERROR_NOT_FOUND;

    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(settingsBlock));
        currentBlockLength = sizeof(settingsBlock) + currentBlock.length;

        if (aKey == currentBlock.key)
        {
            if (currentIndex == aIndex)
            {
                nextBlockStart = i + currentBlockLength;

                if (nextBlockStart < pBuffer->ramBufferLen)
                {
                    memmove(&pBuffer->pRamBuffer[i], &pBuffer->pRamBuffer[nextBlockStart], pBuffer->ramBufferLen - nextBlockStart);
                }

                VerifyOrDie(pBuffer->ramBufferLen >= currentBlockLength);
                pBuffer->ramBufferLen -= currentBlockLength;

                error = RS_ERROR_NONE;
                break;
            }
            else
            {
                currentIndex++;
            }
        }

        i += currentBlockLength;
    }
    RAM_STORAGE_PRINTF("key = %d err = %d", aKey, error);
    return error;
}
