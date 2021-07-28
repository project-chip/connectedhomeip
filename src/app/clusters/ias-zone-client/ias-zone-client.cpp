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
 * @brief  *
 * Client Operation:
 *   1. Look for ZDO device announce notification.
 *   2. Perform ZDO match descriptor on device.
 *   3. If supports IAS Zone Server, Add that server
 *to our known list. Write CIE Address.
 *   4. Read CIE address, verify it is ours.  This is
 *done mostly because the test case requires it.
 *   5. Read the IAS Zone Server attributes.
 *     Record in table.
 *   6. When we get an enroll request, give them our
 *(only) zone ID.
 *   7. When we get a notification, read their
 *attributes.
 *
 * Improvements that could be made:
 *   Add support for multiple endpoints on server.
 *Most often this is a legacy security system
 *retrofitted with a single ZigBee radio.  Therefore
 *   each sensor is on a different endpoint.  Right
 *now our client only handles a single endpoint per
 *node.
 *
 *   Integration with Poll Control.  When the device
 *boots we should configure its polling to make it
 *possible to read/write its attributes.
 *
 *   Update the emberAfIasZoneClientKnownServers list
 *when we know a server un-enrolls. Right now, we
 *don't have any way to tell when we don't need to
 *keep track of a server anymore, i.e., when it
 *un-enrolls. Therefore, we could potentially keep
 *adding servers to our known list, and run out of
 *room to add more. Fortunately, we have two things
 *working for us:
 *     1. Servers will most likely stay around in a
 *network. It is unlikely that an IAS Zone Client in
 *production will have to handle 254 different
 *servers.
 *     2. If a server un-enrolls and then enrolls
 *again, it will get the same Zone ID and have a spot
 *in the list, since we store servers by long address.
 *******************************************************************************
 ******************************************************************************/

#include "ias-zone-client.h"
#include <app/CommandHandler.h>
#include <app/util/af.h>

//-----------------------------------------------------------------------------
// Globals

IasZoneDevice emberAfIasZoneClientKnownServers[EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES];

typedef enum
{
    IAS_ZONE_CLIENT_STATE_NONE,
    IAS_ZONE_CLIENT_STATE_DISCOVER_ENDPOINT,
    IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS,
    IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS,
    IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES,
} IasZoneClientState;

static IasZoneClientState iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
static uint8_t currentIndex                  = NO_INDEX;
static uint8_t myEndpoint                    = 0;

EmberEventControl emberAfPluginIasZoneClientStateMachineEventControl;

//-----------------------------------------------------------------------------
// Forward Declarations

void readIasZoneServerAttributes(EmberNodeId nodeId);
static void iasClientSaveCommand(void);
static void iasClientLoadCommand(void);

//-----------------------------------------------------------------------------
// Functions

void emberAfIasZoneClusterClientInitCallback(EndpointId endpoint)
{
    emAfClearServers();
    myEndpoint = endpoint;
    iasClientLoadCommand();
}

void emAfClearServers(void)
{
    MEMSET(emberAfIasZoneClientKnownServers, 0xFF, sizeof(IasZoneDevice) * EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES);
}

static void clearState(void)
{
    currentIndex       = 0;
    iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
}

static void setServerZoneStatus(uint8_t serverIndex, uint16_t zoneStatus)
{
    emberAfIasZoneClientKnownServers[serverIndex].zoneStatus = zoneStatus;
    iasClientSaveCommand();
}

static void setServerIeee(uint8_t serverIndex, uint8_t * ieeeAddress)
{
    MEMCOPY(emberAfIasZoneClientKnownServers[serverIndex].ieeeAddress, ieeeAddress, EUI64_SIZE);
    iasClientSaveCommand();
}

static void clearServerIeee(uint8_t serverIndex)
{
    MEMSET(emberAfIasZoneClientKnownServers[serverIndex].ieeeAddress, 0xFF, sizeof(IasZoneDevice));
    iasClientSaveCommand();
}

