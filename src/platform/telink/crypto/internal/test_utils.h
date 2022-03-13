/**
 * \file test_utils.h
 *
 * \brief Utility macros for internal use in the library
 */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef MBEDTLS_TEST_UTILS_H
#define MBEDTLS_TEST_UTILS_H

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


void mbedtls_printbuf( const char *comment, const void *buf, size_t len );
void mbedtls_printbuf_c( const char *comment, const void *buf, size_t len );


#ifdef __cplusplus
}
#endif

#endif
