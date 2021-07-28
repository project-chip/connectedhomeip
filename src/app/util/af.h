/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 * @file
 * @brief The master include file for the Ember
 *ApplicationFramework  API.
 *******************************************************************************
 ******************************************************************************/

/**
 * @addtogroup af Zigbee Application Framework API Reference
 * This documentation describes the application programming interface (API)
 * for the Zigbee Application Framework.
 * The file af.h is the master include file for the Zigbee Application
 * Framework modules.
 * @{
 */

#pragma once

#ifndef CONFIGURATION_HEADER
#define CONFIGURATION_HEADER <app/util/config.h>
#endif
#include CONFIGURATION_HEADER

#ifdef EZSP_HOST
// Includes needed for ember related functions for the EZSP host
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp-utils.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/serial-interface.h"
#include "stack/include/ember-random-api.h"
#include "stack/include/ember-types.h"
#include "stack/include/error.h"
#endif // EZSP_HOST

#include <app/util/af-types.h>

#include <app/util/client-api.h>
#include <app/util/debug-printing.h>
#include <app/util/ember-print.h>

#include <lib/support/SafeInt.h>

/** @name Attribute Storage */
// @{

/**
 * @brief locate attribute metadata
 *
 * Function returns pointer to the attribute metadata structure,
 * or NULL if attribute was not found.
 *
 * @param endpoint Zigbee endpoint number.
 * @param clusterId Cluster ID of the sought cluster.
 * @param attributeId Attribute ID of the sought attribute.
 * @param mask CLUSTER_MASK_SERVER or CLUSTER_MASK_CLIENT
 *
 * @return Returns pointer to the attribute metadata location.
 */
EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                          chip::AttributeId attributeId, uint8_t mask, uint16_t manufacturerCode);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Returns true if the attribute exists. */
bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId, uint8_t mask,
                              uint16_t manufacturerCode);
#else
#define emberAfContainsAttribute(endpoint, clusterId, attributeId, mask, manufacturerCode)                                         \
    (emberAfLocateAttributeMetadata(endpoint, clusterId, attributeId, mask, manufacturerCode) != NULL)
#endif

/**
 * @brief Returns true if endpoint contains a cluster, checking for mfg code.
 *
 * This function returns true regardless of whether
 * the endpoint contains server, client or both.
 * For standard libraries (when ClusterId < FC00),
 * the manufacturerCode is ignored.
 */
bool emberAfContainsClusterWithMfgCode(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode);

/**
 * @brief Returns true if endpoint contains the ZCL cluster with specified id.
 *
 * This function returns true regardless of whether
 * the endpoint contains server, client or both in the Zigbee cluster Library.
 * This wraps emberAfContainsClusterWithMfgCode with
 * manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE
 * If this function is used with a manufacturer specific clusterId
 * then this will return the first cluster that it finds in the Cluster table.
 * and will not return any other clusters that share that id.
 */
bool emberAfContainsCluster(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint has cluster server, checking for mfg code.
 *
 * This function returns true if
 * the endpoint contains server of a given cluster.
 * For standard librarys (when ClusterId < FC00), the manufacturerCode is ignored.
 */
bool emberAfContainsServerWithMfgCode(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode);

/**
 * @brief Returns true if endpoint contains the ZCL server with specified id.
 *
 * This function returns true if
 * the endpoint contains server of a given cluster.
 * This wraps emberAfContainsServer with
 * manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE
 * If this function is used with a manufacturer specific clusterId
 * then this will return the first cluster that it finds in the Cluster table.
 * and will not return any other clusters that share that id.
 */
bool emberAfContainsServer(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint contains cluster client.
 *
 * This function returns true if
 * the endpoint contains client of a given cluster.
 * For standard library clusters (when ClusterId < FC00),
 * the manufacturerCode is ignored.
 */
bool emberAfContainsClientWithMfgCode(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode);

/**
 * @brief Returns true if endpoint contains the ZCL client with specified id.
 *
 * This function returns true if
 * the endpoint contains client of a given cluster.
 * This wraps emberAfContainsClient with
 * manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE
 * If this function is used with a manufacturer specific clusterId
 * then this will return the first cluster that it finds in the Cluster table.
 * and will not return any other clusters that share that id.
 */
bool emberAfContainsClient(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief write an attribute, performing all the checks.
 *
 * This function will attempt to write the attribute value from
 * the provided pointer. This function will only check that the
 * attribute exists. If it does it will write the value into
 * the attribute table for the given attribute.
 *
 * This function will not check to see if the attribute is
 * writable since the read only / writable characteristic
 * of an attribute only pertains to external devices writing
 * over the air. Because this function is being called locally
 * it assumes that the device knows what it is doing and has permission
 * to perform the given operation.
 *
 * @see emberAfWriteClientAttribute, emberAfWriteServerAttribute,
 *      emberAfWriteManufacturerSpecificClientAttribute,
 *      emberAfWriteManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t mask,
                                    uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief write a cluster server attribute.
 *
 * This function is the same as emberAfWriteAttribute
 * except that it saves having to pass the cluster mask.
 * This is useful for code savings since write attribute
 * is used frequently throughout the framework
 *
 * @see emberAfWriteClientAttribute,
 *      emberAfWriteManufacturerSpecificClientAttribute,
 *      emberAfWriteManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfWriteServerAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                          uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief write a cluster client attribute.
 *
 * This function is the same as emberAfWriteAttribute
 * except that it saves having to pass the cluster mask.
 * This is useful for code savings since write attribute
 * is used frequently throughout the framework
 *
 * @see emberAfWriteServerAttribute,
 *      emberAfWriteManufacturerSpecificClientAttribute,
 *      emberAfWriteManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfWriteClientAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                          uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief write a manufacturer specific server attribute.
 *
 * This function is the same as emberAfWriteAttribute
 * except that it saves having to pass the cluster mask
 * and allows passing of a manufacturer code.
 * This is useful for code savings since write attribute
 * is used frequently throughout the framework
 *
 * @see emberAfWriteClientAttribute, emberAfWriteServerAttribute,
 *      emberAfWriteManufacturerSpecificClientAttribute
 */
EmberAfStatus emberAfWriteManufacturerSpecificServerAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                              chip::AttributeId attributeID, uint16_t manufacturerCode,
                                                              uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief write a manufacturer specific client attribute.
 *
 * This function is the same as emberAfWriteAttribute
 * except that it saves having to pass the cluster mask.
 * and allows passing of a manufacturer code.
 * This is useful for code savings since write attribute
 * is used frequently throughout the framework
 *
 * @see emberAfWriteClientAttribute, emberAfWriteServerAttribute,
 *      emberAfWriteManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfWriteManufacturerSpecificClientAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                              chip::AttributeId attributeID, uint16_t manufacturerCode,
                                                              uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief Function that test the success of attribute write.
 *
 * This function returns success if attribute write would be successful.
 * It does not actually write anything, just validates for read-only and
 * data-type.
 *
 * @param endpoint Zigbee endpoint number
 * @param cluster Cluster ID of the sought cluster.
 * @param attributeID Attribute ID of the sought attribute.
 * @param mask CLUSTER_MASK_SERVER or CLUSTER_MASK_CLIENT
 * @param dataPtr Location where attribute will be written from.
 * @param dataType ZCL attribute type.
 */
EmberAfStatus emberAfVerifyAttributeWrite(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                          uint8_t mask, uint16_t manufacturerCode, uint8_t * dataPtr,
                                          EmberAfAttributeType dataType);

/**
 * @brief Read the attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store
 * it into the pointer. It will also read the data type.
 * Both dataPtr and dataType may be NULL, signifying that either
 * value or type is not desired.
 *
 * @see emberAfReadClientAttribute, emberAfReadServerAttribute,
 *      emberAfReadManufacturerSpecificClientAttribute,
 *      emberAfReadManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t mask,
                                   uint8_t * dataPtr, uint16_t readLength, EmberAfAttributeType * dataType);

/**
 * @brief Read the server attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store
 * it into the pointer. It will also read the data type.
 * Both dataPtr and dataType may be NULL, signifying that either
 * value or type is not desired.
 *
 * @see emberAfReadClientAttribute,
 *      emberAfReadManufacturerSpecificClientAttribute,
 *      emberAfReadManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfReadServerAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                         uint8_t * dataPtr, uint16_t readLength);

/**
 * @brief Read the client attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store
 * it into the pointer. It will also read the data type.
 * Both dataPtr and dataType may be NULL, signifying that either
 * value or type is not desired.
 *
 * @see emberAfReadServerAttribute,
 *      emberAfReadManufacturerSpecificClientAttribute,
 *      emberAfReadManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfReadClientAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                         uint8_t * dataPtr, uint16_t readLength);

/**
 * @brief Read the manufacturer-specific server attribute value, performing all checks.
 *
 * This function will attempt to read the attribute and store
 * it into the pointer. It will also read the data type.
 * Both dataPtr and dataType may be NULL, signifying that either
 * value or type is not desired.
 *
 * @see emberAfReadClientAttribute, emberAfReadServerAttribute,
 *      emberAfReadManufacturerSpecificClientAttribute
 */
EmberAfStatus emberAfReadManufacturerSpecificServerAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                             chip::AttributeId attributeID, uint16_t manufacturerCode,
                                                             uint8_t * dataPtr, uint16_t readLength);

/**
 * @brief Read the manufacturer-specific client attribute value, performing all checks.
 *
 * This function will attempt to read the attribute and store
 * it into the pointer. It will also read the data type.
 * Both dataPtr and dataType may be NULL, signifying that either
 * value or type is not desired.
 *
 * @see emberAfReadClientAttribute, emberAfReadServerAttribute,
 *      emberAfReadManufacturerSpecificServerAttribute
 */
EmberAfStatus emberAfReadManufacturerSpecificClientAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                             chip::AttributeId attributeID, uint16_t manufacturerCode,
                                                             uint8_t * dataPtr, uint16_t readLength);

/**
 * @brief this function returns the size of the ZCL data in bytes.
 *
 * @param dataType Zcl data type
 * @return size in bytes or 0 if invalid data type
 */
uint8_t emberAfGetDataSize(uint8_t dataType);

/**
 * @brief macro that returns true if the cluster is in the manufacturer specific range
 *
 * @param cluster EmberAfCluster* to consider
 */
#define emberAfClusterIsManufacturerSpecific(cluster) ((cluster)->clusterId >= 0xFC00)

/**
 * @brief macro that returns true if attribute is read only.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsReadOnly(metadata) (((metadata)->mask & ATTRIBUTE_MASK_WRITABLE) == 0)

/**
 * @brief macro that returns true if client attribute, and false if server.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsClient(metadata) (((metadata)->mask & ATTRIBUTE_MASK_CLIENT) != 0)

/**
 * @brief macro that returns true if attribute is saved to token.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsTokenized(metadata) (((metadata)->mask & ATTRIBUTE_MASK_TOKENIZE) != 0)

/**
 * @brief macro that returns true if attribute is saved in external storage.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsExternal(metadata) (((metadata)->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) != 0)

/**
 * @brief macro that returns true if attribute is a singleton
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsSingleton(metadata) (((metadata)->mask & ATTRIBUTE_MASK_SINGLETON) != 0)

/**
 * @brief macro that returns true if attribute is manufacturer specific
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeIsManufacturerSpecific(metadata) (((metadata)->mask & ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC) != 0)

/**
 * @brief macro that returns size of attribute in bytes.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeSize(metadata) ((metadata)->size)

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
// master array of all defined endpoints
extern EmberAfDefinedEndpoint emAfEndpoints[];
#endif

/**
 * @brief Macro that takes index of endpoint, and returns Zigbee endpoint
 */
chip::EndpointId emberAfEndpointFromIndex(uint16_t index);

/**
 * Returns the index of a given endpoint
 */
uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint);

