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

#include <app/util/debug-printing.h>
#include <app/util/ember-print.h>

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

// For now, just define emberAfWriteServerAttribute to emberAfWriteAttribute, to
// minimize code churn.
// TODO: Remove this define.
#define emberAfWriteServerAttribute emberAfWriteAttribute

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

// For now, just define emberAfReadServerAttribute to emberAfReadAttribute, to
// minimize code churn.
// TODO: Remove this define.
#define emberAfReadServerAttribute emberAfReadAttribute

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
 * @brief Returns root endpoint of a composed bridged device
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
 * Returns the endpoint index within a given cluster (Server-side),
 * looking only for standard clusters.
 */
uint16_t emberAfFindClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId clusterId);

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

/**
 * @brief populates the passed EUI64 with the local EUI64 MAC address.
 */
void emberAfGetEui64(EmberEUI64 returnEui64);

/**
 * @brief Returns the node ID of the local node.
 */
EmberNodeId emberAfGetNodeId(void);

/**
 * @brief Returns the current network state.  This call caches the results
 *   on the host to prevent frequent EZSP transactions.
 */
EmberNetworkStatus emberAfNetworkState(void);

/** @} END Miscellaneous */

/** @name Sleep Control */
//@{

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

/** @} END Sleep Control */

/** @name Messaging */
// @{

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
 * @brief Access to client API APS frame.
 */
EmberApsFrame * emberAfGetCommandApsFrame(void);

/**
 * @brief Set the source and destination endpoints in the client API APS frame.
 */
void emberAfSetCommandEndpoints(chip::EndpointId sourceEndpoint, chip::EndpointId destinationEndpoint);

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

/** @} END addtogroup */

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
#if defined(EMBER_TEST)
#define EMBER_TEST_ASSERT(x) assert(x)
#else
#define EMBER_TEST_ASSERT(x)
#endif
#endif

/**
 * @brief API for parsing a cluster-specific message.  Implemented by
 * generated code.
 */
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand * cmd);

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

} // namespace app
} // namespace chip
