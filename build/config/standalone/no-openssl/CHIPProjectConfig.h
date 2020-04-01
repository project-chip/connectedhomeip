/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      Alternate CHIP project configuration for building standalone without OpenSSL.
 *
 */
#ifndef CHIPPROJECTCONFIG_NOOPENSSL_H
#define CHIPPROJECTCONFIG_NOOPENSSL_H

#include "../CHIPProjectConfig.h"

#undef CHIP_CONFIG_USE_OPENSSL_ECC
#undef CHIP_CONFIG_USE_MICRO_ECC
#undef CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
#undef CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
#undef CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL
#undef CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
#undef CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
#undef CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
#undef CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM
#undef CHIP_CONFIG_SUPPORT_PASE_CONFIG0
#undef CHIP_CONFIG_SUPPORT_PASE_CONFIG1
#undef CHIP_CONFIG_SUPPORT_PASE_CONFIG2
#undef CHIP_CONFIG_SUPPORT_PASE_CONFIG3
#undef CHIP_CONFIG_SUPPORT_PASE_CONFIG4
#undef CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT

#define CHIP_CONFIG_USE_OPENSSL_ECC 0
#define CHIP_CONFIG_USE_MICRO_ECC 1
#define CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 1
#define CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG 1
#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 1
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG0 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG1 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG2 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG3 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG4 1
#define CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 0

#endif /* CHIPPROJECTCONFIG_H */
