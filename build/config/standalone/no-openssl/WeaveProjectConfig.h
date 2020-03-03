/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      Alternate Weave project configuration for building standalone without OpenSSL.
 *
 */
#ifndef WEAVEPROJECTCONFIG_NOOPENSSL_H
#define WEAVEPROJECTCONFIG_NOOPENSSL_H

#include "../WeaveProjectConfig.h"

#undef WEAVE_CONFIG_USE_OPENSSL_ECC
#undef WEAVE_CONFIG_USE_MICRO_ECC
#undef WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL
#undef WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
#undef WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL
#undef WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG
#undef WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL
#undef WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI
#undef WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM
#undef WEAVE_CONFIG_SUPPORT_PASE_CONFIG0
#undef WEAVE_CONFIG_SUPPORT_PASE_CONFIG1
#undef WEAVE_CONFIG_SUPPORT_PASE_CONFIG2
#undef WEAVE_CONFIG_SUPPORT_PASE_CONFIG3
#undef WEAVE_CONFIG_SUPPORT_PASE_CONFIG4
#undef WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT

#define WEAVE_CONFIG_USE_OPENSSL_ECC 0
#define WEAVE_CONFIG_USE_MICRO_ECC 1
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL 0
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 1
#define WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL 0
#define WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG 1
#define WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI 1
#define WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM 0
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG0 0
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG1 0
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG2 0
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG3 0
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG4 1
#define WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 0

#endif /* WEAVEPROJECTCONFIG_H */
