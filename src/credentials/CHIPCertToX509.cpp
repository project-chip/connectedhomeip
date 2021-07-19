/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements methods for converting a CHIP
 *      TLV-encoded certificate to a standard X.509 certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <stddef.h>

#include <asn1/ASN1.h>
#include <asn1/ASN1Macros.h>
#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/SafeInt.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Protocols;
using namespace chip::Crypto;

static CHIP_ERROR DecodeConvertDN(TLVReader & reader, ASN1Writer & writer, ChipDN & dn)
{
    CHIP_ERROR err;
    TLVType outerContainer;
    TLVType elemType;
    uint64_t tlvTag;
    uint32_t tlvTagNum;
    OID attrOID;
    uint32_t asn1Tag;
    const uint8_t * asn1AttrVal;
    uint32_t asn1AttrValLen;
    uint8_t chipAttrStr[17];

    // Enter the List TLV element that represents the DN in TLV format.
    err = reader.EnterContainer(outerContainer);
    SuccessOrExit(err);

    // RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
    ASN1_START_SEQUENCE
    {
        // Read the RDN attributes in the List.
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            // RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
            ASN1_START_SET
            {
                // Get the TLV tag, make sure it is a context tag and extract the context tag number.
                tlvTag = reader.GetTag();
                VerifyOrExit(IsContextTag(tlvTag), err = CHIP_ERROR_INVALID_TLV_TAG);
                tlvTagNum = TagNumFromTag(tlvTag);

                // Get the element type.
                elemType = reader.GetType();

                // Derive the OID of the corresponding ASN.1 attribute from the TLV tag number.
                // The numeric value of the OID is encoded in the bottom 7 bits of the TLV tag number.
                // This eliminates the need for a translation table/switch statement but has the
                // effect of tying the two encodings together.
                //
                // NOTE: In the event that the computed OID value is not one that we recognize
                // (specifically, is not in the table of OIDs defined in ASN1OID.h) then the
                // macro call below that encodes the attribute's object id (ASN1_ENCODE_OBJECT_ID)
                // will fail for lack of the OID's encoded representation.  Given this there's no
                // need to test the validity of the OID here.
                //
                attrOID = GetOID(kOIDCategory_AttributeType, static_cast<uint8_t>(tlvTagNum & 0x7f));

                // If the attribute is one of the CHIP-defined DN attributes.
                if (IsChipDNAttr(attrOID))
                {
                    // Verify that the underlying TLV data type is unsigned integer.
                    VerifyOrExit(elemType == kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

                    // Read the value of the CHIP attribute.
                    uint64_t chipAttr;
                    err = reader.Get(chipAttr);
                    SuccessOrExit(err);

                    // Generate the string representation of the CHIP attribute that will appear in the ASN.1 attribute.
                    if (IsChip64bitDNAttr(attrOID))
                    {
                        // For CHIP 64-bit attribute the string representation is 16 uppercase hex characters.
                        snprintf(reinterpret_cast<char *>(chipAttrStr), sizeof(chipAttrStr), ChipLogFormatX64,
                                 ChipLogValueX64(chipAttr));
                        asn1AttrVal    = chipAttrStr;
                        asn1AttrValLen = 16;
                    }
                    else
                    {
                        VerifyOrExit(chipAttr <= UINT32_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

                        // For CHIP 32-bit attribute the string representation is 8 uppercase hex characters.
                        snprintf(reinterpret_cast<char *>(chipAttrStr), sizeof(chipAttrStr), "%08" PRIX32,
                                 static_cast<uint32_t>(chipAttr));
                        asn1AttrVal    = chipAttrStr;
                        asn1AttrValLen = 8;
                    }

                    // The ASN.1 tag for CHIP id attributes is always UTF8String.
                    asn1Tag = kASN1UniversalTag_UTF8String;

                    // Save the CHIP-specific id value in the caller's DN structure.
                    err = dn.AddAttribute(attrOID, chipAttr);
                    SuccessOrExit(err);
                }

                // Otherwise the attribute is one of the supported X.509 attributes
                else
                {
                    // Verify that the underlying data type is UTF8 string.
                    VerifyOrExit(elemType == kTLVType_UTF8String, err = CHIP_ERROR_WRONG_TLV_TYPE);

                    // Get a pointer the underlying string data, plus its length.
                    err = reader.GetDataPtr(asn1AttrVal);
                    SuccessOrExit(err);
                    asn1AttrValLen = reader.GetLength();

                    // Determine the appropriate ASN.1 tag for the DN attribute.
                    // - DomainComponent is always an IA5String.
                    // - For all other ASN.1 defined attributes, bit 0x80 in the TLV tag value conveys whether the attribute
                    //   is a UTF8String or a PrintableString (in some cases the certificate generator has a choice).
                    if (attrOID == kOID_AttributeType_DomainComponent)
                    {
                        asn1Tag = kASN1UniversalTag_IA5String;
                    }
                    else
                    {
                        asn1Tag = (tlvTagNum & 0x80) ? kASN1UniversalTag_PrintableString : kASN1UniversalTag_UTF8String;
                    }

                    // Save the string value in the caller's DN structure.
                    err = dn.AddAttribute(attrOID, ByteSpan(asn1AttrVal, asn1AttrValLen));
                    SuccessOrExit(err);
                }

                // AttributeTypeAndValue ::= SEQUENCE
                ASN1_START_SEQUENCE
                {
                    // type AttributeType
                    // AttributeType ::= OBJECT IDENTIFIER
                    ASN1_ENCODE_OBJECT_ID(attrOID);

                    // value AttributeValue
                    // AttributeValue ::= ANY -- DEFINED BY AttributeType
                    err = writer.PutString(asn1Tag, Uint8::to_const_char(asn1AttrVal), static_cast<uint16_t>(asn1AttrValLen));
                    SuccessOrExit(err);
                }
                ASN1_END_SEQUENCE;
            }
            ASN1_END_SET;
        }
    }
    ASN1_END_SEQUENCE;

    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);

    err = reader.ExitContainer(outerContainer);
    SuccessOrExit(err);

exit:
    return err;
}

