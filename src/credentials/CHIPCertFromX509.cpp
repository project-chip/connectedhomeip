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
 *      This file implements methods for converting a standard X.509
 *      certificate to a CHIP TLV-encoded certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stddef.h>

#include <asn1/ASN1.h>
#include <asn1/ASN1Macros.h>
#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <core/CHIPTLV.h>
#include <core/Optional.h>
#include <credentials/CHIPCert.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Protocols;
using namespace chip::Crypto;

static CHIP_ERROR ParseChipAttribute(ASN1Reader & reader, uint64_t & chipAttrOut)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    const uint8_t * value = reader.GetValue();
    uint32_t valueLen     = reader.GetValueLen();

    chipAttrOut = 0;

    VerifyOrExit(value != nullptr, err = ASN1_ERROR_INVALID_ENCODING);
    VerifyOrExit(valueLen == kChip32bitAttrUTF8Length || valueLen == kChip64bitAttrUTF8Length, err = ASN1_ERROR_INVALID_ENCODING);

    for (uint32_t i = 0; i < valueLen; i++)
    {
        chipAttrOut <<= 4;
        uint8_t ch = value[i];
        if (ch >= '0' && ch <= '9')
        {
            chipAttrOut |= (ch - '0');
        }
        // CHIP Id attribute encodings only support uppercase chars.
        else if (ch >= 'A' && ch <= 'F')
        {
            chipAttrOut |= (ch - 'A' + 10);
        }
        else
        {
            ExitNow(err = ASN1_ERROR_INVALID_ENCODING);
        }
    }

exit:
    return err;
}

static CHIP_ERROR ConvertDistinguishedName(ASN1Reader & reader, TLVWriter & writer, uint64_t tag, uint64_t & subjectOrIssuer,
                                           Optional<uint64_t> & fabric)
{
    CHIP_ERROR err;
    TLVType outerContainer;
    OID attrOID;

    err = writer.StartContainer(tag, kTLVType_List, outerContainer);
    SuccessOrExit(err);

    // RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
    ASN1_PARSE_ENTER_SEQUENCE
    {
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            // RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
            ASN1_ENTER_SET
            {
                // AttributeTypeAndValue ::= SEQUENCE
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    // type AttributeType
                    // AttributeType ::= OBJECT IDENTIFIER
                    ASN1_PARSE_OBJECT_ID(attrOID);
                    VerifyOrExit(GetOIDCategory(attrOID) == kOIDCategory_AttributeType, err = ASN1_ERROR_INVALID_ENCODING);

                    // AttributeValue ::= ANY -- DEFINED BY AttributeType
                    ASN1_PARSE_ANY;

                    // Can only support UTF8String, PrintableString and IA5String.
                    VerifyOrExit(reader.GetClass() == kASN1TagClass_Universal &&
                                     (reader.GetTag() == kASN1UniversalTag_PrintableString ||
                                      reader.GetTag() == kASN1UniversalTag_UTF8String ||
                                      reader.GetTag() == kASN1UniversalTag_IA5String),
                                 err = ASN1_ERROR_UNSUPPORTED_ENCODING);

                    // CHIP attributes must be UTF8Strings.
                    if (IsChipDNAttr(attrOID))
                    {
                        VerifyOrExit(reader.GetTag() == kASN1UniversalTag_UTF8String, err = ASN1_ERROR_INVALID_ENCODING);
                    }

                    // Derive the TLV tag number from the enum value assigned to the attribute type OID. For attributes that can be
                    // either UTF8String or PrintableString, use the high bit in the tag number to distinguish the two.
                    uint8_t tlvTagNum = GetOIDEnum(attrOID);
                    if (reader.GetTag() == kASN1UniversalTag_PrintableString)
                    {
                        tlvTagNum |= 0x80;
                    }

                    // If the attribute is a CHIP-defined attribute that contains a 64-bit or 32-bit value.
                    if (IsChipDNAttr(attrOID))
                    {
                        // Parse the attribute string into a CHIP attribute.
                        uint64_t chipAttr;
                        err = ParseChipAttribute(reader, chipAttr);
                        SuccessOrExit(err);

                        // Write the CHIP attribute value into the TLV.
                        err = writer.Put(ContextTag(tlvTagNum), chipAttr);
                        SuccessOrExit(err);

                        // Certificates use a combination of OIDs for Issuer and Subject.
                        // NOC: Issuer  = kOID_AttributeType_ChipRootId or kOID_AttributeType_ChipICAId
                        //      Subject = kOID_AttributeType_ChipNodeId
                        // ICA: Issuer  = kOID_AttributeType_ChipRootId
                        //      Subject = kOID_AttributeType_ChipICAId
                        // Root: Issuer = kOID_AttributeType_ChipRootId
                        //      Subject = kOID_AttributeType_ChipRootId
                        //
                        // This function is called first for the Issuer DN, and later for Subject DN.
                        // Since the caller knows if Issuer or Subject DN is being parsed, it's left up to
                        // the caller to use the returned value (subjectOrIssuer) appropriately.
                        if (attrOID == chip::ASN1::kOID_AttributeType_ChipNodeId ||
                            attrOID == chip::ASN1::kOID_AttributeType_ChipICAId ||
                            attrOID == chip::ASN1::kOID_AttributeType_ChipRootId)
                        {
                            subjectOrIssuer = chipAttr;
                        }
                        else if (attrOID == chip::ASN1::kOID_AttributeType_ChipFabricId)
                        {
                            fabric.SetValue(chipAttr);
                        }
                    }

                    //
                    else
                    {
                        err =
                            writer.PutString(ContextTag(tlvTagNum), Uint8::to_const_char(reader.GetValue()), reader.GetValueLen());
                        SuccessOrExit(err);
                    }
                }
                ASN1_EXIT_SEQUENCE;

                // Only one AttributeTypeAndValue allowed per RDN.
                err = reader.Next();
                if (err == CHIP_NO_ERROR)
                {
                    ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
                }
                if (err != ASN1_END)
                {
                    ExitNow();
                }
            }
            ASN1_EXIT_SET;
        }
    }
    ASN1_EXIT_SEQUENCE;

    err = writer.EndContainer(outerContainer);
    SuccessOrExit(err);

