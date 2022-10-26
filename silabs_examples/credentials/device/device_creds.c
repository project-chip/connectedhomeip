#include "device_creds.h"
#include "em_chip.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_iostream_init_instances.h"

#include <em_msc.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>
#include <psa/crypto.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define REQ_BUFFER_MAX 200
#ifndef RES_BUFFER_MAX
#define RES_BUFFER_MAX 1024
#endif
#define CREDS_REQ_HEADER_SIZE 5
#define CREDS_RES_HEADER_SIZE 4

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

typedef enum
{
    CREDS_STATE_READ_HEADER = 0x00,
    CREDS_STATE_READ_PAYLOAD = 0x01,
    CREDS_STATE_CSR_GENERATE = 0x02,
    CREDS_STATE_CSR_READY = 0x03,
    CREDS_STATE_FAILURE = 0xff,

} creds_state_t;

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

    psa_set_key_id(&key_attr, key_id);
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&key_attr, 256);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(
        &key_attr, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_lifetime(
        &key_attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_LOCAL_STORAGE));

    int err = psa_generate_key(&key_attr, &key_id);
    ASSERT(!err, return err, "psa_generate_key error %d", err);

    return 0;
}

int key_import(psa_key_id_t key_id, const uint8_t * key_value, size_t key_size)
{
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

static int csr_generate(const char *common_name, unsigned vendor_id, unsigned product_id, char * buffer, size_t * size)
{
    psa_key_id_t key_id = EFR32_CREDENTIALS_DAC_KEY_ID;
    mbedtls_pk_context key_ctx;
    size_t max_size = 0;

    ASSERT(buffer, return -1, "creds_csr_generate, invalid buffer pointer");
    ASSERT(size, return -2, "creds_csr_generate, invalid size pointer");
    ASSERT(*size >= 512, return -2, "creds_csr_generate, buffer too small");
    max_size = *size;
    *size    = 0;

    //
    // Key
    //

    key_destroy(key_id);

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
    snprintf(subject_name, sizeof(subject_name), "CN=%s,VID=%04X,PID=%04X", common_name, vendor_id,
             product_id);
#else
    snprintf(subject_name, sizeof(subject_name), "CN=%s", common_name);
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

    err = mbedtls_x509write_csr_pem(&csr_ctx, (uint8_t *) buffer, max_size, NULL, NULL);
    ASSERT(!err, return err, "mbedtls_x509write_csr_pem() error %d", err);
    *size = strlen(buffer);

    mbedtls_x509write_csr_free(&csr_ctx);
    return err;
}

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

int device_creds_init()
{
    /* Prevent buffering of output/input.*/
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
    setvbuf(stdout, NULL, _IONBF, 0); /*Set unbuffered mode for stdout (newlib)*/
    setvbuf(stdin, NULL, _IONBF, 0);  /*Set unbuffered mode for stdin (newlib)*/
#endif

    // Initialize PSA Crypto
    int err = psa_crypto_init();
    ASSERT(!err, return err, "psa_crypto_init() error %d", err);


    return 0;
}

static unsigned _state = CREDS_STATE_READ_HEADER;
static uint8_t _in_buffer[REQ_BUFFER_MAX];
static size_t _in_size = 0;
static uint32_t _vendor_id = 0;
static uint32_t _product_id = 0;
static uint8_t _cn_size = 0;
static uint8_t _out_buffer[RES_BUFFER_MAX];
static size_t _out_size = 0;

void device_creds_action()
{
    if(CREDS_STATE_READ_HEADER == _state) {
        size_t bytes_read = 0;
        sl_iostream_read(SL_IOSTREAM_STDIN, &_in_buffer[_in_size], sizeof(_in_buffer) - _in_size, &bytes_read);
        _in_size += bytes_read;
        if(_in_size >= CREDS_REQ_HEADER_SIZE)
        {
            // Decode header: vendor_id, product_id, cn_size
            _vendor_id = (_in_buffer[0] << 8) | _in_buffer[1];
            _product_id = (_in_buffer[2] << 8) | _in_buffer[3];
            _cn_size = _in_buffer[4];
            _state = CREDS_STATE_READ_PAYLOAD;
        }
    }
    if(CREDS_STATE_READ_PAYLOAD == _state) {
        size_t bytes_read = 0;
        sl_iostream_read(SL_IOSTREAM_STDIN, &_in_buffer[_in_size], sizeof(_in_buffer) - _in_size, &bytes_read);
        _in_size += bytes_read;
        if(_in_size >= ((size_t)CREDS_REQ_HEADER_SIZE + _cn_size))
        {
            // Common name received
            _in_buffer[CREDS_REQ_HEADER_SIZE + _cn_size] = 0;
            _state = CREDS_STATE_CSR_GENERATE;
        }
    }
    else if(CREDS_STATE_CSR_GENERATE == _state) {

        // Generate CSR
        // size_t offset = CREDS_HEADER_SIZE;
        size_t csr_size = sizeof(_out_buffer) - CREDS_REQ_HEADER_SIZE - _cn_size - 1; // -1 (zero ending)
        char *name = (char *)&_in_buffer[CREDS_REQ_HEADER_SIZE];
        char *csr = (char *)&_out_buffer[CREDS_RES_HEADER_SIZE];
        int32_t err = csr_generate(name, _vendor_id, _product_id, csr, &csr_size);

        // Encode header: error(2), size(2)
        _out_buffer[0] = (err >> 8) & 0xff;
        _out_buffer[1] = (err & 0xff);
        _out_buffer[2] = (csr_size >> 8) & 0xff;
        _out_buffer[3] = (csr_size & 0xff);
        // Zero-terminate
        _out_buffer[CREDS_RES_HEADER_SIZE + csr_size] = 0;
        _out_size = CREDS_RES_HEADER_SIZE + csr_size + 1;
        _state = CREDS_STATE_CSR_READY;
    }
    else if(CREDS_STATE_CSR_READY == _state) {
        sl_iostream_write(SL_IOSTREAM_STDOUT, _out_buffer, _out_size);
        _in_size = 0;
        _state = CREDS_STATE_READ_HEADER;
    }
}