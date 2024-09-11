/*
 *  Copyright (c) 2022, The OpenThread Authors.
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

/**
 *    @file
 *        RAM buffer structure used for keeping NVM records.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "FunctionLib.h"
#include "ram_storage.h"

#ifndef RAM_STORAGE_LOG
#define RAM_STORAGE_LOG 0
#endif

#if RAM_STORAGE_LOG
#include "fsl_debug_console.h"
#define RAM_STORAGE_PRINTF(...)                                                                                                    \
    PRINTF("[%s] ", __FUNCTION__);                                                                                                 \
    PRINTF(__VA_ARGS__);                                                                                                           \
    PRINTF("\n\r");
#else
#define RAM_STORAGE_PRINTF(...)
#endif

rsError ramStorageAdd(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    rsError error                     = RS_ERROR_NONE;
    struct settingsBlock currentBlock = { 0 };
    const uint16_t newBlockLength     = sizeof(struct settingsBlock) + aValueLength;

    assert(pBuffer);
    if (pBuffer->ramBufferLen + newBlockLength <= pBuffer->ramBufferMaxLen)
    {
        currentBlock.key    = aKey;
        currentBlock.length = aValueLength;

        memcpy(&pBuffer->pRamBuffer[pBuffer->ramBufferLen], &currentBlock, sizeof(struct settingsBlock));
        memcpy(&pBuffer->pRamBuffer[pBuffer->ramBufferLen + sizeof(struct settingsBlock)], aValue, aValueLength);
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

    assert(pBuffer);
    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(struct settingsBlock));

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

                    memcpy(aValue, &pBuffer->pRamBuffer[i + sizeof(struct settingsBlock)], readLength);
                }

                valueLength = currentBlock.length;
                error       = RS_ERROR_NONE;
                break;
            }

            currentIndex++;
        }

        i += sizeof(struct settingsBlock) + currentBlock.length;
    }

    if (aValueLength != NULL)
    {
        *aValueLength = valueLength;
    }

    RAM_STORAGE_PRINTF("key = %d err = %d", aKey, error);

    return error;
}

rsError ramStorageSet(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength)
{
    uint16_t i                        = 0;
    uint16_t currentBlockLength       = 0;
    struct settingsBlock currentBlock = { 0 };
    bool_t alreadyExists              = FALSE;
    rsError error                     = RS_ERROR_NONE;

    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(struct settingsBlock));
        currentBlockLength = sizeof(struct settingsBlock) + currentBlock.length;

        if (aKey == currentBlock.key)
        {
            /* unlikely: the updated value has a different length */
            if (currentBlock.length != aValueLength)
            {
                ramStorageDelete(pBuffer, aKey, -1);
                break;
            }

            memcpy(&pBuffer->pRamBuffer[i + sizeof(struct settingsBlock)], aValue, aValueLength);
            alreadyExists = TRUE;
            break;
        }
        else
        {
            i += currentBlockLength;
        }
    }

    if (!alreadyExists)
    {
        error = ramStorageAdd(pBuffer, aKey, aValue, aValueLength);
    }

    return error;
}

rsError ramStorageDelete(ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex)
{
    uint16_t i                        = 0;
    int currentIndex                  = 0;
    uint16_t nextBlockStart           = 0;
    uint16_t currentBlockLength       = 0;
    struct settingsBlock currentBlock = { 0 };
    rsError error                     = RS_ERROR_NOT_FOUND;
    bool_t found                      = FALSE;

    assert(pBuffer);
    while (i < pBuffer->ramBufferLen)
    {
        memcpy(&currentBlock, &pBuffer->pRamBuffer[i], sizeof(struct settingsBlock));
        currentBlockLength = sizeof(struct settingsBlock) + currentBlock.length;

        if (aKey == currentBlock.key)
        {
            if ((currentIndex == aIndex) || (aIndex == -1))
            {
                currentIndex++;

                nextBlockStart = i + currentBlockLength;

                if (nextBlockStart < pBuffer->ramBufferLen)
                {
                    memmove(&pBuffer->pRamBuffer[i], &pBuffer->pRamBuffer[nextBlockStart], pBuffer->ramBufferLen - nextBlockStart);
                }

                assert(pBuffer->ramBufferLen >= currentBlockLength);
                pBuffer->ramBufferLen -= currentBlockLength;
                found = TRUE;

                error = RS_ERROR_NONE;
            }

            if (found && (aIndex != -1))
            {
                break;
            }
        }

        if (!found)
        {
            i += currentBlockLength;
        }
        found = FALSE;
    }
    RAM_STORAGE_PRINTF("key = %d err = %d", aKey, error);
    return error;
}
