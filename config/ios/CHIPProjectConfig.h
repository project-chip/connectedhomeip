/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    Copyright (c) 2019-2020 Google LLC.
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
 *      Project-specific configuration file for iOS builds.
 *
 */
#ifndef WEAVEPROJECTCONFIG_H
#define WEAVEPROJECTCONFIG_H

// Enable use of an ephemeral UDP source port for locally initiated Weave exchanges.
#define WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT 1

// Enable UDP listening on demand in the WeaveDeviceManager
#define WEAVE_CONFIG_DEVICE_MGR_DEMAND_ENABLE_UDP 1

#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT 0

// Enable passcode encryption configuration 1
#define WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY     1

#define WDM_UPDATE_MAX_ITEMS_IN_TRAIT_DIRTY_PATH_STORE 300

// Uncomment this for a large Tunnel MTU.
//#define WEAVE_CONFIG_TUNNEL_INTERFACE_MTU                           (9000)

// Max number of Bindings per WeaveExchangeManager
#define WEAVE_CONFIG_MAX_BINDINGS 8

#define WDM_ENFORCE_EXPIRY_TIME 1

#define WEAVE_CONFIG_ENABLE_WDM_UPDATE 1

#define WEAVE_CONFIG_LEGACY_CASE_AUTH_DELEGATE 0

#define WEAVE_CONFIG_LEGACY_KEY_EXPORT_DELEGATE 0

#define WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC 300

#define WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 1

#define WEAVE_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL 1

#define WEAVE_CONFIG_MAX_SOFTWARE_VERSION_LENGTH 128

#endif /* WEAVEPROJECTCONFIG_H */
