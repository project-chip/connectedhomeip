/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#include "ProvisionCrypto.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <mbedtls/asn1.h>
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>
#include <mbedtls/x509_csr.h>
#include <platform/silabs/SilabsConfig.h>

#if defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
#include <tinycrypt/ecc_dsa.h>
#endif

#include <cstdio>
#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

constexpr size_t kDeviceAttestationKeySizeMax = 128;

struct ScopedPkContext
{
    ScopedPkContext() { mbedtls_pk_init(&context); }
    ~ScopedPkContext() { mbedtls_pk_free(&context); }
    mbedtls_pk_context context;
};

struct ScopedCsrContext
{
    ScopedCsrContext() { mbedtls_x509write_csr_init(&context); }
    ~ScopedCsrContext() { mbedtls_x509write_csr_free(&context); }
    mbedtls_x509write_csr context;
};

#ifndef SLI_SI91X_MCU_INTERFACE
constexpr size_t kSubjectNameLengthMax = 160;
int GetRandom(void *, unsigned char * output, size_t size)
{
    return chip::Crypto::DRBG_get_bytes(output, size) == CHIP_NO_ERROR ? 0 : -1;
}

CHIP_ERROR FormatMatterOidUtf8DerHex(char * destination, size_t destinationSize, uint16_t value)
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%04X", value);
    const int written =
        snprintf(destination, destinationSize, "#0C04%02X%02X%02X%02X", static_cast<unsigned char>(hex[0]),
                 static_cast<unsigned char>(hex[1]), static_cast<unsigned char>(hex[2]), static_cast<unsigned char>(hex[3]));
    return written == 13 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}
#endif // SLI_SI91X_MCU_INTERFACE

CHIP_ERROR ReadKey(MutableByteSpan & key)
{
    size_t size = 0;
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, key.data(), key.size(), size));
    key.reduce_size(size);
    return CHIP_NO_ERROR;
}

#if defined(SLI_SI91X_MCU_INTERFACE) && defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
int UnwrapKeyCallback(void * context, int tag, unsigned char * value, size_t size)
{
    if (tag == MBEDTLS_ASN1_OCTET_STRING)
    {
        auto * key = static_cast<MutableByteSpan *>(context);
        if (size > key->size())
        {
            return MBEDTLS_ERR_ASN1_BUF_TOO_SMALL;
        }
        memcpy(key->data(), value, size);
        key->reduce_size(size);
    }
    return 0;
}

CHIP_ERROR UnwrapKey(const ByteSpan & encoded, MutableByteSpan & key)
{
    uint8_t * current = const_cast<uint8_t *>(encoded.data());
    uint8_t * end     = current + encoded.size();
    const int error   = mbedtls_asn1_traverse_sequence_of(&current, end, 0, 0, 0, 0, UnwrapKeyCallback, &key);
    return error == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}
#else
CHIP_ERROR ConvertAsn1Signature(const ByteSpan & encoded, MutableByteSpan & signature)
{
    VerifyOrReturnError(signature.size() >= 64, CHIP_ERROR_BUFFER_TOO_SMALL);
    uint8_t * current     = const_cast<uint8_t *>(encoded.data());
    const uint8_t * end   = current + encoded.size();
    size_t sequenceLength = 0;
    VerifyOrReturnError(mbedtls_asn1_get_tag(&current, end, &sequenceLength, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) == 0,
                        CHIP_ERROR_INTERNAL);

    for (size_t component = 0; component < 2; ++component)
    {
        size_t integerLength = 0;
        VerifyOrReturnError(mbedtls_asn1_get_tag(&current, end, &integerLength, MBEDTLS_ASN1_INTEGER) == 0, CHIP_ERROR_INTERNAL);
        while (integerLength > 32 && *current == 0)
        {
            ++current;
            --integerLength;
        }
        VerifyOrReturnError(integerLength <= 32 && current + integerLength <= end, CHIP_ERROR_INTERNAL);
        uint8_t * destination = signature.data() + component * 32;
        memset(destination, 0, 32 - integerLength);
        memcpy(destination + 32 - integerLength, current, integerLength);
        current += integerLength;
    }
    signature.reduce_size(64);
    return CHIP_NO_ERROR;
}
#endif
} // namespace

ProvisionCrypto & ProvisionCrypto::GetInstance()
{
    static ProvisionCrypto instance;
    return instance;
}

CHIP_ERROR ProvisionCrypto::GenerateRandom(MutableByteSpan & output)
{
    return chip::Crypto::DRBG_get_bytes(output.data(), output.size());
}

