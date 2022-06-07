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

#pragma once

typedef enum
{
    RS_ERROR_NONE,
    RS_ERROR_NOT_FOUND,
    RS_ERROR_NO_BUFS
} rsError;

/* the structure used for keeping the records has the same structure both in RAM and in NVM:
 * ramBufferLen | ramBufferMaxLen | settingsBlock | .... | settingsBlock
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

#define member_size(type, member) sizeof(((type *) 0)->member)

constexpr size_t kRamDescHeaderSize =
    member_size(ramBufferDescriptor, ramBufferLen) + member_size(ramBufferDescriptor, ramBufferMaxLen);

/* Return a RAM buffer with initialSize and populated with the contents of NVM ID - if found in flash */
ramBufferDescriptor * getRamBuffer(uint16_t nvmId, uint16_t initialSize);

/* search pBuffer for aKey and return its value in aValue. aValueLength will contain the length of aValueLength */
rsError ramStorageGet(const ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex, uint8_t * aValue, uint16_t * aValueLength);

/* search pBuffer for aKey and set its value to aValue (having aValueLength length) */
rsError ramStorageSet(ramBufferDescriptor ** pBuffer, uint16_t aKey, const uint8_t * aValue, uint16_t aValueLength);

/* search pBuffer for aKey and delete it */
rsError ramStorageDelete(ramBufferDescriptor * pBuffer, uint16_t aKey, int aIndex);
