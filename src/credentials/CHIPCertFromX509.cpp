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
#include <credentials/CHIPCert.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Protocols;

static ASN1_ERROR ParseChipIdAttribute(ASN1Reader & reader, uint64_t & chipIdOut)
{
    ASN1_ERROR err        = ASN1_NO_ERROR;
    const uint8_t * value = nullptr;

    static constexpr uint32_t kChipIdUTF8Length = 16;

    VerifyOrExit(reader.GetValueLen() == kChipIdUTF8Length, err = ASN1_ERROR_INVALID_ENCODING);

    value = reader.GetValue();
    VerifyOrExit(value != nullptr, err = ASN1_ERROR_INVALID_ENCODING);

    chipIdOut = 0;

    for (uint32_t i = 0; i < kChipIdUTF8Length; i++)
    {
        chipIdOut <<= 4;
        uint8_t ch = value[i];
        if (ch >= '0' && ch <= '9')
        {
            chipIdOut |= (ch - '0');
        }
        // CHIP Id attribute encodings only support uppercase chars.
        else if (ch >= 'A' && ch <= 'F')
        {
            chipIdOut |= (ch - 'A' + 10);
        }
        else
        {
            ExitNow(err = ASN1_ERROR_INVALID_ENCODING);
        }
    }

exit:
    return err;
}

