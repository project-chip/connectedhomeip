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
 *      This file implements methods for generating CHIP X.509 certificate.
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
#include <credentials/CHIPCert.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::Protocols;

static CHIP_ERROR EncodeSubjectPublicKeyInfo(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        ASN1_START_SEQUENCE
        {
            ASN1_ENCODE_OBJECT_ID(kOID_PubKeyAlgo_ECPublicKey);
            ASN1_ENCODE_OBJECT_ID(kOID_EllipticCurve_prime256v1);
        }
        ASN1_END_SEQUENCE;

        ReturnErrorOnFailure(writer.PutBitString(0, pubkey, static_cast<uint8_t>(pubkey.Length())));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeAuthorityKeyIdentifierExtension(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_AuthorityKeyIdentifier));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                uint8_t keyid[20];
                ReturnErrorOnFailure(Crypto::Hash_SHA1(pubkey, pubkey.Length(), keyid));

                ReturnErrorOnFailure(
                    writer.PutOctetString(kASN1TagClass_ContextSpecific, 0, keyid, static_cast<uint8_t>(sizeof(keyid))));
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeSubjectKeyIdentifierExtension(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_SubjectKeyIdentifier));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            uint8_t keyid[20];
            ReturnErrorOnFailure(Crypto::Hash_SHA1(pubkey, pubkey.Length(), keyid));

            ReturnErrorOnFailure(writer.PutOctetString(keyid, static_cast<uint8_t>(sizeof(keyid))));
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeKeyUsageExtension(uint16_t keyUsageBits, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_KeyUsage));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        // KeyUsage extension MUST be marked as critical.
        ASN1_ENCODE_BOOLEAN(true);
        ASN1_START_OCTET_STRING_ENCAPSULATED { ASN1_ENCODE_BIT_STRING(keyUsageBits); }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeIsCAExtension(bool isCA, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_BasicConstraints));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        // BasicConstraints extension MUST be marked as critical.
        ASN1_ENCODE_BOOLEAN(true);

        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                // cA BOOLEAN
                if (isCA)
                {
                    ASN1_ENCODE_BOOLEAN(isCA);
                }
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeCASpecificExtensions(ASN1Writer & writer)
{
    ReturnErrorOnFailure(EncodeIsCAExtension(true, writer));

    uint16_t keyUsageBits = static_cast<uint16_t>(KeyUsageFlags::kKeyCertSign) | static_cast<uint16_t>(KeyUsageFlags::kCRLSign);

    ReturnErrorOnFailure(EncodeKeyUsageExtension(keyUsageBits, writer));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR EncodeNOCSpecificExtensions(ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t keyUsageBits =
        static_cast<uint16_t>(KeyUsageFlags::kDigitalSignature) | static_cast<uint16_t>(KeyUsageFlags::kKeyEncipherment);

    ReturnErrorOnFailure(EncodeIsCAExtension(false, writer));
    ReturnErrorOnFailure(EncodeKeyUsageExtension(keyUsageBits, writer));

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_ExtendedKeyUsage));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        // ExtKeyUsage extension MUST be marked as critical.
        ASN1_ENCODE_BOOLEAN(true);
        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                ASN1_ENCODE_OBJECT_ID(kOID_KeyPurpose_ClientAuth);
                ASN1_ENCODE_OBJECT_ID(kOID_KeyPurpose_ServerAuth);
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeExtensions(bool isCA, const Crypto::P256PublicKey & SKI, const Crypto::P256PublicKey & AKI,
                                   ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 3)
    {
        ASN1_START_SEQUENCE
        {
            if (isCA)
            {
                ReturnErrorOnFailure(EncodeCASpecificExtensions(writer));
            }
            else
            {
                ReturnErrorOnFailure(EncodeNOCSpecificExtensions(writer));
            }

            ReturnErrorOnFailure(EncodeSubjectKeyIdentifierExtension(SKI, writer));

            ReturnErrorOnFailure(EncodeAuthorityKeyIdentifierExtension(AKI, writer));
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_CONSTRUCTED;

exit:
    return err;
}

struct ChipDNParams
{
    OID mAttrOID;
    uint64_t mValue;
};

static CHIP_ERROR EncodeChipDNs(ChipDNParams * params, uint8_t numParams, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        for (uint8_t i = 0; i < numParams; i++)
        {
            ASN1_START_SET
            {
                uint8_t chipAttrStr[17];
                snprintf(reinterpret_cast<char *>(chipAttrStr), sizeof(chipAttrStr), "%016" PRIX64, params[i].mValue);

                ASN1_START_SEQUENCE
                {
                    ASN1_ENCODE_OBJECT_ID(params[i].mAttrOID);
                    ReturnErrorOnFailure(writer.PutString(kASN1UniversalTag_UTF8String, Uint8::to_const_char(chipAttrStr), 16));
                }
                ASN1_END_SEQUENCE;
            }
            ASN1_END_SET;
        }
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeValidity(uint32_t validityStart, uint32_t validityEnd, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1UniversalTime asn1Time;

    ASN1_START_SEQUENCE
    {
        ReturnErrorOnFailure(ChipEpochToASN1Time(validityStart, asn1Time));
        ASN1_ENCODE_TIME(asn1Time);

        ReturnErrorOnFailure(ChipEpochToASN1Time(validityEnd, asn1Time));
        ASN1_ENCODE_TIME(asn1Time);
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR EncodeChipECDSASignature(Crypto::P256ECDSASignature & signature, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_BIT_STRING_ENCAPSULATED
    {
        ASN1_START_SEQUENCE
        {
            const uint8_t * signatureBuf = signature;
            uint16_t rLength             = (uint16_t) signature.Length() / 2;

            // r INTEGER
            ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, signatureBuf, rLength));

            // s INTEGER
            ReturnErrorOnFailure(
                writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, &signatureBuf[rLength], rLength));
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_ENCAPSULATED;

exit:
    return err;
}

CHIP_ERROR EncodeTBSCert(X509CertRequestParams & requestParams, bool rootCASigner, uint64_t subject,
                         const Crypto::P256PublicKey & subjectPubkey, const Crypto::P256PublicKey & issuerPubkey,
                         ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipDNParams dnParams[2];
    uint8_t numDNs = 1;
    bool isCA      = true;

    ASN1_START_SEQUENCE
    {
        // version [0] EXPLICIT Version DEFAULT v1
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // Version ::= INTEGER { v1(0), v2(1), v3(2) }
            ASN1_ENCODE_INTEGER(2);
        }
        ASN1_END_CONSTRUCTED;

        ReturnErrorOnFailure(writer.PutInteger(requestParams.mSerialNumber));

        ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256); }
        ASN1_END_SEQUENCE;

        // Issuer OID depends on if cert is being signed by the root CA
        if (rootCASigner)
        {
            dnParams[0].mAttrOID = chip::ASN1::kOID_AttributeType_ChipRootId;
        }
        else
        {
            dnParams[0].mAttrOID = chip::ASN1::kOID_AttributeType_ChipICAId;
        }
        dnParams[0].mValue = requestParams.mIssuer;

        if (requestParams.mHasFabricID)
        {
            dnParams[1].mAttrOID = chip::ASN1::kOID_AttributeType_ChipFabricId;
            dnParams[1].mValue   = requestParams.mFabricID;
            numDNs               = 2;
        }
        ReturnErrorOnFailure(EncodeChipDNs(dnParams, numDNs, writer));

        // validity Validity,
        ReturnErrorOnFailure(EncodeValidity(requestParams.mValidityStart, requestParams.mValidityEnd, writer));

        // subject Name
        if (requestParams.mHasNodeID)
        {
            dnParams[0].mAttrOID = chip::ASN1::kOID_AttributeType_ChipNodeId;

            isCA = false;
        }
        else if (subjectPubkey != issuerPubkey)
        {
            dnParams[0].mAttrOID = chip::ASN1::kOID_AttributeType_ChipICAId;
        }
        dnParams[0].mValue = subject;

        ReturnErrorOnFailure(EncodeChipDNs(dnParams, numDNs, writer));

        ReturnErrorOnFailure(EncodeSubjectPublicKeyInfo(subjectPubkey, writer));

        // certificate extensions
        ReturnErrorOnFailure(EncodeExtensions(isCA, subjectPubkey, issuerPubkey, writer));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static CHIP_ERROR NewChipX509Cert(X509CertRequestParams & requestParams, bool rootCASigner, uint64_t subject,
                                  const Crypto::P256PublicKey & subjectPubkey, Crypto::P256Keypair & issuerKeypair,
                                  uint8_t * x509CertBuf, uint32_t x509CertBufSize, uint32_t & x509CertLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Writer writer;
    writer.Init(x509CertBuf, x509CertBufSize);

    ReturnErrorOnFailure(EncodeTBSCert(requestParams, rootCASigner, subject, subjectPubkey, issuerKeypair.Pubkey(), writer));
    writer.Finalize();

    Crypto::P256ECDSASignature signature;
    ReturnErrorOnFailure(issuerKeypair.ECDSA_sign_msg(x509CertBuf, writer.GetLengthWritten(), signature));

    writer.Init(x509CertBuf, x509CertBufSize);

    ASN1_START_SEQUENCE
    {
        ReturnErrorOnFailure(EncodeTBSCert(requestParams, rootCASigner, subject, subjectPubkey, issuerKeypair.Pubkey(), writer));
        SuccessOrExit(err);

        ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256); }
        ASN1_END_SEQUENCE;

        err = EncodeChipECDSASignature(signature, writer);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

    writer.Finalize();
    x509CertLen = writer.GetLengthWritten();

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR NewRootX509Cert(X509CertRequestParams & requestParams, Crypto::P256Keypair & issuerKeypair,
                                      uint8_t * x509CertBuf, uint32_t x509CertBufSize, uint32_t & x509CertLen)
{
    ReturnErrorCodeIf(requestParams.mHasNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    return NewChipX509Cert(requestParams, true, requestParams.mIssuer, issuerKeypair.Pubkey(), issuerKeypair, x509CertBuf,
                           x509CertBufSize, x509CertLen);
}

DLL_EXPORT CHIP_ERROR NewICAX509Cert(X509CertRequestParams & requestParams, uint64_t subject,
                                     const Crypto::P256PublicKey & subjectPubkey, Crypto::P256Keypair & issuerKeypair,
                                     uint8_t * x509CertBuf, uint32_t x509CertBufSize, uint32_t & x509CertLen)
{
    ReturnErrorCodeIf(requestParams.mHasNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    return NewChipX509Cert(requestParams, true, subject, subjectPubkey, issuerKeypair, x509CertBuf, x509CertBufSize, x509CertLen);
}

DLL_EXPORT CHIP_ERROR NewNodeOperationalX509Cert(X509CertRequestParams & requestParams, bool rootCASigner,
                                                 const Crypto::P256PublicKey & subjectPubkey, Crypto::P256Keypair & issuerKeypair,
                                                 uint8_t * x509CertBuf, uint32_t x509CertBufSize, uint32_t & x509CertLen)
{
    VerifyOrReturnError(requestParams.mHasNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(requestParams.mHasFabricID, CHIP_ERROR_INVALID_ARGUMENT);
    return NewChipX509Cert(requestParams, rootCASigner, requestParams.mNodeID, subjectPubkey, issuerKeypair, x509CertBuf,
                           x509CertBufSize, x509CertLen);
}

} // namespace Credentials
} // namespace chip
