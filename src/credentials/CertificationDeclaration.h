/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

static constexpr uint32_t kMaxProductIdsCount        = 100;
static constexpr uint32_t kMaxAuthorizedPAAListCount = 10;
static constexpr uint32_t kCertificateIdLength       = 19;
static constexpr uint32_t kCertificationElements_TLVEncodedMaxLength =
    TLV::EstimateStructOverhead(sizeof(uint16_t), // FormatVersion
                                sizeof(uint16_t), // VendorId
                                // ProductIds. Formally, the following extression should be used here:
                                //     ( TLV::EstimateStructOverhead(sizeof(uint16_t)) * kMaxProductIdsCount ),
                                // Because exact structure of the elements of this array is known, more accurate estimate is used.
                                (1 + sizeof(uint16_t)) * kMaxProductIdsCount,
                                sizeof(uint32_t),                                         // DeviceTypeId
                                kCertificateIdLength,                                     // CertificateId
                                sizeof(uint8_t),                                          // SecurityLevel
                                sizeof(uint16_t),                                         // SecurityInformation
                                sizeof(uint16_t),                                         // VersionNumber
                                sizeof(uint8_t),                                          // CertificationType
                                sizeof(uint16_t),                                         // DACOriginVendorId
                                sizeof(uint16_t),                                         // DACOriginProductId
                                (2 + kKeyIdentifierLength) * kMaxAuthorizedPAAListCount); // AuthorizedPAAList
static constexpr uint32_t kMaxCMSSignedCDMessage = 183 + kCertificationElements_TLVEncodedMaxLength;

struct CertificationElements
{
    typedef uint8_t KeyId[kKeyIdentifierLength];

    uint16_t FormatVersion                              = 0;
    uint16_t VendorId                                   = VendorId::NotSpecified;
    uint16_t ProductIds[kMaxProductIdsCount]            = { 0 };
    uint8_t ProductIdsCount                             = 0;
    uint32_t DeviceTypeId                               = 0;
    char CertificateId[kCertificateIdLength + 1]        = { 0 };
    uint8_t SecurityLevel                               = 0;
    uint16_t SecurityInformation                        = 0;
    uint16_t VersionNumber                              = 0;
    uint8_t CertificationType                           = 0;
    uint16_t DACOriginVendorId                          = VendorId::NotSpecified;
    uint16_t DACOriginProductId                         = 0;
    bool DACOriginVIDandPIDPresent                      = false;
    KeyId AuthorizedPAAList[kMaxAuthorizedPAAListCount] = { { 0 } };
    uint8_t AuthorizedPAAListCount                      = 0;
};

struct CertificationElementsWithoutPIDs
{
    uint16_t formatVersion                       = 0;
    uint16_t vendorId                            = VendorId::NotSpecified;
    uint32_t deviceTypeId                        = 0;
    uint8_t securityLevel                        = 0;
    uint16_t securityInformation                 = 0;
    uint16_t versionNumber                       = 0;
    uint8_t certificationType                    = 0;
    uint16_t dacOriginVendorId                   = VendorId::NotSpecified;
    uint16_t dacOriginProductId                  = 0;
    bool dacOriginVIDandPIDPresent               = false;
    bool authorizedPAAListPresent                = false;
    char certificateId[kCertificateIdLength + 1] = { 0 };
};

enum class CertificationType : uint8_t
{
    kDevelopmentAndTest,
    kProvisional,
    kOfficial,
    kReserved,
};

class CertificationElementsDecoder
{
public:
    bool IsProductIdIn(const ByteSpan & encodedCertElements, uint16_t productId);
    bool HasAuthorizedPAA(const ByteSpan & encodedCertElements, const ByteSpan & authorizedPAA);

private:
    /**
     * @brief Positions mReader inside at the top of an Array with listTag tag.
     *
     * @param[in] encodedCertElements  TLV encoded structure of CD elements.
     * @param[in] listTag              A tag of an array to be found in the encodedCertElements TLV structure.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindAndEnterArray(const ByteSpan & encodedCertElements, TLV::Tag listTag);

    CHIP_ERROR GetNextProductId(uint16_t & productId);
    CHIP_ERROR GetNextAuthorizedPAA(ByteSpan & authorizedPAA);

    TLV::TLVReader mReader;
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
 * @brief Decode certification elements from TLV encoded structure.
 *
 * @param[in]  encodedCertElements  A byte span to read the TLV encoded certification elements.
 * @param[out] certDeclContent         Decoded Certification Declaration Content.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeCertificationElements(const ByteSpan & encodedCertElements, CertificationElementsWithoutPIDs & certDeclContent);

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
