/*
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      mbedTLS based implementation of CHIP crypto primitives related to certificate
 *      generation and validation.
 */

#include "CHIPCryptoPAL.h"
#include "CHIPCryptoPALmbedTLS.h"

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include <mbedtls/ecp.h>
#include <mbedtls/oid.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>

#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

namespace chip {
namespace Crypto {

CHIP_ERROR VerifyCertificateSigningRequest(const uint8_t * csr_buf, size_t csr_length, P256PublicKey & pubkey)
{
#if defined(MBEDTLS_X509_CSR_PARSE_C)
    ReturnErrorOnFailure(VerifyCertificateSigningRequestFormat(csr_buf, csr_length));

    // TODO: For some embedded targets, mbedTLS library doesn't have mbedtls_x509_csr_parse_der, and mbedtls_x509_csr_parse_free.
    //       Taking a step back, embedded targets likely will not process CSR requests. Adding this action item to reevaluate
    //       this if there's a need for this processing for embedded targets.
    CHIP_ERROR error   = CHIP_NO_ERROR;
    size_t pubkey_size = 0;

    mbedtls_ecp_keypair * keypair = nullptr;

    P256ECDSASignature signature;
    MutableByteSpan out_raw_sig_span(signature.Bytes(), signature.Capacity());

    mbedtls_x509_csr csr;
    mbedtls_x509_csr_init(&csr);

    int result = mbedtls_x509_csr_parse_der(&csr, csr_buf, csr_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Verify the signature algorithm and public key type
    VerifyOrExit(csr.CHIP_CRYPTO_PAL_PRIVATE(sig_md) == MBEDTLS_MD_SHA256, error = CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);
    VerifyOrExit(csr.CHIP_CRYPTO_PAL_PRIVATE(sig_pk) == MBEDTLS_PK_ECDSA, error = CHIP_ERROR_WRONG_KEY_TYPE);

    keypair = mbedtls_pk_ec(csr.CHIP_CRYPTO_PAL_PRIVATE_X509(pk));

    // Copy the public key from the CSR
    result = mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                            MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(pubkey), pubkey.Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

    // Convert DER signature to raw signature
    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length,
                                    ByteSpan{ csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE_X509(p),
                                              csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE_X509(len) },
                                    out_raw_sig_span);

    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_raw_sig_span.size() == (kP256_FE_Length * 2), error = CHIP_ERROR_INTERNAL);
    signature.SetLength(out_raw_sig_span.size());

    // Verify the signature using the public key
    error = pubkey.ECDSA_validate_msg_signature(csr.CHIP_CRYPTO_PAL_PRIVATE_X509(cri).CHIP_CRYPTO_PAL_PRIVATE_X509(p),
                                                csr.CHIP_CRYPTO_PAL_PRIVATE_X509(cri).CHIP_CRYPTO_PAL_PRIVATE_X509(len), signature);

    SuccessOrExit(error);

exit:
    mbedtls_x509_csr_free(&csr);
    _log_mbedTLS_error(result);
    return error;
#else
    ChipLogError(Crypto, "MBEDTLS_X509_CSR_PARSE_C is not enabled. CSR cannot be parsed");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

namespace {

#if defined(MBEDTLS_X509_CRT_PARSE_C)
bool IsTimeGreaterThanEqual(const mbedtls_x509_time * const timeA, const mbedtls_x509_time * const timeB)
{

    // checks if two values are different and if yes, then returns first > second.
#define RETURN_STRICTLY_GREATER_IF_DIFFERENT(component)                                                                            \
    {                                                                                                                              \
        auto valueA = timeA->CHIP_CRYPTO_PAL_PRIVATE_X509(component);                                                              \
        auto valueB = timeB->CHIP_CRYPTO_PAL_PRIVATE_X509(component);                                                              \
                                                                                                                                   \
        if (valueA != valueB)                                                                                                      \
        {                                                                                                                          \
            return valueA > valueB;                                                                                                \
        }                                                                                                                          \
    }

    RETURN_STRICTLY_GREATER_IF_DIFFERENT(year);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(mon);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(day);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(hour);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(min);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(sec);

    // all above are equal
    return true;
}

CHIP_ERROR IsCertificateValidAtIssuance(const mbedtls_x509_crt * candidateCertificate, const mbedtls_x509_crt * issuerCertificate)
{
    mbedtls_x509_time candidateNotBeforeTime = candidateCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    mbedtls_x509_time issuerNotBeforeTime    = issuerCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    mbedtls_x509_time issuerNotAfterTime     = issuerCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_to);

