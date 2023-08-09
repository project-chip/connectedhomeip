#ifndef GENERATOR_PLATFORM_H_
#define GENERATOR_PLATFORM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void platform_init(uint32_t creds_base_addr);

int platform_write(const void *buffer, size_t buffer_length);

int platform_read(void *buffer, size_t buffer_length, size_t *bytes_read);

int platform_flash(uint32_t *page_address, const uint8_t *page_buffer, size_t page_size);

int platform_sha256(const uint8_t *value, size_t size, uint8_t *hash, size_t *hash_size);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // GENERATOR_PLATFORM_H_