static void setServerNodeId(uint8_t serverIndex, EmberNodeId nodeId)
{
    emberAfIasZoneClientKnownServers[serverIndex].nodeId = nodeId;
}

static void clearServerNodeId(uint8_t serverIndex)
{
    emberAfIasZoneClientKnownServers[serverIndex].nodeId = EMBER_NULL_NODE_ID;
}

static void setServerZoneState(uint8_t serverIndex, uint8_t zoneState)
{
    emberAfIasZoneClientKnownServers[serverIndex].zoneState = zoneState;
    iasClientSaveCommand();
}

static void setServerEndpoint(uint8_t serverIndex, EndpointId endpoint)
{
    emberAfIasZoneClientKnownServers[serverIndex].endpoint = endpoint;
    iasClientSaveCommand();
}

static void setServerZoneType(uint8_t serverIndex, uint16_t zoneType)
{
    emberAfIasZoneClientKnownServers[serverIndex].zoneType = zoneType;
    iasClientSaveCommand();
}

static void setServerZoneId(uint8_t serverIndex, uint16_t zoneId)
{
    emberAfIasZoneClientKnownServers[serverIndex].zoneId = zoneId;
    iasClientSaveCommand();
}

static void setCurrentIndex(uint8_t serverIndex)
{
    currentIndex = serverIndex;
    iasClientSaveCommand();
}

static void setIasZoneClientState(uint8_t clientState)
{
    iasZoneClientState = clientState;
    iasClientSaveCommand();
}

static void iasClientSaveCommand(void)
{
#if defined(EZSP_HOST) && !defined(EMBER_TEST) && defined(UNIX_HOST)
    FILE * fp;
    uint16_t i, j;

    // save zone server list
    fp = fopen("iaszone.txt", "w");

    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        if (emberAfIasZoneClientKnownServers[i].zoneId != 0xFF)
        {
            fprintf(fp, "%x %x %x %x %x ", emberAfIasZoneClientKnownServers[i].zoneId,
                    emberAfIasZoneClientKnownServers[i].zoneStatus, emberAfIasZoneClientKnownServers[i].zoneState,
                    emberAfIasZoneClientKnownServers[i].endpoint, emberAfIasZoneClientKnownServers[i].zoneType);
            for (j = 0; j < 8; j++)
            {
                fprintf(fp, "%x ", emberAfIasZoneClientKnownServers[i].ieeeAddress[j]);
            }
        }
    }
    // Write something to mark the end of the file.
    fprintf(fp, "ff");
    int res = fclose(fp);
    assert(res == 0);
#endif //#if defined(EZSP_HOST) && !defined(EMBER_TEST) && defined(UNIX_HOST)
}

static void iasClientLoadCommand(void)
{
#if defined(EZSP_HOST) && !defined(EMBER_TEST) && defined(UNIX_HOST)
    FILE * fp;
    uint16_t i, j;

    unsigned int data1, data2, data3, data4, data5;

    fp = fopen("iaszone.txt", "r");

    if (!fp)
    {
        return;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        if (feof(fp))
        {
            break;
        }
        fscanf(fp, "%x ", &data1);
        if (data1 == 0xff)
        {
            break;
        }
        fscanf(fp, "%x %x %x %x ", &data2, &data3, &data4, &data5);

        emberAfIasZoneClientKnownServers[i].zoneId     = (uint8_t) data1;
        emberAfIasZoneClientKnownServers[i].zoneStatus = (uint16_t) data2;
        emberAfIasZoneClientKnownServers[i].zoneState  = (uint8_t) data3;
        emberAfIasZoneClientKnownServers[i].endpoint   = (uint8_t) data4;
        emberAfIasZoneClientKnownServers[i].zoneType   = (uint16_t) data5;

        for (j = 0; j < 8; j++)
        {
            fscanf(fp, "%x ", &data1);
            emberAfIasZoneClientKnownServers[i].ieeeAddress[j] = (uint8_t) data1;
        }
    }
    int res = fclose(fp);
    assert(res == 0);
#endif // #if defined(EZSP_HOST) && !defined(EMBER_TEST) && defined(UNIX_HOST)
}