    // check if candidateCertificate is issued at or after issuerCertificate's notBefore timestamp
    VerifyOrReturnError(IsTimeGreaterThanEqual(&candidateNotBeforeTime, &issuerNotBeforeTime), CHIP_ERROR_CERT_EXPIRED);

    // check if candidateCertificate is issued at or before issuerCertificate's notAfter timestamp
    VerifyOrReturnError(IsTimeGreaterThanEqual(&issuerNotAfterTime, &candidateNotBeforeTime), CHIP_ERROR_CERT_EXPIRED);

    return CHIP_NO_ERROR;
}

int CallbackForCustomValidityCheck(void * data, mbedtls_x509_crt * crt, int depth, uint32_t * flags)
{
    mbedtls_x509_crt * leafCert   = reinterpret_cast<mbedtls_x509_crt *>(data);
    mbedtls_x509_crt * issuerCert = crt;

    // Ignore any time validy error performed by the standard mbedTLS code.
    *flags &= ~(static_cast<uint32_t>(MBEDTLS_X509_BADCERT_EXPIRED | MBEDTLS_X509_BADCERT_FUTURE));

    // Verify that the leaf certificate has a notBefore time valid within the validity period of the issuerCertificate.
    // Note that this callback is invoked for each certificate in the chain.
    if (IsCertificateValidAtIssuance(leafCert, issuerCert) != CHIP_NO_ERROR)
    {
        return MBEDTLS_ERR_X509_INVALID_DATE;
    }

    return 0;
}

constexpr uint8_t sOID_AttributeType_CommonName[]         = { 0x55, 0x04, 0x03 };
constexpr uint8_t sOID_AttributeType_MatterVendorId[]     = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x01 };
constexpr uint8_t sOID_AttributeType_MatterProductId[]    = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x02 };
constexpr uint8_t sOID_SigAlgo_ECDSAWithSHA256[]          = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 };
constexpr uint8_t sOID_Extension_BasicConstraints[]       = { 0x55, 0x1D, 0x13 };
constexpr uint8_t sOID_Extension_KeyUsage[]               = { 0x55, 0x1D, 0x0F };
constexpr uint8_t sOID_Extension_SubjectKeyIdentifier[]   = { 0x55, 0x1D, 0x0E };
constexpr uint8_t sOID_Extension_AuthorityKeyIdentifier[] = { 0x55, 0x1D, 0x23 };
constexpr uint8_t sOID_Extension_CRLDistributionPoint[]   = { 0x55, 0x1D, 0x1F };

/**
 * Compares an mbedtls_asn1_buf structure (oidBuf) to a reference OID represented as uint8_t array (oid).
 */