static CHIP_ERROR DecodeConvertValidity(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    ASN1UniversalTime asn1Time;
    uint64_t chipEpochTime;

    ASN1_START_SEQUENCE
    {
        err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_NotBefore));
        SuccessOrExit(err);

        err = reader.Get(chipEpochTime);
        SuccessOrExit(err);

        VerifyOrExit(chipEpochTime <= UINT32_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
        certData.mNotBeforeTime = static_cast<uint32_t>(chipEpochTime);

        err = ChipEpochToASN1Time(static_cast<uint32_t>(chipEpochTime), asn1Time);
        SuccessOrExit(err);

        ASN1_ENCODE_TIME(asn1Time);

        err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_NotAfter));
        SuccessOrExit(err);

        err = reader.Get(chipEpochTime);
        SuccessOrExit(err);

        VerifyOrExit(chipEpochTime <= UINT32_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
        certData.mNotAfterTime = static_cast<uint32_t>(chipEpochTime);

        err = ChipEpochToASN1Time(static_cast<uint32_t>(chipEpochTime), asn1Time);
        SuccessOrExit(err);

        ASN1_ENCODE_TIME(asn1Time);
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertSubjectPublicKeyInfo(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    uint64_t pubKeyAlgoId, pubKeyCurveId;
    OID pubKeyAlgoOID;

    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_PublicKeyAlgorithm));
    SuccessOrExit(err);
    err = reader.Get(pubKeyAlgoId);
    SuccessOrExit(err);
    VerifyOrExit(pubKeyAlgoId <= UINT8_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    pubKeyAlgoOID           = GetOID(kOIDCategory_PubKeyAlgo, static_cast<uint8_t>(pubKeyAlgoId));
    certData.mPubKeyAlgoOID = pubKeyAlgoOID;

    VerifyOrExit(pubKeyAlgoOID == kOID_PubKeyAlgo_ECPublicKey, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_EllipticCurveIdentifier));
    SuccessOrExit(err);
    err = reader.Get(pubKeyCurveId);
    SuccessOrExit(err);
    VerifyOrExit(pubKeyCurveId <= UINT8_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    certData.mPubKeyCurveOID = GetOID(kOIDCategory_EllipticCurve, static_cast<uint8_t>(pubKeyCurveId));

    // subjectPublicKeyInfo SubjectPublicKeyInfo,
    ASN1_START_SEQUENCE
    {
        // algorithm AlgorithmIdentifier,
        // AlgorithmIdentifier ::= SEQUENCE
        ASN1_START_SEQUENCE
        {
            // algorithm OBJECT IDENTIFIER,
            ASN1_ENCODE_OBJECT_ID(pubKeyAlgoOID);

            // EcpkParameters ::= CHOICE {
            //     ecParameters  ECParameters,
            //     namedCurve    OBJECT IDENTIFIER,
            //     implicitlyCA  NULL }
            //
            // (Only namedCurve supported).
            //
            ASN1_ENCODE_OBJECT_ID(certData.mPubKeyCurveOID);
        }
        ASN1_END_SEQUENCE;

        ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, ContextTag(kTag_EllipticCurvePublicKey)));
        VerifyOrReturnError(reader.GetLength() == certData.mPublicKey.size(), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        const uint8_t * ptr;
        ReturnErrorOnFailure(reader.GetDataPtr(ptr));
        certData.mPublicKey = P256PublicKeySpan(ptr);

        static_assert(P256PublicKeySpan().size() <= UINT16_MAX, "Public key size doesn't fit in a uint16_t");

        // For EC certs, the subjectPublicKey BIT STRING contains the X9.62 encoded EC point.
        ReturnErrorOnFailure(writer.PutBitString(0, certData.mPublicKey.data(), static_cast<uint16_t>(certData.mPublicKey.size())));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertAuthorityKeyIdentifierExtension(TLVReader & reader, ASN1Writer & writer,
                                                               ChipCertificateData & certData)
{
    CHIP_ERROR err;

    certData.mCertFlags.Set(CertFlags::kExtPresent_AuthKeyId);

    // AuthorityKeyIdentifier extension MUST be marked as non-critical (default).

    // AuthorityKeyIdentifier ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // keyIdentifier [0] IMPLICIT KeyIdentifier
        // KeyIdentifier ::= OCTET STRING
        VerifyOrReturnError(reader.GetType() == kTLVType_ByteString, CHIP_ERROR_WRONG_TLV_TYPE);
        VerifyOrReturnError(reader.GetTag() == ContextTag(kTag_AuthorityKeyIdentifier), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        VerifyOrReturnError(reader.GetLength() == certData.mAuthKeyId.size(), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        const uint8_t * ptr;
        ReturnErrorOnFailure(reader.GetDataPtr(ptr));
        certData.mAuthKeyId = CertificateKeyId(ptr);

        static_assert(CertificateKeyId().size() <= UINT16_MAX, "Authority key id size doesn't fit in a uint16_t");

        ReturnErrorOnFailure(writer.PutOctetString(kASN1TagClass_ContextSpecific, 0, certData.mAuthKeyId.data(),
                                                   static_cast<uint16_t>(certData.mAuthKeyId.size())));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertSubjectKeyIdentifierExtension(TLVReader & reader, ASN1Writer & writer,
                                                             ChipCertificateData & certData)
{
    certData.mCertFlags.Set(CertFlags::kExtPresent_SubjectKeyId);

    // SubjectKeyIdentifier extension MUST be marked as non-critical (default).

    // SubjectKeyIdentifier ::= KeyIdentifier
    // KeyIdentifier ::= OCTET STRING
    VerifyOrReturnError(reader.GetType() == kTLVType_ByteString, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == ContextTag(kTag_SubjectKeyIdentifier), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    VerifyOrReturnError(reader.GetLength() == certData.mSubjectKeyId.size(), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    const uint8_t * ptr;
    ReturnErrorOnFailure(reader.GetDataPtr(ptr));
    certData.mSubjectKeyId = CertificateKeyId(ptr);

    static_assert(CertificateKeyId().size() <= UINT16_MAX, "Subject key id size doesn't fit in a uint16_t");

    ReturnErrorOnFailure(
        writer.PutOctetString(certData.mSubjectKeyId.data(), static_cast<uint16_t>(certData.mSubjectKeyId.size())));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR DecodeConvertKeyUsageExtension(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    uint64_t keyUsageBits;

    certData.mCertFlags.Set(CertFlags::kExtPresent_KeyUsage);

    // KeyUsage ::= BIT STRING
    VerifyOrExit(reader.GetTag() == ContextTag(kTag_KeyUsage), err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    VerifyOrExit(reader.GetType() == kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = reader.Get(keyUsageBits);
    SuccessOrExit(err);

    VerifyOrExit(keyUsageBits <= UINT16_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    {
        BitFlags<KeyUsageFlags> keyUsageFlags(static_cast<uint16_t>(keyUsageBits));
        VerifyOrExit(keyUsageFlags.HasOnly(KeyUsageFlags::kDigitalSignature, KeyUsageFlags::kNonRepudiation,
                                           KeyUsageFlags::kKeyEncipherment, KeyUsageFlags::kDataEncipherment,
                                           KeyUsageFlags::kKeyAgreement, KeyUsageFlags::kKeyCertSign, KeyUsageFlags::kCRLSign,
                                           KeyUsageFlags::kEncipherOnly, KeyUsageFlags::kEncipherOnly),
                     err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        ASN1_ENCODE_BIT_STRING(static_cast<uint16_t>(keyUsageBits));

        certData.mKeyUsageFlags = keyUsageFlags;
    }

exit:
    return err;
}

static CHIP_ERROR DecodeConvertBasicConstraintsExtension(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err, nextRes;
    TLVType outerContainer;

    certData.mCertFlags.Set(CertFlags::kExtPresent_BasicConstraints);

    // BasicConstraints ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        VerifyOrExit(reader.GetTag() == ContextTag(kTag_BasicConstraints), err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        VerifyOrExit(reader.GetType() == kTLVType_Structure, err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = reader.EnterContainer(outerContainer);
        SuccessOrExit(err);

        // cA BOOLEAN DEFAULT FALSE
        {
            bool isCA;

            err = reader.Next(kTLVType_Boolean, ContextTag(kTag_BasicConstraints_IsCA));
            SuccessOrExit(err);

            err = reader.Get(isCA);
            SuccessOrExit(err);

            if (isCA)
            {
                ASN1_ENCODE_BOOLEAN(true);
                certData.mCertFlags.Set(CertFlags::kIsCA);
            }

            nextRes = reader.Next();
            VerifyOrExit(nextRes == CHIP_NO_ERROR || nextRes == CHIP_END_OF_TLV, err = nextRes);
        }

        // pathLenConstraint INTEGER (0..MAX) OPTIONAL
        if (reader.GetTag() == ContextTag(kTag_BasicConstraints_PathLenConstraint))
        {
            uint64_t pathLenConstraint;

            VerifyOrExit(reader.GetType() == kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

            err = reader.Get(pathLenConstraint);
            SuccessOrExit(err);

            VerifyOrExit(pathLenConstraint <= UINT8_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

            ASN1_ENCODE_INTEGER(static_cast<int64_t>(pathLenConstraint));

            certData.mPathLenConstraint = static_cast<uint8_t>(pathLenConstraint);

            certData.mCertFlags.Set(CertFlags::kPathLenConstraintPresent);

            reader.Next();
        }

        err = reader.VerifyEndOfContainer();
        SuccessOrExit(err);

        err = reader.ExitContainer(outerContainer);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertExtendedKeyUsageExtension(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err, nextRes;
    TLVType outerContainer;

    certData.mCertFlags.Set(CertFlags::kExtPresent_ExtendedKeyUsage);

    // ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
    ASN1_START_SEQUENCE
    {
        VerifyOrExit(reader.GetTag() == ContextTag(kTag_ExtendedKeyUsage), err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        VerifyOrExit(reader.GetType() == kTLVType_Array, err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = reader.EnterContainer(outerContainer);
        SuccessOrExit(err);

        while ((nextRes = reader.Next(kTLVType_UnsignedInteger, AnonymousTag)) == CHIP_NO_ERROR)
        {
            uint64_t keyPurposeId;
            OID keyPurposeOID;

            err = reader.Get(keyPurposeId);
            SuccessOrExit(err);

            VerifyOrExit(keyPurposeId <= UINT8_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

            keyPurposeOID = GetOID(kOIDCategory_KeyPurpose, static_cast<uint8_t>(keyPurposeId));

            // KeyPurposeId ::= OBJECT IDENTIFIER
            ASN1_ENCODE_OBJECT_ID(keyPurposeOID);

            certData.mKeyPurposeFlags.Set(static_cast<KeyPurposeFlags>(0x01 << (keyPurposeId - 1)));
        }

        VerifyOrExit(nextRes == CHIP_END_OF_TLV, err = nextRes);

        err = reader.ExitContainer(outerContainer);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertFutureExtension(TLVReader & tlvReader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    const uint8_t * extensionSequence;
    uint32_t extensionSequenceLen;
    ASN1Reader reader;

    VerifyOrExit(tlvReader.GetTag() == ContextTag(kTag_FutureExtension), err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tlvReader.GetType() == kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = tlvReader.GetDataPtr(extensionSequence);
    SuccessOrExit(err);

    extensionSequenceLen = tlvReader.GetLength();

    reader.Init(extensionSequence, extensionSequenceLen);

    // Extension ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        OID extensionOID;
        bool critical = false;

        ASN1_PARSE_OBJECT_ID(extensionOID);

        VerifyOrExit(extensionOID == kOID_Unknown, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

        // critical BOOLEAN DEFAULT FALSE,
        ASN1_PARSE_ANY;
        if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_Boolean)
        {
            ASN1_GET_BOOLEAN(critical);

            if (critical)
            {
                certData.mCertFlags.Set(CertFlags::kExtPresent_FutureIsCritical);
            }

            ASN1_PARSE_ANY;
        }
    }
    ASN1_EXIT_SEQUENCE;

    VerifyOrExit(extensionSequenceLen <= UINT16_MAX, err = ASN1_ERROR_INVALID_ENCODING);

    // FutureExtension SEQUENCE
    err = writer.PutConstructedType(extensionSequence, static_cast<uint16_t>(extensionSequenceLen));
    SuccessOrExit(err);

exit:
    return err;
}

static CHIP_ERROR DecodeConvertExtension(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    uint64_t extensionTagNum = TagNumFromTag(reader.GetTag());
    OID extensionOID;

    if (extensionTagNum == kTag_FutureExtension)
    {
        err = DecodeConvertFutureExtension(reader, writer, certData);
        SuccessOrExit(err);
    }
    else
    {
        // Extension ::= SEQUENCE
        ASN1_START_SEQUENCE
        {
            extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(extensionTagNum));

            // extnID OBJECT IDENTIFIER,
            ASN1_ENCODE_OBJECT_ID(extensionOID);

            // BasicConstraints, KeyUsage and ExtKeyUsage extensions MUST be marked as critical.
            if (extensionTagNum == kTag_KeyUsage || extensionTagNum == kTag_BasicConstraints ||
                extensionTagNum == kTag_ExtendedKeyUsage)
            {
                ASN1_ENCODE_BOOLEAN(true);
            }

            // extnValue OCTET STRING
            //           -- contains the DER encoding of an ASN.1 value
            //           -- corresponding to the extension type identified
            //           -- by extnID
            ASN1_START_OCTET_STRING_ENCAPSULATED
            {
                if (extensionTagNum == kTag_AuthorityKeyIdentifier)
                {
                    err = DecodeConvertAuthorityKeyIdentifierExtension(reader, writer, certData);
                }
                else if (extensionTagNum == kTag_SubjectKeyIdentifier)
                {
                    err = DecodeConvertSubjectKeyIdentifierExtension(reader, writer, certData);
                }
                else if (extensionTagNum == kTag_KeyUsage)
                {
                    err = DecodeConvertKeyUsageExtension(reader, writer, certData);
                }
                else if (extensionTagNum == kTag_BasicConstraints)
                {
                    err = DecodeConvertBasicConstraintsExtension(reader, writer, certData);
                }
                else if (extensionTagNum == kTag_ExtendedKeyUsage)
                {
                    err = DecodeConvertExtendedKeyUsageExtension(reader, writer, certData);
                }
                else
                {
                    err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT;
                }
                SuccessOrExit(err);
            }
            ASN1_END_ENCAPSULATED;
        }
        ASN1_END_SEQUENCE;
    }

exit:
    return err;
}

static CHIP_ERROR DecodeConvertExtensions(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    TLVType outerContainer;

    err = reader.Next(kTLVType_List, ContextTag(kTag_Extensions));
    SuccessOrExit(err);

    err = reader.EnterContainer(outerContainer);
    SuccessOrExit(err);

    // extensions [3] EXPLICIT Extensions OPTIONAL
    ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 3)
    {
        // Extensions ::= SEQUENCE SIZE (1..MAX) OF Extension
        ASN1_START_SEQUENCE
        {
            // Read certificate extension in the List.
            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                err = DecodeConvertExtension(reader, writer, certData);
                SuccessOrExit(err);
            }
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_CONSTRUCTED;

    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);

    err = reader.ExitContainer(outerContainer);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR DecodeECDSASignature(TLVReader & reader, ChipCertificateData & certData)
{
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, ContextTag(kTag_ECDSASignature)));

    VerifyOrReturnError(reader.GetLength() == certData.mSignature.size(), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    const uint8_t * ptr;
    ReturnErrorOnFailure(reader.GetDataPtr(ptr));
    certData.mSignature = P256ECDSASignatureSpan(ptr);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR DecodeConvertECDSASignature(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(DecodeECDSASignature(reader, certData));

    // signatureValue BIT STRING
    // Per RFC3279, the ECDSA signature value is encoded in DER encapsulated in the signatureValue BIT STRING.
    ASN1_START_BIT_STRING_ENCAPSULATED { ReturnErrorOnFailure(ConvertECDSASignatureRawToDER(certData.mSignature, writer)); }
    ASN1_END_ENCAPSULATED;

exit:
    return err;
}

/**
 * @brief Decode and convert the To-Be-Signed (TBS) portion of the CHIP certificate
 *        into X.509 DER encoded form.
 *
 * @param reader    A TLVReader positioned at the beginning of the TBS portion
 *                  (certificate serial number) of the CHIP certificates.
 * @param writer    A reference to the ASN1Writer to store DER encoded TBS portion of
 *                  the CHIP certificate.
 * @param certData  Structure containing data extracted from the TBS portion of the
 *                  CHIP certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeConvertTBSCert(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;

    // tbsCertificate TBSCertificate,
    // TBSCertificate ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // version [0] EXPLICIT Version DEFAULT v1
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // Version ::= INTEGER { v1(0), v2(1), v3(2) }
            ASN1_ENCODE_INTEGER(2);
        }
        ASN1_END_CONSTRUCTED;

        err = reader.Next(kTLVType_ByteString, ContextTag(kTag_SerialNumber));
        SuccessOrExit(err);

        // serialNumber CertificateSerialNumber
        // CertificateSerialNumber ::= INTEGER
        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, reader);
        SuccessOrExit(err);

        // signature AlgorithmIdentifier
        // AlgorithmIdentifier ::= SEQUENCE
        ASN1_START_SEQUENCE
        {
            uint64_t sigAlgoId;
            OID sigAlgoOID;

            err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SignatureAlgorithm));
            SuccessOrExit(err);

            err = reader.Get(sigAlgoId);
            SuccessOrExit(err);

            VerifyOrExit(sigAlgoId <= UINT8_MAX, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

            sigAlgoOID = GetOID(kOIDCategory_SigAlgo, static_cast<uint8_t>(sigAlgoId));
            ASN1_ENCODE_OBJECT_ID(sigAlgoOID);

            certData.mSigAlgoOID = sigAlgoOID;
        }
        ASN1_END_SEQUENCE;

        // issuer Name
        err = reader.Next(kTLVType_List, ContextTag(kTag_Issuer));
        SuccessOrExit(err);
        err = DecodeConvertDN(reader, writer, certData.mIssuerDN);
        SuccessOrExit(err);

        // validity Validity,
        err = DecodeConvertValidity(reader, writer, certData);
        SuccessOrExit(err);

        // subject Name
        err = reader.Next(kTLVType_List, ContextTag(kTag_Subject));
        SuccessOrExit(err);
        err = DecodeConvertDN(reader, writer, certData.mSubjectDN);
        SuccessOrExit(err);

        // subjectPublicKeyInfo SubjectPublicKeyInfo,
        err = DecodeConvertSubjectPublicKeyInfo(reader, writer, certData);
        SuccessOrExit(err);

        // certificate extensions
        err = DecodeConvertExtensions(reader, writer, certData);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR DecodeConvertCert(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData)
{
    CHIP_ERROR err;
    uint64_t tag;
    TLVType containerType;

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        err = reader.Next();
        SuccessOrExit(err);
    }
    VerifyOrExit(reader.GetType() == kTLVType_Structure, err = CHIP_ERROR_WRONG_TLV_TYPE);
    tag = reader.GetTag();
    VerifyOrExit(tag == AnonymousTag, err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    err = reader.EnterContainer(containerType);
    SuccessOrExit(err);

    // Certificate ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // tbsCertificate TBSCertificate,
        err = DecodeConvertTBSCert(reader, writer, certData);
        SuccessOrExit(err);

        // signatureAlgorithm   AlgorithmIdentifier
        // AlgorithmIdentifier ::= SEQUENCE
        ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(static_cast<OID>(certData.mSigAlgoOID)); }
        ASN1_END_SEQUENCE;

        // signatureValue BIT STRING
        err = DecodeConvertECDSASignature(reader, writer, certData);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

    // Verify no more elements in certificate.
    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);

    err = reader.ExitContainer(containerType);
    SuccessOrExit(err);

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR ConvertChipCertToX509Cert(const ByteSpan chipCert, uint8_t * x509CertBuf, uint32_t x509CertBufSize,
                                                uint32_t & x509CertLen)
{
    TLVReader reader;
    ASN1Writer writer;
    ChipCertificateData certData;

    VerifyOrReturnError(!chipCert.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint32_t>(chipCert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    reader.Init(chipCert.data(), static_cast<uint32_t>(chipCert.size()));

    writer.Init(x509CertBuf, x509CertBufSize);

    certData.Clear();

    ReturnErrorOnFailure(DecodeConvertCert(reader, writer, certData));

    x509CertLen = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeChipCert(const uint8_t * chipCert, uint32_t chipCertLen, ChipCertificateData & certData)
{
    TLVReader reader;

    reader.Init(chipCert, chipCertLen);

    return DecodeChipCert(reader, certData);
}

CHIP_ERROR DecodeChipCert(TLVReader & reader, ChipCertificateData & certData)
{
    ASN1Writer writer;

    writer.InitNullWriter();

    certData.Clear();

    return DecodeConvertCert(reader, writer, certData);
}

CHIP_ERROR DecodeChipDN(TLVReader & reader, ChipDN & dn)
{
    ASN1Writer writer;

    writer.InitNullWriter();

    dn.Clear();

    return DecodeConvertDN(reader, writer, dn);
}

} // namespace Credentials
} // namespace chip
