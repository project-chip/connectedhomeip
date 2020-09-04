/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements a template object for doing counter (CTR)
 *      mode block ciphers and specialized objects for CTR mode
 *      AES-128 and AES-256.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "CTRMode.h"
#include <stdint.h>
#include <string.h>

namespace chip {
namespace Crypto {

template <class BlockCipher>
CTRMode<BlockCipher>::CTRMode()
{}

template <class BlockCipher>
CTRMode<BlockCipher>::~CTRMode()
{
    Reset();
}

template <class BlockCipher>
void CTRMode<BlockCipher>::SetKey(const uint8_t * key)
{
    mBlockCipher.SetKey(key);
}

template <class BlockCipher>
void CTRMode<BlockCipher>::SetCounter(const uint8_t * counter)
{
    memcpy(Counter, counter, kCounterLength);
}

template <class BlockCipher>
void CTRMode<BlockCipher>::SetChipMessageCounter(uint64_t sendingNodeId, uint32_t msgId)
{
    // Initialize the CTR-mode encryption counter for encrypting/decrypting a CHIP message. In this mode
    // the counter consists of a 128-bit big-endian number with the following format:
    //
    //        (64-bits)     |   (32 bits)  |   (32 bits)
    //    <sending-node-id> | <message-id> | <block-counter>
    //
    Counter[0]  = (uint8_t)(sendingNodeId >> (7 * 8));
    Counter[1]  = (uint8_t)(sendingNodeId >> (6 * 8));
    Counter[2]  = (uint8_t)(sendingNodeId >> (5 * 8));
    Counter[3]  = (uint8_t)(sendingNodeId >> (4 * 8));
    Counter[4]  = (uint8_t)(sendingNodeId >> (3 * 8));
    Counter[5]  = (uint8_t)(sendingNodeId >> (2 * 8));
    Counter[6]  = (uint8_t)(sendingNodeId >> (1 * 8));
    Counter[7]  = (uint8_t)(sendingNodeId);
    Counter[8]  = (uint8_t)(msgId >> (3 * 8));
    Counter[9]  = (uint8_t)(msgId >> (2 * 8));
    Counter[10] = (uint8_t)(msgId >> (1 * 8));
    Counter[11] = (uint8_t)(msgId);
    Counter[12] = 0;
    Counter[13] = 0;
    Counter[14] = 0;
    Counter[15] = 0;
}

template <class BlockCipher>
void CTRMode<BlockCipher>::EncryptData(const uint8_t * inData, uint16_t dataLen, uint8_t * outData)
{
    // Index to next byte of encrypted counter to be used.
    uint32_t encryptedCounterIndex = mMsgIndex % kCounterLength;

    // For each byte of input data...
    for (uint16_t dataIndex = 0; dataIndex < dataLen && mMsgIndex < UINT32_MAX; dataIndex++, mMsgIndex++)
    {
        // If we need more encrypted counter bytes...
        if (encryptedCounterIndex == 0)
        {
            // Encrypt the next counter value.
            mBlockCipher.EncryptBlock(Counter, mEncryptedCounter);

            // Bump the counter. Since the message size is at most UINT32_MAX (and the counter counts blocks)
            // we will never need to update more than the four least-significant bytes.
            Counter[kCounterLength - 1]++;
            if (Counter[kCounterLength - 1] == 0)
            {
                Counter[kCounterLength - 2]++;
                if (Counter[kCounterLength - 2] == 0)
                {
                    Counter[kCounterLength - 3]++;
                    if (Counter[kCounterLength - 3] == 0)
                    {
                        Counter[kCounterLength - 4]++;
                    }
                }
            }
        }

        // XOR the data with the corresponding byte of the encrypted counter.
        outData[dataIndex] = inData[dataIndex] ^ mEncryptedCounter[encryptedCounterIndex];

        // Bump the counter index.
        encryptedCounterIndex++;
        if (encryptedCounterIndex == kCounterLength)
            encryptedCounterIndex = 0;
    }
}

template <class BlockCipher>
void CTRMode<BlockCipher>::Reset()
{
    mBlockCipher.Reset();
    mMsgIndex = 0;
    memset(Counter, 0, sizeof(Counter));
    ClearSecretData(mEncryptedCounter, sizeof(mEncryptedCounter));
}

template class CTRMode<Platform::Security::AES128BlockCipherEnc>;
template class CTRMode<Platform::Security::AES256BlockCipherEnc>;

} /* namespace Crypto */
} /* namespace chip */
