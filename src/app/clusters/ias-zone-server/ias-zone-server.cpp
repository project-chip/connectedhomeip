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
/****************************************************************************
 * @file
 * @brief This is the source for the plugin used to
 *add an IAS Zone cluster server to a project.  This
 *source handles zone enrollment and storing of
 * attributes from a CIE device, and provides an API
 *for different plugins to post updated zone status
 *values.
 *******************************************************************************
 ******************************************************************************/

// *****************************************************************************
// * ias-zone-server.c
// *
// *
// *
// * Copyright 2015 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "ias-zone-server.h"
#include <app/CommandHandler.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/callback.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/binding-table.h>
#include <system/SystemLayer.h>

using namespace chip;

#define UNDEFINED_ZONE_ID 0xFF
#define DELAY_TIMER_MS (1 * MILLISECOND_TICKS_PER_SECOND)
#define IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX 0x02
#define ZCL_FRAME_CONTROL_IDX 0x00

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
#define NUM_QUEUE_ENTRIES EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER_QUEUE_SIZE
#else
#define NUM_QUEUE_ENTRIES EMBER_AF_PLUGIN_IAS_ZONE_SERVER_QUEUE_DEPTH
#endif
#else
#define NUM_QUEUE_ENTRIES 0
#endif

#define DEFAULT_ENROLLMENT_METHOD EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST

// TODO: Need to figure out what needs to happen wrt HAL tokens here, but for
// now define ESZP_HOST to disable it.  See
// https://github.com/project-chip/connectedhomeip/issues/3275
#define EZSP_HOST

typedef struct
{
    EndpointId endpoint;
    uint16_t status;
    uint32_t eventTimeMs;
} IasZoneStatusQueueEntry;

typedef struct
{
    uint8_t entriesInQueue;
    uint8_t startIdx;
    uint8_t lastIdx;
    IasZoneStatusQueueEntry buffer[NUM_QUEUE_ENTRIES];
} IasZoneStatusQueue;

//-----------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginIasZoneServerManageQueueEventControl;
static EmberAfIasZoneEnrollmentMode enrollmentMethod;

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
IasZoneStatusQueue messageQueue;

// Status queue retry parameters
typedef struct
{
    IasZoneStatusQueueRetryConfig config;
    uint32_t currentBackoffTimeSec;
    uint8_t currentRetryCount;
} IasZoneStatusQueueRetryParameters;

// Set up status queue retry parameters.
IasZoneStatusQueueRetryParameters queueRetryParams = {
    .config = { .firstBackoffTimeSec   = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC,
                .backoffSeqCommonRatio = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_BACKOFF_SEQUENCE_COMMON_RATIO,
                .maxBackoffTimeSec     = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_BACKOFF_TIME_SEC,
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER_UNLIMITED_RETRIES
                .unlimitedRetries = true,
#else
                .unlimitedRetries = false,
#endif
                .maxRetryAttempts = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_RETRY_ATTEMPTS },
    .currentBackoffTimeSec = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC,
    .currentRetryCount     = 0,
};

static void resetCurrentQueueRetryParams(void)
{
    queueRetryParams.currentRetryCount     = 0;
    queueRetryParams.currentBackoffTimeSec = queueRetryParams.config.firstBackoffTimeSec;
}

#endif // EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE

//-----------------------------------------------------------------------------
// Forward declarations

static void setZoneId(EndpointId endpoint, uint8_t zoneId);
static bool areZoneServerAttributesTokenized(EndpointId endpoint);
static bool isValidEnrollmentMode(EmberAfIasZoneEnrollmentMode method);
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
static uint16_t computeElapsedTimeQs(IasZoneStatusQueueEntry * entry);
static void bufferInit(IasZoneStatusQueue * ring);
static int16_t copyToBuffer(IasZoneStatusQueue * ring, const IasZoneStatusQueueEntry * entry);
static int16_t popFromBuffer(IasZoneStatusQueue * ring, IasZoneStatusQueueEntry * entry);
#endif

