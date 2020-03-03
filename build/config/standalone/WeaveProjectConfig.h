/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Weave project configuration for standalone builds on Linux and OS X.
 *
 */
#ifndef WEAVEPROJECTCONFIG_H
#define WEAVEPROJECTCONFIG_H


#define WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT 1

// Configure WDM for event offload
#define WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD 1

#define WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1

#define WEAVE_CONFIG_EVENT_LOGGING_NUM_EXTERNAL_CALLBACKS 2

#define WEAVE_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT 1

#define WDM_UPDATE_MAX_ITEMS_IN_TRAIT_DIRTY_PATH_STORE 300

// Uncomment this for a large Tunnel MTU.
//#define WEAVE_CONFIG_TUNNEL_INTERFACE_MTU                           (9000)

// Max number of Bindings per WeaveExchangeManager
#define WEAVE_CONFIG_MAX_BINDINGS 8

// Enable support functions for parsing command-line arguments
#define WEAVE_CONFIG_ENABLE_ARG_PARSER 1

// Enable reading DRBG seed data from /dev/(u)random.
// This is needed for test applications and the Weave device manager to function
// properly when WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG is enabled.
#define WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED 1

#define WEAVE_CONFIG_SECURITY_TEST_MODE 1

#define WDM_ENFORCE_EXPIRY_TIME 1

// Increase session idle timeout in stand-alone builds for the convenience of developers.
#define WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT           120000

#define WEAVE_CONFIG_ENABLE_WDM_UPDATE 1

#define WEAVE_CONFIG_LEGACY_CASE_AUTH_DELEGATE 0

#define WEAVE_CONFIG_LEGACY_KEY_EXPORT_DELEGATE 0

#define WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC 300

#define WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 1

#define WEAVE_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL 1

#endif /* WEAVEPROJECTCONFIG_H */
