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
/***************************************************************************//**
 * @file
 * @brief Attribute storage configuration for unit test
 *******************************************************************************
   ******************************************************************************/

#ifndef __ATTRIBUTE_STORAGE_TEST__
#define __ATTRIBUTE_STORAGE_TEST__

// This is the maximum memory allocated for all attributes on all
// endpoints.
#define ATTRIBUTE_MAX_SIZE 1000

// Maximum number of allowed endpoints. Actual number of endpoints
// is calculated at runtime.
#define FIXED_ENDPOINT_COUNT 10

// This is defined if we have attributes of more than 2 bytes
#define GENERATED_DEFAULTS {                       \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
}

// These are all the EmberAfAttributeMetadata objects that
// this application supports on any cluster.
#define GENERATED_ATTRIBUTES   {                             \
    { 101, 1, 1, 0, { (uint8_t*)(generatedDefaults + 0) } }, \
    { 102, 1, 2, 0, { (uint8_t*)1 } },                       \
    { 103, 1, 3, 0, { (uint8_t*)1 } },                       \
    { 104, 1, 4, 0, { (uint8_t*)1 } },                       \
    { 105, 1, 5, 0, { (uint8_t*)1 } }                        \
}

// These are the EmberAfCluster structures that the application
// supports. These clusters can be organized to any endpoints.
#define GENERATED_CLUSTERS {                                                 \
    { 1, (EmberAfAttributeMetadata*)generatedAttributes, 5, 5 * 5, 0, NULL } \
}

// These are the EmberAfEndpointType structs that the application supports.
// Each endpoint can be one of these endpoint types.
#define GENERATED_ENDPOINT_TYPES {                    \
    { (EmberAfCluster*)&generatedClusters, 1, 5 * 5 } \
}

// These are the networks that the application supports.
#define EM_AF_GENERATED_NETWORK_TYPES { \
    EM_AF_NETWORK_TYPE_ZIGBEE_PRO,      \
}
#define EM_AF_GENERATED_ZIGBEE_PRO_NETWORKS { \
    {                                         \
      ZA_COORDINATOR,                         \
      EMBER_AF_SECURITY_PROFILE_HA,           \
    },                                        \
}

// Manufacturer codes for mf specific attributes and clusters
#define GENERATED_CLUSTER_MANUFACTURER_CODES { \
    { 1, 0xbeef }                              \
}
#define GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT 1
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODES { \
    { 8, 0xabba },                               \
    { 9, 0xfeed }                                \
}
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT 2

// There are 2 ways to configure endpoints

// ===== Endpoint configuration method 1: =====

// Total count of endpoints. If this macro is defined, this method is used
// #define FIXED_ENDPOINT_COUNT 3
// Actual endpoint numbers
// #define FIXED_ENDPOINT_ARRAY { 50, 51, 52 }
// Indices into GENERATED_ENDPOINT_TYPES array
// #define FIXED_ENDPOINT_TYPES { 0, 0, 0 }
// #define FIXED_NETWORK { 0, 0, 0 }

// ===== Endpoint configuration method 2: =====

// These two macros are used in initializing endpoints if
// macro FIXED_ENDPIONT_COUNT is not defined.
// For each endpoint (by index) the actual Zigbee endpoint
// is determined from endpointNumber() and actual endpointType()
// from the endpointType(). These are primarily used
// for dynamic run-time endpoint determination and would typically
// be more complex than these examples below. They may call an actual
// C function that would look at registers and GPIO configuration.
#define endpointNumber(index) (50 + index)
#define endpointTypeMacro(index)   ((EmberAfEndpointType*)&(generatedEmberAfEndpointTypes[0]))
#define endpointNetworkIndex(index) (0)
#define endpointProfileId(index) (0xABBA)
#define endpointDeviceId(index)  (0xBEEF)
#define endpointDeviceVersion(index) (0xBA)

// Just to get the defaults
#include "tokens.h"

#endif // __ATTRIBUTE_STORAGE_TEST__
