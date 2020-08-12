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
 *      Implementation of HMAC-based Extract-and-Expand Key Derivation Function (RFC-5869)
 *
 */

#ifndef HKDF_H_
#define HKDF_H_

#include "CHIPCrypto.h"
#include "HMAC.h"

namespace chip {
namespace Crypto {

template <class H>
class DLL_EXPORT HKDF
{
public:
    enum
    {
        kPseudoRandomKeyLength = H::kHashLength
    };

    uint8_t PseudoRandomKey[kPseudoRandomKeyLength];

    HKDF(void);
    ~HKDF(void);

    void BeginExtractKey(const uint8_t * salt, uint16_t saltLen);
    void AddKeyMaterial(const uint8_t * keyData, uint16_t keyDataLen);
#if CHIP_WITH_OPENSSL
    void AddKeyMaterial(const BIGNUM & num);
#endif
    CHIP_ERROR FinishExtractKey(void);

    CHIP_ERROR ExpandKey(const uint8_t * info, uint16_t infoLen, uint16_t keyLen, uint8_t * outKey);

    static CHIP_ERROR DeriveKey(const uint8_t * salt, uint16_t saltLen, const uint8_t * keyMaterial1, uint16_t keyMaterial1Len,
                                const uint8_t * keyMaterial2, uint16_t keyMaterial2Len, const uint8_t * info, uint16_t infoLen,
                                uint8_t * outKey, uint16_t outKeyBufSize, uint16_t outKeyLen);

    void Reset(void);

private:
    HMAC<H> mHMAC;
};

typedef HKDF<chip::Platform::Security::SHA1> HKDFSHA1;

typedef HKDF<chip::Platform::Security::SHA256> HKDFSHA256;

class HKDFSHA1Or256
{
public:
    enum
    {
        kMaxPseudoRandomKeyLength = chip::Platform::Security::SHA256::kHashLength
    };

    uint8_t PseudoRandomKey[kMaxPseudoRandomKeyLength];

    HKDFSHA1Or256(bool useSHA1);
    ~HKDFSHA1Or256(void) { Reset(); }

    void BeginExtractKey(const uint8_t * salt, uint16_t saltLen);
    void AddKeyMaterial(const uint8_t * keyData, uint16_t keyDataLen);
    CHIP_ERROR FinishExtractKey(void);
    CHIP_ERROR ExpandKey(const uint8_t * info, uint16_t infoLen, uint16_t keyLen, uint8_t * outKey);
    void Reset(void);

private:
    union
    {
        uint8_t mObjBuf[1];
        uint8_t mSHA1ObjBuf[sizeof(HKDFSHA1)];
        uint8_t mSHA256ObjBuf[sizeof(HKDFSHA256)];
        uint64_t mForceAlign;
    };
    bool mUseSHA1;

    void * ObjBuf() { return mObjBuf; }

    HKDFSHA1 * HKDFSHA1Obj() { return (HKDFSHA1 *) ObjBuf(); }
    HKDFSHA256 * HKDFSHA256Obj() { return (HKDFSHA256 *) ObjBuf(); }
};

} // namespace Crypto
} // namespace chip

#endif /* HKDF_H_ */
