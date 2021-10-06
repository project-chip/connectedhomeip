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
 *      This file implements data types, objects and APIs for
 *      working with Certification Declaration elements.
 */

#include <inttypes.h>
#include <stddef.h>

#include <credentials/CertificationDeclaration.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Crypto;

static constexpr uint8_t sOID_ContentType_PKCS7Data[]       = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01 };
static constexpr uint8_t sOID_ContentType_PKCS7SignedData[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02 };
static constexpr uint8_t sOID_DigestAlgo_SHA256[]           = { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 };
static constexpr uint8_t sOID_SigAlgo_ECDSAWithSHA256[]     = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 };

/** Certification Declaration Element TLV Tags
 */
enum
{
    kTag_VendorId            = 1, /**< [ unsigned int ] Vedor identifier. */
    kTag_ProductIds          = 2, /**< [ array ] Product identifiers (each is unsigned int). */
    kTag_ServerCategoryId    = 3, /**< [ unsigned int ] Server category identifier. */
    kTag_ClientCategoryId    = 4, /**< [ unsigned int ] Client category identifier. */
    kTag_SecurityLevel       = 5, /**< [ unsigned int ] Security level. */
    kTag_SecurityInformation = 6, /**< [ unsigned int ] Security information. */
    kTag_VersionNumber       = 7, /**< [ unsigned int ] Version number. */
    kTag_CertificationType   = 8, /**< [ unsigned int ] Certification Type. */
};

CHIP_ERROR EncodeCertificationElements(const CertificationElements & certElements, MutableByteSpan & encodedCertElements)
{
    TLVWriter writer;
    TLVType outerContainer1, outerContainer2;

    writer.Init(encodedCertElements);

    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag, kTLVType_Structure, outerContainer1));

    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_VendorId), certElements.VendorId));

    VerifyOrReturnError(certElements.ProductIdsCount > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(certElements.ProductIdsCount < kMaxProductIdsCountPerCD, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_ProductIds), kTLVType_Array, outerContainer2));
    for (uint8_t i = 0; i < certElements.ProductIdsCount; i++)
    {
        ReturnErrorOnFailure(writer.Put(AnonymousTag, certElements.ProductIds[i]));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerContainer2));

    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_ServerCategoryId), certElements.ServerCategoryId));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_ClientCategoryId), certElements.ClientCategoryId));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SecurityLevel), certElements.SecurityLevel));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SecurityInformation), certElements.SecurityInformation));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_VersionNumber), certElements.VersionNumber));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_CertificationType), certElements.CertificationType));

    ReturnErrorOnFailure(writer.EndContainer(outerContainer1));

    ReturnErrorOnFailure(writer.Finalize());

    encodedCertElements.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeCertificationElements(const ByteSpan & encodedCertElements, CertificationElements & certElements)
{
    TLVReader reader;
    TLVType outerContainer1, outerContainer2;
    uint64_t element;

    reader.Init(encodedCertElements);

    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag));

    ReturnErrorOnFailure(reader.EnterContainer(outerContainer1));

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_VendorId)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.VendorId = static_cast<uint16_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_Array, ContextTag(kTag_ProductIds)));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer2));

    certElements.ProductIdsCount = 0;
    while (reader.Next(kTLVType_UnsignedInteger, AnonymousTag) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(element));
        VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
        certElements.ProductIds[certElements.ProductIdsCount++] = static_cast<uint16_t>(element);
    }
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    ReturnErrorOnFailure(reader.ExitContainer(outerContainer2));

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_ServerCategoryId)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.ServerCategoryId = static_cast<uint16_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_ClientCategoryId)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.ClientCategoryId = static_cast<uint16_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SecurityLevel)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint8_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.SecurityLevel = static_cast<uint8_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SecurityInformation)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.SecurityInformation = static_cast<uint16_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_VersionNumber)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint16_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.VersionNumber = static_cast<uint16_t>(element);

    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_CertificationType)));
    ReturnErrorOnFailure(reader.Get(element));
    VerifyOrReturnError(CanCastTo<uint8_t>(element), CHIP_ERROR_INVALID_TLV_ELEMENT);
    certElements.CertificationType = static_cast<uint8_t>(element);

    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    ReturnErrorOnFailure(reader.ExitContainer(outerContainer1));

    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

