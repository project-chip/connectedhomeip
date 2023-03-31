#include "credentials.h"
#include "config.h"
#include "assert.h"
#include <em_msc.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>
#include <mbedtls/asn1.h>
#include <mbedtls/pk.h>
#include <stdio.h>
#include <string.h>

using namespace chip::DeviceLayer::Internal;

#if defined(SEMAILBOX_PRESENT) && (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_SL_SE_OPAQUE
#elif defined(CRYPTOACC_PRESENT) && defined(SEPUF_PRESENT) && defined(SL_TRUSTZONE_NONSECURE)
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_SL_CRYPTOACC_OPAQUE
#else
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_LOCAL_STORAGE
#endif

static constexpr uint32_t kCreds_KeyId_Default = (PSA_KEY_ID_USER_MIN + 1);


Credentials::Credentials()
{
    uint32_t key_id = 0;
    int err = Config::Read(SilabsConfig::kConfigKey_Creds_KeyId, key_id);
    _key_id = err ? kCreds_KeyId_Default : key_id;
}

Credentials::Credentials(psa_key_id_t kid)
{
    _key_id = kid ? kid : kCreds_KeyId_Default;
    Config::Write(SilabsConfig::kConfigKey_Creds_KeyId, _key_id);
}


int Credentials::destroyKey()
{
    psa_key_handle_t key_handle;

    int err = psa_open_key(_key_id, &key_handle);
    if (err)
    {
        psa_close_key(_key_id);
    }
    else
    {
        err = psa_destroy_key(_key_id);
    }
    return err;
}


int Credentials::generateKey()
{
    destroyKey();

    psa_key_attributes_t attr = psa_key_attributes_init();
    psa_set_key_id(&attr, this->_key_id);
    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, 256);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(
        &attr, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_lifetime(
        &attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_CRYPTO_LOCATION_FOR_DEVICE));

    int err = psa_generate_key(&attr, &this->_key_id);
    ASSERT(!err, return err, "psa_generate_key error %d", err);

    return 0;
}


int asn1_callback(void *ctx, int tag, unsigned char *value, size_t size)
{
    if(MBEDTLS_ASN1_OCTET_STRING == tag)
    {
        Credentials *creds = (Credentials*) ctx;
        return creds->ImportRawKey(value, size);
    }
    return 0;
}


int Credentials::importKey(const uint8_t * asn1, size_t size)
{
    uint8_t *p = (uint8_t *)asn1;
    int err = 0;

    err = mbedtls_asn1_traverse_sequence_of(&p, asn1 + size, 0, 0, 0, 0, asn1_callback, this);
    ASSERT(err, return err, "ASN1 decode errror");

    return 0;
}


int Credentials::ImportRawKey(const uint8_t * key, size_t size)
{
    int err = 0;
   
    destroyKey();

    psa_key_attributes_t attr = psa_key_attributes_init();
    psa_set_key_id(&attr, _key_id);
    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    // psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR());
    psa_set_key_bits(&attr, 256);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(
        &attr, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_lifetime(
        &attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_CRYPTO_LOCATION_FOR_DEVICE));

    err = psa_import_key(&attr, key, size, &_key_id);
    psa_reset_key_attributes(&attr);
    ASSERT(err, return err, "ASN1 decode errror");

    return 0;
}


int Credentials::generateCSR(const char *common_name, uint16_t vendor_id, uint16_t product_id, char * buffer, size_t max_size, size_t * size)
{
    mbedtls_pk_context key_ctx;
    mbedtls_x509write_csr csr_ctx;

    ASSERT(buffer, return -1, "creds_csr_generate, invalid buffer pointer");
    ASSERT(size, return -2, "creds_csr_generate, invalid size pointer");
    ASSERT(max_size >= 512, return -2, "creds_csr_generate, buffer too small");
    *size    = 0;

    mbedtls_x509write_csr_init(&csr_ctx);

    // Subject name
    char subject_name[64] = { 0 };
    snprintf(subject_name, sizeof(subject_name), "CN=%s Mvid:%04X Mpid:%04X", common_name, vendor_id,
             product_id);
    int err = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
    ASSERT(!err, return err, "mbedtls_x509write_csr_set_subject_name() error %d", err);

    // Algorithm
    mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);

    // Signing key
    mbedtls_pk_init(&key_ctx);
    err = mbedtls_pk_setup_opaque(&key_ctx, this->_key_id);
    ASSERT(!err, return err, "mbedtls_pk_setup_opaque() error %d", err);
    mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);

    // Generate
    err = mbedtls_x509write_csr_pem(&csr_ctx, (uint8_t *) buffer, max_size, NULL, NULL);
    ASSERT(!err, return err, "mbedtls_x509write_csr_pem() error %d", err);
    *size = strlen(buffer);

    // Release
    mbedtls_x509write_csr_free(&csr_ctx);
    return err;
}

int Credentials::parseCert(const uint8_t * cert_data, size_t cert_size, uint8_t *serial_num, size_t serial_max, size_t &serial_size)
{
    mbedtls_x509_crt cert;
    
    mbedtls_x509_crt_init(&cert);
    
    serial_size = 0;
    int err = mbedtls_x509_crt_parse_der(&cert, cert_data, cert_size);
    ASSERT(!err, goto exit, "X.509 parse error");
    
    serial_size = cert.serial.len > serial_max ? serial_max : cert.serial.len;
    memcpy(serial_num, cert.serial.p, serial_size);

exit:
    mbedtls_x509_crt_free(&cert);
    return err;
}