static CHIP_ERROR ConvertDistinguishedName(ASN1Reader & reader, TLVWriter & writer, uint64_t tag)
{
    CHIP_ERROR err;
    TLVType outerContainer;
    OID attrOID;

    err = writer.StartContainer(tag, kTLVType_Path, outerContainer);
    SuccessOrExit(err);

    // RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
    ASN1_PARSE_ENTER_SEQUENCE
    {
        while ((err = reader.Next()) == ASN1_NO_ERROR)
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

                    // CHIP id attributes must be UTF8Strings.
                    if (IsChipIdX509Attr(attrOID))
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

                    // If the attribute is a CHIP-defined attribute that contains a 64-bit CHIP id...
                    if (IsChipIdX509Attr(attrOID))
                    {
                        // Parse the attribute string into a 64-bit CHIP id.
                        uint64_t chipId;
                        err = ParseChipIdAttribute(reader, chipId);
                        SuccessOrExit(err);

                        // Write the CHIP id into the TLV.
                        err = writer.Put(ContextTag(tlvTagNum), chipId);
                        SuccessOrExit(err);
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
                if (err == ASN1_NO_ERROR)
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
    ASN1UniversalTime notBeforeTime, notAfterTime;
    uint32_t packedNotBeforeTime, packedNotAfterTime;

    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_TIME(notBeforeTime);
        err = PackCertTime(notBeforeTime, packedNotBeforeTime);
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_NotBefore), packedNotBeforeTime);
        SuccessOrExit(err);

        ASN1_PARSE_TIME(notAfterTime);
        err = PackCertTime(notAfterTime, packedNotAfterTime);
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_NotAfter), packedNotAfterTime);
        SuccessOrExit(err);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR ConvertAuthorityKeyIdentifierExtension(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;

    // AuthorityKeyIdentifier ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        err = reader.Next();

        // keyIdentifier [0] IMPLICIT KeyIdentifier OPTIONAL,
        // KeyIdentifier ::= OCTET STRING
        if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 0)
        {
            VerifyOrExit(reader.IsConstructed() == false, err = ASN1_ERROR_INVALID_ENCODING);

            err = writer.PutBytes(ContextTag(kTag_AuthorityKeyIdentifier_KeyIdentifier), reader.GetValue(), reader.GetValueLen());
            SuccessOrExit(err);

            err = reader.Next();
        }

        if (err != ASN1_END)
        {
            SuccessOrExit(err);
        }
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
    TLVType outerContainer, outerContainer2;
    OID extensionOID;
    bool critical = false;

    // Extension ::= SEQUENCE
    ASN1_ENTER_SEQUENCE
    {
        // extnID OBJECT IDENTIFIER,
        ASN1_PARSE_OBJECT_ID(extensionOID);

        if (extensionOID == kOID_Unknown)
        {
            ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
        }
        if (GetOIDCategory(extensionOID) != kOIDCategory_Extension)
        {
            ExitNow(err = ASN1_ERROR_INVALID_ENCODING);
        }

        // critical BOOLEAN DEFAULT FALSE,
        ASN1_PARSE_ANY;
        if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_Boolean)
        {
            ASN1_GET_BOOLEAN(critical);

            if (!critical)
            {
                ExitNow(err = ASN1_ERROR_INVALID_ENCODING);
            }

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
                err = writer.StartContainer(ContextTag(kTag_AuthorityKeyIdentifier), kTLVType_Structure, outerContainer);
                SuccessOrExit(err);

                if (critical)
                {
                    err = writer.PutBoolean(ContextTag(kTag_AuthorityKeyIdentifier_Critical), critical);
                    SuccessOrExit(err);
                }

                err = ConvertAuthorityKeyIdentifierExtension(reader, writer);
                SuccessOrExit(err);
            }
            else if (extensionOID == kOID_Extension_SubjectKeyIdentifier)
            {
                // SubjectKeyIdentifier ::= KeyIdentifier
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);

                err = writer.StartContainer(ContextTag(kTag_SubjectKeyIdentifier), kTLVType_Structure, outerContainer);
                SuccessOrExit(err);

                if (critical)
                {
                    err = writer.PutBoolean(ContextTag(kTag_SubjectKeyIdentifier_Critical), critical);
                    SuccessOrExit(err);
                }

                err = writer.PutBytes(ContextTag(kTag_SubjectKeyIdentifier_KeyIdentifier), reader.GetValue(), reader.GetValueLen());
                SuccessOrExit(err);
            }
            else if (extensionOID == kOID_Extension_KeyUsage)
            {
                // KeyUsage ::= BIT STRING
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_BitString);

                err = writer.StartContainer(ContextTag(kTag_KeyUsage), kTLVType_Structure, outerContainer);
                SuccessOrExit(err);

                if (critical)
                {
                    err = writer.PutBoolean(ContextTag(kTag_KeyUsage_Critical), critical);
                    SuccessOrExit(err);
                }

                uint32_t keyUsageBits;
                err = reader.GetBitString(keyUsageBits);
                SuccessOrExit(err);
                VerifyOrExit(keyUsageBits <= UINT16_MAX, err = ASN1_ERROR_INVALID_ENCODING);

                // Check that only supported flags are set.
                BitFlags<uint16_t, KeyUsageFlags> keyUsageFlags;
                keyUsageFlags.SetRaw(static_cast<uint16_t>(keyUsageBits));
                VerifyOrExit(keyUsageFlags.HasOnly(
                                 KeyUsageFlags::kDigitalSignature, KeyUsageFlags::kNonRepudiation, KeyUsageFlags::kKeyEncipherment,
                                 KeyUsageFlags::kDataEncipherment, KeyUsageFlags::kKeyAgreement, KeyUsageFlags::kKeyCertSign,
                                 KeyUsageFlags::kCRLSign, KeyUsageFlags::kEncipherOnly, KeyUsageFlags::kEncipherOnly),
                             err = ASN1_ERROR_INVALID_ENCODING);

                err = writer.Put(ContextTag(kTag_KeyUsage_KeyUsage), keyUsageBits);
                SuccessOrExit(err);
            }
            else if (extensionOID == kOID_Extension_BasicConstraints)
            {
                // BasicConstraints ::= SEQUENCE
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    bool isCA                 = false;
                    int64_t pathLenConstraint = -1;

                    // cA BOOLEAN DEFAULT FALSE
                    err = reader.Next();
                    if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_Universal &&
                        reader.GetTag() == kASN1UniversalTag_Boolean)
                    {
                        ASN1_GET_BOOLEAN(isCA);

                        VerifyOrExit(isCA == true, err = ASN1_ERROR_INVALID_ENCODING);

                        err = reader.Next();
                    }

                    // pathLenConstraint INTEGER (0..MAX) OPTIONAL
                    if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_Universal &&
                        reader.GetTag() == kASN1UniversalTag_Integer)
                    {
                        ASN1_GET_INTEGER(pathLenConstraint);

                        VerifyOrExit(pathLenConstraint <= UINT8_MAX, err = ASN1_ERROR_INVALID_ENCODING);
                        VerifyOrExit(pathLenConstraint >= 0, err = ASN1_ERROR_INVALID_ENCODING);
                    }

                    err = writer.StartContainer(ContextTag(kTag_BasicConstraints), kTLVType_Structure, outerContainer);
                    SuccessOrExit(err);

                    if (critical)
                    {
                        err = writer.PutBoolean(ContextTag(kTag_BasicConstraints_Critical), critical);
                        SuccessOrExit(err);
                    }

                    if (isCA)
                    {
                        err = writer.PutBoolean(ContextTag(kTag_BasicConstraints_IsCA), isCA);
                        SuccessOrExit(err);
                    }

                    if (pathLenConstraint != -1)
                    {
                        err = writer.Put(ContextTag(kTag_BasicConstraints_PathLenConstraint),
                                         static_cast<uint8_t>(pathLenConstraint));
                        SuccessOrExit(err);
                    }
                }
                ASN1_EXIT_SEQUENCE;
            }
            else if (extensionOID == kOID_Extension_ExtendedKeyUsage)
            {
                err = writer.StartContainer(ContextTag(kTag_ExtendedKeyUsage), kTLVType_Structure, outerContainer);
                SuccessOrExit(err);

                if (critical)
                {
                    err = writer.PutBoolean(ContextTag(kTag_ExtendedKeyUsage_Critical), critical);
                    SuccessOrExit(err);
                }

                err = writer.StartContainer(ContextTag(kTag_ExtendedKeyUsage_KeyPurposes), kTLVType_Array, outerContainer2);
                SuccessOrExit(err);

                // ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    while ((err = reader.Next()) == ASN1_NO_ERROR)
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

                err = writer.EndContainer(outerContainer2);
                SuccessOrExit(err);
            }
            else
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);

            err = writer.EndContainer(outerContainer);
            SuccessOrExit(err);
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

    // Extensions ::= SEQUENCE SIZE (1..MAX) OF Extension
    ASN1_PARSE_ENTER_SEQUENCE
    {
        while ((err = reader.Next()) == ASN1_NO_ERROR)
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

exit:
    return err;
}