exit:
    return err;
}

static CHIP_ERROR ConvertValidity(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;
    ASN1UniversalTime asn1Time;
    uint32_t chipEpochTime;

    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_TIME(asn1Time);

        err = ASN1ToChipEpochTime(asn1Time, chipEpochTime);
        SuccessOrExit(err);

        err = writer.Put(ContextTag(kTag_NotBefore), chipEpochTime);
        SuccessOrExit(err);

        ASN1_PARSE_TIME(asn1Time);

        err = ASN1ToChipEpochTime(asn1Time, chipEpochTime);
        SuccessOrExit(err);

        err = writer.Put(ContextTag(kTag_NotAfter), chipEpochTime);
        SuccessOrExit(err);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR ConvertSubjectPublicKeyInfo(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;
    OID pubKeyAlgoOID, pubKeyCurveOID;

    // subjectPublicKeyInfo SubjectPublicKeyInfo,
    ASN1_PARSE_ENTER_SEQUENCE
    {
        // algorithm AlgorithmIdentifier,
        // AlgorithmIdentifier ::= SEQUENCE
        ASN1_PARSE_ENTER_SEQUENCE
        {
            // algorithm OBJECT IDENTIFIER,
            ASN1_PARSE_OBJECT_ID(pubKeyAlgoOID);

            // Verify that the algorithm type is supported.
            VerifyOrExit(pubKeyAlgoOID == kOID_PubKeyAlgo_ECPublicKey, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

            err = writer.Put(ContextTag(kTag_PublicKeyAlgorithm), GetOIDEnum(pubKeyAlgoOID));
            SuccessOrExit(err);

            // EcpkParameters ::= CHOICE {
            //     ecParameters  ECParameters,
            //     namedCurve    OBJECT IDENTIFIER,
            //     implicitlyCA  NULL }
            ASN1_PARSE_ANY;

            // ecParameters and implicitlyCA not supported.
            if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_Sequence)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }
            if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_Null)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            ASN1_VERIFY_TAG(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);

            ASN1_GET_OBJECT_ID(pubKeyCurveOID);

            // Verify the curve name is recognized.
            VerifyOrExit(GetOIDCategory(pubKeyCurveOID) == kOIDCategory_EllipticCurve, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

            err = writer.Put(ContextTag(kTag_EllipticCurveIdentifier), GetOIDEnum(pubKeyCurveOID));
            SuccessOrExit(err);
        }
        ASN1_EXIT_SEQUENCE;

        // subjectPublicKey BIT STRING
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_BitString);

        // Verify public key length.
        VerifyOrExit(reader.GetValueLen() > 0, err = ASN1_ERROR_INVALID_ENCODING);

        // The first byte is Unused Bit Count value, which should be zero.
        VerifyOrExit(reader.GetValue()[0] == 0, err = ASN1_ERROR_INVALID_ENCODING);

        // Copy the X9.62 encoded EC point into the CHIP certificate as a byte string.
        // Skip the first Unused Bit Count byte.
        err = writer.PutBytes(ContextTag(kTag_EllipticCurvePublicKey), reader.GetValue() + 1, reader.GetValueLen() - 1);
        SuccessOrExit(err);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR ConvertExtension(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVType outerContainer;
    OID extensionOID;
    bool critical = false;
    const uint8_t * extensionSequence;
    uint32_t extensionSequenceLen;

    err = reader.GetConstructedType(extensionSequence, extensionSequenceLen);
    SuccessOrExit(err);

    // Extension ::= SEQUENCE
    ASN1_ENTER_SEQUENCE
    {
        // extnID OBJECT IDENTIFIER,
        ASN1_PARSE_OBJECT_ID(extensionOID);

        // The kOID_Unknown will be interpreted and encoded as future-extension.
        if (extensionOID != kOID_Unknown)
        {
            VerifyOrExit(GetOIDCategory(extensionOID) == kOIDCategory_Extension, err = ASN1_ERROR_INVALID_ENCODING);
        }

        // critical BOOLEAN DEFAULT FALSE,
        ASN1_PARSE_ANY;
        if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_Boolean)
        {
            ASN1_GET_BOOLEAN(critical);

            VerifyOrExit(critical, err = ASN1_ERROR_INVALID_ENCODING);

            ASN1_PARSE_ANY;
        }

        // extnValue OCTET STRING
        //           -- contains the DER encoding of an ASN.1 value
        //           -- corresponding to the extension type identified
        //           -- by extnID
        ASN1_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_OctetString)
        {
            if (extensionOID == kOID_Extension_AuthorityKeyIdentifier)
            {
                // This extension MUST be marked as non-critical.
                VerifyOrExit(!critical, err = ASN1_ERROR_INVALID_ENCODING);

                // AuthorityKeyIdentifier ::= SEQUENCE
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    err = reader.Next();
                    VerifyOrExit(err == CHIP_NO_ERROR, err = ASN1_ERROR_INVALID_ENCODING);

                    // keyIdentifier [0] IMPLICIT KeyIdentifier,
                    // KeyIdentifier ::= OCTET STRING
                    VerifyOrExit(reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 0,
                                 err = ASN1_ERROR_INVALID_ENCODING);

                    VerifyOrExit(reader.IsConstructed() == false, err = ASN1_ERROR_INVALID_ENCODING);
                    VerifyOrExit(reader.GetValueLen() == kKeyIdentifierLength, err = ASN1_ERROR_INVALID_ENCODING);

                    err = writer.PutBytes(ContextTag(kTag_AuthorityKeyIdentifier), reader.GetValue(), reader.GetValueLen());
                    SuccessOrExit(err);

                    err = reader.Next();
                    VerifyOrExit(err == ASN1_END, err = ASN1_ERROR_INVALID_ENCODING);
                }
                ASN1_EXIT_SEQUENCE;
            }
            else if (extensionOID == kOID_Extension_SubjectKeyIdentifier)
            {
                // This extension MUST be marked as non-critical.
                VerifyOrExit(!critical, err = ASN1_ERROR_INVALID_ENCODING);

                // SubjectKeyIdentifier ::= KeyIdentifier
                // KeyIdentifier ::= OCTET STRING
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);

                VerifyOrExit(reader.GetValueLen() == kKeyIdentifierLength, err = ASN1_ERROR_INVALID_ENCODING);

                err = writer.PutBytes(ContextTag(kTag_SubjectKeyIdentifier), reader.GetValue(), reader.GetValueLen());
                SuccessOrExit(err);
            }
            else if (extensionOID == kOID_Extension_KeyUsage)
            {
                // This extension MUST be marked as critical.
                VerifyOrExit(critical, err = ASN1_ERROR_INVALID_ENCODING);

                // KeyUsage ::= BIT STRING
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_BitString);

                uint32_t keyUsageBits;
                err = reader.GetBitString(keyUsageBits);
                SuccessOrExit(err);
                VerifyOrExit(keyUsageBits <= UINT16_MAX, err = ASN1_ERROR_INVALID_ENCODING);

                // Check that only supported flags are set.
                BitFlags<KeyUsageFlags> keyUsageFlags(static_cast<uint16_t>(keyUsageBits));
                VerifyOrExit(keyUsageFlags.HasOnly(
                                 KeyUsageFlags::kDigitalSignature, KeyUsageFlags::kNonRepudiation, KeyUsageFlags::kKeyEncipherment,
                                 KeyUsageFlags::kDataEncipherment, KeyUsageFlags::kKeyAgreement, KeyUsageFlags::kKeyCertSign,
                                 KeyUsageFlags::kCRLSign, KeyUsageFlags::kEncipherOnly, KeyUsageFlags::kEncipherOnly),
                             err = ASN1_ERROR_INVALID_ENCODING);

                err = writer.Put(ContextTag(kTag_KeyUsage), keyUsageBits);
                SuccessOrExit(err);
            }
            else if (extensionOID == kOID_Extension_BasicConstraints)
            {
                // This extension MUST be marked as critical.
                VerifyOrExit(critical, err = ASN1_ERROR_INVALID_ENCODING);

                // BasicConstraints ::= SEQUENCE
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    bool isCA                 = false;
                    int64_t pathLenConstraint = -1;

                    // cA BOOLEAN DEFAULT FALSE
                    err = reader.Next();
                    if (err == CHIP_NO_ERROR && reader.GetClass() == kASN1TagClass_Universal &&
                        reader.GetTag() == kASN1UniversalTag_Boolean)
                    {
                        ASN1_GET_BOOLEAN(isCA);

                        VerifyOrExit(isCA, err = ASN1_ERROR_INVALID_ENCODING);

                        err = reader.Next();
                    }

                    // pathLenConstraint INTEGER (0..MAX) OPTIONAL
                    if (err == CHIP_NO_ERROR && reader.GetClass() == kASN1TagClass_Universal &&
                        reader.GetTag() == kASN1UniversalTag_Integer)
                    {
                        ASN1_GET_INTEGER(pathLenConstraint);

                        VerifyOrExit(pathLenConstraint <= UINT8_MAX, err = ASN1_ERROR_INVALID_ENCODING);
                        VerifyOrExit(pathLenConstraint >= 0, err = ASN1_ERROR_INVALID_ENCODING);

                        // pathLenConstraint is present only when cA is TRUE
                        VerifyOrExit(isCA, err = ASN1_ERROR_INVALID_ENCODING);
                    }

                    err = writer.StartContainer(ContextTag(kTag_BasicConstraints), kTLVType_Structure, outerContainer);
                    SuccessOrExit(err);

                    // Set also when cA is FALSE
                    err = writer.PutBoolean(ContextTag(kTag_BasicConstraints_IsCA), isCA);
                    SuccessOrExit(err);

                    if (pathLenConstraint != -1)
                    {
                        err = writer.Put(ContextTag(kTag_BasicConstraints_PathLenConstraint),
                                         static_cast<uint8_t>(pathLenConstraint));
                        SuccessOrExit(err);
                    }

                    err = writer.EndContainer(outerContainer);
                    SuccessOrExit(err);
                }
                ASN1_EXIT_SEQUENCE;
            }
            else if (extensionOID == kOID_Extension_ExtendedKeyUsage)
            {
                // This extension MUST be marked as critical.
                VerifyOrExit(critical, err = ASN1_ERROR_INVALID_ENCODING);

                err = writer.StartContainer(ContextTag(kTag_ExtendedKeyUsage), kTLVType_Array, outerContainer);
                SuccessOrExit(err);

                // ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    while ((err = reader.Next()) == CHIP_NO_ERROR)
                    {
                        // KeyPurposeId ::= OBJECT IDENTIFIER
                        OID keyPurposeOID;
                        ASN1_GET_OBJECT_ID(keyPurposeOID);

                        VerifyOrExit(keyPurposeOID != kOID_Unknown, err = ASN1_ERROR_UNSUPPORTED_ENCODING);
                        VerifyOrExit(GetOIDCategory(keyPurposeOID) == kOIDCategory_KeyPurpose, err = ASN1_ERROR_INVALID_ENCODING);

                        err = writer.Put(AnonymousTag, GetOIDEnum(keyPurposeOID));
                        SuccessOrExit(err);
                    }
                    if (err != ASN1_END)
                    {
                        SuccessOrExit(err);
                    }
                }
                ASN1_EXIT_SEQUENCE;

                err = writer.EndContainer(outerContainer);
                SuccessOrExit(err);
            }
            // Any other extension is treated as FutureExtension
            else
            {
                err = writer.PutBytes(ContextTag(kTag_FutureExtension), extensionSequence, extensionSequenceLen);
                SuccessOrExit(err);

                ASN1_PARSE_ANY;
            }
        }
        ASN1_EXIT_ENCAPSULATED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR ConvertExtensions(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVType containerType;

    err = writer.StartContainer(ContextTag(kTag_Extensions), kTLVType_List, containerType);
    SuccessOrExit(err);

    // Extensions ::= SEQUENCE SIZE (1..MAX) OF Extension
    ASN1_PARSE_ENTER_SEQUENCE
    {
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            err = ConvertExtension(reader, writer);
            SuccessOrExit(err);
        }

        if (err != ASN1_END)
        {
            SuccessOrExit(err);
        }
    }
    ASN1_EXIT_SEQUENCE;

    err = writer.EndContainer(containerType);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConvertECDSASignatureDERToRaw(ASN1Reader & reader, TLVWriter & writer, uint64_t tag)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t rawSig[kP256_ECDSA_Signature_Length_Raw];

    // Per RFC3279, the ECDSA signature value is encoded in DER encapsulated in the signatureValue BIT STRING.
    ASN1_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_BitString)
    {
        // Ecdsa-Sig-Value ::= SEQUENCE
        ASN1_PARSE_ENTER_SEQUENCE
        {
            // r INTEGER
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);
            ReturnErrorOnFailure(
                ConvertIntegerDERToRaw(ByteSpan(reader.GetValue(), reader.GetValueLen()), rawSig, kP256_FE_Length));

            // s INTEGER
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);
            ReturnErrorOnFailure(ConvertIntegerDERToRaw(ByteSpan(reader.GetValue(), reader.GetValueLen()), rawSig + kP256_FE_Length,
                                                        kP256_FE_Length));
        }
        ASN1_EXIT_SEQUENCE;
    }
    ASN1_EXIT_ENCAPSULATED;

    ReturnErrorOnFailure(writer.PutBytes(tag, rawSig, kP256_ECDSA_Signature_Length_Raw));

