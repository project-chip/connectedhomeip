/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Telink mbedtls configuration file.
 *
 */

#ifndef MBEDTLS_TSLR9_CONF_H
#define MBEDTLS_TSLR9_CONF_H

#define MBEDTLS_HKDF_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CSR_WRITE_C

#define MBEDTLS_AES_ALT
#define MBEDTLS_ECP_ALT

#endif /* MBEDTLS_TSLR9_CONF_H */
