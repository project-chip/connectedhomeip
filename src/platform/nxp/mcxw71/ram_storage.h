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

#ifndef RAM_STORAGE_K32W_H_
#define RAM_STORAGE_K32W_H_

#include <stdint.h>

typedef enum
{
    RS_ERROR_NONE,
    RS_ERROR_NOT_FOUND,
    RS_ERROR_NO_BUFS
} rsError;

/* the structure used for keeping the records has the same structure both in RAM and in NVM:
 * ramBufferLen | ramBufferMaxLen | settingsBlock+Data | .... | settingsBlock+Data
 *
 * ramBufferLen shows how much of the RAM buffer is currently occupied with settingsBlock structures
 * ramBufferMaxLen shows the total malloc'ed size. Dynamic re-allocation is possible
 */
typedef struct
{
    uint16_t ramBufferLen;
    uint16_t ramBufferMaxLen;
    uint8_t pRamBuffer[1];
} ramBufferDescriptor;

struct settingsBlock
{
    uint16_t key;
    uint16_t length;
} __attribute__((packed));

#if defined(PDM_USE_DYNAMIC_MEMORY) && PDM_USE_DYNAMIC_MEMORY && defined(OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE) &&                \
    OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#define ENABLE_STORAGE_DYNAMIC_MEMORY 1
#else
#define ENABLE_STORAGE_DYNAMIC_MEMORY 0
#endif

/* increment size for dynamic memory re-allocation in case the
 * initial RAM buffer size gets insufficient
 */
#define kRamBufferReallocSize 512
#define kRamBufferMaxAllocSize 10240

#define kRamDescHeaderSize offsetof(ramBufferDescriptor, pRamBuffer)

#ifdef __cplusplus
extern "C" {
#endif

/* search RAM Buffer for aKey and return its value in aValue. aValueLength will contain the length of aValueLength */
rsError ramStorageGet(const ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength);

/* search RAM buffer for aKey and set its value to aValue (having aValueLength length)
 * - aValue and aValueLength can be NULL - the function checks only for the existence of aKey
 * - if only aValue is NULL and aKey exists in the RAM buffer - the function will return its value in aValueLength
 */
rsError ramStorageSet(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength);

/* adds a settingsBlock (aKey:aValue) to the end of the RAM Buffer:
 * - doesn't check if aKey already exists in the RAM Buffer
 * - aValueLength can be 0
 */
rsError ramStorageAdd(ramBufferDescriptor * pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength);

/* search RAM Buffer for aKey (with aIndex) and delete it:
 * - if aIndex is -1 then all the  occurrences of aKey are deleted
 */
rsError ramStorageDelete(ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex);

#ifdef __cplusplus
}
#endif

#endif /* RAM_STORAGE_K32W_H_ */