#define OID_CMP(oid, oidBuf)                                                                                                       \
    ((MBEDTLS_ASN1_OID == (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(tag)) &&                                                           \
     (sizeof(oid) == (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(len)) &&                                                                \
     (memcmp((oid), (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(p), (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(len)) == 0))

#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

} // anonymous namespace

CHIP_ERROR VerifyAttestationCertificateFormat(const ByteSpan & cert, AttestationCertType certType)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    bool extBasicPresent      = false;
    bool extKeyUsagePresent   = false;

    VerifyOrReturnError(!cert.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(cert.data()), cert.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // "version" value is 1 higher than the actual encoded value.
    VerifyOrExit(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(version) - 1 == 2, error = CHIP_ERROR_INTERNAL);

    // Verify signature algorithms is ECDSA with SHA256.
    VerifyOrExit(OID_CMP(sOID_SigAlgo_ECDSAWithSHA256, mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(sig_oid)),
                 error = CHIP_ERROR_INTERNAL);

    // Verify public key presence and format.
    {
        Crypto::P256PublicKey pubkey;
        SuccessOrExit(error = ExtractPubkeyFromX509Cert(cert, pubkey));
    }

    p      = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end    = p + mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    while (p < end)
    {
        mbedtls_x509_buf extOID = { 0, 0, nullptr };
        int extCritical         = 0;

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        /* Get extension ID */
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(tag) = MBEDTLS_ASN1_OID;
        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(len) = len;
        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(p)   = p;
        p += len;

        /* Get optional critical */
        result = mbedtls_asn1_get_bool(&p, end, &extCritical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_INTERNAL);

        /* Data should be octet string type */
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        if (OID_CMP(sOID_Extension_BasicConstraints, extOID))
        {
            int isCA    = 0;
            int pathLen = -1;

            VerifyOrExit(extCritical, error = CHIP_ERROR_INTERNAL);
            extBasicPresent = true;

            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
            if (len > 0)
            {
                unsigned char * seqStart = p;
                result                   = mbedtls_asn1_get_bool(&p, end, &isCA);
                VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_INTERNAL);

                // Check if pathLen is there by validating if the cursor didn't get to the end of
                // of the internal SEQUENCE for the basic constraints encapsulation.
                // Missing pathLen optional tag will leave pathLen == -1 for following checks.
                bool hasPathLen = (p != (seqStart + len));
                if (hasPathLen)
                {
                    // Extract pathLen value, making sure it's a valid format.
                    result = mbedtls_asn1_get_int(&p, end, &pathLen);
                    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
                }
            }

            if (certType == AttestationCertType::kDAC)
            {
                VerifyOrExit(!isCA && pathLen == -1, error = CHIP_ERROR_INTERNAL);
            }
            else if (certType == AttestationCertType::kPAI)
            {
                VerifyOrExit(isCA && pathLen == 0, error = CHIP_ERROR_INTERNAL);
            }
            else
            {
                // For PAA, pathlen must be absent or equal to 1 (see Matter 1.1 spec 6.2.2.5)
                VerifyOrExit(isCA && (pathLen == -1 || pathLen == 1), error = CHIP_ERROR_INTERNAL);
            }
        }
        else if (OID_CMP(sOID_Extension_KeyUsage, extOID))
        {
            mbedtls_x509_bitstring bs = { 0, 0, nullptr };
            unsigned int keyUsage     = 0;

            VerifyOrExit(extCritical, error = CHIP_ERROR_INTERNAL);
            extKeyUsagePresent = true;

            result = mbedtls_asn1_get_bitstring(&p, p + len, &bs);
            VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

            for (size_t i = 0; i < bs.CHIP_CRYPTO_PAL_PRIVATE_X509(len) && i < sizeof(unsigned int); i++)
            {
                keyUsage |= static_cast<unsigned int>(bs.CHIP_CRYPTO_PAL_PRIVATE_X509(p)[i]) << (8 * i);
            }

            if (certType == AttestationCertType::kDAC)
            {
                // SHALL only have the digitalSignature bit set.
                VerifyOrExit(keyUsage == MBEDTLS_X509_KU_DIGITAL_SIGNATURE, error = CHIP_ERROR_INTERNAL);
            }
            else
            {
                bool keyCertSignFlag = keyUsage & MBEDTLS_X509_KU_KEY_CERT_SIGN;
                bool crlSignFlag     = keyUsage & MBEDTLS_X509_KU_CRL_SIGN;
                bool otherFlags      = keyUsage &
                    ~static_cast<unsigned int>(MBEDTLS_X509_KU_CRL_SIGN | MBEDTLS_X509_KU_KEY_CERT_SIGN |
                                               MBEDTLS_X509_KU_DIGITAL_SIGNATURE);
                VerifyOrExit(keyCertSignFlag && crlSignFlag && !otherFlags, error = CHIP_ERROR_INTERNAL);
            }
        }
        else
        {
            p += len;
        }
    }

    // Verify basic and key usage extensions are present.
    VerifyOrExit(extBasicPresent && extKeyUsagePresent, error = CHIP_ERROR_INTERNAL);

    // Verify that SKID and AKID extensions are present.
    {
        uint8_t kidBuf[kSubjectKeyIdentifierLength];
        MutableByteSpan kid(kidBuf);
        SuccessOrExit(error = ExtractSKIDFromX509Cert(cert, kid));
        if (certType == AttestationCertType::kDAC || certType == AttestationCertType::kPAI)
        {
            // Mandatory extension for DAC and PAI certs.
            SuccessOrExit(error = ExtractAKIDFromX509Cert(cert, kid));
        }
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) cert;
    (void) certType;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen,
                                    CertificateChainValidationResult & result)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt certChain;
    mbedtls_x509_crt rootCert;
    int mbedResult;
    uint32_t flags = 0;

    result = CertificateChainValidationResult::kInternalFrameworkError;

    VerifyOrReturnError(rootCertificate != nullptr && rootCertificateLen != 0,
                        (result = CertificateChainValidationResult::kRootArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(leafCertificate != nullptr && leafCertificateLen != 0,
                        (result = CertificateChainValidationResult::kLeafArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));

    mbedtls_x509_crt_init(&certChain);
    mbedtls_x509_crt_init(&rootCert);

    /* Start of chain  */
    mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(leafCertificate), leafCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kLeafFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Add the intermediate to the chain, if present */
    if (caCertificate != nullptr && caCertificateLen > 0)
    {
        mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(caCertificate), caCertificateLen);
        VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kICAFormatInvalid, error = CHIP_ERROR_INTERNAL));
    }

    /* Parse the root cert */
    mbedResult = mbedtls_x509_crt_parse(&rootCert, Uint8::to_const_uchar(rootCertificate), rootCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kRootFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Verify the chain against the root */
    mbedResult =
        mbedtls_x509_crt_verify(&certChain, &rootCert, nullptr, nullptr, &flags, CallbackForCustomValidityCheck, &certChain);

    switch (mbedResult)
    {
    case 0:
        VerifyOrExit(flags == 0, (result = CertificateChainValidationResult::kInternalFrameworkError, error = CHIP_ERROR_INTERNAL));
        result = CertificateChainValidationResult::kSuccess;
        break;
    case MBEDTLS_ERR_X509_INVALID_DATE:
    case MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:
        result = CertificateChainValidationResult::kChainInvalid;
        error  = CHIP_ERROR_CERT_NOT_TRUSTED;
        break;
    default:
        result = CertificateChainValidationResult::kInternalFrameworkError;
        error  = CHIP_ERROR_INTERNAL;
        break;
    }

exit:
    _log_mbedTLS_error(mbedResult);
    mbedtls_x509_crt_free(&certChain);
    mbedtls_x509_crt_free(&rootCert);

#else
    (void) rootCertificate;
    (void) rootCertificateLen;
    (void) caCertificate;
    (void) caCertificateLen;
    (void) leafCertificate;
    (void) leafCertificateLen;
    (void) result;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR IsCertificateValidAtIssuance(const ByteSpan & candidateCertificate, const ByteSpan & issuerCertificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbedCandidateCertificate;
    mbedtls_x509_crt mbedIssuerCertificate;
    int result;

    VerifyOrReturnError(!candidateCertificate.empty() && !issuerCertificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCandidateCertificate);
    mbedtls_x509_crt_init(&mbedIssuerCertificate);

    result = mbedtls_x509_crt_parse(&mbedCandidateCertificate, Uint8::to_const_uchar(candidateCertificate.data()),
                                    candidateCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result =
        mbedtls_x509_crt_parse(&mbedIssuerCertificate, Uint8::to_const_uchar(issuerCertificate.data()), issuerCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Verify that the candidateCertificate has a notBefore time valid within the validity period of the issuerCertificate.
    SuccessOrExit(error = IsCertificateValidAtIssuance(&mbedCandidateCertificate, &mbedIssuerCertificate));

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCandidateCertificate);
    mbedtls_x509_crt_free(&mbedIssuerCertificate);

#else
    (void) candidateCertificate;
    (void) issuerCertificate;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR IsCertificateValidAtCurrentTime(const ByteSpan & certificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbedCertificate;
    int result;

    VerifyOrReturnError(!certificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCertificate);

    result = mbedtls_x509_crt_parse(&mbedCertificate, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // check if certificate's notBefore timestamp is earlier than or equal to current time.
    result = mbedtls_x509_time_is_past(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from));
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

    // check if certificate's notAfter timestamp is later than current time.
    result = mbedtls_x509_time_is_future(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_to));
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCertificate);

#else
    (void) certificate;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    mbedtls_ecp_keypair * keypair = nullptr;
    size_t pubkey_size            = 0;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mbedtls_pk_get_type(&(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk))) == MBEDTLS_PK_ECKEY,
                 error = CHIP_ERROR_INVALID_ARGUMENT);

    keypair = mbedtls_pk_ec(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk));
    VerifyOrExit(keypair->CHIP_CRYPTO_PAL_PRIVATE(grp).id == MapECPGroupId(pubkey.Type()), error = CHIP_ERROR_INVALID_ARGUMENT);
    // Copy the public key from the cert in raw point format
    result =
        mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                       MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(pubkey.Bytes()), pubkey.Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) pubkey;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

