/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      CHIP project configuration for standalone builds on Linux and OS X.
 *
 */
#ifndef CHIPPROJECTCONFIG_H
#define CHIPPROJECTCONFIG_H

#define CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT 1

#define CHIP_CONFIG_EVENT_LOGGING_NUM_EXTERNAL_CALLBACKS 2

#define CHIP_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT 1

// Uncomment this for a large Tunnel MTU.
//#define CHIP_CONFIG_TUNNEL_INTERFACE_MTU                           (9000)

// Enable support functions for parsing command-line arguments
#define CHIP_CONFIG_ENABLE_ARG_PARSER 1

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT 1000
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT                                                                                   \
    "{ 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20, 0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 }"
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER                                                                               \
    "{ 0xab, 0xa6, 0x0c, 0x30, 0x41, 0x6b, 0x8f, 0x41, 0x77, 0xf5, 0xe1, 0x6a, 0xd5, 0x14, 0xcf, 0xd9, 0x57, 0x75, 0x13, 0xf0, 0x2f,  \
        0xd6, 0x05, 0x06, 0xb1, 0x04, 0x9d, 0x0f, 0x2c, 0x73, 0x10, 0x01, 0x0e, 0x5e, 0x40, 0xbf, 0xd8, 0x6b, 0x4e, 0xf6, 0x81,    \
        0xa8, 0x8b, 0x71, 0xe9, 0xe2, 0xa8, 0x53, 0x98, 0x5a, 0x7d, 0xef, 0x91, 0x6e, 0xa3, 0x0e, 0x01, 0xb8, 0x72, 0x2f, 0xbf,    \
        0x7d, 0x0e, 0x38, 0x85, 0x6c, 0x12, 0xcd, 0x64, 0xc2, 0x25, 0xbb, 0x24, 0xef, 0x21, 0x41, 0x7e, 0x0e, 0x44, 0xe5 }"

// Enable reading DRBG seed data from /dev/(u)random.
// This is needed for test applications and the CHIP device manager to function
// properly when CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG is enabled.
#define CHIP_CONFIG_DEV_RANDOM_DRBG_SEED 1

// For convenience, Chip Security Test Mode can be enabled and the
// requirement for authentication in various protocols can be disabled.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
//    To build with this flag, pass 'treat_warnings_as_errors=false' to gn/ninja.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#define CHIP_CONFIG_REQUIRE_AUTH 1

#define CHIP_CONFIG_ENABLE_UPDATE 1

#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 0

#define CHIP_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL 1

#ifndef CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 4
#endif

//
// Default of 8 ECs is not sufficient for some of the unit tests
// that try to validate multiple simultaneous interactions.
//
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 16

//
// Set this to a value greater than the value for CHIP_IM_MAX_NUM_READ_HANDLER
// to ensure some of the tests for validating resource exhaustion and heap allocation
// pass correctly.
//
// TODO: Once we fix ReadClients to be heap allocated, we no longer need this override.
//
#define CHIP_IM_MAX_NUM_READ_CLIENT 6

#define CONFIG_IM_BUILD_FOR_UNIT_TEST 1

#endif /* CHIPPROJECTCONFIG_H */
