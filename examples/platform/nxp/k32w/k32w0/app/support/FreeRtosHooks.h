/*
 *
 * SPDX-FileCopyrightText: 2020 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

typedef void * mbedtls_threading_mutex_t;

extern void mbedtls_threading_set_alt(void (*mutex_init)(mbedtls_threading_mutex_t *),
                                      void (*mutex_free)(mbedtls_threading_mutex_t *),
                                      int (*mutex_lock)(mbedtls_threading_mutex_t *),
                                      int (*mutex_unlock)(mbedtls_threading_mutex_t *));

extern void mbedtls_threading_free_alt(void);

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Function for initializing alternative MbedTLS mutexes to enable the usage of the FreeRTOS implementation. */
void freertos_mbedtls_mutex_init(void);

/**@brief Function for releasing MbedTLS alternative mutexes. */
void freertos_mbedtls_mutex_free(void);

#ifdef __cplusplus
}
#endif
