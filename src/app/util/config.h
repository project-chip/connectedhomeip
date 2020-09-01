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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @brief This file is the main configuration settings
 *for the Zigbee app. The zigbee app can become a Home
 *Automation (HA) device, a Smart Energy (SE) device,
 *or a Custom Zigbee device.
 *
 * This application can be configured using
 *AppBuilder. AppBuilder generates a file containing
 *defines that setup what pieces of the code is used
 *(which clusters, security settings, zigbee device
 *type, serial port, etc). These defines are added to
 *a new file and included by setting
 *ZA_GENERATED_HEADER to the new filename so these
 *defines are sourced first.
 *
 * This file also contains default values for the
 *defines so some can be set by the user but defaults
 *are always available.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories, Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc.  Your use of this software is
 *governed by the terms of  Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software  is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#ifndef __EMBER_AF_CONFIG_H__
#define __EMBER_AF_CONFIG_H__

// include generated configuration information from AppBuilder.
// ZA_GENERATED_HEADER is defined in the project file
#ifdef ZA_GENERATED_HEADER
#include ZA_GENERATED_HEADER
#else
#include "gen/gen_config.h"
#endif

#ifdef ATTRIBUTE_STORAGE_CONFIGURATION
#include ATTRIBUTE_STORAGE_CONFIGURATION
#else
#include "gen/endpoint_config.h"
#endif

#include "gen/gen_tokens.h" // For GENERATED_TOKEN_LOADER/SAVER macros

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

#define CBA_PROFILE_ID (0x0105)
#define HA_PROFILE_ID (0x0104)
#define SE_PROFILE_ID (0x0109)

// A subtle distniction:
//   EMBER_AF_MANUFACTURER_CODE is the MFG code set by AppBuilder
//     for use in the App Framework (AF).  If not set by AppBuilder we default
//     it to 0x0000. The customer should be setting this value.
//   EMBER_COMPANY_MANUFACTURER_CODE is the Ember Corporation's Manufacturer
//     ID allocated by the Zigbee alliance.  This shall not change.
#define EMBER_COMPANY_MANUFACTURER_CODE 0x1002
#ifndef EMBER_AF_MANUFACTURER_CODE
#define EMBER_AF_MANUFACTURER_CODE 0x0000
#endif

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
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
#define ZA_MAX_HOPS 6
#else
#define ZA_MAX_HOPS 12
#endif
#endif

#ifndef EMBER_AF_SOURCE_ROUTING_RESERVED_PAYLOAD_LENGTH
#define EMBER_AF_SOURCE_ROUTING_RESERVED_PAYLOAD_LENGTH 0
#endif

// The maximum APS payload, not including any APS options.  This value is also
// available from emberMaximumApsPayloadLength() or ezspMaximumPayloadLength().
// See http://portal.ember.com/faq/payload for more information.
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_NONE
#define EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH 100 - EMBER_AF_SOURCE_ROUTING_RESERVED_PAYLOAD_LENGTH
#else
#define EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH 82 - EMBER_AF_SOURCE_ROUTING_RESERVED_PAYLOAD_LENGTH
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
// If fragmenation is enabled, and fragmentation length is bigger than default, then use that
#if defined(EMBER_AF_PLUGIN_FRAGMENTATION) && (EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE > EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH)
#define EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE
#define EMBER_AF_INCOMING_BUFFER_LENGTH EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE
#else
#define EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH
#define EMBER_AF_INCOMING_BUFFER_LENGTH EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH
#endif

// *******************************************************************
// Application configuration of Flash
//
// This section gives the application options for turning on or off
// features that affect the amount of flash used.

// *******************************************************************
// Defines needed for enabling security
//

// Unless we are not using security, our stack profile is 2 (ZigBee Pro).  The
// stack will set up other configuration values based on profile.
#ifndef EMBER_AF_HAS_SECURITY_PROFILE_NONE
#define EMBER_STACK_PROFILE 2
#else
#ifndef EMBER_STACK_PROFILE
#define EMBER_STACK_PROFILE 0
#endif
#ifndef EMBER_SECURITY_LEVEL
#define EMBER_SECURITY_LEVEL 0
#endif
#endif

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
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_Z3 // Z3 Compliant end devices must send a rejoin request on reboot
#define Z3_NETWORK_INIT_BEHAVIOR EMBER_NETWORK_INIT_END_DEVICE_REJOIN_ON_REBOOT
#else // EMBER_AF_HAS_SECURITY_PROFILE_Z3
#define Z3_NETWORK_INIT_BEHAVIOR EMBER_NETWORK_INIT_NO_OPTIONS
#endif // EMBER_AF_HAS_SECURITY_PROFILE_Z3

// We always want to store our parent info in a token. This prevents doing an
// orphan scan upon reboot, which can suffer from the multiple-parent-
// responses issue
#define EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS (EMBER_NETWORK_INIT_PARENT_INFO_IN_TOKEN | Z3_NETWORK_INIT_BEHAVIOR)
#endif // EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS

/**
 * @brief CHIP uses millisecond ticks
 */
#define MILLISECOND_TICKS_PER_SECOND 1000

#endif // __EMBER_AF_CONFIG_H__