static CHIP_ERROR ConvertCertificate(ASN1Reader & reader, TLVWriter & writer)
{
    CHIP_ERROR err;
    int64_t version;
    OID sigAlgoOID;
    TLVType containerType;

    err = writer.StartContainer(ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificate), kTLVType_Structure, containerType);
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
            err = ConvertDistinguishedName(reader, writer, ContextTag(kTag_Issuer));
            SuccessOrExit(err);

            // validity Validity,
            err = ConvertValidity(reader, writer);
            SuccessOrExit(err);

            // subject Name,
            err = ConvertDistinguishedName(reader, writer, ContextTag(kTag_Subject));
            SuccessOrExit(err);

            err = ConvertSubjectPublicKeyInfo(reader, writer);
            SuccessOrExit(err);

            err = reader.Next();

            // issuerUniqueID [1] IMPLICIT UniqueIdentifier OPTIONAL,
            // Not supported.
            if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 1)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            // subjectUniqueID [2] IMPLICIT UniqueIdentifier OPTIONAL,
            // Not supported.
            if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 2)
            {
                ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
            }

            // extensions [3] EXPLICIT Extensions OPTIONAL
            if (err == ASN1_NO_ERROR && reader.GetClass() == kASN1TagClass_ContextSpecific && reader.GetTag() == 3)
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

        // Per RFC3279, the ECDSA signature value is encoded in DER encapsulated in the signatureValue BIT STRING.
        ASN1_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_BitString)
        {
            TLVType outerContainer;

            err = writer.StartContainer(ContextTag(kTag_ECDSASignature), kTLVType_Structure, outerContainer);
            SuccessOrExit(err);

            // Ecdsa-Sig-Value ::= SEQUENCE
            ASN1_PARSE_ENTER_SEQUENCE
            {
                // r INTEGER
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);
                err = writer.PutBytes(ContextTag(kTag_ECDSASignature_r), reader.GetValue(), reader.GetValueLen());
                SuccessOrExit(err);

                // s INTEGER
                ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);
                err = writer.PutBytes(ContextTag(kTag_ECDSASignature_s), reader.GetValue(), reader.GetValueLen());
                SuccessOrExit(err);
            }
            ASN1_EXIT_SEQUENCE;

            err = writer.EndContainer(outerContainer);
            SuccessOrExit(err);
        }
        ASN1_EXIT_ENCAPSULATED;
    }
    ASN1_EXIT_SEQUENCE;

    err = writer.EndContainer(containerType);
    SuccessOrExit(err);

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR ConvertX509CertToChipCert(const uint8_t * x509Cert, uint32_t x509CertLen, uint8_t * chipCertBuf,
                                                uint32_t chipCertBufSize, uint32_t & chipCertLen)
{
    CHIP_ERROR err;
    ASN1Reader reader;
    TLVWriter writer;

    reader.Init(x509Cert, x509CertLen);

    writer.Init(chipCertBuf, chipCertBufSize);

    err = ConvertCertificate(reader, writer);
    SuccessOrExit(err);

    err = writer.Finalize();
    SuccessOrExit(err);

    chipCertLen = writer.GetLengthWritten();

exit:
    return err;
}

} // namespace Credentials
} // namespace chip
