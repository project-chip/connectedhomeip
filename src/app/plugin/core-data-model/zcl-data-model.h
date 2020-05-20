/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *      This file provides the API to the attribute database implementation
 *      used by the CHIP ZCL Application Layer
 *
 */

#ifndef CHIP_ZCL_ATTRIBUTE_DB
#define CHIP_ZCL_ATTRIBUTE_DB

#include "chip-zcl.h"

#include "gen.h"

#define CHIP_ZCL_INVALID_CLUSTER_ID 0xFFFF

#define CHIP_ZCL_INVALID_ENDPOINT 0xFF

#define CHIP_ZCL_INVALID_PAN_ID 0xFFFF

/**
 * @brief macro that returns true if the cluster is in the manufacturer specific range
 *
 * @param cluster EmberAfCluster* to consider
 */
#define chipZclClusterIsManufacturerSpecific(cluster) ((cluster)->clusterId >= 0xFC00)

/**
 * @brief macro that returns true if attribute is read only.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsReadOnly(metadata) (((metadata)->mask & ATTRIBUTE_MASK_WRITABLE) == 0)

/**
 * @brief macro that returns true if client attribute, and false if server.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsClient(metadata) (((metadata)->mask & ATTRIBUTE_MASK_CLIENT) != 0)

/**
 * @brief macro that returns true if attribute is saved to token.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsTokenized(metadata) (((metadata)->mask & ATTRIBUTE_MASK_TOKENIZE) != 0)

/**
 * @brief macro that returns true if attribute is saved in external storage.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsExternal(metadata) (((metadata)->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) != 0)

/**
 * @brief macro that returns true if attribute is a singleton
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsSingleton(metadata) (((metadata)->mask & ATTRIBUTE_MASK_SINGLETON) != 0)

/**
 * @brief macro that returns true if attribute is manufacturer specific
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsManufacturerSpecific(metadata) (((metadata)->mask & ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC) != 0)

/**
 * @brief macro that returns size of attribute in bytes.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeSize(metadata) ((metadata)->size)

/**
 *@brief Returns true if type is signed, false otherwise.
 */
bool chipZclIsTypeSigned(ChipZclAttributeType dataType);

/**
 * @brief Simple integer comparison function.
 * Compares two values of a known length as integers.
 * Signed integer comparison are supported for numbers with length of
 * 4 (bytes) or less.
 * The integers are in native endianness.
 *
 * @return -1, if val1 is smaller
 *          0, if they are the same or if two negative numbers with length
 *          greater than 4 is being compared
 *          1, if val2 is smaller.
 */
int8_t chipZclCompareValues(uint8_t * val1, uint8_t * val2, uint8_t len, bool signedNumber);

/**
 * @brief Returns the total number of endpoints (dynamic and pre-compiled).
 */
uint8_t chipZclEndpointCount(void);

/*
 * @brief Function that determines the length of a zigbee Cluster Library string
 *   (where the first byte is assumed to be the length).
 */
uint8_t chipZclStringLength(const uint8_t * buffer);
/*
 * @brief Function that determines the length of a zigbee Cluster Library long string.
 *   (where the first two bytes are assumed to be the length).
 */
uint16_t chipZclLongStringLength(const uint8_t * buffer);

/*
 * @brief Function that returns a metadata structure for the given attribute.
 * Returns null if none is found.
 */
ChipZclAttributeMetadata * chipZclLocateAttributeMetadata(uint8_t endpoint, ChipZclClusterId clusterId,
                                                          ChipZclAttributeId attributeId, uint8_t mask, uint16_t manufacturerCode);

/*
 * @brief Initialization of the endpoint structure. Before this call, endpoints are not enabled. After this call endpoints
 * are enabled and linked together properly for use.
 */
void chipZclEndpointInit(void);

#endif // SILABS_AF_API_TYPES