// TODO: https://github.com/project-chip/connectedhomeip/issues/3276 needs to be
// fixed to implement this for real.
EmberNetworkStatus emberAfNetworkState(void)
{
    return EMBER_JOINED_NETWORK;
}

//-----------------------------------------------------------------------------
// Functions

static EmberStatus sendToClient(EndpointId endpoint)
{
    EmberStatus status;

    // If the device is not a network, there is no one to send to, so do nothing
    if (emberAfNetworkState() != EMBER_JOINED_NETWORK)
    {
        return EMBER_NETWORK_DOWN;
    }

    // Remote endpoint need not be set, since it will be provided by the call to
    // emberAfSendCommandUnicastToBindings()
    emberAfSetCommandEndpoints(endpoint, 0);

    // A binding table entry is created on Zone Enrollment for each endpoint, so
    // a simple call to SendCommandUnicastToBinding will handle determining the
    // destination endpoint, address, etc for us.
    status = emberAfSendCommandUnicastToBindings();

    if (EMBER_SUCCESS != status)
    {
        return status;
    }

    return status;
}

static void enrollWithClient(EndpointId endpoint)
{
    EmberStatus status;
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_ZONE_ENROLL_REQUEST_COMMAND_ID, "vv", EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE,
                              EMBER_AF_MANUFACTURER_CODE);
    status = sendToClient(endpoint);
    if (status == EMBER_SUCCESS)
    {
        emberAfIasZoneClusterPrintln("Sent enroll request to IAS Zone client.");
    }
    else
    {
        emberAfIasZoneClusterPrintln("Error sending enroll request: 0x%x\n", status);
    }
}

