/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/***********************************************************************************
 * Device Identification Configuration
 ***********************************************************************************/
/* The SoftwareVersionString attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.3.1.327"

/* The SoftwareVersion attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x00130547

// -------------------- Test Configuration --------------------
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