CHIP_ERROR ProvisionCrypto::Hash256(const ByteSpan & input, MutableByteSpan & output)
{
    VerifyOrReturnError(output.size() >= chip::Crypto::kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(chip::Crypto::Hash_SHA256(input.data(), input.size(), output.data()));
    output.reduce_size(chip::Crypto::kSHA256_Hash_Length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionCrypto::ImportDeviceAttestationKey(const ByteSpan & key)
{
    VerifyOrReturnError(!key.empty() && key.size() <= kDeviceAttestationKeySizeMax, CHIP_ERROR_INVALID_ARGUMENT);
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, key.data(), key.size());
}

CHIP_ERROR ProvisionCrypto::GenerateDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & commonName,
                                                         MutableCharSpan & csr)
{
#if defined(SLI_SI91X_MCU_INTERFACE)
    (void) vid;
    (void) pid;
    (void) commonName;
    (void) csr;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else
    VerifyOrReturnError(csr.data() != nullptr && csr.size() >= 512, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(commonName.size() <= 64, CHIP_ERROR_INVALID_ARGUMENT);

    char commonNameBuffer[65] = { 0 };
    if (!commonName.empty())
    {
        memcpy(commonNameBuffer, commonName.data(), commonName.size());
    }
    else
    {
        memcpy(commonNameBuffer, "Matter Device", sizeof("Matter Device"));
    }

    char vidDer[16];
    char pidDer[16];
    ReturnErrorOnFailure(FormatMatterOidUtf8DerHex(vidDer, sizeof(vidDer), vid));
    ReturnErrorOnFailure(FormatMatterOidUtf8DerHex(pidDer, sizeof(pidDer), pid));
    char subjectName[kSubjectNameLengthMax] = { 0 };
    const int subjectLength =
        snprintf(subjectName, sizeof(subjectName), "CN=%s, 1.3.6.1.4.1.37244.2.1=%s, 1.3.6.1.4.1.37244.2.2=%s", commonNameBuffer,
                 vidDer, pidDer);
    VerifyOrReturnError(subjectLength > 0 && static_cast<size_t>(subjectLength) < sizeof(subjectName), CHIP_ERROR_INTERNAL);

    // Declared before csrWriter so the key outlives the CSR writer that references it.
    ScopedPkContext key;
    ScopedCsrContext csrWriter;

    VerifyOrReturnError(mbedtls_pk_setup(&key.context, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mbedtls_ecdsa_genkey(mbedtls_pk_ec(key.context), MBEDTLS_ECP_DP_SECP256R1, GetRandom, nullptr) == 0,
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mbedtls_x509write_csr_set_subject_name(&csrWriter.context, subjectName) == 0, CHIP_ERROR_INTERNAL);
    mbedtls_x509write_csr_set_md_alg(&csrWriter.context, MBEDTLS_MD_SHA256);

    mbedtls_x509write_csr_set_key(&csrWriter.context, &key.context);
    VerifyOrReturnError(
        mbedtls_x509write_csr_pem(&csrWriter.context, reinterpret_cast<uint8_t *>(csr.data()), csr.size(), GetRandom, nullptr) == 0,
        CHIP_ERROR_INTERNAL);
    csr.reduce_size(strlen(csr.data()) + 1);

    uint8_t encodedKey[kDeviceAttestationKeySizeMax] = { 0 };
    const int encodedSize                            = mbedtls_pk_write_key_der(&key.context, encodedKey, sizeof(encodedKey));
    VerifyOrReturnError(encodedSize > 0 && static_cast<size_t>(encodedSize) <= sizeof(encodedKey), CHIP_ERROR_INTERNAL);
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, encodedKey + sizeof(encodedKey) - encodedSize,
                                             encodedSize);
#endif
}

CHIP_ERROR ProvisionCrypto::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    uint8_t encodedKey[kDeviceAttestationKeySizeMax] = { 0 };
    MutableByteSpan encodedKeySpan(encodedKey);
    ReturnErrorOnFailure(ReadKey(encodedKeySpan));

#if defined(SLI_SI91X_MCU_INTERFACE) && defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
    uint8_t privateKey[32] = { 0 };
    MutableByteSpan privateKeySpan(privateKey);
    ReturnErrorOnFailure(UnwrapKey(encodedKeySpan, privateKeySpan));
    VerifyOrReturnError(privateKeySpan.size() == sizeof(privateKey), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signature.size() >= 64, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t hash[chip::Crypto::kSHA256_Hash_Length] = { 0 };
    VerifyOrReturnError(mbedtls_sha256(message.data(), message.size(), hash, 0) == 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(uECC_sign(privateKey, hash, sizeof(hash), signature.data()) == UECC_SUCCESS, CHIP_ERROR_INTERNAL);
    signature.reduce_size(64);
    return CHIP_NO_ERROR;
#else
    uint8_t hash[chip::Crypto::kSHA256_Hash_Length]                     = { 0 };
    uint8_t encodedSignature[chip::Crypto::kMax_ECDSA_Signature_Length] = { 0 };
    size_t encodedSignatureSize                                         = 0;
    VerifyOrReturnError(mbedtls_sha256(message.data(), message.size(), hash, 0) == 0, CHIP_ERROR_INTERNAL);

    ScopedPkContext key;
    VerifyOrReturnError(
        mbedtls_pk_parse_key(&key.context, encodedKeySpan.data(), encodedKeySpan.size(), nullptr, 0, GetRandom, nullptr) == 0,
        CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mbedtls_pk_sign(&key.context, MBEDTLS_MD_SHA256, hash, sizeof(hash), encodedSignature,
                                        sizeof(encodedSignature), &encodedSignatureSize, GetRandom, nullptr) == 0,
                        CHIP_ERROR_INTERNAL);
    return ConvertAsn1Signature(ByteSpan(encodedSignature, encodedSignatureSize), signature);
#endif
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