EmberAfStatus emberAfIasZoneClusterServerPreAttributeChangedCallback(EndpointId endpoint, AttributeId attributeId,
                                                                     EmberAfAttributeType attributeType, uint16_t size,
                                                                     uint8_t * value)
{
    uint8_t i;
    bool zeroAddress;
    EmberBindingTableEntry bindingEntry;
    EmberBindingTableEntry currentBind;
    NodeId destNodeId;
    uint8_t ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // If this is not a CIE Address write, the CIE address has already been
    // written, or the IAS Zone server is already enrolled, do nothing.
    if (attributeId != ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID || emberAfCurrentCommand() == NULL)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    memcpy(&destNodeId, value, sizeof(NodeId));

    // Create the binding table entry

    // This code assumes that the endpoint and device that is setting the CIE
    // address is the CIE device itself, and as such the remote endpoint to bind
    // to is the endpoint that generated the attribute change.  This
    // assumption is made based on analysis of the behavior of CIE devices
    // currently existing in the field.
    bindingEntry.type      = EMBER_UNICAST_BINDING;
    bindingEntry.local     = endpoint;
    bindingEntry.clusterId = ZCL_IAS_ZONE_CLUSTER_ID;
    bindingEntry.remote    = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
    bindingEntry.nodeId    = destNodeId;

    // Cycle through the binding table until we find a valid entry that is not
    // being used, then use the created entry to make the bind.
    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        if (emberGetBinding(i, &currentBind) != EMBER_SUCCESS)
        {
            // break out of the loop to ensure that an error message still prints
            break;
        }
        if (currentBind.type != EMBER_UNUSED_BINDING)
        {
            // If the binding table entry created based on the response already exists
            // do nothing.
            if ((currentBind.local == bindingEntry.local) && (currentBind.clusterId == bindingEntry.clusterId) &&
                (currentBind.remote == bindingEntry.remote) && (currentBind.type == bindingEntry.type))
            {
                break;
            }
            // If this spot in the binding table already exists, move on to the next
            continue;
        }
        else
        {
            emberSetBinding(i, &bindingEntry);
            break;
        }
    }

    zeroAddress = true;
    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID, (uint8_t *) ieeeAddress, 8);
    for (i = 0; i < 8; i++)
    {
        if (ieeeAddress[i] != 0)
        {
            zeroAddress = false;
        }
    }
    emberAfAppPrint("\nzero address: %d\n", zeroAddress);

    if ((zeroAddress == true) && (enrollmentMethod == EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST))
    {
        // Only send the enrollment request if the mode is AUTO-ENROLL-REQUEST.
        // We need to delay to get around a bug where we can't send a command
        // at this point because then the Write Attributes response will not
        // be sent.  But we also delay to give the client time to configure us.
        emberAfIasZoneClusterPrintln("Sending enrollment after %d ms", DELAY_TIMER_MS);
        emberAfScheduleServerTickExtended(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, DELAY_TIMER_MS, EMBER_AF_SHORT_POLL,
                                          EMBER_AF_STAY_AWAKE);
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfPluginIasZoneClusterSetEnrollmentMethod(EndpointId endpoint, EmberAfIasZoneEnrollmentMode method)
{
    EmberAfStatus status;

    if (emberAfIasZoneClusterAmIEnrolled(endpoint))
    {
        emberAfIasZoneClusterPrintln("Error: Already enrolled");
        status = EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    }
    else if (!isValidEnrollmentMode(method))
    {
        emberAfIasZoneClusterPrintln("Invalid IAS Zone Server Enrollment Mode: %d", method);
        status = EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else
    {
        enrollmentMethod = method;
#ifndef EZSP_HOST
        halCommonSetToken(TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD, &enrollmentMethod);
#endif
        emberAfIasZoneClusterPrintln("IAS Zone Server Enrollment Mode: %d", method);
        status = EMBER_ZCL_STATUS_SUCCESS;
    }
    return status;
}

static bool isValidEnrollmentMode(EmberAfIasZoneEnrollmentMode method)
{
    return ((method == EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_TRIP_TO_PAIR) ||
            (method == EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_AUTO_ENROLLMENT_RESPONSE) ||
            (method == EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST));
}

bool emberAfIasZoneClusterAmIEnrolled(EndpointId endpoint)
{
    EmberAfIasZoneState zoneState = EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED; // Clear this out completely.
    EmberAfStatus status;
    status =
        emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATE_ATTRIBUTE_ID, (unsigned char *) &zoneState,
                                   1); // uint8_t size

    return (status == EMBER_ZCL_STATUS_SUCCESS && zoneState == EMBER_ZCL_IAS_ZONE_STATE_ENROLLED);
}

static void updateEnrollState(EndpointId endpoint, bool enrolled)
{
    EmberAfIasZoneState zoneState = (enrolled ? EMBER_ZCL_IAS_ZONE_STATE_ENROLLED : EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED);

    emberAfWriteServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATE_ATTRIBUTE_ID, (uint8_t *) &zoneState,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
    emberAfIasZoneClusterPrintln("IAS Zone Server State: %pEnrolled", (enrolled ? "" : "NOT "));
}

bool emberAfIasZoneClusterZoneEnrollResponseCallback(chip::app::CommandHandler * commandObj, uint8_t enrollResponseCode,
                                                     uint8_t zoneId)
{
    EndpointId endpoint;
    uint8_t epZoneId;
    EmberAfStatus status;

    endpoint = emberAfCurrentEndpoint();
    status   = emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ID_ATTRIBUTE_ID, &epZoneId, sizeof(uint8_t));
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        if (enrollResponseCode == EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS)
        {
            updateEnrollState(endpoint, true);
            setZoneId(endpoint, zoneId);
        }
        else
        {
            updateEnrollState(endpoint, false);
            setZoneId(endpoint, UNDEFINED_ZONE_ID);
        }

        return true;
    }

    emberAfAppPrintln("ERROR: IAS Zone Server unable to read zone ID attribute");
    return true;
}

static EmberStatus sendZoneUpdate(uint16_t zoneStatus, uint16_t timeSinceStatusOccurredQs, EndpointId endpoint)
{
    EmberStatus status;

    if (!emberAfIasZoneClusterAmIEnrolled(endpoint))
    {
        return EMBER_INVALID_CALL;
    }
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID, "vuuv", zoneStatus,
                              0 /*extended status, must be zero per spec*/, emberAfPluginIasZoneServerGetZoneId(endpoint),
                              timeSinceStatusOccurredQs /* called "delay" in the spec */);
    status = sendToClient(endpoint);

    return status;
}

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
static void addNewEntryToQueue(const IasZoneStatusQueueEntry * newEntry)
{
    emberAfIasZoneClusterPrintln("Adding new entry to queue");
    copyToBuffer(&messageQueue, newEntry);
}
#endif

EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(EndpointId endpoint, uint16_t newStatus, uint16_t timeSinceStatusOccurredQs)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    IasZoneStatusQueueEntry newBufferEntry;
    newBufferEntry.endpoint    = endpoint;
    newBufferEntry.status      = newStatus;
    newBufferEntry.eventTimeMs = System::Clock::GetMonotonicMilliseconds();
#endif
    EmberStatus sendStatus = EMBER_SUCCESS;

    emberAfWriteServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATUS_ATTRIBUTE_ID, (uint8_t *) &newStatus,
                                ZCL_INT16U_ATTRIBUTE_TYPE);

    if (enrollmentMethod == EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_TRIP_TO_PAIR)
    {
        // If unenrolled, send Zone Enroll Request command.
        if (!emberAfIasZoneClusterAmIEnrolled(endpoint))
        {
            emberAfScheduleServerTick(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, DELAY_TIMER_MS);
            // Don't send the zone status update since not enrolled.
            return EMBER_SUCCESS;
        }
    }

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    // If there are items in the queue waiting to send, this event should not
    // be transmitted, as that could cause the client to receive the events out
    // of order.  Instead, just add the device to the queue
    if (messageQueue.entriesInQueue == 0)
    {
        sendStatus = sendZoneUpdate(newStatus, timeSinceStatusOccurredQs, endpoint);
    }
    else
    {
        // Add a new element to the status queue and depending on the network state
        // either try to resend the first element in the queue immediately or try to
        // restart the parent research pattern.
        addNewEntryToQueue(&newBufferEntry);

        EmberNetworkStatus networkState = emberAfNetworkState();

        if (networkState == EMBER_JOINED_NETWORK_NO_PARENT)
        {
            emberAfStartMoveCallback();
        }
        else if (networkState == EMBER_JOINED_NETWORK)
        {
            resetCurrentQueueRetryParams();
            emberEventControlSetActive(&emberAfPluginIasZoneServerManageQueueEventControl);
        }

        return EMBER_SUCCESS;
    }

#else
    sendStatus = sendZoneUpdate(newStatus, timeSinceStatusOccurredQs, endpoint);
#endif

    if (sendStatus == EMBER_SUCCESS)
    {
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
        // Add a new entry to the zoneUpdate buffer
        addNewEntryToQueue(&newBufferEntry);
#endif
    }
    else
    {
        // If we're not on a network and never were, we don't need to do anything.
        // If we used to be on a network and can't talk to our parent, we should
        // try to rejoin the network and add the message to the queue
        if (emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT)
        {
            emberAfStartMoveCallback();
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
            // Add a new entry to the zoneUpdate buffer
            addNewEntryToQueue(&newBufferEntry);
#endif
        }
        emberAfIasZoneClusterPrintln("Failed to send IAS Zone update. Err 0x%x", sendStatus);
    }
    return sendStatus;
}

