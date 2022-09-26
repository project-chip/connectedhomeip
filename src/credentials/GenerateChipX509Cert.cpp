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
 *      This file implements methods for generating CHIP X.509 certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <algorithm>
#include <inttypes.h>
#include <stddef.h>

#include <credentials/CHIPCert.h>
#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <protocols/Protocols.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::Crypto;
using namespace chip::Protocols;

namespace {

enum IsCACert
{
    kCACert,
    kNotCACert,
};

CHIP_ERROR EncodeSubjectPublicKeyInfo(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
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

CHIP_ERROR EncodeAuthorityKeyIdentifierExtension(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
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
                uint8_t keyid[kSHA1_Hash_Length];
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

CHIP_ERROR EncodeSubjectKeyIdentifierExtension(const Crypto::P256PublicKey & pubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        OID extensionOID = GetOID(kOIDCategory_Extension, static_cast<uint8_t>(kTag_SubjectKeyIdentifier));

        ASN1_ENCODE_OBJECT_ID(extensionOID);

        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            uint8_t keyid[kSHA1_Hash_Length];
            ReturnErrorOnFailure(Crypto::Hash_SHA1(pubkey, pubkey.Length(), keyid));

            ReturnErrorOnFailure(writer.PutOctetString(keyid, static_cast<uint8_t>(sizeof(keyid))));
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR EncodeKeyUsageExtension(uint16_t keyUsageBits, ASN1Writer & writer)
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

CHIP_ERROR EncodeIsCAExtension(IsCACert isCA, ASN1Writer & writer)
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
                if (isCA == kCACert)
                {
                    // Encode the boolean only if isCA is true
                    ASN1_ENCODE_BOOLEAN(true);
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

CHIP_ERROR EncodeCASpecificExtensions(ASN1Writer & writer)
{
    ReturnErrorOnFailure(EncodeIsCAExtension(kCACert, writer));

    uint16_t keyUsageBits = static_cast<uint16_t>(KeyUsageFlags::kKeyCertSign) | static_cast<uint16_t>(KeyUsageFlags::kCRLSign);

    ReturnErrorOnFailure(EncodeKeyUsageExtension(keyUsageBits, writer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeNOCSpecificExtensions(ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t keyUsageBits = static_cast<uint16_t>(KeyUsageFlags::kDigitalSignature);

    ReturnErrorOnFailure(EncodeIsCAExtension(kNotCACert, writer));
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

CHIP_ERROR EncodeFutureExtension(const Optional<FutureExtension> & futureExt, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(futureExt.HasValue(), CHIP_NO_ERROR);

    ASN1_START_SEQUENCE
    {
        ReturnErrorOnFailure(writer.PutObjectId(futureExt.Value().OID.data(), static_cast<uint16_t>(futureExt.Value().OID.size())));

        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ReturnErrorOnFailure(writer.PutOctetString(futureExt.Value().Extension.data(),
                                                       static_cast<uint16_t>(futureExt.Value().Extension.size())));
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR EncodeExtensions(bool isCA, const Crypto::P256PublicKey & SKI, const Crypto::P256PublicKey & AKI,
                            const Optional<FutureExtension> & futureExt, ASN1Writer & writer)
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

            ReturnErrorOnFailure(EncodeFutureExtension(futureExt, writer));
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_CONSTRUCTED;

exit:
    return err;
}

CHIP_ERROR EncodeValidity(uint32_t validityStart, uint32_t validityEnd, ASN1Writer & writer)
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

CHIP_ERROR EncodeChipECDSASignature(Crypto::P256ECDSASignature & signature, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_BIT_STRING_ENCAPSULATED
    {
        // Convert RAW signature to DER when generating X509 certs.
        P256ECDSASignatureSpan raw_sig(signature.Bytes());
        ReturnErrorOnFailure(ConvertECDSASignatureRawToDER(raw_sig, writer));
    }
    ASN1_END_ENCAPSULATED;

exit:
    return err;
}

} // namespace

CHIP_ERROR EncodeTBSCert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                         const Crypto::P256PublicKey & issuerPubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t certType;
    bool isCA;

    VerifyOrReturnError(requestParams.SerialNumber >= 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(requestParams.ValidityEnd >= requestParams.ValidityStart, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    isCA = (certType == kCertType_ICA || certType == kCertType_Root);

    ASN1_START_SEQUENCE
    {
        // version [0] EXPLICIT Version DEFAULT v1
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // Version ::= INTEGER { v1(0), v2(1), v3(2) }
            ASN1_ENCODE_INTEGER(2);
        }
        ASN1_END_CONSTRUCTED;

        ReturnErrorOnFailure(writer.PutInteger(requestParams.SerialNumber));

        ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256); }
        ASN1_END_SEQUENCE;

        // issuer Name
        ReturnErrorOnFailure(requestParams.IssuerDN.EncodeToASN1(writer));

        // validity Validity,
        ReturnErrorOnFailure(EncodeValidity(requestParams.ValidityStart, requestParams.ValidityEnd, writer));

        // subject Name
        ReturnErrorOnFailure(requestParams.SubjectDN.EncodeToASN1(writer));

        ReturnErrorOnFailure(EncodeSubjectPublicKeyInfo(subjectPubkey, writer));

        // certificate extensions
        ReturnErrorOnFailure(EncodeExtensions(isCA, subjectPubkey, issuerPubkey, requestParams.FutureExt, writer));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR NewChipX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                           Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Writer writer;
    writer.Init(x509Cert);

    ReturnErrorOnFailure(EncodeTBSCert(requestParams, subjectPubkey, issuerKeypair.Pubkey(), writer));

    Crypto::P256ECDSASignature signature;
    ReturnErrorOnFailure(issuerKeypair.ECDSA_sign_msg(x509Cert.data(), writer.GetLengthWritten(), signature));

    writer.Init(x509Cert);

    ASN1_START_SEQUENCE
    {
        ReturnErrorOnFailure(EncodeTBSCert(requestParams, subjectPubkey, issuerKeypair.Pubkey(), writer));

        ASN1_START_SEQUENCE { ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256); }
        ASN1_END_SEQUENCE;

        ReturnErrorOnFailure(EncodeChipECDSASignature(signature, writer));
    }
    ASN1_END_SEQUENCE;

    x509Cert.reduce_size(writer.GetLengthWritten());

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR NewRootX509Cert(const X509CertRequestParams & requestParams, Crypto::P256Keypair & issuerKeypair,
                                      MutableByteSpan & x509Cert)
{
    uint8_t certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_Root, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(requestParams.SubjectDN.IsEqual(requestParams.IssuerDN), CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, issuerKeypair.Pubkey(), issuerKeypair, x509Cert);
}

DLL_EXPORT CHIP_ERROR NewICAX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                                     Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert)
{
    uint8_t certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_ICA, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.IssuerDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_Root, CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, subjectPubkey, issuerKeypair, x509Cert);
}

DLL_EXPORT CHIP_ERROR NewNodeOperationalX509Cert(const X509CertRequestParams & requestParams,
                                                 const Crypto::P256PublicKey & subjectPubkey, Crypto::P256Keypair & issuerKeypair,
                                                 MutableByteSpan & x509Cert)
{
    uint8_t certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_Node, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.IssuerDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_ICA || certType == kCertType_Root, CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, subjectPubkey, issuerKeypair, x509Cert);
}

} // namespace Credentials
} // namespace chip