static uint8_t findIasZoneServerByIeee(uint8_t * ieeeAddress)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        if (0 == memcmp(ieeeAddress, emberAfIasZoneClientKnownServers[i].ieeeAddress, EUI64_SIZE))
        {
            return i;
        }
    }
    return NO_INDEX;
}

static uint8_t findIasZoneServerByNodeId(EmberNodeId nodeId)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        if (nodeId == emberAfIasZoneClientKnownServers[i].nodeId)
        {
            return i;
        }
    }

    // If we didn't find the node ID in the table, see if the stack knows about
    // it.
    EmberEUI64 eui64;
    if (emberLookupEui64ByNodeId(nodeId, eui64) == EMBER_SUCCESS)
    {
        i = findIasZoneServerByIeee(eui64);
        if (i != NO_INDEX)
        {
            setServerNodeId(i, nodeId);
        }
    }

    return i;
}

bool emberAfIasZoneClusterZoneStatusChangeNotificationCallback(chip::app::CommandHandler * commandObj, uint16_t zoneStatus,
                                                               uint8_t extendedStatus, uint8_t zoneId, uint16_t delay)
{
    uint8_t serverIndex = findIasZoneServerByNodeId(emberAfCurrentCommand()->source);
    uint8_t status      = EMBER_ZCL_STATUS_NOT_FOUND;
    if (serverIndex != NO_INDEX)
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
        setServerZoneStatus(serverIndex, zoneStatus);

        emberAfIasZoneClusterPrintln("Zone %d status change, 0x%2X from 0x%2X", zoneId, zoneStatus,
                                     emberAfCurrentCommand()->source);

        // The Test case calls for readding attributes after status change.
        //   that is silly for the production device.
        // readIasZoneServerAttributes(emberAfCurrentCommand()->source);
    }
    emberAfSendDefaultResponse(emberAfCurrentCommand(), status);
    return true;
}

bool emberAfIasZoneClusterZoneEnrollRequestCallback(chip::app::CommandHandler * commandObj, uint16_t zoneType,
                                                    uint16_t manufacturerCode)
{
    EmberAfIasEnrollResponseCode responseCode = EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT;
    uint8_t zoneId                            = UNKNOWN_ZONE_ID;
    uint8_t serverIndex                       = findIasZoneServerByNodeId(emberAfCurrentCommand()->source);
    EmberStatus status;

    if (serverIndex != NO_INDEX)
    {
        responseCode = EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS;
        zoneId       = serverIndex;
        setServerZoneId(serverIndex, zoneId);
    }
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID, "uu", responseCode, zoneId);
    // Need to send this command with our source EUI because the server will
    // check our EUI64 against his CIE Address to see if we're his CIE.
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    status = emberAfSendResponse();
    emberAfCorePrintln("Sent enroll response with responseCode: 0x%X, zoneId: 0x%X, status: 0x%X", responseCode, zoneId, status);
    return true;
}

void emberAfPluginIasZoneClientStateMachineEventHandler(void)
{
    emberAfIasZoneClusterPrintln("IAS Zone Client Timeout waiting for message response.");
    emberEventControlSetInactive(emberAfPluginIasZoneClientStateMachineEventControl);
    clearState();
}