void emberAfPluginIasZoneServerManageQueueEventHandler(void)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    IasZoneStatusQueueEntry * bufferStart;
    uint16_t status;
    uint16_t elapsedTimeQs;
    uint16_t airTimeRemainingMs;

    // If the queue was emptied without our interaction, do nothing
    if (messageQueue.entriesInQueue == 0)
    {
        emberEventControlSetInactive(&emberAfPluginIasZoneServerManageQueueEventControl);
        return;
    }

    // Otherwise, pull out the first item and attempt to retransmit it.  The
    // message complete callback will handle removing items from the queue

    // To prevent an activity storm from flooding with retry requests, only
    // re-send a message if it's been at least
    // EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS since it was sent.
    bufferStart   = &(messageQueue.buffer[messageQueue.startIdx]);
    elapsedTimeQs = computeElapsedTimeQs(bufferStart);

    if (elapsedTimeQs < (EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS / (MILLISECOND_TICKS_PER_SECOND / 4)))
    {
        airTimeRemainingMs = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS - (elapsedTimeQs * MILLISECOND_TICKS_PER_SECOND / 4);
        emberAfIasZoneClusterPrintln("Not enough time passed for a retry, sleeping %d more mS", airTimeRemainingMs);
        emberEventControlSetDelayMS(emberAfPluginIasZoneServerManageQueueEventControl, airTimeRemainingMs);
    }
    else
    {
        status = bufferStart->status;
        emberAfIasZoneClusterPrintln("Attempting to resend a queued zone status update (status: 0x%02X, "
                                     "event time (s): %d) with time of %d. Retry count: %d",
                                     bufferStart->status, bufferStart->eventTimeMs / MILLISECOND_TICKS_PER_SECOND, elapsedTimeQs,
                                     queueRetryParams.currentRetryCount);
        sendZoneUpdate(status, elapsedTimeQs, bufferStart->endpoint);
        emberEventControlSetInactive(&emberAfPluginIasZoneServerManageQueueEventControl);
    }
#else
    emberEventControlSetInactive(&emberAfPluginIasZoneServerManageQueueEventControl);
#endif
}

void emberAfIasZoneClusterServerInitCallback(EndpointId endpoint)
{
    EmberAfIasZoneType zoneType;
    if (!areZoneServerAttributesTokenized(endpoint))
    {
        emberAfAppPrint("WARNING: ATTRIBUTES ARE NOT BEING STORED IN FLASH! ");
        emberAfAppPrintln("DEVICE WILL NOT FUNCTION PROPERLY AFTER REBOOTING!!");
    }

#ifndef EZSP_HOST
    halCommonGetToken(&enrollmentMethod, TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD);
#else
    enrollmentMethod = DEFAULT_ENROLLMENT_METHOD;
#endif
    if (!isValidEnrollmentMode(enrollmentMethod))
    {
        // Default Enrollment Method to AUTO-ENROLL-REQUEST.
        enrollmentMethod = DEFAULT_ENROLLMENT_METHOD;
    }

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    bufferInit(&messageQueue);
#endif

    zoneType = (EmberAfIasZoneType) EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;
    emberAfWriteAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_TYPE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, (uint8_t *) &zoneType,
                          ZCL_INT16U_ATTRIBUTE_TYPE);

    emberAfPluginIasZoneServerUpdateZoneStatus(endpoint,
                                               0,  // status: All alarms cleared
                                               0); // time since status occurred
}

void emberAfIasZoneClusterServerTickCallback(EndpointId endpoint)
{
    enrollWithClient(endpoint);
}

uint8_t emberAfPluginIasZoneServerGetZoneId(EndpointId endpoint)
{
    uint8_t zoneId = UNDEFINED_ZONE_ID;
    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ID_ATTRIBUTE_ID, &zoneId,
                               emberAfGetDataSize(ZCL_INT8U_ATTRIBUTE_TYPE));
    return zoneId;
}

//------------------------------------------------------------------------------
//
// This function will verify that all attributes necessary for the IAS zone
// server to properly retain functionality through a power failure are
// tokenized.
//
//------------------------------------------------------------------------------
static bool areZoneServerAttributesTokenized(EndpointId endpoint)
{
    EmberAfAttributeMetadata * metadata;

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_TYPE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ID_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    return true;
}

static void setZoneId(EndpointId endpoint, uint8_t zoneId)
{
    emberAfIasZoneClusterPrintln("IAS Zone Server Zone ID: 0x%X", zoneId);
    emberAfWriteServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ID_ATTRIBUTE_ID, &zoneId, ZCL_INT8U_ATTRIBUTE_TYPE);
}

