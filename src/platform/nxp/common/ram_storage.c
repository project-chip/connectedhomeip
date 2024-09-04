/*
 *  Copyright (c) 2021-2022, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "FunctionLib.h"
#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"
#include "fwk_filesystem.h"
#include "ram_storage.h"

#ifndef RAM_STORAGE_LOG
#define RAM_STORAGE_LOG 0
#endif

#ifndef KEY_BYTE_MAX_SIZE
#define KEY_BYTE_MAX_SIZE 28
#endif

#if RAM_STORAGE_LOG
#define RAM_STORAGE_PRINTF(key, size, ...)                                                                                         \
    PRINTF("[%s] ", __FUNCTION__);                                                                                                 \
    uint8_t printIt;                                                                                                               \
    PRINTF("[key = ");                                                                                                             \
    for (printIt = 0; printIt < size; printIt++)                                                                                   \
    {                                                                                                                              \
        PRINTF("0x%x ", key[printIt]);                                                                                             \
    }                                                                                                                              \
    PRINTF("]");                                                                                                                   \
    PRINTF(__VA_ARGS__);                                                                                                           \
    PRINTF("\n\r");
#else
#define RAM_STORAGE_PRINTF(...)
#endif

struct settingsBlock
{
    uint8_t keyBytes[KEY_BYTE_MAX_SIZE];
    uint8_t keyBytesLen;
    uint16_t length;
} __attribute__((packed));

static OSA_MUTEX_HANDLE_DEFINE(mRamStorageMutexId);

static rsError ramStorageAdd(ramBufferDescriptor * pBuffer, const uint8_t * pAKey, uint8_t keySize, const uint8_t * aValue,
                             uint16_t aValueLength)
{
    rsError error;
    struct settingsBlock * currentBlock;
    const uint16_t newBlockLength = sizeof(struct settingsBlock) + aValueLength;

    (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

    if (*(pBuffer->ramBufferLen) + newBlockLength <= pBuffer->ramBufferMaxLen)
    {
        currentBlock = (struct settingsBlock *) &pBuffer->pRamBuffer[*(pBuffer->ramBufferLen)];
        memset(currentBlock, 0x0, sizeof(struct settingsBlock));
        memcpy(currentBlock->keyBytes, pAKey, keySize);
        currentBlock->keyBytesLen = keySize;
        currentBlock->length      = aValueLength;

        memcpy(&pBuffer->pRamBuffer[*(pBuffer->ramBufferLen) + sizeof(struct settingsBlock)], aValue, aValueLength);
        *(pBuffer->ramBufferLen) += newBlockLength;

        assert(*(pBuffer->ramBufferLen) <= pBuffer->ramBufferMaxLen);

        error = RS_ERROR_NONE;
    }
    else
    {
        assert(0);
        error = RS_ERROR_NO_BUFS;
    }

    (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);

    RAM_STORAGE_PRINTF(pAKey, keySize, "lengthWriten = %d err = %d, newLen = %d", aValueLength, error, *(pBuffer->ramBufferLen));

    return error;
}

rsError ramStorageGet(const ramBufferDescriptor * pBuffer, const uint8_t * pAKey, uint8_t keySize, int aIndex, uint8_t * aValue,
                      uint16_t * aValueLength)
{
    uint16_t i           = 0;
    uint16_t valueLength = 0;
    uint16_t readLength;
    int currentIndex = 0;
    const struct settingsBlock * currentBlock;
    rsError error = RS_ERROR_NOT_FOUND;

    if (keySize <= KEY_BYTE_MAX_SIZE)
    {
        (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

        while (i < *(pBuffer->ramBufferLen))
        {
            currentBlock = (struct settingsBlock *) &pBuffer->pRamBuffer[i];

            /* Assert allowing to make sure that the dataset is not corrupted */
            assert(currentBlock->length <= pBuffer->ramBufferMaxLen);

            if (currentBlock->keyBytesLen == keySize && memcmp(pAKey, currentBlock->keyBytes, keySize) == 0)
            {
                if (currentIndex == aIndex)
                {
                    readLength = currentBlock->length;

                    // Perform read only if an input buffer was passed in
                    if (aValue != NULL && aValueLength != NULL)
                    {
                        // Adjust read length if input buffer size is smaller
                        if (readLength > *aValueLength)
                        {
                            readLength = *aValueLength;
                        }

                        memcpy(aValue, &pBuffer->pRamBuffer[i + sizeof(struct settingsBlock)], readLength);
                    }

                    valueLength = currentBlock->length;
                    error       = RS_ERROR_NONE;
                    break;
                }

                currentIndex++;
            }

            i += sizeof(struct settingsBlock) + currentBlock->length;
        }

        (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);
    }
    else
    {
        error = RS_ERROR_WRONG_ARG;
    }

    if (aValueLength != NULL)
    {
        *aValueLength = valueLength;
    }

    RAM_STORAGE_PRINTF(pAKey, keySize, "err = %d", error);

    return error;
}

