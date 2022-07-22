/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#ifndef CHIPPROJECTCONFIG_H
#define CHIPPROJECTCONFIG_H

// Enable UDP listening on demand in the CHIPDeviceController
#define CHIP_CONFIG_DEVICE_CONTROLLER_DEMAND_ENABLE_UDP 1

#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT 0

// Uncomment this for a large Tunnel MTU.
//#define CHIP_CONFIG_TUNNEL_INTERFACE_MTU                           (9000)

#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 0

#define CHIP_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL 1

#define CHIP_CONFIG_MAX_SOFTWARE_VERSION_LENGTH 128

#ifndef CHIP_CONFIG_KVS_PATH
#define CHIP_CONFIG_KVS_PATH "chip.store"
#endif

// The session pool size limits how many subscriptions we can have live at
// once.  Home supports up to 1000 accessories, and we subscribe to all of them,
// so we need to make sure the pool is big enough for that.
#define CHIP_CONFIG_SECURE_SESSION_POOL_SIZE 1000

#endif /* CHIPPROJECTCONFIG_H */
