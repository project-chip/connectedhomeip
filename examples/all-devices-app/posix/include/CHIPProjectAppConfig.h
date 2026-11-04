/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

// CHIPProjectAppConfig.h is used as both chip_project_config_include (included early by
// SystemConfig.h before CHIPDeviceBuildConfig.h) and chip_device_project_config_include
// (included by CHIPDeviceConfig.h after CHIPDeviceBuildConfig.h). Evaluate this device-layer
// override whenever CHIPDeviceBuildConfig.h has been included, even if CHIPProjectAppConfig.h
// was already included earlier via SystemConfig.h.
#if defined(CHIP_DEVICE_CONFIG_ENABLE_THREAD_MESHCOP) && defined(CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_MESHCOP && !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#ifndef CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
#define CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION 0
#endif
#endif
#endif

#ifndef CHIP_ALL_DEVICES_PROJECT_APP_CONFIG_H
#define CHIP_ALL_DEVICES_PROJECT_APP_CONFIG_H

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY 1

// This proxy serves one commissioning session at a time, shared across every transport.
// It is not merely a preference: the transport drivers track a single in-flight connect in
// one static (sPendingConnect in CommissioningProxyBleTransport.cpp) and rely on
// MaxSessions == 1 to make it unique, so a larger value here would let the cluster accept
// a second concurrent connect the driver cannot represent. This matches the SDK default,
// but it is restated here because the driver constraint is this app's, not the cluster's:
// raising it means giving the drivers per-session connect state first.
#define CHIP_CONFIG_COMMISSIONING_PROXY_MAX_SESSIONS 1

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>

#endif // CHIP_ALL_DEVICES_PROJECT_APP_CONFIG_H
