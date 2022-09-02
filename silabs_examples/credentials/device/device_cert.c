#include "device_creds.h"
#include <psa/crypto.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>
#include <em_msc.h>
#include <string.h>

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

static int key_destroy(psa_key_id_t key_id)
{
    psa_key_handle_t key_handle;

    int err = psa_open_key(key_id, &key_handle);
    if (err)
    {
        psa_close_key(key_id);
    }
    else
    {
        err = psa_destroy_key(key_id);
    }
    return err;
}

int key_generate(psa_key_id_t key_id)
{
    psa_key_attributes_t key_attr = psa_key_attributes_init();

    key_destroy(key_id);

    psa_set_key_id(&key_attr, key_id);
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&key_attr, 256);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&key_attr,
                            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH |
                                PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_lifetime(&key_attr,
                         PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                             PSA_KEY_LIFETIME_PERSISTENT,
                             PSA_KEY_LOCATION_LOCAL_STORAGE));

    int err = psa_generate_key(&key_attr, &key_id);
    ASSERT(!err, return err, "psa_generate_key error %d", err);

    return 0;
}

int key_import(psa_key_id_t key_id, const uint8_t *key_value, size_t key_size)
{
    key_destroy(key_id);

    psa_key_attributes_t key_attr = psa_key_attributes_init();
    psa_set_key_id(&key_attr, key_id);
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&key_attr, 256);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(
        &key_attr, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_lifetime(
        &key_attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_LOCAL_STORAGE));

    int err = psa_import_key(&key_attr, key_value, key_size, &key_id);
    psa_reset_key_attributes(&key_attr);
    return err;
}

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

int device_csr_generate(creds_file_t *csr)
{
    psa_key_id_t key_id = EFR32_CREDENTIALS_DAC_KEY_ID;
    mbedtls_pk_context key_ctx;

    // Initialize
    csr->offset = 0;
    csr->size = 0;

    //
    // Key
    //

    // int err = key_import(key_id, _examples_dac_priv_raw, sizeof(_examples_dac_priv_raw));
    // ASSERT(!err, return err, "Key generation error %d", err);

    int err = key_generate(key_id);
    ASSERT(!err, return err, "Key generation error %d", err);

    //
    // CSR
    //

    static mbedtls_x509write_csr csr_ctx;

    mbedtls_x509write_csr_init(&csr_ctx);

    // Subject name

    char subject_name[512];
#if MATTER_X509_EXTENSIONS
    snprintf(subject_name, sizeof(subject_name), "CN=%s,VID=%04X,PID=%04X", MATTER_COMMON_NAME, MATTER_VENDOR_ID, MATTER_PRODUCT_ID);
#else
    snprintf(subject_name, sizeof(subject_name), "CN=%s", MATTER_COMMON_NAME);
#endif

    mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);
    err = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
    ASSERT(!err, return err, "mbedtls_x509write_csr_set_subject_name() error %d", err);

    mbedtls_pk_init(&key_ctx);
    err = mbedtls_pk_setup_opaque(&key_ctx, key_id);
    ASSERT(!err, return err, "mbedtls_pk_setup_opaque() error %d", err);

    // Signing key

    mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);

    // Generate

    err = mbedtls_x509write_csr_pem(&csr_ctx, csr->data, csr->max, NULL, NULL);
    ASSERT(!err, return err, "mbedtls_x509write_csr_pem() error %d", err);
    csr->size = strlen((char *)csr->data);

    mbedtls_x509write_csr_free(&csr_ctx);
    return err;
}
