/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines data types, objects and APIs for
 *      working with Certification Declaration elements.
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

static constexpr uint32_t kMaxProductIdsCountPerCD = 100;
static constexpr uint32_t kCertificateIdLength     = 19;

// TODO: share code with EstimateTLVStructOverhead to estimate TLV structure size.
static constexpr uint32_t kCertificationElements_TLVEncodedMaxLength = (1 + 1) + // Length of header and end of outer TLV structure.
    (3 + kCertificateIdLength) +                                                 // Encoded length of CertificateId string.
    (1 + sizeof(uint16_t)) * kMaxProductIdsCountPerCD + 3 + // Max encoding length of an array of 100 uint16_t elements.
    (2 + sizeof(uint8_t)) * 2 +                             // Encoding length of two uint8_t element.
    (2 + sizeof(uint16_t)) * 7;                             // Max total encoding length of seven uint16_t elements.

static constexpr uint32_t kMaxCMSSignedCDMessage = 183 + kCertificationElements_TLVEncodedMaxLength;

struct CertificationElements
{
    uint16_t FormatVersion;
    uint16_t VendorId;
    uint16_t ProductIds[kMaxProductIdsCountPerCD];
    uint8_t ProductIdsCount;
    uint32_t DeviceTypeId;
    char CertificateId[kCertificateIdLength + 1];
    uint8_t SecurityLevel;
    uint16_t SecurityInformation;
    uint16_t VersionNumber;
    uint8_t CertificationType;
    uint16_t DACOriginVendorId;
    uint16_t DACOriginProductId;
    bool DACOriginVIDandPIDPresent;
};

/**
 * @brief Encode certification elements in TLV format.
 *
 * @param[in]  certElements         Certification elements to encode.
 * @param[out] encodedCertElements  A byte span to write to the TLV encoded certification elements.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR EncodeCertificationElements(const CertificationElements & certElements, MutableByteSpan & encodedCertElements);

/**
 * @brief Decode certification elements from TLV encoded structure.
 *
 * @param[in]  encodedCertElements  A byte span to read the TLV encoded certification elements.
 * @param[out] certElements         Decoded certification elements.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeCertificationElements(const ByteSpan & encodedCertElements, CertificationElements & certElements);

/**
 * @brief Generate CMS signed message.
 *
 * @param[in]  cdContent       A byte span with Certification Declaration TLV encoded content.
 * @param[in]  signerKeyId     A byte span with the signer key identifier.
 * @param[in]  signerKeypair   A reference to keypair used to sign the message.
 * @param[out] signedMessage   A byte span to hold a signed CMS message.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR CMS_Sign(const ByteSpan & cdContent, const ByteSpan & signerKeyId, Crypto::P256Keypair & signerKeypair,
                    MutableByteSpan & signedMessage);

/**
 * @brief Verify CMS signed message.
 *
 * @param[in]  signedMessage   A byte span with CMS signed message.
 * @param[in]  signerX509Cert  A byte span with the signer certificate in X509 form.
 * @param[out] cdContent       A byte span to hold a CD content extracted from the CMS signed message.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR CMS_Verify(const ByteSpan & signedMessage, const ByteSpan & signerX509Cert, ByteSpan & cdContent);

/**
 * @brief Verify CMS signed message.
 *
 * @param[in]  signedMessage   A byte span with CMS signed message.
 * @param[in]  signerPubkey    A reference to public key associated with the private key that was used to sign the message.
 * @param[out] cdContent       A byte span to hold a CD content extracted from the CMS signed message.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR CMS_Verify(const ByteSpan & signedMessage, const Crypto::P256PublicKey & signerPubkey, ByteSpan & cdContent);

/**
 * @brief Extract Certification Declaration content from the CMS signed message.
 *
 * @param[in]  signedMessage   A byte span with CMS signed message.
 * @param[out] cdContent       A byte span to hold a CD content extracted from the CMS signed message.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR CMS_ExtractCDContent(const ByteSpan & signedMessage, ByteSpan & cdContent);

/**
 * @brief Extract key identifier from the CMS signed message.
 *
 * @param[in]  signedMessage   A byte span with CMS signed message.
 * @param[out] signerKeyId     A byte span to hold a key identifier extracted from the CMS signed message.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR CMS_ExtractKeyId(const ByteSpan & signedMessage, ByteSpan & signerKeyId);

} // namespace Credentials
} // namespace chip
