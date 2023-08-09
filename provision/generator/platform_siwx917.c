
#include "platform.h"
#include <SEGGER_RTT.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>


extern void setNvm3End(uint32_t addr);


void platform_init(uint32_t creds_base_addr)
{
    // TODO
}


int platform_write(const void *buffer, size_t buffer_length)
{
    return SEGGER_RTT_Write(0, buffer, buffer_length);
}


int platform_read(void *buffer, size_t buffer_length, size_t *bytes_read)
{
    return SEGGER_RTT_Read(0, buffer, buffer_length, bytes_read);
}


int platform_flash(uint32_t *page_address, const uint8_t *page_buffer, size_t page_size)
{
    // TODO
    return 0;
}


int platform_sha256(const uint8_t *value, size_t size, uint8_t *hash, size_t hash_size)
{
    return mbedtls_sha256((const unsigned char *)(value), size, hash, 0);
}