namespace {

CHIP_ERROR EncodeEncapsulatedContent(const ByteSpan & cdContent, ASN1Writer & writer)
{
    /**
     * EncapsulatedContentInfo ::= SEQUENCE {
     *   eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1),
     *   eContent [0] EXPLICIT OCTET STRING cd_content }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        // eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1)
        ReturnErrorOnFailure(writer.PutObjectId(sOID_ContentType_PKCS7Data, sizeof(sOID_ContentType_PKCS7Data)));

        // eContent [0] EXPLICIT OCTET STRING cd_content
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // OCTET STRING cd_content
            ReturnErrorOnFailure(writer.PutOctetString(cdContent.data(), static_cast<uint16_t>(cdContent.size())));
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR DecodeEncapsulatedContent(ASN1Reader & reader, ByteSpan & cdContent)
{
    /**
     * EncapsulatedContentInfo ::= SEQUENCE {
     *   eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1),
     *   eContent [0] EXPLICIT OCTET STRING cd_content }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_PARSE_ENTER_SEQUENCE
    {
        // eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1)
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        VerifyOrReturnError(ByteSpan(reader.GetValue(), reader.GetValueLen()).data_equal(ByteSpan(sOID_ContentType_PKCS7Data)),
                            ASN1_ERROR_UNSUPPORTED_ENCODING);

        // eContent [0] EXPLICIT OCTET STRING cd_content
        ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // OCTET STRING cd_content
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
            cdContent = ByteSpan(reader.GetValue(), reader.GetValueLen());
        }
        ASN1_EXIT_CONSTRUCTED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR EncodeSignerInfo(const ByteSpan & signerKeyId, const P256ECDSASignature & signature, ASN1Writer & writer)
{
    /**
     * SignerInfo ::= SEQUENCE {
     *   version INTEGER ( v3(3) ),
     *   subjectKeyIdentifier OCTET STRING,
     *   digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1),
     *   signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2),
     *   signature OCTET STRING }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SET
    {
        ASN1_START_SEQUENCE
        {
            // version INTEGER ( v3(3) )
            ASN1_ENCODE_INTEGER(3);

            // subjectKeyIdentifier OCTET STRING
            ReturnErrorOnFailure(writer.PutOctetString(kASN1TagClass_ContextSpecific, 0, signerKeyId.data(),
                                                       static_cast<uint16_t>(signerKeyId.size())));

            // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
            ASN1_START_SEQUENCE
            {
                ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA256, sizeof(sOID_DigestAlgo_SHA256)));
            }
            ASN1_END_SEQUENCE;

            // signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2)
            ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256); }
            ASN1_END_SEQUENCE;

            uint8_t asn1SignatureBuf[kMax_ECDSA_Signature_Length_Der];
            MutableByteSpan asn1Signature(asn1SignatureBuf);
            ReturnErrorOnFailure(EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan(signature, signature.Length()), asn1Signature));

            // signature OCTET STRING
            ReturnErrorOnFailure(writer.PutOctetString(asn1Signature.data(), static_cast<uint16_t>(asn1Signature.size())));
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_SET;

exit:
    return err;
}

CHIP_ERROR DecodeSignerInfo(ASN1Reader & reader, ByteSpan & signerKeyId, P256ECDSASignature & signature)
{
    /**
     * SignerInfo ::= SEQUENCE {
     *   version INTEGER ( v3(3) ),
     *   subjectKeyIdentifier OCTET STRING,
     *   digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1),
     *   signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2),
     *   signature OCTET STRING }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_PARSE_ENTER_SET
    {
        ASN1_PARSE_ENTER_SEQUENCE
        {
            // version INTEGER ( v3(3) )
            {
                int64_t version;
                ASN1_PARSE_INTEGER(version);

                // Verify that the CMS version is v3
                VerifyOrExit(version == 3, err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            // subjectKeyIdentifier OCTET STRING
            ASN1_PARSE_ELEMENT(kASN1TagClass_ContextSpecific, 0);
            signerKeyId = ByteSpan(reader.GetValue(), reader.GetValueLen());

            // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
                VerifyOrReturnError(ByteSpan(reader.GetValue(), reader.GetValueLen()).data_equal(ByteSpan(sOID_DigestAlgo_SHA256)),
                                    ASN1_ERROR_UNSUPPORTED_ENCODING);
            }
            ASN1_EXIT_SEQUENCE;

            // signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2)
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
                VerifyOrReturnError(
                    ByteSpan(reader.GetValue(), reader.GetValueLen()).data_equal(ByteSpan(sOID_SigAlgo_ECDSAWithSHA256)),
                    ASN1_ERROR_UNSUPPORTED_ENCODING);
            }
            ASN1_EXIT_SEQUENCE;

            // signature OCTET STRING
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);

            MutableByteSpan signatureSpan(signature, signature.Capacity());
            ReturnErrorOnFailure(
                EcdsaAsn1SignatureToRaw(kP256_FE_Length, ByteSpan(reader.GetValue(), reader.GetValueLen()), signatureSpan));
            ReturnErrorOnFailure(signature.SetLength(signatureSpan.size()));
        }
        ASN1_EXIT_SEQUENCE;
    }
    ASN1_EXIT_SET;

exit:
    return err;
}

} // namespace

CHIP_ERROR CMS_Sign(const ByteSpan & cdContent, const ByteSpan & signerKeyId, Crypto::P256Keypair & signerKeypair,
                    MutableByteSpan & signedMessage)
{
    /**
     * CertificationDeclaration ::= SEQUENCE {
     *   version INTEGER ( v3(3) ),
     *   digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1),
     *   encapContentInfo EncapsulatedContentInfo,
     *   signerInfo SignerInfo }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Writer writer;
    uint32_t size = static_cast<uint32_t>(std::min(static_cast<size_t>(UINT32_MAX), signedMessage.size()));

    writer.Init(signedMessage.data(), size);

    ASN1_START_SEQUENCE
    {
        // OID identifies the CMS signed-data content type
        ReturnErrorOnFailure(writer.PutObjectId(sOID_ContentType_PKCS7SignedData, sizeof(sOID_ContentType_PKCS7SignedData)));

        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_START_SEQUENCE
            {
                // version INTEGER ( v3(3) )
                ASN1_ENCODE_INTEGER(3);

                // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
                ASN1_START_SET
                {
                    ASN1_START_SEQUENCE
                    {
                        ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA256, sizeof(sOID_DigestAlgo_SHA256)));
                    }
                    ASN1_END_SEQUENCE;
                }
                ASN1_END_SET;

                // encapContentInfo EncapsulatedContentInfo
                ReturnErrorOnFailure(EncodeEncapsulatedContent(cdContent, writer));

                Crypto::P256ECDSASignature signature;
                ReturnErrorOnFailure(signerKeypair.ECDSA_sign_msg(cdContent.data(), cdContent.size(), signature));

                // signerInfo SignerInfo
                ReturnErrorOnFailure(EncodeSignerInfo(signerKeyId, signature, writer));
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;

    signedMessage.reduce_size(writer.GetLengthWritten());

exit:
    return err;
}

CHIP_ERROR CMS_Verify(const ByteSpan & signedMessage, const ByteSpan & signerX509Cert, ByteSpan & cdContent)
{
    P256PublicKey signerPubkey;

    ReturnErrorOnFailure(ExtractPubkeyFromX509Cert(signerX509Cert, signerPubkey));

    return CMS_Verify(signedMessage, signerPubkey, cdContent);
}

CHIP_ERROR CMS_Verify(const ByteSpan & signedMessage, const Crypto::P256PublicKey & signerPubkey, ByteSpan & cdContent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Reader reader;
    uint32_t size = signedMessage.size() > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(signedMessage.size());

    reader.Init(signedMessage.data(), size);

    // SignedData ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);

        // id-signedData OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs7(7) 2 }
        // OID identifies the CMS signed-data content type
        VerifyOrReturnError(
            ByteSpan(reader.GetValue(), reader.GetValueLen()).data_equal(ByteSpan(sOID_ContentType_PKCS7SignedData)),
            ASN1_ERROR_UNSUPPORTED_ENCODING);

        // version [0] EXPLICIT Version DEFAULT v3
        ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                // Version ::= INTEGER { v3(3) }
                int64_t version;
                ASN1_PARSE_INTEGER(version);

                // Verify that the CMS version is v3
                VerifyOrExit(version == 3, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

                // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
                ASN1_PARSE_ENTER_SET
                {
                    ASN1_PARSE_ENTER_SEQUENCE
                    {
                        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
                        VerifyOrReturnError(
                            ByteSpan(reader.GetValue(), reader.GetValueLen()).data_equal(ByteSpan(sOID_DigestAlgo_SHA256)),
                            ASN1_ERROR_UNSUPPORTED_ENCODING);
                    }
                    ASN1_EXIT_SEQUENCE;
                }
                ASN1_EXIT_SET;

                // encapContentInfo EncapsulatedContentInfo
                ReturnErrorOnFailure(DecodeEncapsulatedContent(reader, cdContent));

                // signerInfo SignerInfo
                ByteSpan signerKeyId;
                P256ECDSASignature signature;
                ReturnErrorOnFailure(DecodeSignerInfo(reader, signerKeyId, signature));

                // Validate CD Signature
                ReturnErrorOnFailure(signerPubkey.ECDSA_validate_msg_signature(cdContent.data(), cdContent.size(), signature));
            }
            ASN1_EXIT_SEQUENCE;
        }
        ASN1_EXIT_CONSTRUCTED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR CMS_ExtractKeyId(const ByteSpan & signedMessage, ByteSpan & signerKeyId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Reader reader;
    uint32_t size = signedMessage.size() > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(signedMessage.size());

    reader.Init(signedMessage.data(), size);

    // SignedData ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        // id-signedData OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs7(7) 2 }
        // OID identifies the CMS signed-data content type
        ASN1_PARSE_ANY;

        // version [0] EXPLICIT Version DEFAULT v3
        ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                // Version ::= INTEGER { v3(3) }
                ASN1_PARSE_ANY;

                // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
                ASN1_PARSE_ANY;

                // encapContentInfo EncapsulatedContentInfo
                ASN1_PARSE_ANY;

                // signerInfo SignerInfo
                P256ECDSASignature signature;
                ReturnErrorOnFailure(DecodeSignerInfo(reader, signerKeyId, signature));
            }
            ASN1_EXIT_SEQUENCE;
        }
        ASN1_EXIT_CONSTRUCTED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR CMS_ExtractCDContent(const ByteSpan & signedMessage, ByteSpan & cdContent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Reader reader;
    uint32_t size = signedMessage.size() > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(signedMessage.size());

    reader.Init(signedMessage.data(), size);

    // SignedData ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        // id-signedData OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs7(7) 2 }
        // OID identifies the CMS signed-data content type
        ASN1_PARSE_ANY;

        // version [0] EXPLICIT Version DEFAULT v3
        ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                // Version ::= INTEGER { v3(3) }
                ASN1_PARSE_ANY;

                // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
                ASN1_PARSE_ANY;

                // encapContentInfo EncapsulatedContentInfo
                ReturnErrorOnFailure(DecodeEncapsulatedContent(reader, cdContent));

                // signerInfo SignerInfo
                ASN1_PARSE_ANY;
            }
            ASN1_EXIT_SEQUENCE;
        }
        ASN1_EXIT_CONSTRUCTED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

} // namespace Credentials
} // namespace chip
