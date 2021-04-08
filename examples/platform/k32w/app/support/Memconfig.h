#pragma once

#ifndef SRC_PLATFORM_K32W_MEMCONFIG_H_
#define SRC_PLATFORM_K32W_MEMCONFIG_H_

extern "C" {
size_t xPortMallocUsableSize( void *pv );
void * __wrap_malloc(size_t size);
void __wrap_free(void * ptr);
void * __wrap_calloc(size_t num, size_t size);
void * __wrap_realloc(void * ptr, size_t new_size);
void * __wrap__malloc_r(void * REENT, size_t size);
void __wrap__free_r(void * REENT, void * ptr);
void * __wrap__realloc_r(void * REENT, void * ptr, size_t new_size);
}

#endif /* SRC_PLATFORM_K32W_MEMCONFIG_H_ */