namespace {

CHIP_ERROR ExtractKIDFromX509Cert(bool extractSKID, const ByteSpan & certificate, MutableByteSpan & kid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // TODO: The mbedTLS team is working on supporting SKID and AKID extensions processing.
    // Once it is supported, this code should be updated.

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID    = { MBEDTLS_ASN1_OID, len, p };
        bool extractCurrentExtSKID = extractSKID && OID_CMP(sOID_Extension_SubjectKeyIdentifier, extOID);
        bool extractCurrentExtAKID = !extractSKID && OID_CMP(sOID_Extension_AuthorityKeyIdentifier, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        if (extractCurrentExtSKID || extractCurrentExtAKID)
        {
            if (extractCurrentExtSKID)
            {
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
            }
            else
            {
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
                // Other optional fields, authorityCertIssuer and authorityCertSerialNumber,
                // will be skipped if present.
            }
            VerifyOrExit(len == kSubjectKeyIdentifierLength, error = CHIP_ERROR_WRONG_CERT_TYPE);
            VerifyOrExit(len <= kid.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(kid.data(), p, len);
            if (kid.size() > len)
            {
                kid.reduce_size(len);
            }
            ExitNow(error = CHIP_NO_ERROR);
            break;
        }
        p += len;
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) kid;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

} // namespace

CHIP_ERROR ExtractSKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & skid)
{
    return ExtractKIDFromX509Cert(true, certificate, skid);
}

CHIP_ERROR ExtractAKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & akid)
{
    return ExtractKIDFromX509Cert(false, certificate, akid);
}