exit:
    return err;
}

static CHIP_ERROR ConvertCertificate(ASN1Reader & reader, TLVWriter & writer, uint64_t tag, uint64_t & issuer, uint64_t & subject,
                                     Optional<uint64_t> & fabric)
{
    CHIP_ERROR err;
    int64_t version;
    OID sigAlgoOID;
    TLVType containerType;

    err = writer.StartContainer(tag, kTLVType_Structure, containerType);
    SuccessOrExit(err);

    // Certificate ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        // tbsCertificate TBSCertificate,
        // TBSCertificate ::= SEQUENCE
        ASN1_PARSE_ENTER_SEQUENCE
        {
            // version [0] EXPLICIT Version DEFAULT v1
            ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
            {
                // Version ::= INTEGER { v1(0), v2(1), v3(2) }
                ASN1_PARSE_INTEGER(version);

                // Verify that the X.509 certificate version is v3
                VerifyOrExit(version == 2, err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }
            ASN1_EXIT_CONSTRUCTED;

            // serialNumber CertificateSerialNumber
            // CertificateSerialNumber ::= INTEGER
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);
            err = writer.PutBytes(ContextTag(kTag_SerialNumber), reader.GetValue(), reader.GetValueLen());
            SuccessOrExit(err);

            // signature AlgorithmIdentifier
            // AlgorithmIdentifier ::= SEQUENCE
            ASN1_PARSE_ENTER_SEQUENCE
            {
                // algorithm OBJECT IDENTIFIER,
                ASN1_PARSE_OBJECT_ID(sigAlgoOID);

                VerifyOrExit(sigAlgoOID == kOID_SigAlgo_ECDSAWithSHA256, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

                err = writer.Put(ContextTag(kTag_SignatureAlgorithm), GetOIDEnum(sigAlgoOID));
                SuccessOrExit(err);
            }
            ASN1_EXIT_SEQUENCE;

            // issuer Name
            err = ConvertDistinguishedName(reader, writer, ContextTag(kTag_Issuer), issuer, fabric);
            SuccessOrExit(err);

            // validity Validity,
            err = ConvertValidity(reader, writer);
            SuccessOrExit(err);

            // subject Name,
            err = ConvertDistinguishedName(reader, writer, ContextTag(kTag_Subject), subject, fabric);
            SuccessOrExit(err);

            err = ConvertSubjectPublicKeyInfo(reader, writer);
            SuccessOrExit(err);

            err = reader.Next();

            // issuerUniqueID [1] IMPLICIT UniqueIdentifier OPTIONAL,
            // Not supported.
            if (err == CHIP_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 1)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            // subjectUniqueID [2] IMPLICIT UniqueIdentifier OPTIONAL,
            // Not supported.
            if (err == CHIP_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 2)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            // extensions [3] EXPLICIT Extensions OPTIONAL
            if (err == CHIP_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 3)
            {
                ASN1_ENTER_CONSTRUCTED(kASN1TagClass_ContextSpecific, 3)
                {
                    err = ConvertExtensions(reader, writer);
                    SuccessOrExit(err);
                }
                ASN1_EXIT_CONSTRUCTED;

                err = reader.Next();
            }

            if (err != ASN1_END)
            {
                ExitNow();
            }
        }
        ASN1_EXIT_SEQUENCE;

        // signatureAlgorithm AlgorithmIdentifier
        // AlgorithmIdentifier ::= SEQUENCE
        ASN1_PARSE_ENTER_SEQUENCE
        {
            OID localSigAlgoOID;

            // algorithm OBJECT IDENTIFIER,
            ASN1_PARSE_OBJECT_ID(localSigAlgoOID);

            // Verify that the signatureAlgorithm is the same as the "signature" field in TBSCertificate.
            VerifyOrExit(localSigAlgoOID == sigAlgoOID, err = ASN1_ERROR_UNSUPPORTED_ENCODING);
        }
        ASN1_EXIT_SEQUENCE;

        // signatureValue BIT STRING
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_BitString);

        ReturnErrorOnFailure(ConvertECDSASignatureDERToRaw(reader, writer, ContextTag(kTag_ECDSASignature)));
    }
    ASN1_EXIT_SEQUENCE;

    err = writer.EndContainer(containerType);
    SuccessOrExit(err);

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR ConvertX509CertToChipCert(const ByteSpan x509Cert, uint8_t * chipCertBuf, uint32_t chipCertBufSize,
                                                uint32_t & chipCertLen)
{
    ASN1Reader reader;
    TLVWriter writer;

    uint64_t issuer, subject;
    Optional<uint64_t> fabric;

    VerifyOrReturnError(!x509Cert.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint32_t>(x509Cert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    reader.Init(x509Cert.data(), static_cast<uint32_t>(x509Cert.size()));

    writer.Init(chipCertBuf, chipCertBufSize);

    ReturnErrorOnFailure(ConvertCertificate(
        reader, writer, ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificate), issuer, subject, fabric));

    ReturnErrorOnFailure(writer.Finalize());

    chipCertLen = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertX509CertsToChipCertArray(const ByteSpan & x509NOC, const ByteSpan & x509ICAC, MutableByteSpan & chipCertArray)
{
    // NOC is mandatory
    VerifyOrReturnError(!x509NOC.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    TLVWriter writer;

    // We can still generate the certificate if the output chip cert buffer is bigger than UINT32_MAX,
    // since generated cert needs less space than UINT32_MAX.
    uint32_t chipCertBufLen = (chipCertArray.size() > UINT32_MAX) ? UINT32_MAX : static_cast<uint32_t>(chipCertArray.size());
    writer.Init(chipCertArray.data(), chipCertBufLen);

    TLVType outerContainer;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag, kTLVType_Array, outerContainer));

    ASN1Reader reader;
    VerifyOrReturnError(CanCastTo<uint32_t>(x509NOC.size()), CHIP_ERROR_INVALID_ARGUMENT);
    reader.Init(x509NOC.data(), static_cast<uint32_t>(x509NOC.size()));
    uint64_t nocIssuer, nocSubject;
    Optional<uint64_t> nocFabric;
    ReturnErrorOnFailure(ConvertCertificate(reader, writer, AnonymousTag, nocIssuer, nocSubject, nocFabric));
    VerifyOrReturnError(nocFabric.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

    // ICAC is optional
    if (!x509ICAC.empty())
    {
        VerifyOrReturnError(CanCastTo<uint32_t>(x509ICAC.size()), CHIP_ERROR_INVALID_ARGUMENT);
        reader.Init(x509ICAC.data(), static_cast<uint32_t>(x509ICAC.size()));
        uint64_t icaIssuer, icaSubject;
        Optional<uint64_t> icaFabric;
        ReturnErrorOnFailure(ConvertCertificate(reader, writer, AnonymousTag, icaIssuer, icaSubject, icaFabric));
        VerifyOrReturnError(icaSubject == nocIssuer, CHIP_ERROR_INVALID_ARGUMENT);
        if (icaFabric.HasValue())
        {
            // Match ICA's fabric ID if the ICA certificate has provided it
            VerifyOrReturnError(icaFabric == nocFabric, CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    ReturnErrorOnFailure(writer.EndContainer(outerContainer));
    ReturnErrorOnFailure(writer.Finalize());

    ReturnErrorCodeIf(writer.GetLengthWritten() > chipCertBufLen, CHIP_ERROR_INTERNAL);
    chipCertArray.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractCertsFromCertArray(const ByteSpan & opCertArray, ByteSpan & noc, ByteSpan & icac)
{
    TLVType outerContainerType;
    TLVReader reader;
    reader.Init(opCertArray.data(), static_cast<uint32_t>(opCertArray.size()));

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }
    VerifyOrReturnError(reader.GetType() == kTLVType_Array, CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outerContainerType));

    {
        TLVType nocContainerType;
        const uint8_t * nocBegin = reader.GetReadPoint();

        if (reader.GetType() == kTLVType_NotSpecified)
        {
            ReturnErrorOnFailure(reader.Next());
        }
        VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
        uint64_t tag = reader.GetTag();
        VerifyOrReturnError(tag == ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificate) ||
                                tag == AnonymousTag,
                            CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

        ReturnErrorOnFailure(reader.EnterContainer(nocContainerType));
        ReturnErrorOnFailure(reader.ExitContainer(nocContainerType));
        noc = ByteSpan(nocBegin, static_cast<size_t>(reader.GetReadPoint() - nocBegin));
    }

    {
        TLVType icacContainerType;
        const uint8_t * icacBegin = reader.GetReadPoint();

        if (reader.GetType() == kTLVType_NotSpecified)
        {
            CHIP_ERROR err = reader.Next();
            if (err == CHIP_END_OF_TLV)
            {
                icac = ByteSpan(nullptr, 0);
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
        }
        VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
        uint64_t tag = reader.GetTag();
        VerifyOrReturnError(tag == ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificate) ||
                                tag == AnonymousTag,
                            CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

        ReturnErrorOnFailure(reader.EnterContainer(icacContainerType));
        ReturnErrorOnFailure(reader.ExitContainer(icacContainerType));
        icac = ByteSpan(icacBegin, static_cast<size_t>(reader.GetReadPoint() - icacBegin));
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));

    return CHIP_NO_ERROR;
}

} // namespace Credentials
} // namespace chip
