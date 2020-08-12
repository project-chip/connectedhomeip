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
 *      This file defines interfaces for encoding and decoding CHIP
 *      elliptic curve private keys.
 *
 */

#ifndef CHIPPRIVATEKEY_H_
#define CHIPPRIVATEKEY_H_

#include <support/ASN1.h>
#include <support/crypto/EllipticCurve.h>

namespace chip {
namespace Profiles {
namespace Security {

using chip::ASN1::OID;
using chip::Crypto::EncodedECPrivateKey;
using chip::Crypto::EncodedECPublicKey;

// Utility functions for encoding/decoding private keys in CHIP TLV format.

extern CHIP_ERROR EncodeChipECPrivateKey(uint32_t weaveCurveId, const EncodedECPublicKey * pubKey,
                                         const EncodedECPrivateKey & privKey, uint8_t * outBuf, uint32_t outBufSize,
                                         uint32_t & outLen);

extern CHIP_ERROR DecodeChipECPrivateKey(const uint8_t * buf, uint32_t len, uint32_t & weaveCurveId, EncodedECPublicKey & pubKey,
                                         EncodedECPrivateKey & privKey);

} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPPRIVATEKEY_H_ */