static uint8_t addServer(EmberNodeId nodeId, uint8_t * ieeeAddress)
{
    uint8_t i = findIasZoneServerByIeee(ieeeAddress);
    if (i != NO_INDEX)
    {
        return i;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        const uint8_t unsetEui64[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        if (0 == memcmp(emberAfIasZoneClientKnownServers[i].ieeeAddress, unsetEui64, EUI64_SIZE))
        {
            setServerIeee(i, ieeeAddress);
            setServerNodeId(i, nodeId);
            setServerEndpoint(i, UNKNOWN_ENDPOINT);
            return i;
        }
    }
    return NO_INDEX;
}

static void removeServer(uint8_t * ieeeAddress)
{
    uint8_t index = findIasZoneServerByIeee(ieeeAddress);
    clearServerIeee(index);
    clearServerNodeId(index);
}

static EmberStatus sendCommand(EmberNodeId destAddress)
{
    emberAfSetCommandEndpoints(myEndpoint, emberAfIasZoneClientKnownServers[currentIndex].endpoint);
    EmberStatus status = emberAfSendCommandUnicast(MessageSendDestination::Direct(destAddress));
    emberAfIasZoneClusterPrintln("Sent IAS Zone Client Command to 0x%2X (%d -> %d) status: 0x%X", destAddress, myEndpoint,
                                 emberAfIasZoneClientKnownServers[currentIndex].endpoint, status);
    if (status != EMBER_SUCCESS)
    {
        clearState();
    }
    return status;
}

static void setCieAddress(EmberNodeId destAddress)
{
    uint8_t writeAttributes[] = {
        EMBER_LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
        EMBER_HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
        ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, // ieee (filled in later)
    };
    emberAfGetEui64(&writeAttributes[3]);
    emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_WRITE_ATTRIBUTES_COMMAND_ID, "b", writeAttributes, sizeof(writeAttributes));
    emberAfIasZoneClusterPrintln("Writing CIE Address to IAS Zone Server");
    if (EMBER_SUCCESS == sendCommand(destAddress))
    {
        setIasZoneClientState(IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS);
    }
}

static void iasZoneClientServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult * result)
{
    if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE &&
        result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST)
    {
        const EmberAfEndpointList * endpointList = (const EmberAfEndpointList *) result->responseData;
        if (endpointList->count > 0)
        {
            setServerEndpoint(currentIndex, endpointList->list[0]);
            emberAfIasZoneClusterPrintln("Device 0x%2X supports IAS Zone Server", result->matchAddress);
            setCieAddress(result->matchAddress);
            return;
        }
    }
    clearState();
}

static void checkForIasZoneServer(EmberNodeId emberNodeId, uint8_t * ieeeAddress)
{
    uint8_t endpointIndex = emberAfIndexFromEndpoint(myEndpoint);
    uint8_t serverIndex   = addServer(emberNodeId, ieeeAddress);

    if (serverIndex == NO_INDEX)
    {
        emberAfIasZoneClusterPrintln("Error: Could not add IAS Zone server.");
        return;
    }

    setCurrentIndex(serverIndex);

    if (emberAfIasZoneClientKnownServers[serverIndex].endpoint != UNKNOWN_ENDPOINT)
    {
        // If a remote endpoint that you have already seen announces itself,
        // write your IEEE in them just in case they left and are rejoining. --agkeesle
        // Bug: EMAPPFWKV2-1078
        setCieAddress(emberNodeId);
        emberAfIasZoneClusterPrintln("Node 0x%2X already known to IAS client", emberNodeId);
        return;
    }

    EmberStatus status = emberAfFindDevicesByCluster(emberNodeId, ZCL_IAS_ZONE_CLUSTER_ID,
                                                     true, // server cluster?
                                                     iasZoneClientServiceDiscoveryCallback);

    if (status != EMBER_SUCCESS)
    {
        emberAfIasZoneClusterPrintln("Error: Failed to initiate service discovery for IAS Zone Server 0x%2X", emberNodeId);
        clearState();
    }
}

void emberAfPluginIasZoneClientZdoMessageReceivedCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message,
                                                          uint16_t length)
{
    emberAfIasZoneClusterPrintln("Incoming ZDO, Cluster: 0x%2X", apsFrame->clusterId);
    if (apsFrame->clusterId == END_DEVICE_ANNOUNCE)
    {
        checkForIasZoneServer(emberNodeId, &(message[3]));
    }
}

