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

#include <algorithm>
#include <initializer_list>
#include <inttypes.h>
#include <stddef.h>

#include <credentials/CHIPCert_Internal.h>
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
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_AuthorityKeyIdentifier);

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
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_SubjectKeyIdentifier);

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

CHIP_ERROR EncodeExtKeyUsageExtension(std::initializer_list<OID> keyPurposeOIDs, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_ExtendedKeyUsage);

        // ExtKeyUsage extension MUST be marked as critical.
        ASN1_ENCODE_BOOLEAN(true);
        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                for (auto && oid : keyPurposeOIDs)
                {
                    ASN1_ENCODE_OBJECT_ID(oid);
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

CHIP_ERROR EncodeKeyUsageExtension(BitFlags<KeyUsageFlags> keyUsageFlags, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_KeyUsage);

        // KeyUsage extension MUST be marked as critical.
        ASN1_ENCODE_BOOLEAN(true);
        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_ENCODE_BIT_STRING(keyUsageFlags.Raw());
        }
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
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_BasicConstraints);

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
    ReturnErrorOnFailure(
        EncodeKeyUsageExtension(BitFlags<KeyUsageFlags>(KeyUsageFlags::kKeyCertSign, KeyUsageFlags::kCRLSign), writer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeNOCSpecificExtensions(ASN1Writer & writer)
{
    ReturnErrorOnFailure(EncodeIsCAExtension(kNotCACert, writer));
    ReturnErrorOnFailure(EncodeKeyUsageExtension(KeyUsageFlags::kDigitalSignature, writer));
    ReturnErrorOnFailure(EncodeExtKeyUsageExtension({ kOID_KeyPurpose_ClientAuth, kOID_KeyPurpose_ServerAuth }, writer));
    return CHIP_NO_ERROR;
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

CHIP_ERROR EncodeTBSCert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                         const Crypto::P256PublicKey & issuerPubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CertType certType;
    bool isCA;

    VerifyOrReturnError(requestParams.SerialNumber >= 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(requestParams.ValidityEnd == kNullCertTime || requestParams.ValidityEnd >= requestParams.ValidityStart,
                        CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    isCA = (certType == CertType::kICA || certType == CertType::kRoot);

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

        ASN1_START_SEQUENCE
        {
            ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        }
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

} // namespace

CHIP_ERROR EncodeNetworkIdentityTBSCert(const P256PublicKey & pubkey, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipDN issuerAndSubject;
    InitNetworkIdentitySubject(issuerAndSubject);

    ASN1_START_SEQUENCE
    {
        // version [0] EXPLICIT Version DEFAULT v1
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_ENCODE_INTEGER(2); // Version ::= INTEGER { v1(0), v2(1), v3(2) }
        }
        ASN1_END_CONSTRUCTED;

        ReturnErrorOnFailure(writer.PutInteger(kNetworkIdentitySerialNumber));

        ASN1_START_SEQUENCE
        {
            ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        }
        ASN1_END_SEQUENCE;

        // issuer Name
        ReturnErrorOnFailure(issuerAndSubject.EncodeToASN1(writer));

        // validity Validity,
        ReturnErrorOnFailure(EncodeValidity(kNetworkIdentityNotBeforeTime, kNetworkIdentityNotAfterTime, writer));

        // subject Name
        ReturnErrorOnFailure(issuerAndSubject.EncodeToASN1(writer));

        ReturnErrorOnFailure(EncodeSubjectPublicKeyInfo(pubkey, writer));

        // certificate extensions
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 3)
        {
            ASN1_START_SEQUENCE
            {
                EncodeIsCAExtension(kNotCACert, writer);
                EncodeKeyUsageExtension(KeyUsageFlags::kDigitalSignature, writer);
                EncodeExtKeyUsageExtension({ kOID_KeyPurpose_ClientAuth, kOID_KeyPurpose_ServerAuth }, writer);
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR NewChipX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                           const Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert)
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

        ASN1_START_SEQUENCE
        {
            ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        }
        ASN1_END_SEQUENCE;

        ReturnErrorOnFailure(EncodeChipECDSASignature(signature, writer));
    }
    ASN1_END_SEQUENCE;

    x509Cert.reduce_size(writer.GetLengthWritten());

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR NewRootX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256Keypair & issuerKeypair,
                                      MutableByteSpan & x509Cert)
{
    CertType certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == CertType::kRoot, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(requestParams.SubjectDN.IsEqual(requestParams.IssuerDN), CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, issuerKeypair.Pubkey(), issuerKeypair, x509Cert);
}

DLL_EXPORT CHIP_ERROR NewICAX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                                     const Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert)
{
    CertType certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == CertType::kICA, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.IssuerDN.GetCertType(certType));
    VerifyOrReturnError(certType == CertType::kRoot, CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, subjectPubkey, issuerKeypair, x509Cert);
}

DLL_EXPORT CHIP_ERROR NewNodeOperationalX509Cert(const X509CertRequestParams & requestParams,
                                                 const Crypto::P256PublicKey & subjectPubkey,
                                                 const Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert)
{
    CertType certType;

    ReturnErrorOnFailure(requestParams.SubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == CertType::kNode, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(requestParams.IssuerDN.GetCertType(certType));
    VerifyOrReturnError(certType == CertType::kICA || certType == CertType::kRoot, CHIP_ERROR_INVALID_ARGUMENT);

    return NewChipX509Cert(requestParams, subjectPubkey, issuerKeypair, x509Cert);
}

} // namespace Credentials
} // namespace chip