/**
 * Returns the index of a given endpoint; Does not ignore disabled endpoints
 */
uint16_t emberAfIndexFromEndpointIncludingDisabledEndpoints(chip::EndpointId endpoint);

/**
 * Returns the endpoint index within a given cluster (Client-side),
 * looking only for standard clusters.
 */
uint16_t emberAfFindClusterClientEndpointIndex(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * Returns the endpoint index within a given cluster (Server-side),
 * looking only for standard clusters.
 */
uint16_t emberAfFindClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Macro that takes index of endpoint, and returns device Id for it
 */
#define emberAfDeviceIdFromIndex(index) (emAfEndpoints[(index)].deviceId)

/**
 * @brief Macro that takes index of endpoint, and returns device version for it
 */
#define emberAfDeviceVersionFromIndex(index) (emAfEndpoints[(index)].deviceVersion)

/**
 * @brief Macro that takes index of endpoint, and returns network index for it
 */
#define emberAfNetworkIndexFromEndpointIndex(index) (emAfEndpoints[(index)].networkIndex)

/**
 * @brief Macro that returns the primary endpoint.
 */
#define emberAfPrimaryEndpoint() (emAfEndpoints[0].endpoint)

/**
 * @brief Returns the total number of endpoints (dynamic and pre-compiled).
 */
uint16_t emberAfEndpointCount(void);

/**
 * @brief Returns the number of pre-compiled endpoints.
 */
uint16_t emberAfFixedEndpointCount(void);

/**
 * Data types are either analog or discrete. This makes a difference for
 * some of the ZCL global commands
 */
enum
{
    EMBER_AF_DATA_TYPE_ANALOG   = 0,
    EMBER_AF_DATA_TYPE_DISCRETE = 1,
    EMBER_AF_DATA_TYPE_NONE     = 2
};

/**
 * @brief Returns the type of the attribute, either ANALOG, DISCRETE or NONE
 */
uint8_t emberAfGetAttributeAnalogOrDiscreteType(uint8_t dataType);

/**
 *@brief Returns true if type is signed, false otherwise.
 */
bool emberAfIsTypeSigned(EmberAfAttributeType dataType);

/**
 * @brief Function that extracts a 64-bit integer from the message buffer
 */
uint64_t emberAfGetInt64u(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
#define emberAfGetInt64s(message, currentIndex, msgLen) chip::CastToSigned(emberAfGetInt64u(message, currentIndex, msgLen))

/**
 * @brief Function that extracts a 32-bit integer from the message buffer
 */
uint32_t emberAfGetInt32u(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
#define emberAfGetInt32s(message, currentIndex, msgLen) chip::CastToSigned(emberAfGetInt32u(message, currentIndex, msgLen))

/**
 * @brief Function that extracts a 24-bit integer from the message buffer
 */
uint32_t emberAfGetInt24u(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
#define emberAfGetInt24s(message, currentIndex, msgLen) chip::CastToSigned(emberAfGetInt24u(message, currentIndex, msgLen))

/**
 * @brief Function that extracts a 16-bit integer from the message buffer
 */
uint16_t emberAfGetInt16u(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
#define emberAfGetInt16s(message, currentIndex, msgLen) chip::CastToSigned(emberAfGetInt16u(message, currentIndex, msgLen))

/**
 * @brief Function that extracts a ZCL string from the message buffer
 */
uint8_t * emberAfGetString(uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
/**
 * @brief Function that extracts a ZCL long string from the message buffer
 */
uint8_t * emberAfGetLongString(uint8_t * message, uint16_t currentIndex, uint16_t msgLen);
/*
 * @brief Function that extracts a ZCL Date from the message buffer and returns it
 * in the given destination. Returns the number of bytes copied.
 */
uint8_t emberAfGetDate(uint8_t * message, uint16_t currentIndex, uint16_t msgLen, EmberAfDate * destination);

/**
 * @brief Macro for consistency, that extracts single byte out of the message
 */
#define emberAfGetInt8u(message, currentIndex, msgLen) message[currentIndex]
#define emberAfGetInt8s(message, currentIndex, msgLen) chip::CastToSigned(emberAfGetInt8u(message, currentIndex, msgLen))

/**
 * @brief Macro for consistency that copies a uint8_t from variable into buffer.
 */
#define emberAfCopyInt8u(data, index, x) (data[index] = (x))
/**
 * @brief function that copies a uint16_t value into a buffer
 */
void emberAfCopyInt16u(uint8_t * data, uint16_t index, uint16_t x);
/**
 * @brief function that copies a uint24_t value into a buffer
 */
void emberAfCopyInt24u(uint8_t * data, uint16_t index, uint32_t x);
/**
 * @brief function that copies a uint32_t value into a buffer
 */
void emberAfCopyInt32u(uint8_t * data, uint16_t index, uint32_t x);
/*
 * @brief Function that copies a ZCL string type into a buffer.  The size
 * parameter should indicate the maximum number of characters to copy to the
 * destination buffer not including the length byte.
 */
void emberAfCopyString(uint8_t * dest, const uint8_t * src, size_t size);
/*
 * @brief Function that copies a ZCL long string into a buffer.  The size
 * parameter should indicate the maximum number of characters to copy to the
 * destination buffer not including the length bytes.
 */
void emberAfCopyLongString(uint8_t * dest, const uint8_t * src, size_t size);
/*
 * @brief Function that determines the length of a zigbee Cluster Library string
 *   (where the first byte is assumed to be the length).
 */
uint8_t emberAfStringLength(const uint8_t * buffer);
/*
 * @brief Function that determines the length of a zigbee Cluster Library long string.
 *   (where the first two bytes are assumed to be the length).
 */
uint16_t emberAfLongStringLength(const uint8_t * buffer);

/*
 * @brief Function that copies (part of) a ZCL typed list into a buffer. The index parameter
 * may indicate a specific member of the list, or the list length, or the whole list if it
 * is equal to -1.
 *
 * Individual elements may be accessed by an index of type 16-bit unsigned integer.
 * Elements are numbered from 1 upwards. The element with index 0 is always of type
 * uint16, and holds the number of elements contained in the list, which may be zero.
 * If the zeroth element contains 0xffff, the list is a non value and is considered
 * undefined.
 *
 * When writing, dest points to the list to write to, src points to the value to write, and index is the index to write at.
 *
 * When reading (i.e write is false), dest is the location to read into, src points to the list, and index is the index to read
 * from.
 *
 * When reading or writing if the index leads to read or write outside of the
 * allocated size for the list, this function will return 0.
 *
 * @return The number of bytes copied
 */
uint16_t emberAfCopyList(chip::ClusterId clusterId, EmberAfAttributeMetadata * am, bool write, uint8_t * dest, uint8_t * src,
                         int32_t index);

/*
 * @brief Function that determines the size of a zigbee Cluster Library
 * attribute value (where the attribute could be non-string, string, or long
 * string). For strings, the size includes the length of the string plus the
 * number of the string's length prefix byte(s).
 */
uint16_t emberAfAttributeValueSize(chip::ClusterId clusterId, chip::AttributeId attributeId, EmberAfAttributeType dataType,
                                   const uint8_t * buffer);

/*
 * @brief Function that determines the size of a zigbee Cluster Library
 * attribute List[T] where T could be of any type.
 * The size is expressed in bytes, and includes the used length consumed
 * by list entries plus the 2 bytes used to represent the number of actual
 * entries in the list.
 */
uint16_t emberAfAttributeValueListSize(chip::ClusterId clusterId, chip::AttributeId attributeId, const uint8_t * buffer);

/** @} END Attribute Storage */

/** @name Device Control */
// @{

/**
 * @brief Function that checks if endpoint is enabled.
 *
 * This function returns true if device at a given endpoint is
 * enabled. At startup all endpoints are enabled.
 *
 * @param endpoint Zigbee endpoint number
 */
bool emberAfIsDeviceEnabled(chip::EndpointId endpoint);

/**
 * @brief Function that checks if endpoint is identifying
 *
 * This function returns true if device at a given endpoint is
 * identifying.
 *
 * @param endpoint Zigbee endpoint number
 */
bool emberAfIsDeviceIdentifying(chip::EndpointId endpoint);

/**
 * @brief Function that enables or disables an endpoint.
 *
 * By calling this function, you turn off all processing of incoming traffic
 * for a given endpoint.
 *
 * @param endpoint Zigbee endpoint number
 */
void emberAfSetDeviceEnabled(chip::EndpointId endpoint, bool enabled);

/** @} END Device Control */

/** @name Miscellaneous */
// @{

/**
 * @brief Enable/disable endpoints
 */
bool emberAfEndpointEnableDisable(chip::EndpointId endpoint, bool enable);

/**
 * @brief Determine if an endpoint at the specified index is enabled or disabled
 */
bool emberAfEndpointIndexIsEnabled(uint16_t index);

/**
 * @brief Returns true if a given ZCL data type is a string type.
 *
 * You should use this function if you need to perform a different
 * memory operation on a certain attribute because it is a string type.
 * Since ZCL strings carry length as the first byte(s), it is often required
 * to treat them differently than regular data types.
 *
 * @return true if data type is a string.
 */
bool emberAfIsThisDataTypeAStringType(EmberAfAttributeType dataType);

/** @brief Returns true if the given attribute type is a string. */
bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType);

/** @brief Returns true if the given attribute type is a long string. */
bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType);

/** @brief Returns true if a given ZCL data type is a list type. */
bool emberAfIsThisDataTypeAListType(EmberAfAttributeType dataType);

/**
 * @brief The mask applied by ::emberAfNextSequence when generating ZCL
 * sequence numbers.
 */
#define EMBER_AF_ZCL_SEQUENCE_MASK 0x7F

/**
 * @brief The mask applied to generated message tags used by the framework when sending messages via EZSP.
 * Customers who call ezspSend functions directly must use message tags outside this mask
 */
#define EMBER_AF_MESSAGE_TAG_MASK 0x7F

/**
 * @brief Increments the ZCL sequence number and returns the value.
 *
 * ZCL messages have sequence numbers so that they can be matched up with other
 * messages in the transaction.  To avoid conflicts with sequence numbers
 * generated independently by the application, this API returns sequence
 * numbers with the high bit clear.  If the application generates its own
 * sequence numbers, it should use numbers with the high bit set.
 *
 * @return The next ZCL sequence number.
 */
uint8_t emberAfNextSequence(void);

/**
 * @brief Retrieves the last sequence number that was used.
 *
 */
uint8_t emberAfGetLastSequenceNumber(void);

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
int8_t emberAfCompareValues(uint8_t * val1, uint8_t * val2, uint16_t len, bool signedNumber);

/**
 * @brief populates the passed EUI64 with the local EUI64 MAC address.
 */
void emberAfGetEui64(EmberEUI64 returnEui64);

#if (BIGENDIAN_CPU) || defined(EZSP_HOST)
// Normally this is provided by the stack code, but on the host
// it is provided by the application code.
void emberReverseMemCopy(uint8_t * dest, const uint8_t * src, uint16_t length);
#endif

/**
 * @brief Returns the node ID of the local node.
 */
EmberNodeId emberAfGetNodeId(void);

#if defined(DOXYGEN_SHOULD_SKIP_THIS) || defined(EZSP_HOST)
/**
 * @brief Generates a random key (link, network, or master).
 */
EmberStatus emberAfGenerateRandomKey(EmberKeyData * result);
#else
#define emberAfGenerateRandomKey(result) emberGenerateRandomKey(result)
#endif

/**
 * @brief Returns the PAN ID of the local node.
 */
EmberPanId emberAfGetPanId(void);

/**
 * @brief Returns the radioChannel of the current network
 */
uint8_t emberAfGetRadioChannel(void);

/*
 * @brief Returns a binding index that matches the current incoming message, if
 * known.
 */
uint8_t emberAfGetBindingIndex(void);

/*
 * @brief Returns an address index that matches the current incoming message,
 * if known.
 */
uint8_t emberAfGetAddressIndex(void);

/**
 * @brief Returns the current network state.  This call caches the results
 *   on the host to prevent frequent EZSP transactions.
 */
EmberNetworkStatus emberAfNetworkState(void);

/**
 * @brief Get this node's radio channel for the current network.
 */
uint8_t emberAfGetRadioChannel(void);

/**
 * @brief Returns the current network parameters.
 */
EmberStatus emberAfGetNetworkParameters(EmberNodeType * nodeType, EmberNetworkParameters * parameters);

/**
 * @brief Returns the current node type.
 */
EmberStatus emberAfGetNodeType(EmberNodeType * nodeType);

/**
 */
#define EMBER_AF_REJOIN_DUE_TO_END_DEVICE_MOVE 0xA0
#define EMBER_AF_REJOIN_DUE_TO_TC_KEEPALIVE_FAILURE 0xA1
#define EMBER_AF_REJOIN_DUE_TO_CLI_COMMAND 0xA2
#define EMBER_AF_REJOIN_DUE_TO_WWAH_CONNECTIVITY_MANAGER 0xA3

#define EMBER_AF_REJOIN_FIRST_REASON EMBER_AF_REJOIN_DUE_TO_END_DEVICE_MOVE
#define EMBER_AF_REJOIN_LAST_REASON EMBER_AF_REJOIN_DUE_TO_END_DEVICE_MOVE

/**
 * @brief Enables local permit join and optionally broadcasts the ZDO
 * Mgmt_Permit_Join_req message. This API can be called from any device
 * type and still return EMBER_SUCCESS. If the API is called from an
 * end device, the permit association bit will just be left off.
 *
 * @param duration the duration that the permit join bit will remain on
 * and other devices will be able to join the current network.
 * @param broadcastMgmtPermitJoin whether or not to broadcast the ZDO
 * Mgmt_Permit_Join_req message.
 *
 * @returns status of whether or not permit join was enabled.
 */
EmberStatus emberAfPermitJoin(uint8_t duration, bool broadcastMgmtPermitJoin);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Enables local permit join and broadcasts the ZDO
 * Mgmt_Permit_Join_req message. This API can be called from any device
 * type and still return EMBER_SUCCESS. If the API is called from an
 * end device, the permit association bit will just be left off.
 *
 * @param duration the duration that the permit join bit will remain on
 * and other devices will be able to join the current network.
 *
 * @returns status of whether or not permit join was enabled.
 */
EmberStatus emberAfBroadcastPermitJoin(uint8_t duration);
#else
#define emberAfBroadcastPermitJoin(duration) emberAfPermitJoin((duration), true)
#endif

/** @} END Miscellaneous */

/** @name Sleep Control */
//@{

/**
 * @brief A function used to add a task to the task register.
 */
#define emberAfAddToCurrentAppTasks(x) emberAfAddToCurrentAppTasksCallback(x)

/**
 * @brief A function used to remove a task from the task register.
 */
#define emberAfRemoveFromCurrentAppTasks(x) emberAfRemoveFromCurrentAppTasksCallback(x)

/**
 * @brief A macro used to retrieve the bitmask of all application
 * frameowrk tasks currently in progress. This can be useful for debugging if
 * some task is holding the device out of hibernation.
 */
#define emberAfCurrentAppTasks() emberAfGetCurrentAppTasksCallback()

/**
 * @brief a function used to run the application framework's
 *        event mechanism. This function passes the application
 *        framework's event tables to the ember stack's event
 *        processing code.
 */
void emberAfRunEvents(void);

/**
 * @brief Friendly define for use in the scheduling or canceling client events
 * with emberAfScheduleClusterTick() and emberAfDeactivateClusterTick().
 */
#define EMBER_AF_CLIENT_CLUSTER_TICK true

/**
 * @brief Friendly define for use in the scheduling or canceling server events
 * with emberAfScheduleClusterTick() and emberAfDeactivateClusterTick().
 */
#define EMBER_AF_SERVER_CLUSTER_TICK false

/**
 * @brief This function is used to schedule a cluster-related event inside the
 * application framework's event mechanism.  This function provides a wrapper
 * for the Ember stack event mechanism which allows the cluster code to access
 * its events by their endpoint, cluster id, and client/server identity.  The
 * passed poll and sleep controls allow the cluster to indicate whether it
 * needs to long or short poll and whether it needs to stay awake or if it can
 * sleep.
 *
 * @param endpoint the endpoint of the event to be scheduled.
 * @param clusterId the cluster id of the event to be scheduled.
 * @param isClient ::EMBER_AF_CLIENT_CLUSTER_TICK if the event to be scheduled
 *        is associated with a client cluster or ::EMBER_AF_SERVER_CLUSTER_TICK
 *        otherwise.
 * @param delayMs the number of milliseconds until the event should be called.
 * @param pollControl ::EMBER_AF_SHORT_POLL if the cluster needs to short poll
 *        or ::EMBER_AF_LONG_POLL otherwise.
 * @param sleepControl ::EMBER_AF_STAY_AWAKE if the cluster needs to stay awake
 *        or EMBER_AF_OK_TO_SLEEP otherwise.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleTickExtended(chip::EndpointId endpoint, chip::ClusterId clusterId, bool isClient, uint32_t delayMs,
                                        EmberAfEventPollControl pollControl, EmberAfEventSleepControl sleepControl);

/**
 * @brief This function is used to schedule a cluster-related event inside the
 * This function is a wrapper for ::emberAfScheduleTickExtended.  The cluster
 * on the given endpoint will be set to long poll if sleepControl is set to
 * ::EMBER_AF_OK_TO_HIBERNATE or will be set to short poll otherwise.  It will
 * stay awake if sleepControl is ::EMBER_AF_STAY_AWAKE and will sleep
 * otherwise.
 *
 * @param endpoint the endpoint of the event to be scheduled.
 * @param clusterId the cluster id of the event to be scheduled.
 * @param isClient ::EMBER_AF_CLIENT_CLUSTER_TICK if the event to be scheduled
 *        is associated with a client cluster or ::EMBER_AF_SERVER_CLUSTER_TICK
 *        otherwise.
 * @param delayMs the number of milliseconds until the event should be called.
 * @param sleepControl the priority of the event, what the processor should
 *        be allowed to do in terms of sleeping while the event is active.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleClusterTick(chip::EndpointId endpoint, chip::ClusterId clusterId, bool isClient, uint32_t delayMs,
                                       EmberAfEventSleepControl sleepControl);

/**
 * @brief A function used to schedule a cluster client event.  This function
 * is a wrapper for ::emberAfScheduleTickExtended.
 *
 * @param endpoint the endpoint of the event to be scheduled
 * @param clusterId the cluster id of the event to be scheduled
 * @param delayMs the number of milliseconds until the event should be called.
 * @param pollControl ::EMBER_AF_SHORT_POLL if the cluster needs to short poll
 *        or ::EMBER_AF_LONG_POLL otherwise.
 * @param sleepControl ::EMBER_AF_STAY_AWAKE if the cluster needs to stay awake
 *        or EMBER_AF_OK_TO_SLEEP otherwise.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleClientTickExtended(chip::EndpointId endpoint, chip::ClusterId clusterId, uint32_t delayMs,
                                              EmberAfEventPollControl pollControl, EmberAfEventSleepControl sleepControl);

/**
 * @brief A function used to schedule a cluster client event.  This function
 * is a wrapper for ::emberAfScheduleClientTickExtended.  It indicates that
 * the cluster client on the given endpoint can long poll and can sleep.
 *
 * @param endpoint the endpoint of the event to be scheduled.
 * @param clusterId the cluster id of the event to be scheduled.
 * @param delayMs the number of milliseconds until the event should be called.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleClientTick(chip::EndpointId endpoint, chip::ClusterId clusterId, uint32_t delayMs);

/**
 * @brief A function used to schedule a cluster server event.  This function
 * is a wrapper for ::emberAfScheduleTickExtended.
 *
 * @param endpoint the endpoint of the event to be scheduled.
 * @param clusterId the cluster id of the event to be scheduled.
 * @param delayMs the number of milliseconds until the event should be called.
 * @param pollControl ::EMBER_AF_SHORT_POLL if the cluster needs to short poll
 *        or ::EMBER_AF_LONG_POLL otherwise.
 * @param sleepControl ::EMBER_AF_STAY_AWAKE if the cluster needs to stay awake
 *        or EMBER_AF_OK_TO_SLEEP otherwise.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleServerTickExtended(chip::EndpointId endpoint, chip::ClusterId clusterId, uint32_t delayMs,
                                              EmberAfEventPollControl pollControl, EmberAfEventSleepControl sleepControl);

/**
 * @brief A function used to schedule a cluster server event.  This function
 * is a wrapper for ::emberAfScheduleServerTickExtended.  It indicates that
 * the cluster server on the given endpoint can long poll and can sleep.
 *
 * @param endpoint the endpoint of the event to be scheduled
 * @param clusterId the cluster id of the event to be scheduled.
 * @param delayMs the number of milliseconds until the event should be called.
 *
 * @return EMBER_SUCCESS if the event was scheduled or an error otherwise.
 */
EmberStatus emberAfScheduleServerTick(chip::EndpointId endpoint, chip::ClusterId clusterId, uint32_t delayMs);

/**
 * @brief A function used to deactivate a cluster-related event.  This function
 * provides a wrapper for the Ember stack's event mechanism which allows an
 * event to be accessed by its endpoint, cluster id, and client/server
 * identity.
 *
 * @param endpoint the endpoint of the event to be deactivated.
 * @param clusterId the cluster id of the event to be deactivated.
 * @param isClient ::EMBER_AF_CLIENT_CLUSTER_TICK if the event to be
 *        deactivated is a client cluster ::EMBER_AF_SERVER_CLUSTER_TICK
 *        otherwise.
 *
 * @return EMBER_SUCCESS if the event was deactivated or an error otherwise.
 */
EmberStatus emberAfDeactivateClusterTick(chip::EndpointId endpoint, chip::ClusterId clusterId, bool isClient);

/**
 * @brief A function used to deactivate a cluster client event.  This function
 * is a wrapper for ::emberAfDeactivateClusterTick.
 *
 * @param endpoint the endpoint of the event to be deactivated.
 * @param clusterId the cluster id of the event to be deactivated.
 *
 * @return EMBER_SUCCESS if the event was deactivated or an error otherwise.
 */
EmberStatus emberAfDeactivateClientTick(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief A function used to deactivate a cluster server event.  This function
 * is a wrapper for ::emberAfDeactivateClusterTick.
 *
 * @param endpoint the endpoint of the event to be deactivated.
 * @param clusterId the cluster id of the event to be deactivated.
 *
 * @return EMBER_SUCCESS if the event was deactivated or an error otherwise.
 */
EmberStatus emberAfDeactivateServerTick(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Sets the ::EmberEventControl to run "delayMs" milliseconds in the
 * future.  This function first verifies that the delay is within the
 * acceptable range before scheduling the event.
 *
 * @param control a pointer to the event control.
 * @param delayMs the number of milliseconds until the next event.
 *
 * @return If delayMs is less than or equal to
           ::EMBER_MAX_EVENT_CONTROL_DELAY_MS, this function will schedule the
           event and return ::EMBER_SUCCESS.  Otherwise it will return
           ::EMBER_BAD_ARGUMENT.
 */
EmberStatus emberEventControlSetDelayMS(EmberEventControl * control, uint32_t delayMs);

/**
 * @brief Sets the ::EmberEventControl to run "delayQs" quarter seconds in the
 * future.  The 'quarter seconds' are actually 256 milliseconds long.  This
 * function first verifies that the delay is within the acceptable range before
 * scheduling the event.
 *
 * @param control a pointer to the event control.
 * @param delayQs the number of quarter seconds until the next event.
 *
 * @return If delayQs is less than or equal to
           ::EMBER_MAX_EVENT_CONTROL_DELAY_QS, this function will schedule the
           event and return ::EMBER_SUCCESS.  Otherwise it will return
           ::EMBER_BAD_ARGUMENT.
 */
EmberStatus emberAfEventControlSetDelayQS(EmberEventControl * control, uint32_t delayQs);

/**
 * @brief Sets the ::EmberEventControl to run "delayM" minutes in the future.
 * The 'minutes' are actually 65536 (0x10000) milliseconds long.  This function
 * first verifies that the delay is within the acceptable range before
 * scheduling the event.
 *
 * @param control a pointer to the event control.
 * @param delayM the number of minutes until the next event.
 *
 * @return If delayM is less than or equal to
           ::EMBER_MAX_EVENT_CONTROL_DELAY_MINUTES, this function will schedule
           the event and return ::EMBER_SUCCESS.  Otherwise it will return
           ::EMBER_BAD_ARGUMENT.
 */
EmberStatus emberAfEventControlSetDelayMinutes(EmberEventControl * control, uint16_t delayM);

/**
 * @brief Sets the ::EmberEventControl for the current network, and only
 * the current network, as inactive.  See ::emberEventControlSetInactive.
 */
void emberAfNetworkEventControlSetInactive(EmberEventControl * controls);
/**
 * @brief Returns true if the event for the current network, and only the
 * current network, is active.  See ::emberEventControlGetActive.
 */
bool emberAfNetworkEventControlGetActive(EmberEventControl * controls);
/**
 * @brief Sets the ::EmberEventControl for the current network, and only
 * current network, to run at the next available opportunity.  See
 * ::emberEventControlSetActive.
 */
void emberAfNetworkEventControlSetActive(EmberEventControl * controls);
/**
 * @brief Sets the ::EmberEventControl for the current network, and only the
 * current network, to run "delayMs" milliseconds in the future.  See
 * ::emberEventControlSetDelayMS.
 */
EmberStatus emberAfNetworkEventControlSetDelayMS(EmberEventControl * controls, uint32_t delayMs);
#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Sets the ::EmberEventControl for the current network, and only the
 * current network, to run "delayMs" milliseconds in the future.  See
 * ::emberEventControlSetDelayMS.
 */
EmberStatus emberAfNetworkEventControlSetDelay(EmberEventControl * controls, uint32_t delayMs);
#else
#define emberAfNetworkEventControlSetDelay(controls, delayMs) emberAfNetworkEventControlSetDelayMS(controls, delayMs);
#endif
/**
 * @brief Sets the ::EmberEventControl for the current network, and only the
 * current network, to run "delayQs" quarter seconds in the future.  See
 * ::emberAfEventControlSetDelayQS.
 */
EmberStatus emberAfNetworkEventControlSetDelayQS(EmberEventControl * controls, uint32_t delayQs);
/**
 * @brief Sets the ::EmberEventControl for the current network, and only the
 * current network, to run "delayM" minutes in the future.  See
 * ::emberAfEventControlSetDelayMinutes.
 */
EmberStatus emberAfNetworkEventControlSetDelayMinutes(EmberEventControl * controls, uint16_t delayM);

/**
 * @brief Sets the ::EmberEventControl for the specified endpoint as inactive.
 * See ::emberEventControlSetInactive.
 */
EmberStatus emberAfEndpointEventControlSetInactive(EmberEventControl * controls, chip::EndpointId endpoint);
/**
 * @brief Returns true if the event for the current number is active.  See
 * ::emberEventControlGetActive.
 */
bool emberAfEndpointEventControlGetActive(EmberEventControl * controls, chip::EndpointId endpoint);
/**
 * @brief Sets the ::EmberEventControl for the specified endpoint to run at the
 * next available opportunity.  See ::emberEventControlSetActive.
 */
EmberStatus emberAfEndpointEventControlSetActive(EmberEventControl * controls, chip::EndpointId endpoint);
/**
 * @brief Sets the ::EmberEventControl for the specified endpoint to run
 * "delayMs" milliseconds in the future.  See ::emberEventControlSetDelayMS.
 */
EmberStatus emberAfEndpointEventControlSetDelayMS(EmberEventControl * controls, chip::EndpointId endpoint, uint32_t delayMs);
#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Sets the ::EmberEventControl for the specified endpoint to run
 * "delayMs" milliseconds in the future.  See ::emberEventControlSetDelayMS.
 */
EmberStatus emberAfEndpointEventControlSetDelay(EmberEventControl * controls, chip::EndpointId endpoint, uint32_t delayMs);
#else
#define emberAfEndpointEventControlSetDelay(controls, endpoint, delayMs)                                                           \
    emberAfEndpointEventControlSetDelayMS(controls, endpoint, delayMs);
#endif
/**
 * @brief Sets the ::EmberEventControl for the specified endpoint to run
 * "delayQs" quarter seconds in the future.  See
 * ::emberAfEventControlSetDelayQS.
 */
EmberStatus emberAfEndpointEventControlSetDelayQS(EmberEventControl * controls, chip::EndpointId endpoint, uint32_t delayQs);
/**
 * @brief Sets the ::EmberEventControl for the specified endpoint to run
 * "delayM" minutes in the future.  See ::emberAfEventControlSetDelayMinutes.
 */
EmberStatus emberAfEndpointEventControlSetDelayMinutes(EmberEventControl * controls, chip::EndpointId endpoint, uint16_t delayM);

/**
 * @brief A function used to retrieve the number of milliseconds until
 * the next event scheduled in the application framework's event
 * mechanism.
 * @param maxMs the maximum number of milliseconds until the next
 *              event.
 * @return The number of milliseconds until the next event or
 * maxMs if no event is scheduled before then.
 */
uint32_t emberAfMsToNextEvent(uint32_t maxMs);

/** @brief This is the same as the function emberAfMsToNextEvent() with the
 *  following addition.  If returnIndex is non-NULL it returns the index
 *  of the event that is ready to fire next.
 */
uint32_t emberAfMsToNextEventExtended(uint32_t maxMs, uint8_t * returnIndex);

/**
 * @brief A function used to retrieve the number of quarter seconds until
 * the next event scheduled in the application framework's event
 * mechanism. This function will round down and will return 0 if the
 * next event must fire within a quarter second.
 * @param maxQS, the maximum number of quarter seconds until the next
 *        event.
 * @return The number of quarter seconds until the next event or
 * maxQS if no event is scheduled before then.
 */
#define emberAfQSToNextEvent(maxQS)                                                                                                \
    (emberAfMsToNextEvent(maxQS * MILLISECOND_TICKS_PER_QUARTERSECOND) / MILLISECOND_TICKS_PER_QUARTERSECOND)

/**
 * @brief A function for retrieving the most restrictive sleep
 * control value for all scheduled events. This function is
 * used by emberAfOkToNap and emberAfOkToHibernate to makes sure
 * that there are no events scheduled which will keep the device
 * from hibernating or napping.
 * @return The most restrictive sleep control value for all
 *         scheduled events or the value returned by
 *         emberAfGetDefaultSleepControl()
 *         if no events are currently scheduled. The default
 *         sleep control value is initialized to
 *         EMBER_AF_OK_TO_HIBERNATE but can be changed at any
 *         time using the emberAfSetDefaultSleepControl() function.
 */
#define emberAfGetCurrentSleepControl() emberAfGetCurrentSleepControlCallback()

/**
 * @brief A function for setting the default sleep control
 *        value against which all scheduled event sleep control
 *        values will be evaluated. This can be used to keep
 *        a device awake for an extended period of time by setting
 *        the default to EMBER_AF_STAY_AWAKE and then resetting
 *        the value to EMBER_AF_OK_TO_HIBERNATE once the wake
 *        period is complete.
 */
#define emberAfSetDefaultSleepControl(x) emberAfSetDefaultSleepControlCallback(x)

/**
 * @brief A function used to retrieve the default sleep control against
 *        which all event sleep control values are evaluated. The
 *        default sleep control value is initialized to
 *        EMBER_AF_OK_TO_HIBERNATE but can be changed by the application
 *        at any time using the emberAfSetDefaultSleepControl() function.
 * @return The current default sleep control value.
 */
#define emberAfGetDefaultSleepControl() emberAfGetDefaultSleepControlCallback()

/** @} END Sleep Control */

/** @name Messaging */
// @{

/**
 * @brief This function sends a ZCL response, based on the information
 * that is currently in the outgoing buffer. It is expected that a complete
 * ZCL message is present, including header.  The application may use
 * this method directly from within the message handling function
 * and associated callbacks.  However this will result in the
 * response being sent before the APS Ack is sent which is not
 * ideal.
 *
 * NOTE:  This will overwrite the ZCL sequence number of the message
 * to use the LAST received sequence number.
 */
EmberStatus emberAfSendResponse(void);

/**
 * @brief Send ZCL response with attached message sent callback
 */
EmberStatus emberAfSendResponseWithCallback(EmberAfMessageSentFunction callback);

/**
 * @brief Sends multicast.
 */
EmberStatus emberAfSendMulticast(chip::GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message);

/**
 * @brief Multicasts the message to the group in the binding table that
 * matches the cluster and source endpoint in the APS frame.  Note: if the
 * binding table contains many matching entries, calling this API cause a
 * significant amount of network traffic. Care should be taken when considering
 * the effects of broadcasts in a network.
 */
EmberStatus emberAfSendMulticastToBindings(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message);

/**
 * @brief Sends Multicast with alias with attached message sent callback
 */
EmberStatus emberAfSendMulticastWithAliasWithCallback(chip::GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength,
                                                      uint8_t * message, EmberNodeId alias, uint8_t sequence,
                                                      EmberAfMessageSentFunction callback);

/**
 * @brief Sends multicast with attached message sent callback.
 */
EmberStatus emberAfSendMulticastWithCallback(chip::GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength,
                                             uint8_t * message, EmberAfMessageSentFunction callback);

/**
 * @brief Sends broadcast.
 */
// EmberStatus emberAfSendBroadcast(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message);

/**
 * @brief Sends broadcast with attached message sent callback.
 */
// EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength,
//                                             uint8_t * message, EmberAfMessageSentFunction callback);

/**
 * @brief Sends broadcast with alias with attached message sent callback.
 */
// EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength,
//                                                      uint8_t * message, EmberNodeId alias, uint8_t sequence,
//                                                      EmberAfMessageSentFunction callback);

/**
 * @brief Sends unicast.
 */
EmberStatus emberAfSendUnicast(const chip::MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t messageLength,
                               uint8_t * message);

/**
 * @brief Sends unicast with attached message sent callback.
 */
EmberStatus emberAfSendUnicastWithCallback(const chip::MessageSendDestination & destination, EmberApsFrame * apsFrame,
                                           uint16_t messageLength, uint8_t * message, EmberAfMessageSentFunction callback);

/**
 * @brief Unicasts the message to each remote node in the binding table that
 * matches the cluster and source endpoint in the APS frame.  Note: if the
 * binding table contains many matching entries, calling this API cause a
 * significant amount of network traffic.
 */
EmberStatus emberAfSendUnicastToBindings(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message);

/**
 * @brief emberAfSendUnicastToBindings with attached message sent callback.
 */
EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message,
                                                     EmberAfMessageSentFunction callback);

/**
 * @brief Sends interpan message.
 */
EmberStatus emberAfSendInterPan(EmberPanId panId, const EmberEUI64 destinationLongId, EmberNodeId destinationShortId,
                                chip::GroupId multicastId, chip::ClusterId clusterId, uint16_t messageLength,
                                uint8_t * messageBytes);

/**
 * @brief Sends end device binding request.
 */
EmberStatus emberAfSendEndDeviceBind(chip::EndpointId endpoint);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API. It
 * will be sent as unicast to each remote node in the binding table that
 * matches the cluster and source endpoint in the APS frame.  Note: if the
 * binding table contains many matching entries, calling this API cause a
 * significant amount of network traffic.
 */
EmberStatus emberAfSendCommandUnicastToBindings(void);

/**
 * @brief emberAfSendCommandUnicastToBindings with attached message sent callback.
 */
EmberStatus emberAfSendCommandUnicastToBindingsWithCallback(EmberAfMessageSentFunction callback);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API. It
 * will be sent as multicast.
 */
EmberStatus emberAfSendCommandMulticast(chip::GroupId multicastId);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API. It
 * will be sent as multicast.
 */
EmberStatus emberAfSendCommandMulticastWithAlias(chip::GroupId multicastId, EmberNodeId alias, uint8_t sequence);

/**
 * @brief emberAfSendCommandMulticast with attached message sent callback.
 */
EmberStatus emberAfSendCommandMulticastWithCallback(chip::GroupId multicastId, EmberAfMessageSentFunction callback);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API. It
 * will be sent as multicast to the group specified in the binding table that
 * matches the cluster and source endpoint in the APS frame.  Note: if the
 * binding table contains many matching entries, calling this API cause a
 * significant amount of network traffic.
 */
EmberStatus emberAfSendCommandMulticastToBindings(void);
/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API.
 * It will be sent as unicast.
 */
EmberStatus emberAfSendCommandUnicast(const chip::MessageSendDestination & destination);

/**
 * @brief emberAfSendCommandUnicast with attached message sent callback.
 */
EmberStatus emberAfSendCommandUnicastWithCallback(const chip::MessageSendDestination & destination,
                                                  EmberAfMessageSentFunction callback);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API.
 */
EmberStatus emberAfSendCommandBroadcast(EmberNodeId destination);

/**
 * @brief emberAfSendCommandBroadcast with attached message sent callback.
 */
EmberStatus emberAfSendCommandBroadcastWithCallback(EmberNodeId destination, EmberAfMessageSentFunction callback);

/**
 * @brief emberAfSendCommandBroadcast from alias with attached message sent callback.
 */
EmberStatus emberAfSendCommandBroadcastWithAliasWithCallback(EmberNodeId destination, EmberNodeId alias, uint8_t sequence,
                                                             EmberAfMessageSentFunction callback);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API.
 */
EmberStatus emberAfSendCommandBroadcastWithAlias(EmberNodeId destination, EmberNodeId alias, uint8_t sequence);
/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API.
 * It will be sent via inter-PAN.  If destinationLongId is not NULL, the message
 * will be sent to that long address using long addressing mode; otherwise, the
 * message will be sent to destinationShortId using short address mode.  IF
 * multicastId is not zero, the message will be sent using multicast mode.
 */
EmberStatus emberAfSendCommandInterPan(EmberPanId panId, const EmberEUI64 destinationLongId, EmberNodeId destinationShortId,
                                       chip::GroupId multicastId);

/**
 * @brief Sends a default response to a cluster command.
 *
 * This function is used to prepare and send a default response to a cluster
 * command.
 *
 * @param cmd The cluster command to which to respond.
 * @param status Status code for the default response command.
 * @return An ::EmberStatus value that indicates the success or failure of
 * sending the response.
 */
EmberStatus emberAfSendDefaultResponse(const EmberAfClusterCommand * cmd, EmberAfStatus status);

/**
 * @brief emberAfSendDefaultResponse with attached message sent callback.
 */
EmberStatus emberAfSendDefaultResponseWithCallback(const EmberAfClusterCommand * cmd, EmberAfStatus status,
                                                   EmberAfMessageSentFunction callback);

/**
 * @brief Sends a default response to a cluster command using the
 * current command.
 *
 * This function is used to prepare and send a default response to a cluster
 * command.
 *
 * @param status Status code for the default response command.
 * @return An ::EmberStatus value that indicates the success or failure of
 * sending the response.
 */
EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status);

/**
 * @brief emberAfSendImmediateDefaultResponse with attached message sent callback.
 */
EmberStatus emberAfSendImmediateDefaultResponseWithCallback(EmberAfStatus status, EmberAfMessageSentFunction callback);

/**
 * @brief Access to client API APS frame.
 */
EmberApsFrame * emberAfGetCommandApsFrame(void);

/**
 * @brief Set the source and destination endpoints in the client API APS frame.
 */
void emberAfSetCommandEndpoints(chip::EndpointId sourceEndpoint, chip::EndpointId destinationEndpoint);

/**
 * @brief Friendly define for use in discovering client clusters with
 * ::emberAfFindDevicesByCluster().
 */
#define EMBER_AF_CLIENT_CLUSTER_DISCOVERY false

/**
 * @brief Friendly define for use in discovering server clusters with
 * ::emberAfFindDevicesByCluster().
 */
#define EMBER_AF_SERVER_CLUSTER_DISCOVERY true

/**
 * @brief Use this function to find devices in the network with endpoints
 *   matching a given cluster ID in their descriptors.
 *   Target may either be a specific device, or the broadcast
 *   address EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS.
 *
 * With this function a service discovery is initiated and received
 * responses are returned by executing the callback function passed in.
 * For unicast discoveries, the callback will be executed only once.
 * Either the target will return a result or a timeout will occur.
 * For broadcast discoveries, the callback may be called multiple times
 * and after a period of time the discovery will be finished with a final
 * call to the callback.
 *
 * @param target The destination node ID for the discovery; either a specific
 *  node's ID or EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS.
 * @param clusterId The cluster being discovered.
 * @param serverCluster EMBER_AF_SERVER_CLUSTER_DISCOVERY (true) if discovering
 *  servers for the target cluster; EMBER_AF_CLIENT_CLUSTER_DISCOVERY (false)
 *  if discovering clients for that cluster.
 * @param callback Function pointer for the callback function triggered when
 *  a match is discovered.  (For broadcast discoveries, this is called once per
 *  matching node, even if a node has multiple matching endpoints.)
 */
EmberStatus emberAfFindDevicesByCluster(EmberNodeId target, chip::ClusterId clusterId, bool serverCluster,
                                        EmberAfServiceDiscoveryCallback * callback);

/**
 * @brief Use this function to find all of the given in and out clusters
 *   implemented on a devices given endpoint. Target should only be the
 *   short address of a specific device.
 *
 * With this function a single service discovery is initiated and the response
 * is passed back to the passed callback.
 *
 * @param target The destination node ID for the discovery. This should be a
 *  specific node's ID and should not be a broadcast address.
 * @param targetEndpoint The endpoint to target with the discovery process.
 * @param callback Function pointer for the callback function triggered when
 *  the discovery is returned.
 */
EmberStatus emberAfFindClustersByDeviceAndEndpoint(EmberNodeId target, uint8_t targetEndpoint,
                                                   EmberAfServiceDiscoveryCallback * callback);

/**
 * @brief Use this function to initiate a discovery for the IEEE address
 *   of the specified node id.  This will send a unicast sent to the target
 *   node ID.
 */
EmberStatus emberAfFindIeeeAddress(EmberNodeId shortAddress, EmberAfServiceDiscoveryCallback * callback);

/**
 * @brief Use this function to initiate a discovery for the short ID of the
 *   specified long address.  This will send a broadcast to all
 *   rx-on-when-idle devices (non-sleepies).
 */
EmberStatus emberAfFindNodeId(EmberEUI64 longAddress, EmberAfServiceDiscoveryCallback * callback);

/**
 * @brief Initiate an Active Endpoint request ZDO message to the target node ID.
 */
EmberStatus emberAfFindActiveEndpoints(EmberNodeId target, EmberAfServiceDiscoveryCallback * callback);

/**
 * @brief Use this function to add an entry for a remote device to the address
 * table.
 *
 * If the EUI64 already exists in the address table, the index of the existing
 * entry will be returned.  Otherwise, a new entry will be created and the new
 * new index will be returned.  The framework will remember how many times the
 * returned index has been referenced.  When the address table entry is no
 * longer needed, the application should remove its reference by calling
 * ::emberAfRemoveAddressTableEntry.
 *
 * @param longId The EUI64 of the remote device.
 * @param shortId The node id of the remote device or ::EMBER_UNKNOWN_NODE_ID
 * if the node id is currently unknown.
 * @return The index of the address table entry for this remove device or
 * ::EMBER_NULL_ADDRESS_TABLE_INDEX if an error occurred (e.g., the address
 * table is full).
 */
uint8_t emberAfAddAddressTableEntry(EmberEUI64 longId, EmberNodeId shortId);

/**
 * @brief Use this function to add an entry for a remote device to the address
 * table at a specific location.
 *
 * The framework will remember how many times an address table index has been
 * referenced through ::emberAfAddAddressTableEntry.  If the reference count
 * for the index passed to this function is not zero, the entry will be not
 * changed.   When the address table entry is no longer needed, the application
 * should remove its reference by calling ::emberAfRemoveAddressTableEntry.
 *
 * @param index The index of the address table entry.
 * @param longId The EUI64 of the remote device.
 * @param shortId The node id of the remote device or ::EMBER_UNKNOWN_NODE_ID
 * if the node id is currently unknown.
 * @return ::EMBER_SUCCESS if the address table entry was successfully set,
 * ::EMBER_ADDRESS_TABLE_ENTRY_IS_ACTIVE if any messages are being sent using
 * the existing entry at that index or the entry is still referenced in the
 * framework, or ::EMBER_ADDRESS_TABLE_INDEX_OUT_OF_RANGE if the index is out
 * of range.
 */
EmberStatus emberAfSetAddressTableEntry(uint8_t index, EmberEUI64 longId, EmberNodeId shortId);

/**
 * @brief Use this function to remove a specific entry from the address table.
 *
 * The framework will remember how many times an address table index has been
 * referenced through ::emberAfAddAddressTableEntry and
 * ::emberAfSetAddressTableEntry.  The address table entry at this index will
 * not actually be removed until its reference count reaches zero.
 *
 * @param index The index of the address table entry.
 * @return ::EMBER_SUCCESS if the address table entry was successfully removed
 * or ::EMBER_ADDRESS_TABLE_INDEX_OUT_OF_RANGE if the index is out of range.
 */
EmberStatus emberAfRemoveAddressTableEntry(uint8_t index);

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
/**
 * @brief Use this macro to retrieve the current command. This
 * macro may only be used within the command parsing context. For instance
 * Any of the command handling callbacks may use this macro. If this macro
 * is used outside the command context, the returned EmberAfClusterCommand pointer
 * will be null.
 */
#define emberAfCurrentCommand() (emAfCurrentCommand)
extern EmberAfClusterCommand * emAfCurrentCommand;
#endif

/**
 * @brief returns the current endpoint that is being served.
 *
 * The purpose of this macro is mostly to access endpoint that
 * is being served in the command callbacks.
 */
#define emberAfCurrentEndpoint() (emberAfCurrentCommand()->apsFrame->destinationEndpoint)

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Use this function to initiate key establishment with a remote node.
 * ::emberAfKeyEstablishmentCallback will be called as events occur and when
 * key establishment completes.
 *
 * @param nodeId The node id of the remote device.
 * @param endpoint The endpoint on the remote device.
 * @return ::EMBER_SUCCESS if key establishment was initiated successfully
 */
EmberStatus emberAfInitiateKeyEstablishment(EmberNodeId nodeId, chip::EndpointId endpoint);

/** @brief Use this function to initiate key establishment with a remote node on
 * a different PAN.  ::emberAfInterPanKeyEstablishmentCallback will be called
 * as events occur and when key establishment completes.
 *
 * @param panId The PAN id of the remote device.
 * @param eui64 The EUI64 of the remote device.
 * @return ::EMBER_SUCCESS if key establishment was initiated successfully
 */
EmberStatus emberAfInitiateInterPanKeyEstablishment(EmberPanId panId, const EmberEUI64 eui64);

/** @brief Use this function to tell if the device is in the process of
 * performing key establishment.
 *
 * @return ::true if key establishment is in progress.
 */
bool emberAfPerformingKeyEstablishment(void);

/** @brief Use this function to initiate partner link key exchange with a
 * remote node.
 *
 * @param target The node id of the remote device.
 * @param endpoint The key establishment endpoint of the remote device.
 * @param callback The callback that should be called when the partner link
 * key exchange completes.
 * @return ::EMBER_SUCCESS if the partner link key exchange was initiated
 * successfully.
 */
EmberStatus emberAfInitiatePartnerLinkKeyExchange(EmberNodeId target, chip::EndpointId endpoint,
                                                  EmberAfPartnerLinkKeyExchangeCallback * callback);
#else
#define emberAfInitiateKeyEstablishment(nodeId, endpoint) emberAfInitiateKeyEstablishmentCallback(nodeId, endpoint)
#define emberAfInitiateInterPanKeyEstablishment(panId, eui64) emberAfInitiateInterPanKeyEstablishmentCallback(panId, eui64)
#define emberAfPerformingKeyEstablishment() emberAfPerformingKeyEstablishmentCallback()
#define emberAfInitiatePartnerLinkKeyExchange(target, endpoint, callback)                                                          \
    emberAfInitiatePartnerLinkKeyExchangeCallback(target, endpoint, callback)
#endif

/** @} END Messaging */

/** @name ZCL macros */
// @{
// Frame control fields (8 bits total)
// Bits 0 and 1 are Frame Type Sub-field
#define ZCL_FRAME_CONTROL_FRAME_TYPE_MASK (EMBER_BIT(0) | EMBER_BIT(1))
#define ZCL_CLUSTER_SPECIFIC_COMMAND EMBER_BIT(0)
#define ZCL_PROFILE_WIDE_COMMAND 0
#define ZCL_GLOBAL_COMMAND (ZCL_PROFILE_WIDE_COMMAND)
// Bit 2 is Manufacturer Specific Sub-field
#define ZCL_MANUFACTURER_SPECIFIC_MASK EMBER_BIT(2)
// Bit 3 is Direction Sub-field
#define ZCL_FRAME_CONTROL_DIRECTION_MASK EMBER_BIT(3)
#define ZCL_FRAME_CONTROL_SERVER_TO_CLIENT EMBER_BIT(3)
#define ZCL_FRAME_CONTROL_CLIENT_TO_SERVER 0
// Bit 4 is Disable Default Response Sub-field
#define ZCL_DISABLE_DEFAULT_RESPONSE_MASK EMBER_BIT(4)
// Bits 5 to 7 are reserved

#define ZCL_DIRECTION_CLIENT_TO_SERVER 0
#define ZCL_DIRECTION_SERVER_TO_CLIENT 1

// Packet must be at least 3 bytes for ZCL overhead.
//   Frame Control (1-byte)
//   Sequence Number (1-byte)
//   Command Id (1-byte)
#define EMBER_AF_ZCL_OVERHEAD 3
#define EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD 5

// Permitted values for emberAfSetFormAndJoinMode
#define FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ EMBER_BIT(0)
#define FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ EMBER_BIT(1)
#define FIND_AND_JOIN_MODE_ALLOW_BOTH (FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ | FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ)

/** @} END ZCL macros */

/** @name Network utility functions */
// ${

/** @brief Use this function to form a new network using the specified network
 * parameters.
 *
 * @param parameters Specification of the new network.
 * @return An ::EmberStatus value that indicates either the successful formation
 * of the new network or the reason that the network formation failed.
 */
EmberStatus emberAfFormNetwork(EmberNetworkParameters * parameters);

/** @brief Use this function to associate with the network using the specified
 * network parameters.
 *
 * @param parameters Specification of the network with which the node should
 * associate.
 * @return An ::EmberStatus value that indicates either that the association
 * process began successfully or the reason for failure.
 */
EmberStatus emberAfJoinNetwork(EmberNetworkParameters * parameters);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Use this function to find an unused PAN id and form a new network.
 *
 * @return An ::EmberStatus value that indicates either the process begin
 * successfully or the reason for failure.
 */
EmberStatus emberAfFindUnusedPanIdAndForm(void);
/** @brief Use this function to find a joinable network and join it.
 *
 * @return An ::EmberStatus value that indicates either the process begin
 * successfully or the reason for failure.
 */
EmberStatus emberAfStartSearchForJoinableNetwork(void);
#else
#define emberAfFindUnusedPanIdAndForm() emberAfFindUnusedPanIdAndFormCallback()
#define emberAfStartSearchForJoinableNetwork() emberAfStartSearchForJoinableNetworkCallback()
#endif

/** @brief Sets the current network to that of the given index and adds it to
 * the stack of networks maintained by the framework.  Every call to this API
 * must be paired with a subsequent call to ::emberAfPopNetworkIndex.
 */

EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex);
/** @brief Sets the current network to the callback network and adds it to
 * the stack of networks maintained by the framework.  Every call to this API
 * must be paired with a subsequent call to ::emberAfPopNetworkIndex.
 */

EmberStatus emberAfPushCallbackNetworkIndex(void);
/** @brief Sets the current network to that of the given endpoint and adds it
 * to the stack of networks maintained by the framework.  Every call to this
 * API must be paired with a subsequent call to ::emberAfPopNetworkIndex.
 */

EmberStatus emberAfPushEndpointNetworkIndex(chip::EndpointId endpoint);
/** @brief Removes the topmost network from the stack of networks maintained by
 * the framework and sets the current network to the new topmost network.
 * Every call to this API must be paired with a prior call to
 * ::emberAfPushNetworkIndex, ::emberAfPushCallbackNetworkIndex, or
 * ::emberAfPushEndpointNetworkIndex.
 */

EmberStatus emberAfPopNetworkIndex(void);
/** @brief Returns the primary endpoint of the given network index or 0xFF if
 * no endpoints belong to the network.
 */

uint8_t emberAfPrimaryEndpointForNetworkIndex(uint8_t networkIndex);
/** @brief Returns the primary endpoint of the current network index or 0xFF if
 * no endpoints belong to the current network.
 */

uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void);

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
/** @brief Initializes the stack of networks maintained by the framework,
 * including setting the default network.
 *
 * @return An ::EmberStatus value that indicates either that the network stack
 * has been successfully initialized or the reason for failure.
 */
EmberStatus emAfInitializeNetworkIndexStack(void);
void emAfAssertNetworkIndexStackIsEmpty(void);
#endif
/** @brief Basic initialization API to be invoked before ::emberAfMain.
 */
void emberAfMainInit(void);

/** @brief This function effectively serves as the application main.
 */
int emberAfMain(MAIN_FUNCTION_PARAMETERS);

/** @} End network utility functions */

/** @} END addtogroup */

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
#if defined(EMBER_TEST)
#define EMBER_TEST_ASSERT(x) assert(x)
#else
#define EMBER_TEST_ASSERT(x)
#endif
#endif

/** @brief The maximum power level that can be used by the chip.
 */
// Note:  This is a #define for now but could be a real function call in the future.
#define emberAfMaxPowerLevel() (3)

/**
 * @brief API for parsing a cluster-specific message.  Implemented by
 * generated code.
 */
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand * cmd);
