/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#include "ProvisionCrypto.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <mbedtls/asn1.h>
#include <mbedtls/pk.h>
#include <mbedtls/x509_csr.h>
#include <psa/crypto.h>
#include <sl_psa_crypto.h>
#if defined(__ZEPHYR__)
#include <platform/Zephyr/ZephyrConfig.h>
#include <provision/zephyr/ProvisionStorageZephyr.h>
#else
#include <platform/silabs/SilabsConfig.h>
#endif

#if defined(SLI_SI91X_MCU_INTERFACE) && !defined(__ZEPHYR__)
#include <sl_si91x_psa_wrap.h>
#endif

#include <cstdio>
#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
#if defined(__ZEPHYR__)
using ConfigStore = chip::DeviceLayer::Internal::ZephyrConfig;
#else
using ConfigStore = chip::DeviceLayer::Internal::SilabsConfig;
#endif

constexpr uint32_t kDefaultDeviceAttestationKeyId = 2;
constexpr size_t kSubjectNameLengthMax            = 160;

#if defined(__ZEPHYR__)
const char * DeviceAttestationKeyIdKey()
{
    return ZephyrStorage::kConfigKeyDacKeyId;
}
#else
ConfigStore::Key DeviceAttestationKeyIdKey()
{
    return ConfigStore::kConfigKey_Creds_KeyId;
}
#endif

// Key id used for import and CSR generation: the id already stored in config, or the default.
uint32_t ResolveDeviceAttestationKeyId()
{
    uint32_t keyId = 0;
    if (ConfigStore::ReadConfigValue(DeviceAttestationKeyIdKey(), keyId) == CHIP_NO_ERROR && keyId != 0)
    {
        return keyId;
    }
    return kDefaultDeviceAttestationKeyId;
}

CHIP_ERROR StoreDeviceAttestationKeyId(uint32_t keyId)
{
    return ConfigStore::WriteConfigValue(DeviceAttestationKeyIdKey(), keyId);
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

void ConfigureKeyAttributes(psa_key_attributes_t & attributes, uint32_t keyId)
{
    psa_set_key_id(&attributes, keyId);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes,
                            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE |
                                PSA_KEY_USAGE_VERIFY_MESSAGE);
#if defined(SLI_SI91X_MCU_INTERFACE) && !defined(__ZEPHYR__)
    psa_set_key_lifetime(
        &attributes,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#else
    psa_set_key_lifetime(
        &attributes,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, sl_psa_get_most_secure_key_location()));
#endif
}

psa_status_t GenerateKey(uint32_t keyId)
{
    (void) psa_destroy_key(static_cast<psa_key_id_t>(keyId));
    psa_key_attributes_t attributes = psa_key_attributes_init();
    ConfigureKeyAttributes(attributes, keyId);
    psa_key_id_t generatedId = 0;
    const psa_status_t error = psa_generate_key(&attributes, &generatedId);
    psa_reset_key_attributes(&attributes);
    return error;
}

psa_status_t ImportKey(uint32_t keyId, const uint8_t * value, size_t size)
{
    (void) psa_destroy_key(static_cast<psa_key_id_t>(keyId));
    psa_key_attributes_t attributes = psa_key_attributes_init();
    ConfigureKeyAttributes(attributes, keyId);
    psa_key_id_t importedId  = 0;
    const psa_status_t error = psa_import_key(&attributes, value, size, &importedId);
    psa_reset_key_attributes(&attributes);
    return error;
}

struct ImportKeyContext
{
    uint32_t keyId;
    bool imported;
};

int ImportKeyCallback(void * context, int tag, unsigned char * value, size_t size)
{
    if (tag != MBEDTLS_ASN1_OCTET_STRING)
    {
        return 0;
    }
    auto * importContext      = static_cast<ImportKeyContext *>(context);
    const psa_status_t status = ImportKey(importContext->keyId, value, size);
    importContext->imported   = status == PSA_SUCCESS;
    return static_cast<int>(status);
}
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
    VerifyOrReturnError(!key.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    uint8_t * current = const_cast<uint8_t *>(key.data());
    uint8_t * end     = current + key.size();
    ImportKeyContext context{ ResolveDeviceAttestationKeyId(), false };
    const int error = mbedtls_asn1_traverse_sequence_of(&current, end, 0, 0, 0, 0, ImportKeyCallback, &context);
    VerifyOrReturnError(error == 0 && context.imported, CHIP_ERROR_INTERNAL);
    return StoreDeviceAttestationKeyId(context.keyId);
}

CHIP_ERROR ProvisionCrypto::GenerateDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & commonName,
                                                         MutableCharSpan & csr)
{
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

    const uint32_t keyId = ResolveDeviceAttestationKeyId();

    mbedtls_pk_context keyContext;
    mbedtls_x509write_csr csrContext;
    mbedtls_pk_init(&keyContext);
    mbedtls_x509write_csr_init(&csrContext);

    CHIP_ERROR result = CHIP_ERROR_INTERNAL;
    int error         = mbedtls_x509write_csr_set_subject_name(&csrContext, subjectName);
    VerifyOrExit(error == 0, result = CHIP_ERROR_INTERNAL);
    mbedtls_x509write_csr_set_md_alg(&csrContext, MBEDTLS_MD_SHA256);

    VerifyOrExit(GenerateKey(keyId) == PSA_SUCCESS, result = CHIP_ERROR_INTERNAL);
#if MBEDTLS_VERSION_MAJOR >= 4
    error = mbedtls_pk_wrap_psa(&keyContext, static_cast<mbedtls_svc_key_id_t>(keyId));
#else
    error = mbedtls_pk_setup_opaque(&keyContext, keyId);
#endif
    VerifyOrExit(error == 0, result = CHIP_ERROR_INTERNAL);
    mbedtls_x509write_csr_set_key(&csrContext, &keyContext);

#if MBEDTLS_VERSION_MAJOR >= 4
    error = mbedtls_x509write_csr_pem(&csrContext, reinterpret_cast<uint8_t *>(csr.data()), csr.size());
#else
    error = mbedtls_x509write_csr_pem(&csrContext, reinterpret_cast<uint8_t *>(csr.data()), csr.size(), nullptr, nullptr);
#endif
    VerifyOrExit(error == 0, result = CHIP_ERROR_INTERNAL);
    csr.reduce_size(strlen(csr.data()) + 1);
    result = StoreDeviceAttestationKeyId(keyId);

exit:
    mbedtls_x509write_csr_free(&csrContext);
    mbedtls_pk_free(&keyContext);
    return result;
}

CHIP_ERROR ProvisionCrypto::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    VerifyOrReturnError(signature.size() >= 64, CHIP_ERROR_BUFFER_TOO_SMALL);

    // No stored key id means no provisioned DAC key; callers fall back to example credentials on NOT_FOUND.
    VerifyOrReturnError(ConfigStore::ConfigValueExists(DeviceAttestationKeyIdKey()), CHIP_ERROR_NOT_FOUND);
    uint32_t keyId = 0;
    ReturnErrorOnFailure(ConfigStore::ReadConfigValue(DeviceAttestationKeyIdKey(), keyId));

    size_t signatureSize      = 0;
    const psa_status_t status = psa_sign_message(static_cast<psa_key_id_t>(keyId), PSA_ALG_ECDSA(PSA_ALG_SHA_256), message.data(),
                                                 message.size(), signature.data(), signature.size(), &signatureSize);
    if (status == PSA_ERROR_INVALID_HANDLE || status == PSA_ERROR_DOES_NOT_EXIST)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(signatureSize == 64, CHIP_ERROR_INTERNAL);
    signature.reduce_size(signatureSize);
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
