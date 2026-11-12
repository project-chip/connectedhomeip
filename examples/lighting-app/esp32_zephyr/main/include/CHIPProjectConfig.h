/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      Example project configuration file for CHIP.
 *
 *      This is a place to put application or project-specific overrides
 *      to the default configuration values for general CHIP features.
 */

#pragma once

// Use the example (test) Device Attestation Credentials. Not for production.
#define CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS 1

// Test onboarding payload, matching the chip-tool defaults.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0x0F00

#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0"

// Reduced buffer sizes for the ESP32-C6 SRAM budget.
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 16
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE 256
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE 256
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE 256