static void unenrollSecurityDevice(EndpointId endpoint)
{
    uint8_t ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint16_t zoneType     = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;

    emberAfWriteServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID, (uint8_t *) ieeeAddress,
                                ZCL_NODE_ID_ATTRIBUTE_TYPE);

    emberAfWriteServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_TYPE_ATTRIBUTE_ID, (uint8_t *) &zoneType,
                                ZCL_INT16U_ATTRIBUTE_TYPE);

    setZoneId(endpoint, UNDEFINED_ZONE_ID);
    // Restore the enrollment method back to its default value.
    emberAfPluginIasZoneClusterSetEnrollmentMethod(endpoint, DEFAULT_ENROLLMENT_METHOD);
    updateEnrollState(endpoint, false); // enrolled?
}

// If you leave the network, unenroll yourself.
void emberAfPluginIasZoneServerStackStatusCallback(EmberStatus status)
{
    EndpointId endpoint;
    uint8_t networkIndex;
    uint8_t i;

    // If the device has left the network, unenroll all endpoints on the device
    // that are servers of the IAS Zone Cluster
    if (status == EMBER_NETWORK_DOWN && emberAfNetworkState() == EMBER_NO_NETWORK)
    {
        for (i = 0; i < emberAfEndpointCount(); i++)
        {
            endpoint     = emberAfEndpointFromIndex(i);
            networkIndex = emberAfNetworkIndexFromEndpointIndex(i);
            if (networkIndex == 0 /* emberGetCurrentNetwork() */ && emberAfContainsServer(endpoint, ZCL_IAS_ZONE_CLUSTER_ID))
            {
                unenrollSecurityDevice(endpoint);
            }
        }
    }
    else if (status == EMBER_NETWORK_UP)
    {
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
        // If we're reconnecting, send any items still in the queue
        emberAfIasZoneClusterPrintln("Rejoined network, retransmiting any queued event");
        emberEventControlSetActive(&emberAfPluginIasZoneServerManageQueueEventControl);
#endif
    }
}

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
EmberStatus emberAfIasZoneServerConfigStatusQueueRetryParams(IasZoneStatusQueueRetryConfig * retryConfig)
{
    if (!(retryConfig->firstBackoffTimeSec) || (!retryConfig->backoffSeqCommonRatio) ||
        (retryConfig->maxBackoffTimeSec < retryConfig->firstBackoffTimeSec) ||
        (retryConfig->maxBackoffTimeSec > IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC) || (!retryConfig->maxRetryAttempts))
    {
        return EMBER_BAD_ARGUMENT;
    }

    queueRetryParams.config.firstBackoffTimeSec   = retryConfig->firstBackoffTimeSec;
    queueRetryParams.config.backoffSeqCommonRatio = retryConfig->backoffSeqCommonRatio;
    queueRetryParams.config.maxBackoffTimeSec     = retryConfig->maxBackoffTimeSec;
    queueRetryParams.config.unlimitedRetries      = retryConfig->unlimitedRetries;
    queueRetryParams.config.maxRetryAttempts      = retryConfig->maxRetryAttempts;

    queueRetryParams.currentBackoffTimeSec = retryConfig->firstBackoffTimeSec;
    queueRetryParams.currentRetryCount     = 0;

    return EMBER_SUCCESS;
}

void emberAfIasZoneServerSetStatusQueueRetryParamsToDefault(void)
{
    queueRetryParams.config.firstBackoffTimeSec   = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC;
    queueRetryParams.config.backoffSeqCommonRatio = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_BACKOFF_SEQUENCE_COMMON_RATIO;
    queueRetryParams.config.maxBackoffTimeSec     = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_BACKOFF_TIME_SEC;
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER_UNLIMITED_RETRIES
    queueRetryParams.config.unlimitedRetries = true;
#else
    queueRetryParams.config.unlimitedRetries = false;
#endif
    queueRetryParams.config.maxRetryAttempts = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_RETRY_ATTEMPTS;

    queueRetryParams.currentBackoffTimeSec = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC;
    queueRetryParams.currentRetryCount     = 0;
}

void emberAfIasZoneServerDiscardPendingEventsInStatusQueue(void)
{
    emberEventControlSetInactive(&emberAfPluginIasZoneServerManageQueueEventControl);
    bufferInit(&messageQueue);
    resetCurrentQueueRetryParams();
}