rsError ramStorageSet(ramBufferDescriptor * pBuffer, const uint8_t * pAKey, uint8_t keySize, const uint8_t * aValue,
                      uint16_t aValueLength)
{
    uint16_t i = 0;
    uint16_t currentBlockLength;
    uint16_t nextBlockStart;
    const struct settingsBlock * currentBlock;
    rsError error = RS_ERROR_WRONG_ARG;

    assert(keySize <= KEY_BYTE_MAX_SIZE);

    if (keySize <= KEY_BYTE_MAX_SIZE)
    {
        (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

        // Delete all entries of aKey
        while (i < *(pBuffer->ramBufferLen))
        {
            currentBlock       = (struct settingsBlock *) &pBuffer->pRamBuffer[i];
            currentBlockLength = sizeof(struct settingsBlock) + currentBlock->length;

            if (currentBlock->keyBytesLen == keySize && memcmp(pAKey, currentBlock->keyBytes, keySize) == 0)
            {
                nextBlockStart = i + currentBlockLength;

                if (nextBlockStart < *(pBuffer->ramBufferLen))
                {
                    memmove(&pBuffer->pRamBuffer[i], &pBuffer->pRamBuffer[nextBlockStart],
                            *(pBuffer->ramBufferLen) - nextBlockStart);
                }

                assert(*(pBuffer->ramBufferLen) >= currentBlockLength);
                *(pBuffer->ramBufferLen) -= currentBlockLength;
            }
            else
            {
                i += currentBlockLength;
            }
        }
        error = ramStorageAdd(pBuffer, pAKey, keySize, aValue, aValueLength);
        (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);
    }

    return error;
}

rsError ramStorageDelete(ramBufferDescriptor * pBuffer, const uint8_t * pAKey, uint8_t keySize, int aIndex)
{
    uint16_t i       = 0;
    int currentIndex = 0;
    uint16_t nextBlockStart;
    uint16_t currentBlockLength;
    const struct settingsBlock * currentBlock;
    rsError error = RS_ERROR_NOT_FOUND;

    if (keySize <= KEY_BYTE_MAX_SIZE)
    {
        (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

        while (i < *(pBuffer->ramBufferLen))
        {
            currentBlock       = (struct settingsBlock *) &pBuffer->pRamBuffer[i];
            currentBlockLength = sizeof(struct settingsBlock) + currentBlock->length;

            if (currentBlock->keyBytesLen == keySize && memcmp(pAKey, currentBlock->keyBytes, keySize) == 0)
            {
                if (currentIndex == aIndex)
                {
                    nextBlockStart = i + currentBlockLength;

                    if (nextBlockStart < *(pBuffer->ramBufferLen))
                    {
                        memmove(&pBuffer->pRamBuffer[i], &pBuffer->pRamBuffer[nextBlockStart],
                                *(pBuffer->ramBufferLen) - nextBlockStart);
                    }

                    assert(*(pBuffer->ramBufferLen) >= currentBlockLength);
                    *(pBuffer->ramBufferLen) -= currentBlockLength;

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

        (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);
    }
    else
    {
        error = RS_ERROR_WRONG_ARG;
        assert(0);
    }
    RAM_STORAGE_PRINTF(pAKey, keySize, "err = %d", error);
    return error;
}

void ramStorageInit(void)
{
    /* Mutex create */
    (void) OSA_MutexCreate(mRamStorageMutexId);
    assert(NULL != mRamStorageMutexId);
}

int ramStorageReadFromFlash(const char * file_name, uint8_t * buffer, uint32_t buf_length)
{
    int res;

    (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

    res = FS_ReadBufferFromFile(file_name, buffer, buf_length);

    (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);

    return res;
}

/* Buffer size must be superior to the max ram buffer size CHIP_CONFIG_RAM_BUFFER_KEY_STRING_SIZE */
static uint8_t bufferIdleWriteToFlash[4 * 5000];

int ramStorageSavetoFlash(const char * file_name, uint8_t * buffer, uint32_t buf_length)
{
    int res;

    assert(sizeof(bufferIdleWriteToFlash) >= buf_length);

    /**
     * Copy buffer to minimise the task lock duration
     * TODO : Improve ram buffer management to minimize ram buffer usage
     */
    (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);
    memcpy(bufferIdleWriteToFlash, buffer, buf_length);
    (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);

    res = FS_WriteBufferToFile(file_name, bufferIdleWriteToFlash, buf_length);

    return res;
}

void ramStorageDump(const ramBufferDescriptor * pBuffer)
{
    uint16_t i = 0;
    uint16_t j = 0;
    const struct settingsBlock * currentBlock;

    (void) OSA_MutexLock((osa_mutex_handle_t) mRamStorageMutexId, osaWaitForever_c);

    while (i < *(pBuffer->ramBufferLen))
    {
        currentBlock = (struct settingsBlock *) &pBuffer->pRamBuffer[i];
        PRINTF("key = ");
        for (j = 0; j < currentBlock->keyBytesLen; j++)
        {
            PRINTF("%c", currentBlock->keyBytes[j]);
        }
        PRINTF("\nlen = %d\n", currentBlock->length);
        i += sizeof(struct settingsBlock) + currentBlock->length;
    }
    (void) OSA_MutexUnlock((osa_mutex_handle_t) mRamStorageMutexId);
}
