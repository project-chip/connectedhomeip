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

#pragma once

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY 1

// This proxy serves one commissioning session at a time, shared across every transport.
// It is not merely a preference: each transport driver tracks its single in-flight connect in
// one static (sPendingConnect in CommissioningProxyBleTransport.cpp and in
// CommissioningProxyPafTransport.cpp) and relies on MaxSessions == 1 to make it unique, so a
// larger value here would let the cluster accept a second concurrent connect the drivers
// cannot represent. This matches the SDK default, but it is restated here because the driver
// constraint is this app's, not the cluster's: raising it means giving the drivers
// per-session connect state first.
#define CHIP_CONFIG_COMMISSIONING_PROXY_MAX_SESSIONS 1

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>
