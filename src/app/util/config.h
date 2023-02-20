/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

// include generated configuration information from AppBuilder.
// ZA_GENERATED_HEADER is defined in the project file
#ifdef ZA_GENERATED_HEADER
#include ZA_GENERATED_HEADER
#else
#include <zap-generated/gen_config.h>
#endif

#ifdef ATTRIBUTE_STORAGE_CONFIGURATION
#include ATTRIBUTE_STORAGE_CONFIGURATION
#else
#include <zap-generated/endpoint_config.h>
#endif

// User options for plugin Binding Table Library
// TODO: Make this a CHIP_CONFIG value.
#ifndef EMBER_BINDING_TABLE_SIZE
#define EMBER_BINDING_TABLE_SIZE 10
#endif // EMBER_BINDING_TABLE_SIZE

// Legacy definition that we can remove once Scenes is no longer using the
// Zigbee message-writing bits.
#define EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH 1024

/**
 * @brief CHIP uses millisecond ticks
 */
#define MILLISECOND_TICKS_PER_SECOND 1000