void readIasZoneServerAttributes(EmberNodeId nodeId)
{
    uint8_t iasZoneAttributeIds[] = {
        EMBER_LOW_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),  EMBER_HIGH_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),

        EMBER_LOW_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),   EMBER_HIGH_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),

        EMBER_LOW_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID), EMBER_HIGH_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID),
    };
    emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_READ_ATTRIBUTES_COMMAND_ID, "b", iasZoneAttributeIds, sizeof(iasZoneAttributeIds));
    if (EMBER_SUCCESS == sendCommand(nodeId))
    {
        setIasZoneClientState(IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES);
    }
}

void readIasZoneServerCieAddress(EmberNodeId nodeId)
{
    uint8_t iasZoneAttributeIds[] = {
        EMBER_LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
        EMBER_HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    };
    emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), ZCL_IAS_ZONE_CLUSTER_ID,
                              ZCL_READ_ATTRIBUTES_COMMAND_ID, "b", iasZoneAttributeIds, sizeof(iasZoneAttributeIds));
    if (EMBER_SUCCESS == sendCommand(nodeId))
    {
        setIasZoneClientState(IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS);
    }
}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID && iasZoneClientState == IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS &&
        buffer[0] == EMBER_ZCL_STATUS_SUCCESS)
    {
        readIasZoneServerCieAddress(emberAfCurrentCommand()->source);
        return;
    }
    return;
}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    uint8_t zoneStatus, zoneType, zoneState;
    if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID &&
        (iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES ||
         iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS))
    {
        uint16_t i = 0;
        while ((i + 3) <= bufLen)
        { // 3 to insure we can read at least the attribute ID
          // and the status
            AttributeId attributeId = buffer[i] + (buffer[i + 1] << 8);
            uint8_t status          = buffer[i + 2];
            i += 3;
            // emberAfIasZoneClusterPrintln("Parsing Attribute 0x%2X, Status: 0x%X", attributeId, status);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                if ((i + 1) > bufLen)
                {
                    // Too short, dump the message.
                    return;
                }
                i++; // skip the type of the attribute.  We already know what it should be.
                switch (attributeId)
                {
                case ZCL_ZONE_STATUS_ATTRIBUTE_ID:
                    if ((i + 2) > bufLen)
                    {
                        // Too short, dump the message.
                        return;
                    }
                    zoneStatus = (buffer[i] + (buffer[i + 1] << 8));
                    setServerZoneStatus(currentIndex, zoneStatus);
                    i += 2;
                    break;
                case ZCL_ZONE_TYPE_ATTRIBUTE_ID:
                    if ((i + 2) > bufLen)
                    {
                        // Too short, dump the message.
                        return;
                    }
                    zoneType = (buffer[i] + (buffer[i + 1] << 8));
                    setServerZoneType(currentIndex, zoneType);
                    i += 2;
                    break;
                case ZCL_ZONE_STATE_ATTRIBUTE_ID:
                    if ((i + 1) > bufLen)
                    {
                        // Too short, dump the message
                        return;
                    }
                    zoneState = buffer[i];
                    setServerZoneState(currentIndex, zoneState);
                    i++;
                    break;
                case ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID: {
                    uint8_t myIeee[EUI64_SIZE];
                    emberAfGetEui64(myIeee);
                    if ((i + 8) > bufLen)
                    {
                        // Too short, dump the message
                    }
                    else if (0 != memcmp(&(buffer[i]), myIeee, EUI64_SIZE))
                    {
                        emberAfIasZoneClusterPrintln("CIE Address not set to mine, removing IAS zone server.");
                        removeServer(&(buffer[i]));
                        clearState();
                    }
                    else
                    {
                        readIasZoneServerAttributes(emberAfCurrentCommand()->source);
                    }
                    return;
                }
                }
            }
        }
        emberAfIasZoneClusterPrintln("Retrieved IAS Zone Server attributes from 0x%2X", emberAfCurrentCommand()->source);
        clearState();
    }
}

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message, uint16_t length) {}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(ClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(ClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}