#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
EmberStatus emberAfWwahAppEventRetryManagerConfigBackoffParamsCallback(uint8_t firstBackoffTimeSeconds,
                                                                       uint8_t backoffSeqCommonRatio,
                                                                       uint32_t maxBackoffTimeSeconds,
                                                                       uint8_t maxRedeliveryAttempts)
{
    IasZoneStatusQueueRetryConfig retryConfig = { firstBackoffTimeSeconds, backoffSeqCommonRatio, maxBackoffTimeSeconds,
                                                  (maxRedeliveryAttempts == 0xFF), maxRedeliveryAttempts };

    // Setting up retry parameters
    return emberAfIasZoneServerConfigStatusQueueRetryParams(&retryConfig);
}

void emberAfWwahAppEventRetryManagerSetBackoffParamsToDefault(void)
{
    emberAfIasZoneServerSetStatusQueueRetryParamsToDefault();
}
#endif // defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)

void emberAfPluginIasZoneServerPrintQueue(void)
{
    emberAfIasZoneClusterPrintln("%d/%d entries", messageQueue.entriesInQueue, NUM_QUEUE_ENTRIES);
    for (int i = 0; i < messageQueue.entriesInQueue; i++)
    {
        emberAfIasZoneClusterPrintln("Entry %d: Endpoint: %d Status: %d EventTimeMs: %d", i, messageQueue.buffer[i].endpoint,
                                     messageQueue.buffer[i].status, messageQueue.buffer[i].eventTimeMs);
    }
}

void emberAfPluginIasZoneServerPrintQueueConfig(void)
{
    emberAfCorePrintln("First backoff time (sec): %d", queueRetryParams.config.firstBackoffTimeSec);
    emberAfCorePrintln("Backoff sequence common ratio: %d", queueRetryParams.config.backoffSeqCommonRatio);
    emberAfCorePrintln("Max backoff time (sec): %d", queueRetryParams.config.maxBackoffTimeSec);
    emberAfCorePrintln("Max redelivery attempts: %d", queueRetryParams.config.maxRetryAttempts);
}

#endif // defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)

// This callback will be generated any time the node receives an ACK or a NAK
// for a message transmitted for the IAS Zone Cluster Server.  Note that this
// will not be called in the case that the message was not delivered to the
// destination when the destination is the only router the node is joined to.
// In that case, the command will never have been sent, as the device will have
// had no router by which to send the command.
void emberAfIasZoneClusterServerMessageSentCallback(const MessageSendDestination & destination, EmberApsFrame * apsFrame,
                                                    uint16_t msgLen, uint8_t * message, EmberStatus status)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    uint8_t frameControl;
    CommandId commandId;

    IasZoneStatusQueueEntry dummyEntry;

    // Verify that this response is for a ZoneStatusChangeNotification command
    // by checking the message length, the command and direction bits of the
    // Frame Control byte, and the command ID
    if (msgLen < IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX)
    {
        return;
    }

    frameControl = message[ZCL_FRAME_CONTROL_IDX];
    if (!(frameControl & ZCL_CLUSTER_SPECIFIC_COMMAND) || !(frameControl & ZCL_FRAME_CONTROL_SERVER_TO_CLIENT))
    {
        return;
    }

    commandId = message[IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX];
    if (commandId != ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID)
    {
        return;
    }

    // If a change status change notification command is not received by the
    // client, delay the option specified amount of time and try to resend it.
    // The event handler will perform the retransmit per the preset queue retry
    // parameteres, and the original send request will handle populating the buffer.
    // Do not try to retransmit again if the maximum number of retries attempts
    // is reached, this is however discarded if configured for unlimited retries.
    if ((status == EMBER_DELIVERY_FAILED) &&
        (queueRetryParams.config.unlimitedRetries ||
         (queueRetryParams.currentRetryCount < queueRetryParams.config.maxRetryAttempts)))
    {
        queueRetryParams.currentRetryCount++;

        emberAfIasZoneClusterPrintln("Status command update failed to send... Retrying in %d seconds...",
                                     queueRetryParams.currentBackoffTimeSec);

        // Delay according to the current retransmit backoff time.
        emberEventControlSetDelayMS(emberAfPluginIasZoneServerManageQueueEventControl,
                                    queueRetryParams.currentBackoffTimeSec * MILLISECOND_TICKS_PER_SECOND);

        // The backoff time needs to be increased if the maximum backoff time is not reached yet.
        if ((queueRetryParams.currentBackoffTimeSec * queueRetryParams.config.backoffSeqCommonRatio) <=
            queueRetryParams.config.maxBackoffTimeSec)
        {
            queueRetryParams.currentBackoffTimeSec *= queueRetryParams.config.backoffSeqCommonRatio;
        }
    }
    else
    {
        // If a command message was sent or max redelivery attempts were reached,
        // remove it from the queue and move on to the next queued message until the queue is empty.
        if (status == EMBER_SUCCESS)
        {
            emberAfIasZoneClusterPrintln("\nZone update successful, remove entry from queue");
        }
        else
        {
            emberAfIasZoneClusterPrintln("\nZone update unsuccessful, max retry attempts reached, remove entry from queue");
        }
        popFromBuffer(&messageQueue, &dummyEntry);

        // Reset queue retry parameters.
        resetCurrentQueueRetryParams();

        if (messageQueue.entriesInQueue)
        {
            emberEventControlSetActive(&emberAfPluginIasZoneServerManageQueueEventControl);
        }
    }
