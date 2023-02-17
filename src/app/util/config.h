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

// *******************************************************************
// pre-defined Devices
//
// use these to determine which type of device the current application is.
// do not use the EMBER_* versions from ember-types.h as these are in an
// enum and are not available at preprocessor time. These need to be set
// before the devices are loaded from ha-devices.h and se-devices.h
#define ZA_COORDINATOR 1
#define ZA_ROUTER 2
#define ZA_END_DEVICE 3
#define ZA_SLEEPY_END_DEVICE 4

// This file determines the security profile used, and from that various
// other security parameters.
// #include "app/framework/security/security-config.h"

// *******************************************************************
// Application configuration of RAM for cluster usage
//
// This section defines constants that size the tables used by the cluster
// code.

// This is the max hops that the network can support - used to determine
// the max source route overhead and broadcast radius
// if we havent defined MAX_HOPS then define based on profile ID
#ifndef ZA_MAX_HOPS
#define ZA_MAX_HOPS 12
#endif

// Max PHY size = 128
//   -1 byte for PHY length
//   -2 bytes for MAC CRC
#define EMBER_AF_MAXIMUM_INTERPAN_LENGTH 125

// The additional overhead required for APS encryption (security = 5, MIC = 4).
#define EMBER_AF_APS_ENCRYPTION_OVERHEAD 9

// The additional overhead required for APS fragmentation.
#define EMBER_AF_APS_FRAGMENTATION_OVERHEAD 2

// The additional overhead required for network source routing (relay count = 1,
// relay index = 1).  This does not include the size of the relay list itself.
#define EMBER_AF_NWK_SOURCE_ROUTE_OVERHEAD 2

// The additional overhead required per relay address for network source
// routing.  This is in addition to the overhead defined above.
#define EMBER_AF_NWK_SOURCE_ROUTE_PER_RELAY_ADDRESS_OVERHEAD 2

// defines the largest size payload allowed to send and receive. This
// affects the payloads generated from the CLI and the payloads generated
// as responses.
// Maximum payload length.
#define EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH 1024
#define EMBER_AF_INCOMING_BUFFER_LENGTH 1024

// *******************************************************************
// Application configuration of Flash
//
// This section gives the application options for turning on or off
// features that affect the amount of flash used.

// *******************************************************************
// Defines needed for enabling security
//

// Our stack profile is 2 (ZigBee Pro).
#define EMBER_STACK_PROFILE 2

// *******************************************************************
// Application Handlers
//
// By default, a number of stub handlers are automatically provided
// that have no effect.  If the application would like to implement any
// of these handlers itself, it needs to define the appropriate macro

#define EMBER_APPLICATION_HAS_REMOTE_BINDING_HANDLER
#define EMBER_APPLICATION_HAS_ENERGY_SCAN_RESULT_HANDLER
#define EMBER_APPLICATION_HAS_GET_ENDPOINT
#define EMBER_APPLICATION_HAS_TRUST_CENTER_JOIN_HANDLER
#define EMBER_APPLICATION_HAS_BUTTON_HANDLER
#define EMBER_APPLICATION_HAS_ZIGBEE_KEY_ESTABLISHMENT_HANDLER

#define EZSP_APPLICATION_HAS_ENERGY_SCAN_RESULT_HANDLER
#define EZSP_APPLICATION_HAS_INCOMING_SENDER_EUI64_HANDLER
#define EZSP_APPLICATION_HAS_TRUST_CENTER_JOIN_HANDLER
#define EZSP_APPLICATION_HAS_BUTTON_HANDLER
#define EZSP_APPLICATION_HAS_ZIGBEE_KEY_ESTABLISHMENT_HANDLER

#ifndef EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE
#define EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE EMBER_APS_UNICAST_MESSAGE_COUNT
#endif // EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE

#define EMBER_APPLICATION_HAS_COMMAND_ACTION_HANDLER

// *******************************************************************
// Default values for required defines
//

// define the serial port that the application uses to be 1 if this is not set
#ifndef HAL_CONFIG // HAL Config handles serial port allocation
#ifndef APP_SERIAL
#define APP_SERIAL 1
#endif
#ifdef EMBER_TEST
#include "app/util/ezsp/uart-simulation-remap.h"
#endif
#endif

// The address table plugin is enabled by default. If it gets disabled for some
// reason, we still need to define these #defines to some default value.
#ifndef EMBER_AF_PLUGIN_ADDRESS_TABLE
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE 2
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE 2
#endif

// The total size of the address table is the size of the section used by the
// application plus the size of section used for the trust center address cache.
// The NCP allows each section to be sized independently, but the SOC requires
// a single configuration for the whole table.
#ifndef EMBER_ADDRESS_TABLE_SIZE
#define EMBER_ADDRESS_TABLE_SIZE (EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE + EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE)
#endif

#ifndef EMBER_AF_DEFAULT_APS_OPTIONS
// BUGZID 12261: Concentrators use MTORRs for route discovery and should not
// enable route discovery in the APS options.
#ifdef EMBER_AF_PLUGIN_CONCENTRATOR
#define EMBER_AF_DEFAULT_APS_OPTIONS (EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY)
#else
#define EMBER_AF_DEFAULT_APS_OPTIONS                                                                                               \
    (EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY)
#endif
#endif

// *******************************************************************
// // Default values for required defines
// //

#ifdef EMBER_AF_DEFAULT_RESPONSE_POLICY_NEVER
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#elif defined(EMBER_AF_DEFAULT_RESPONSE_POLICY_CONDITIONAL)
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS 0
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#else
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS 0
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES 0
#endif // EMBER_AF_DEFAULT_RESPONSE_POLICY_NEVER

#ifdef EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS
#ifdef EMBER_AF_USE_STANDARD_NETWORK_INIT
#error "Custom options cannot be used with the standard network init"
#endif
#else
// We always want to store our parent info in a token. This prevents doing an
// orphan scan upon reboot, which can suffer from the multiple-parent-
// responses issue
#define EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS (EMBER_NETWORK_INIT_PARENT_INFO_IN_TOKEN | Z3_NETWORK_INIT_BEHAVIOR)
#endif // EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS

/**
 * @brief CHIP uses millisecond ticks
 */
#define MILLISECOND_TICKS_PER_SECOND 1000
