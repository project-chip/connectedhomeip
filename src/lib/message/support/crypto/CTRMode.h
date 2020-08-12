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
 *      This file defines a template object for doing counter (CTR) mode
 *      block ciphers and specialized objects for CTR mode AES-128 and
 *      AES-256.
 *
 */

#include <support/DLLUtil.h>

#include "AESBlockCipher.h"

#ifndef CTRMODE_H_
#define CTRMODE_H_

namespace chip {
namespace Crypto {

template <class BlockCipher>
class DLL_EXPORT CTRMode
{
public:
    enum
    {
        kKeyLength     = BlockCipher::kKeyLength,
        kCounterLength = BlockCipher::kBlockLength
    };

    CTRMode(void);
    ~CTRMode(void);

    // The CTR-mode encryption counter. The algorithm assumes the counter is in big-endian
    // form and increments it once for each encrypted block.
    uint8_t Counter[kCounterLength];

    void SetKey(const uint8_t * key);
    void SetCounter(const uint8_t * counter);
    void SetChipMessageCounter(uint64_t sendingNodeId, uint32_t msgId);
    void EncryptData(const uint8_t * inData, uint16_t dataLen, uint8_t * outData);

    void Reset(void);

private:
    BlockCipher mBlockCipher;
    uint32_t mMsgIndex;
    uint8_t mEncryptedCounter[kCounterLength];
};

typedef CTRMode<chip::Platform::Security::AES128BlockCipherEnc> AES128CTRMode;
typedef CTRMode<chip::Platform::Security::AES256BlockCipherEnc> AES256CTRMode;

} /* namespace Crypto */
} /* namespace chip */

#endif /* CTRMODE_H_ */