#endif
}

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
static void bufferInit(IasZoneStatusQueue * ring)
{
    ring->entriesInQueue = 0;
    ring->startIdx       = 0;
    ring->lastIdx        = NUM_QUEUE_ENTRIES - 1;
}

// Add the entry to the buffer by copying, returning the index at which it was
// added.  If the buffer is full, return -1, but still copy the entry over the
// last item of the buffer, to ensure that the last item in the buffer is
// always representative of the last known device state.
static int16_t copyToBuffer(IasZoneStatusQueue * ring, const IasZoneStatusQueueEntry * entry)
{
    if (ring->entriesInQueue == NUM_QUEUE_ENTRIES)
    {
        ring->buffer[ring->lastIdx] = *entry;
        return -1;
    }

    // Increment the last pointer.  If it rolls over the size, circle it back to
    // zero.
    ring->lastIdx++;
    if (ring->lastIdx >= NUM_QUEUE_ENTRIES)
    {
        ring->lastIdx = 0;
    }

    ring->buffer[ring->lastIdx].endpoint    = entry->endpoint;
    ring->buffer[ring->lastIdx].status      = entry->status;
    ring->buffer[ring->lastIdx].eventTimeMs = entry->eventTimeMs;

    ring->entriesInQueue++;
    return ring->lastIdx;
}

// Return the idx of the popped entry, or -1 if the buffer was empty.
static int16_t popFromBuffer(IasZoneStatusQueue * ring, IasZoneStatusQueueEntry * entry)
{
    int16_t retVal;

    if (ring->entriesInQueue == 0)
    {
        return -1;
    }

    // Copy out the first entry, then increment the start pointer.  If it rolls
    // over, circle it back to zero.
    *entry = ring->buffer[ring->startIdx];
    retVal = ring->startIdx;

    ring->startIdx++;
    if (ring->startIdx >= NUM_QUEUE_ENTRIES)
    {
        ring->startIdx = 0;
    }

    ring->entriesInQueue--;

    return retVal;
}

uint16_t computeElapsedTimeQs(IasZoneStatusQueueEntry * entry)
{
    uint32_t currentTimeMs = System::Clock::GetMonotonicMilliseconds();
    int64_t deltaTimeMs    = currentTimeMs - entry->eventTimeMs;

    if (deltaTimeMs < 0)
    {
        deltaTimeMs = -deltaTimeMs + (0xFFFFFFFF - currentTimeMs);
    }

    return deltaTimeMs / MILLISECOND_TICKS_PER_QUARTERSECOND;
}
#endif
