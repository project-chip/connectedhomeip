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

#ifndef __PLUGIN_CONFIG_H__
#define __PLUGIN_CONFIG_H__

// TODO CLeanup Plugins Macro Issue #3627

// Use this macro to check if Antenna Stub plugin is included
#define EMBER_AF_PLUGIN_ANTENNA_STUB

// Use this macro to check if Barrier Control Server Cluster plugin is included
#define EMBER_AF_PLUGIN_BARRIER_CONTROL_SERVER

// Use this macro to check if Basic Server Cluster plugin is included
#define EMBER_AF_PLUGIN_BASIC

// Use this macro to check if Binding Table Library plugin is included
#define EMBER_AF_PLUGIN_BINDING_TABLE_LIBRARY
// User options for plugin Binding Table Library
#define EMBER_BINDING_TABLE_SIZE 10

// Use this macro to check if CCM* Encryption plugin is included
#define EMBER_AF_PLUGIN_CCM_ENCRYPTION
// User options for plugin CCM* Encryption
#define EMBER_AF_PLUGIN_CCM_ENCRYPTION_SOFTWARE_CCM
#define USE_SOFTWARE_CCM

// Use this macro to check if Radio Coexistence Stub plugin is included
#define EMBER_AF_PLUGIN_COEXISTENCE_STUB

// Use this macro to check if Color Control Cluster Server plugin is included
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER
// User options for plugin Color Control Cluster Server
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL 65535
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_XY_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_TEMP_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HUE_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_SATURATION_CHANGE 1

// Use this macro to check if Debug Basic Library plugin is included
#define EMBER_AF_PLUGIN_DEBUG_BASIC_LIBRARY

// Use this macro to check if Debug JTAG plugin is included
#define EMBER_AF_PLUGIN_DEBUG_JTAG

// Use this macro to check if Door Lock Server Cluster plugin is included
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER

// Use this macro to check if Ember Minimal Printf plugin is included
#define EMBER_AF_PLUGIN_EMBER_MINIMAL_PRINTF

// Use this macro to check if Groups Client Cluster plugin is included
#define EMBER_AF_PLUGIN_GROUPS_CLIENT

// Use this macro to check if Groups Server Cluster plugin is included
#define EMBER_AF_PLUGIN_GROUPS_SERVER

// Use this macro to check if HAL Library plugin is included
#define EMBER_AF_PLUGIN_HAL_LIBRARY

// Use this macro to check if IAS Zone Client plugin is included
//#define EMBER_AF_PLUGIN_IAS_ZONE_CLIENT
// User options for plugin IAS Zone Client
#define EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES 10

// Use this macro to check if IAS Zone Server plugin is included
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER
// User options for plugin IAS Zone Server
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE 541

// Use this macro to check if Identify Cluster plugin is included
#define EMBER_AF_PLUGIN_IDENTIFY

// Use this macro to check if Level Control Server Cluster plugin is included
#define EMBER_AF_PLUGIN_LEVEL_CONTROL
// User options for plugin Level Control Server Cluster
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL 255
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL 0
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE 0

// Use this macro to check if mbed TLS plugin is included
#define EMBER_AF_PLUGIN_MBEDTLS
// User options for plugin mbed TLS
#define EMBER_AF_PLUGIN_MBEDTLS_CONF_DEVICE_ACCELERATION
#define EMBER_AF_PLUGIN_MBEDTLS_CONF_DEVICE_ACCELERATION_APP

// Use this macro to check if NVM3 Library plugin is included
#define EMBER_AF_PLUGIN_NVM3
// User options for plugin NVM3 Library
#define EMBER_AF_PLUGIN_NVM3_FLASH_PAGES 18
#define EMBER_AF_PLUGIN_NVM3_CACHE_SIZE 200
#define EMBER_AF_PLUGIN_NVM3_MAX_OBJECT_SIZE 254
#define EMBER_AF_PLUGIN_NVM3_USER_REPACK_HEADROOM 0

// Use this macro to check if On/Off Server Cluster plugin is included
#define EMBER_AF_PLUGIN_ON_OFF

// Use this macro to check if Packet Validate Library plugin is included
#define EMBER_AF_PLUGIN_PACKET_VALIDATE_LIBRARY

// Use this macro to check if RAIL Library plugin is included
#define EMBER_AF_PLUGIN_RAIL_LIBRARY
// User options for plugin RAIL Library
#define EMBER_AF_PLUGIN_RAIL_LIBRARY_RAILPHYDEF 1

// Use this macro to check if Reporting plugin is included
#define EMBER_AF_PLUGIN_REPORTING
// User options for plugin Reporting
#define EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE 5
#define EMBER_AF_PLUGIN_REPORTING_ENABLE_GROUP_BOUND_REPORTS

// Use this macro to check if Scenes Server Cluster plugin is included
#define EMBER_AF_PLUGIN_SCENES
// User options for plugin Scenes Server Cluster
#define EMBER_AF_PLUGIN_SCENES_TABLE_SIZE 3

// Use this macro to check if Scenes Client Cluster plugin is included
#define EMBER_AF_PLUGIN_SCENES_CLIENT

// Use this macro to check if Serial plugin is included
#define EMBER_AF_PLUGIN_SERIAL

// Use this macro to check if Simulated EEPROM version 2 to NVM3 Upgrade Stub plugin is included
#define EMBER_AF_PLUGIN_SIM_EEPROM2_TO_NVM3_UPGRADE_STUB

// Use this macro to check if Simple Main plugin is included
#define EMBER_AF_PLUGIN_SIMPLE_MAIN

// Use this macro to check if Strong Random plugin is included
#define EMBER_AF_PLUGIN_STRONG_RANDOM
// User options for plugin Strong Random
#define EMBER_AF_PLUGIN_STRONG_RANDOM_RADIO_PRNG
#define USE_RADIO_API_FOR_TRNG

// Use this macro to check if Temperature Measurement Server Cluster plugin is included
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
// User options for plugin Temperature Measurement Server Cluster
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S 300
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_TEMPERATURE_CHANGE_M_C 500
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_ASSERT_WARNING_THRESHOLD 55
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_DEASSERT_WARNING_THRESHOLD 50
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_ASSERT_CRITICAL_THRESHOLD 60
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_DEASSERT_CRITICAL_THRESHOLD 55

// Use this macro to check if Temperature Si7053 Stub plugin is included
#define EMBER_AF_PLUGIN_TEMPERATURE_SI7053_STUB

// Use this macro to check if ZCL Framework Core plugin is included
#define EMBER_AF_PLUGIN_ZCL_FRAMEWORK_CORE
// User options for plugin ZCL Framework Core
#define EMBER_AF_PLUGIN_ZCL_FRAMEWORK_CORE_CLI_ENABLED
#define ZA_CLI_FULL

#endif // __PLUGIN_CONFIG_H__