CHIP_ERROR ExtractCRLDistributionPointURIFromX509Cert(const ByteSpan & certificate, MutableCharSpan & cdpurl)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    size_t cdpExtCount        = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID = { MBEDTLS_ASN1_OID, len, p };
        bool isCurrentExtCDP    = OID_CMP(sOID_Extension_CRLDistributionPoint, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        unsigned char * end_of_ext = p + len;

        if (isCurrentExtCDP)
        {
            // Only one CRL Distribution Point Extension is allowed.
            cdpExtCount++;
            VerifyOrExit(cdpExtCount <= 1, error = CHIP_ERROR_NOT_FOUND);

            // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
            //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
            //
            // This implementation only supports a single DistributionPoint (sequence of size 1),
            // which is verified by comparing (p + len == end_of_ext)
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPoint is a sequence of three optional elements:
            //     DistributionPoint ::= SEQUENCE {
            //         distributionPoint       [0]     DistributionPointName OPTIONAL,
            //         reasons                 [1]     ReasonFlags OPTIONAL,
            //         cRLIssuer               [2]     GeneralNames OPTIONAL }
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPointName is:
            //     DistributionPointName ::= CHOICE {
            //         fullName                [0]     GeneralNames,
            //         nameRelativeToCRLIssuer [1]     RelativeDistinguishedName }
            //
            // The URI should be encoded in the fullName element.
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 0);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            //     GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            unsigned char * end_of_general_names = p + len;

            // The CDP URI is encoded as a uniformResourceIdentifier field of the GeneralName:
            //     GeneralName ::= CHOICE {
            //         otherName                       [0]     OtherName,
            //         rfc822Name                      [1]     IA5String,
            //         dNSName                         [2]     IA5String,
            //         x400Address                     [3]     ORAddress,
            //         directoryName                   [4]     Name,
            //         ediPartyName                    [5]     EDIPartyName,
            //         uniformResourceIdentifier       [6]     IA5String,
            //         iPAddress                       [7]     OCTET STRING,
            //         registeredID                    [8]     OBJECT IDENTIFIER }
            result =
                mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_X509_SAN_UNIFORM_RESOURCE_IDENTIFIER);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            // Only single URI instance in the GeneralNames is supported
            VerifyOrExit(p + len == end_of_general_names, error = CHIP_ERROR_NOT_FOUND);

            const char * urlptr = reinterpret_cast<const char *>(p);
            VerifyOrExit((len > strlen(kValidCDPURIHttpPrefix) &&
                          strncmp(urlptr, kValidCDPURIHttpPrefix, strlen(kValidCDPURIHttpPrefix)) == 0) ||
                             (len > strlen(kValidCDPURIHttpsPrefix) &&
                              strncmp(urlptr, kValidCDPURIHttpsPrefix, strlen(kValidCDPURIHttpsPrefix)) == 0),
                         error = CHIP_ERROR_NOT_FOUND);
            error = CopyCharSpanToMutableCharSpan(CharSpan(urlptr, len), cdpurl);
            SuccessOrExit(error);
        }
        p = end_of_ext;
    }

    VerifyOrExit(cdpExtCount == 1, error = CHIP_ERROR_NOT_FOUND);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) cdpurl;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractCDPExtensionCRLIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & crlIssuer)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    size_t cdpExtCount        = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID = { MBEDTLS_ASN1_OID, len, p };
        bool isCurrentExtCDP    = OID_CMP(sOID_Extension_CRLDistributionPoint, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        unsigned char * end_of_ext = p + len;

        if (isCurrentExtCDP)
        {
            // Only one CRL Distribution Point Extension is allowed.
            cdpExtCount++;
            VerifyOrExit(cdpExtCount <= 1, error = CHIP_ERROR_NOT_FOUND);

            // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
            //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
            //
            // This implementation only supports a single DistributionPoint (sequence of size 1),
            // which is verified by comparing (p + len == end_of_ext)
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPoint is a sequence of three optional elements:
            //     DistributionPoint ::= SEQUENCE {
            //         distributionPoint       [0]     DistributionPointName OPTIONAL,
            //         reasons                 [1]     ReasonFlags OPTIONAL,
            //         cRLIssuer               [2]     GeneralNames OPTIONAL }
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // If distributionPoint element presents, ignore it
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 0);
            if (result == 0)
            {
                p += len;
                VerifyOrExit(p < end_of_ext, error = CHIP_ERROR_NOT_FOUND);
            }

            // Check if cRLIssuer element present
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 2);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            // The CRL Issuer is encoded as a directoryName field of the GeneralName:
            //     GeneralName ::= CHOICE {
            //         otherName                       [0]     OtherName,
            //         rfc822Name                      [1]     IA5String,
            //         dNSName                         [2]     IA5String,
            //         x400Address                     [3]     ORAddress,
            //         directoryName                   [4]     Name,
            //         ediPartyName                    [5]     EDIPartyName,
            //         uniformResourceIdentifier       [6]     IA5String,
            //         iPAddress                       [7]     OCTET STRING,
            //         registeredID                    [8]     OBJECT IDENTIFIER }
            result = mbedtls_asn1_get_tag(
                &p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_X509_SAN_DIRECTORY_NAME);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            error = CopySpanToMutableSpan(ByteSpan(p, len), crlIssuer);
            SuccessOrExit(error);
        }
        p = end_of_ext;
    }

    VerifyOrExit(cdpExtCount == 1, error = CHIP_ERROR_NOT_FOUND);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) crlIssuer;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractSerialNumberFromX509Cert(const ByteSpan & certificate, MutableByteSpan & serialNumber)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t * p      = nullptr;
    size_t len       = 0;
    mbedtls_x509_crt mbed_cert;

    mbedtls_x509_crt_init(&mbed_cert);

    result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(serial).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    len = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(serial).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    VerifyOrExit(len <= serialNumber.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(serialNumber.data(), p, len);
    serialNumber.reduce_size(len);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) serialNumber;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractVIDPIDFromX509Cert(const ByteSpan & certificate, AttestationCertVidPid & vidpid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    mbedtls_asn1_named_data * dnIterator = nullptr;
    AttestationCertVidPid vidpidFromCN;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    for (dnIterator = &mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(subject); dnIterator != nullptr;
         dnIterator = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(next))
    {
        DNAttrType attrType = DNAttrType::kUnspecified;
        if (OID_CMP(sOID_AttributeType_CommonName, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kCommonName;
        }
        else if (OID_CMP(sOID_AttributeType_MatterVendorId, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kMatterVID;
        }
        else if (OID_CMP(sOID_AttributeType_MatterProductId, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kMatterPID;
        }

        size_t val_len  = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        uint8_t * val_p = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
        error           = ExtractVIDPIDFromAttributeString(attrType, ByteSpan(val_p, val_len), vidpid, vidpidFromCN);
        SuccessOrExit(error);
    }

    // If Matter Attributes were not found use values extracted from the CN Attribute,
    // which might be uninitialized as well.
    if (!vidpid.Initialized())
    {
        vidpid = vidpidFromCN;
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) vidpid;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

namespace {
CHIP_ERROR ExtractRawDNFromX509Cert(bool extractSubject, const ByteSpan & certificate, MutableByteSpan & dn)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t * p      = nullptr;
    size_t len       = 0;
    mbedtls_x509_crt mbedCertificate;

    ReturnErrorCodeIf(certificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCertificate);
    result = mbedtls_x509_crt_parse(&mbedCertificate, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    if (extractSubject)
    {
        len = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(subject_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        p   = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(subject_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    }
    else
    {
        len = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(issuer_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        p   = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(issuer_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    }

    VerifyOrExit(len <= dn.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(dn.data(), p, len);
    dn.reduce_size(len);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCertificate);

#else
    (void) certificate;
    (void) dn;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}
} // namespace

CHIP_ERROR ExtractSubjectFromX509Cert(const ByteSpan & certificate, MutableByteSpan & subject)
{
    return ExtractRawDNFromX509Cert(true, certificate, subject);
}

CHIP_ERROR ExtractIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & issuer)
{
    return ExtractRawDNFromX509Cert(false, certificate, issuer);
}

CHIP_ERROR ReplaceCertIfResignedCertFound(const ByteSpan & referenceCertificate, const ByteSpan * candidateCertificates,
                                          size_t candidateCertificatesCount, ByteSpan & outCertificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    uint8_t referenceSubjectBuf[kMaxCertificateDistinguishedNameLength];
    uint8_t referenceSKIDBuf[kSubjectKeyIdentifierLength];
    MutableByteSpan referenceSubject(referenceSubjectBuf);
    MutableByteSpan referenceSKID(referenceSKIDBuf);

    outCertificate = referenceCertificate;

    ReturnErrorCodeIf(candidateCertificates == nullptr || candidateCertificatesCount == 0, CHIP_NO_ERROR);

    ReturnErrorOnFailure(ExtractSubjectFromX509Cert(referenceCertificate, referenceSubject));
    ReturnErrorOnFailure(ExtractSKIDFromX509Cert(referenceCertificate, referenceSKID));

    for (size_t i = 0; i < candidateCertificatesCount; i++)
    {
        const ByteSpan candidateCertificate = candidateCertificates[i];
        uint8_t candidateSubjectBuf[kMaxCertificateDistinguishedNameLength];
        uint8_t candidateSKIDBuf[kSubjectKeyIdentifierLength];
        MutableByteSpan candidateSubject(candidateSubjectBuf);
        MutableByteSpan candidateSKID(candidateSKIDBuf);

        ReturnErrorOnFailure(ExtractSubjectFromX509Cert(candidateCertificate, candidateSubject));
        ReturnErrorOnFailure(ExtractSKIDFromX509Cert(candidateCertificate, candidateSKID));

        if (referenceSKID.data_equal(candidateSKID) && referenceSubject.data_equal(candidateSubject))
        {
            outCertificate = candidateCertificate;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_NO_ERROR;
#else
    (void) referenceCertificate;
    (void) candidateCertificates;
    (void) candidateCertificatesCount;
    (void) outCertificate;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
}

} // namespace Crypto
} // namespace chip
