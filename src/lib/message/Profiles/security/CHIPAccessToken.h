/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      Utility functions for interacting with CHIP Access Tokens.
 *
 */

#include <Profiles/CHIPProfiles.h>
#include <Profiles/security/CHIPCert.h>
#include <Profiles/security/CHIPSecurity.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/ASN1.h>
#include <support/CodeUtils.h>
#include <support/crypto/EllipticCurve.h>
#include <support/crypto/HashAlgos.h>

namespace chip {
namespace Profiles {
namespace Security {

extern CHIP_ERROR LoadAccessTokenCerts(const uint8_t * accessToken, uint32_t accessTokenLen, ChipCertificateSet & certSet,
                                       uint16_t decodeFlags, ChipCertificateData *& accessTokenCert);
extern CHIP_ERROR LoadAccessTokenCerts(TLVReader & reader, ChipCertificateSet & certSet, uint16_t decodeFlags,
                                       ChipCertificateData *& accessTokenCert);

extern CHIP_ERROR CASECertInfoFromAccessToken(const uint8_t * accessToken, uint32_t accessTokenLen, uint8_t * buf, uint16_t bufSize,
                                              uint16_t & certInfoLen);
extern CHIP_ERROR CASECertInfoFromAccessToken(TLVReader & reader, TLVWriter & writer);

extern CHIP_ERROR ExtractPrivateKeyFromAccessToken(const uint8_t * accessToken, uint32_t accessTokenLen, uint8_t * privKeyBuf,
                                                   uint16_t privKeyBufSize, uint16_t & privKeyLen);
extern CHIP_ERROR ExtractPrivateKeyFromAccessToken(TLVReader & reader, TLVWriter & writer);

} // namespace Security
} // namespace Profiles
} // namespace chip
