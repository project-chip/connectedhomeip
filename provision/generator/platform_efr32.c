
#include "platform.h"
#include <sl_iostream_rtt.h>
#include <em_msc.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include <psa/crypto.h>


extern void setNvm3End(uint32_t addr);


void platform_init(uint32_t creds_base_addr)
{
    MSC_Init();
    setNvm3End(creds_base_addr);
    psa_crypto_init();
}


int platform_write(const void *buffer, size_t buffer_length)
{
    return sl_iostream_write(sl_iostream_rtt_handle, buffer, buffer_length);
}


int platform_read(void *buffer, size_t buffer_length, size_t *bytes_read)
{
    return sl_iostream_read(sl_iostream_rtt_handle, buffer, buffer_length, bytes_read);
}


int platform_flash(uint32_t *page_address, const uint8_t *page_buffer, size_t page_size)
{
    // Erase page
    MSC_ErasePage(page_address);
    // Write to flash
    MSC_WriteWord(page_address, page_buffer, page_size);
    return 0;
}

int platform_sha256(const uint8_t *value, size_t size, uint8_t *hash, size_t *hash_size)
{
    return psa_hash_compute(PSA_ALG_SHA_256, value, size, hash, PSA_HASH_LENGTH(PSA_ALG_SHA_256), hash_size);
}

