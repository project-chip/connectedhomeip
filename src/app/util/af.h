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

#include <app/util/endpoint-config-api.h>

#include <lib/core/DataModelTypes.h>
#include <lib/support/Iterators.h>
#include <lib/support/SafeInt.h>

/** @name Attribute Storage */
// @{

static constexpr uint16_t kEmberInvalidEndpointIndex = 0xFFFF;

/**
 * @brief locate attribute metadata
 *
 * Function returns pointer to the attribute metadata structure,
 * or NULL if attribute was not found.
 *
 * @param endpoint Zigbee endpoint number.
 * @param clusterId Cluster ID of the sought cluster.
 * @param attributeId Attribute ID of the sought attribute.
 *
 * @return Returns pointer to the attribute metadata location.
 */
const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                chip::AttributeId attributeId);
/**
 * @brief Returns true if endpoint contains the ZCL server with specified id.
 *
 * This function returns true if
 * the endpoint contains server of a given cluster.
 */
bool emberAfContainsServer(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint of given index contains the ZCL server with specified id.
 *
 * This function returns true if
 * the endpoint of given index contains server of a given cluster.
 * If this function is used with a manufacturer specific clusterId
 * then this will return the first cluster that it finds in the Cluster table.
 * and will not return any other clusters that share that id.
 */
bool emberAfContainsServerFromIndex(uint16_t index, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint contains the ZCL client with specified id.
 *
 * This function returns true if
 * the endpoint contains client of a given cluster.
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
 */
EmberAfStatus emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                    uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief Read the attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store it into the
 * pointer.
 *
 * dataPtr may be NULL, signifying that we don't need the value, just the status
 * (i.e. whether the attribute can be read).
 */
EmberAfStatus emberAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                   uint8_t * dataPtr, uint16_t readLength);

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
 * @brief Returns parent endpoint for a given endpoint index
 */
chip::EndpointId emberAfParentEndpointFromIndex(uint16_t index);

/**
 * Returns the index of a given endpoint.  Will return 0xFFFF if this is not a
 * valid endpoint id or if the endpoint is disabled.
 */
uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint);

/**
 * Returns the index of a given endpoint; Does not ignore disabled endpoints.
 * Will return 0xFFFF if this is not a valid endpoint id.
 */
uint16_t emberAfIndexFromEndpointIncludingDisabledEndpoints(chip::EndpointId endpoint);

/**
 *  @brief Returns the index of the given endpoint in the list of all endpoints that might support the given cluster server.
 *
 * Returns kEmberInvalidEndpointIndex if the given endpoint does not support the
 * given cluster or if the given endpoint is disabled.
 *
 * This function always returns the same index for a given endpointId instance, fixed or dynamic.
 *
 * The return index for fixed endpoints will range from 0 to (fixedClusterServerEndpointCount - 1),
 * For dynamic endpoints the indexing assumes that any dynamic endpoint could start supporting
 * the given server cluster and their index will range from fixedClusterServerEndpointCount to
 * (fixedClusterServerEndpointCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT - 1).
 *
 * For example, if a device has 4 fixed endpoints (ids 0-3) and 2 dynamic
 * endpoints, and cluster X is supported on endpoints 1 and 3, then
 * fixedClusterServerEndpointCount should be 2 and
 *
 * 1) emberAfGetClusterServerEndpointIndex(0, X) returns kEmberInvalidEndpointIndex
 * 2) emberAfGetClusterServerEndpointIndex(1, X) returns 0
 * 3) emberAfGetClusterServerEndpointIndex(2, X) returns kEmberInvalidEndpointIndex
 * 4) emberAfGetClusterServerEndpointIndex(3, X) returns 1

 * The Dynamic endpoints are placed after the fixed ones;
 * therefore their return index will always be >= to fixedClusterServerEndpointCount
 *
 * If a dynamic endpoint, supporting cluster X, is defined to dynamic index 1 with endpoint id 7,
 * (via emberAfSetDynamicEndpoint(1, 7, ...))
 * then emberAfGetClusterServerEndpointIndex(7, X) returns 3 (fixedClusterServerEndpointCount{2} + DynamicEndpointIndex {1}).
 *
 * If now a second dynamic endpoint, also supporting cluster X, is defined to dynamic index 0
 * with endpoint id 9  (via emberAfSetDynamicEndpoint(0, 9, ...)),
 * emberAfGetClusterServerEndpointIndex(9, X) returns 2. (fixedClusterServerEndpointCount{2} + DynamicEndpointIndex {0}).
 * and emberAfGetClusterServerEndpointIndex(7, X) still returns 3
 *
 * @param endpoint Endpoint number
 * @param cluster Id the of the Cluster server you are interrested on
 * @param fixedClusterServerEndpointCount The number of fixed endpoints containing this cluster server.  Typically one of the
 EMBER_AF_*_CLUSTER_SERVER_ENDPOINT_COUNT constants.
 */
uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                              uint16_t fixedClusterServerEndpointCount);

/**
 * @brief Returns the number of pre-compiled endpoints.
 */
uint16_t emberAfFixedEndpointCount(void);

/**
 *@brief Returns true if type is signed, false otherwise.
 */
bool emberAfIsTypeSigned(EmberAfAttributeType dataType);

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

/** @} END Attribute Storage */

/** @name Device Control */
// @{

/**
 * @brief Function that checks if endpoint is identifying
 *
 * This function returns true if device at a given endpoint is
 * identifying.
 *
 * @param endpoint Zigbee endpoint number
 */
bool emberAfIsDeviceIdentifying(chip::EndpointId endpoint);

/** @} END Device Control */

/** @name Miscellaneous */
// @{

/** @brief Returns true if a given ZCL data type is a list type. */
bool emberAfIsThisDataTypeAListType(EmberAfAttributeType dataType);

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
int8_t emberAfCompareValues(const uint8_t * val1, const uint8_t * val2, uint16_t len, bool signedNumber);

/** @} END Miscellaneous */

/** @} END addtogroup */

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
#if defined(EMBER_TEST)
#define EMBER_TEST_ASSERT(x) assert(x)
#else
#define EMBER_TEST_ASSERT(x)
#endif
#endif

/**
 * Returns the pointer to the data version storage for the given endpoint and
 * cluster.  Can return null in the following cases:
 *
 * 1) There is no such endpoint.
 * 2) There is no such server cluster on the given endpoint.
 * 3) No storage for a data version was provided for the endpoint.
 */
chip::DataVersion * emberAfDataVersionStorage(const chip::app::ConcreteClusterPath & aConcreteClusterPath);

namespace chip {
namespace app {

class EnabledEndpointsWithServerCluster
{
public:
    EnabledEndpointsWithServerCluster(ClusterId clusterId);

    // Instead of having a separate Iterator class, optimize for codesize by
    // just reusing ourselves as our own iterator.  We could do a bit better
    // here with C++17 and using a different type for the end iterator, but this
    // is the best I've found with C++14 so far.
    //
    // This does mean that you can only iterate a given
    // EnabledEndpointsWithServerCluster once, but that's OK given how we use it
    // in practice.
    EnabledEndpointsWithServerCluster & begin() { return *this; }
    const EnabledEndpointsWithServerCluster & end() const { return *this; }

    bool operator!=(const EnabledEndpointsWithServerCluster & other) const { return mEndpointIndex != mEndpointCount; }

    EnabledEndpointsWithServerCluster & operator++();

    EndpointId operator*() const { return emberAfEndpointFromIndex(mEndpointIndex); }

private:
    void EnsureMatchingEndpoint();

    uint16_t mEndpointIndex = 0;
    uint16_t mEndpointCount = emberAfEndpointCount();
    ClusterId mClusterId;
};

/**
 * @brief Sets the parent endpoint for a given endpoint
 */
CHIP_ERROR SetParentEndpointForEndpoint(EndpointId childEndpoint, EndpointId parentEndpoint);

/**
 * @brief Sets an Endpoint to use Flat Composition
 */
CHIP_ERROR SetFlatCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Sets an Endpoint to use Tree Composition
 */
CHIP_ERROR SetTreeCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Returns true is an Endpoint has flat composition
 */
bool IsFlatCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Returns true is an Endpoint has tree composition
 */
bool IsTreeCompositionForEndpoint(EndpointId endpoint);

} // namespace app
} // namespace chip
