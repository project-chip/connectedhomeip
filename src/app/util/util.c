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
 * @file
 * @brief This file contains all of the common ZCL
 *command and attribute handling code for Ember's ZCL
 *implementation
 *******************************************************************************
 ******************************************************************************/

#include "af-main.h"
#include "af.h"
#include "common.h"
//#include "../plugin/time-server/time-server.h"
//#include "app/framework/util/af-event.h"
//#include "app/framework/util/time-util.h"
#include "gen/znet-bookkeeping.h"
//#include "hal/micro/crc.h"

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

// Storage and functions for turning on and off devices
bool afDeviceEnabled[MAX_ENDPOINT_COUNT];

#ifdef EMBER_AF_ENABLE_STATISTICS
// a variable containing the number of messages send from the utilities
// since emberAfInit was called.
uint32_t afNumPktsSent;
#endif

const EmberAfClusterName zclClusterNames[] = {
    CLUSTER_IDS_TO_NAMES                                            // defined in print-cluster.h
    { ZCL_NULL_CLUSTER_ID, EMBER_AF_NULL_MANUFACTURER_CODE, NULL }, // terminator
};

static const EmberAfClusterCommand staticCmd;
EmberAfClusterCommand curCmd;
// A pointer to the current command being processed
// This struct is allocated on the stack inside
// emberAfProcessMessage. The pointer below is set
// to NULL when the function exits.
EmberAfClusterCommand * emAfCurrentCommand;

// variable used for toggling Aps Link security. Set by the CLI
uint8_t emAfTestApsSecurityOverride = APS_TEST_SECURITY_DEFAULT;

// DEPRECATED.
uint8_t emberAfIncomingZclSequenceNumber = 0xFF;

static bool afNoSecurityForDefaultResponse = false;

// Sequence used for outgoing messages if they are
// not responses.
uint8_t emberAfSequenceNumber = 0xFF;

// A bool value so we know when the device is performing
// key establishment.
bool emAfDeviceIsPerformingKeyEstablishment = false;

static uint8_t /*enum EmberAfRetryOverride*/ emberAfApsRetryOverride                      = EMBER_AF_RETRY_OVERRIDE_NONE;
static uint8_t /*enum EmberAfDisableDefaultResponse*/ emAfDisableDefaultResponse          = EMBER_AF_DISABLE_DEFAULT_RESPONSE_NONE;
static uint8_t /*enum EmberAfDisableDefaultResponse*/ emAfSavedDisableDefaultResponseVale = EMBER_AF_DISABLE_DEFAULT_RESPONSE_NONE;

// Holds the response type
uint8_t emberAfResponseType = ZCL_UTIL_RESP_NORMAL;

static EmberAfInterpanHeader interpanResponseHeader;

static const uint8_t emberAfAnalogDiscreteThresholds[] = { 0x07, EMBER_AF_DATA_TYPE_NONE,   0x1F, EMBER_AF_DATA_TYPE_DISCRETE,
                                                           0x2F, EMBER_AF_DATA_TYPE_ANALOG, 0x37, EMBER_AF_DATA_TYPE_DISCRETE,
                                                           0x3F, EMBER_AF_DATA_TYPE_ANALOG, 0x57, EMBER_AF_DATA_TYPE_DISCRETE,
                                                           0xDF, EMBER_AF_DATA_TYPE_NONE,   0xE7, EMBER_AF_DATA_TYPE_ANALOG,
                                                           0xFF, EMBER_AF_DATA_TYPE_NONE };

uint8_t emAfExtendedPanId[EXTENDED_PAN_ID_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0,
};

#ifdef EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_DECLARATIONS
#endif
#ifdef EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
#endif

//------------------------------------------------------------------------------

// Device enabled/disabled functions
bool emberAfIsDeviceEnabled(uint8_t endpoint)
{
    uint8_t index;
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
    bool deviceEnabled;
    if (emberAfReadServerAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_DEVICE_ENABLED_ATTRIBUTE_ID, (uint8_t *) &deviceEnabled,
                                   sizeof(deviceEnabled)) == EMBER_ZCL_STATUS_SUCCESS)
    {
        return deviceEnabled;
    }
#endif
    index = emberAfIndexFromEndpoint(endpoint);
    if (index != 0xFF && index < sizeof(afDeviceEnabled))
    {
        return afDeviceEnabled[index];
    }
    return false;
}

void emberAfSetDeviceEnabled(uint8_t endpoint, bool enabled)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    if (index != 0xFF && index < sizeof(afDeviceEnabled))
    {
        afDeviceEnabled[index] = enabled;
    }
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
    emberAfWriteServerAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_DEVICE_ENABLED_ATTRIBUTE_ID, (uint8_t *) &enabled,
                                ZCL_BOOLEAN_ATTRIBUTE_TYPE);
#endif
}

// Is the device identifying?
bool emberAfIsDeviceIdentifying(uint8_t endpoint)
{
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
    uint16_t identifyTime;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_IDENTIFY_CLUSTER_ID, ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                      (uint8_t *) &identifyTime, sizeof(identifyTime));
    return (status == EMBER_ZCL_STATUS_SUCCESS && 0 < identifyTime);
#else
    return false;
#endif
}

// Calculates difference. See EmberAfDifferenceType for the maximum data size
// that this function will support.
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize)
{
    EmberAfDifferenceType value2 = 0, diff;
    uint8_t i;

    // only support data types up to 8 bytes
    if (dataSize > sizeof(EmberAfDifferenceType))
    {
        return 0;
    }

    // get the value
    for (i = 0; i < dataSize; i++)
    {
        value2 = value2 << 8;
#if (BIGENDIAN_CPU)
        value2 += pData[i];
#else  // BIGENDIAN
        value2 += pData[dataSize - i - 1];
#endif // BIGENDIAN
    }

    if (value > value2)
    {
        diff = value - value2;
    }
    else
    {
        diff = value2 - value;
    }

    return diff;
}

// --------------------------------------------------

static void prepareForResponse(const EmberAfClusterCommand * cmd)
{
    emberAfResponseApsFrame.profileId           = cmd->apsFrame->profileId;
    emberAfResponseApsFrame.clusterId           = cmd->apsFrame->clusterId;
    emberAfResponseApsFrame.sourceEndpoint      = cmd->apsFrame->destinationEndpoint;
    emberAfResponseApsFrame.destinationEndpoint = cmd->apsFrame->sourceEndpoint;

    // Use the default APS options for the response, but also use encryption and
    // retries if the incoming message used them.  The rationale is that the
    // sender of the request cares about some aspects of the delivery, so we as
    // the receiver should make equal effort for the response.
    emberAfResponseApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
    if ((cmd->apsFrame->options & EMBER_APS_OPTION_ENCRYPTION) != 0U)
    {
        emberAfResponseApsFrame.options |= EMBER_APS_OPTION_ENCRYPTION;
    }
    if ((cmd->apsFrame->options & EMBER_APS_OPTION_RETRY) != 0U)
    {
        emberAfResponseApsFrame.options |= EMBER_APS_OPTION_RETRY;
    }

    if (cmd->interPanHeader == NULL)
    {
        emberAfResponseDestination = cmd->source;
        emberAfResponseType &= ~ZCL_UTIL_RESP_INTERPAN;
    }
    else
    {
        emberAfResponseType |= ZCL_UTIL_RESP_INTERPAN;
        memmove(&interpanResponseHeader, cmd->interPanHeader, sizeof(EmberAfInterpanHeader));
        // Always send responses as unicast
        interpanResponseHeader.messageType = EMBER_AF_INTER_PAN_UNICAST;
    }
}

// ****************************************
// Initialize Clusters
// ****************************************
void emberAfInit(void)
{
    uint8_t i;
#ifdef EMBER_AF_ENABLE_STATISTICS
    afNumPktsSent = 0;
#endif

    for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++)
    {
        // FIXME: Do we need to support more than one network?
        // emberAfPushNetworkIndex(i);
        emberAfInitializeAttributes(EMBER_BROADCAST_ENDPOINT);
        // emberAfPopNetworkIndex();
    }

    memset(afDeviceEnabled, true, emberAfEndpointCount());

    // Set up client API buffer.
    emberAfSetExternalBuffer(appResponseData, EMBER_AF_RESPONSE_BUFFER_LEN, &appResponseLength, &emberAfResponseApsFrame);

    // initialize event management system
    // emAfInitEvents();

#ifdef EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_CALLS
    EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_CALLS
#endif

    emAfCallInits();
}

void emberAfTick(void)
{
    // Call the AFV2-specific per-endpoint callbacks
    // Anything that defines callbacks as void *TickCallback(void) is called in
    // emAfInit(), which is a generated file
#ifdef EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_CALLS
    EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_CALLS
#endif
}

// ****************************************
// This function is called by the application when the stack goes down,
// such as after a leave network. This allows zcl utils to clear state
// that should not be kept when changing networks
// ****************************************
void emberAfStackDown(void)
{
    // (Case 14696) Clearing the report table is only necessary if the stack is
    // going down for good; if we're rejoining, leave the table intact since we'll
    // be right back, hopefully.
    // (Issue 77101) Also don't clear the table if the stack has gone down as a
    // a result of losing its parent or some other transient state where a future
    // rejoin is expected to get us back online.
    if (false
        // emberStackIsPerformingRejoin() == false
        // && emberNetworkState() == EMBER_NO_NETWORK
    )
    {
        // the report table should be cleared when the stack comes down.
        // going to a new network means new report devices should be discovered.
        // if the table isnt cleared the device keeps trying to send messages.
        emberAfClearReportTableCallback();
    }

    emberAfRegistrationAbortCallback();
    emberAfTrustCenterKeepaliveAbortCallback();
}

// ****************************************
// Print out information about each cluster
// ****************************************

uint16_t emberAfFindClusterNameIndexWithMfgCode(uint16_t cluster, uint16_t mfgCode)
{
    uint16_t index = 0;
    while (zclClusterNames[index].id != ZCL_NULL_CLUSTER_ID)
    {
        if (zclClusterNames[index].id == cluster
            // This check sees if its a standard cluster, in which mfgCode is ignored
            // due to the name being well defined.
            // If it is manufacturer specific, then we try to check to see if we
            // know the name of the cluster within the list.
            // If the mfgCode we are given is null, then we just ignore it for backward
            // compatibility reasons
            && (cluster < 0xFC00 || zclClusterNames[index].mfgCode == mfgCode || mfgCode == EMBER_AF_NULL_MANUFACTURER_CODE))
        {
            return index;
        }
        index++;
    }
    return 0xFFFF;
}

uint16_t emberAfFindClusterNameIndex(uint16_t cluster)
{
    return emberAfFindClusterNameIndexWithMfgCode(cluster, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// This function parses into the cluster name table, and tries to find
// the index in the table that has the two keys: cluster + mfgcode.
void emberAfDecodeAndPrintClusterWithMfgCode(uint16_t cluster, uint16_t mfgCode)
{
    uint16_t index = emberAfFindClusterNameIndexWithMfgCode(cluster, mfgCode);
    if (index == 0xFFFF)
    {
        emberAfPrint(emberAfPrintActiveArea, "(Unknown clus. [0x%2x])", cluster);
    }
    else
    {
        emberAfPrint(emberAfPrintActiveArea, "(%p)", zclClusterNames[index].name);
    }
}

void emberAfDecodeAndPrintCluster(uint16_t cluster)
{
    emberAfDecodeAndPrintClusterWithMfgCode(cluster, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// This function makes the assumption that
// emberAfCurrentCommand will either be NULL
// when invalid, or will have a valid mfgCode
// when called.
// If it is invalid, we just return the
// EMBER_AF_NULL_MANUFACTURER_CODE, which we tend to use
// for references to the standard library.
uint16_t emberAfGetMfgCodeFromCurrentCommand(void)
{
    if (emberAfCurrentCommand() != NULL)
    {
        return emberAfCurrentCommand()->mfgCode;
    }
    else
    {
        return EMBER_AF_NULL_MANUFACTURER_CODE;
    }
}

static void printIncomingZclMessage(const EmberAfClusterCommand * cmd)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
    if (emberAfPrintReceivedMessages)
    {
        // emberAfAppPrint("\r\nT%4x:", emberAfGetCurrentTime());
        emberAfAppPrint("RX len %d, ep %x, clus 0x%2x ", cmd->bufLen, cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId);
        emberAfAppDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(cmd->apsFrame->clusterId, cmd->mfgCode));
        if (cmd->mfgSpecific)
        {
            emberAfAppPrint(" mfgId %2x", cmd->mfgCode);
        }
        emberAfAppPrint(" FC %x seq %x cmd %x payload[",
                        cmd->buffer[0], // frame control
                        cmd->seqNum, cmd->commandId);
        emberAfAppFlush();
        emberAfAppPrintBuffer(cmd->buffer + cmd->payloadStartIndex, // message
                              cmd->bufLen - cmd->payloadStartIndex, // length
                              true);                                // spaces?
        emberAfAppFlush();
        emberAfAppPrintln("]");
    }
#endif
}

static bool dispatchZclMessage(EmberAfClusterCommand * cmd)
{
    uint8_t index = emberAfIndexFromEndpoint(cmd->apsFrame->destinationEndpoint);

    if (index == 0xFF)
    {
        emberAfDebugPrint("Drop cluster 0x%2x command 0x%x", cmd->apsFrame->clusterId, cmd->commandId);
        emberAfDebugPrint(" due to invalid endpoint: ");
        emberAfDebugPrintln("0x%x", cmd->apsFrame->destinationEndpoint);
        return false;
    }
    else if (emberAfNetworkIndexFromEndpointIndex(index) != cmd->networkIndex)
    {
        emberAfDebugPrint("Drop cluster 0x%2x command 0x%x", cmd->apsFrame->clusterId, cmd->commandId);
        emberAfDebugPrint(" for endpoint 0x%x due to wrong %s: ", cmd->apsFrame->destinationEndpoint, "network");
        emberAfDebugPrintln("%d", cmd->networkIndex);
        return false;
    }
    else if (emberAfProfileIdFromIndex(index) != cmd->apsFrame->profileId &&
             (cmd->apsFrame->profileId != EMBER_WILDCARD_PROFILE_ID ||
              (EMBER_MAXIMUM_STANDARD_PROFILE_ID < emberAfProfileIdFromIndex(index))))
    {
        emberAfDebugPrint("Drop cluster 0x%2x command 0x%x", cmd->apsFrame->clusterId, cmd->commandId);
        emberAfDebugPrint(" for endpoint 0x%x due to wrong %s: ", cmd->apsFrame->destinationEndpoint, "profile");
        emberAfDebugPrintln("0x%02x", cmd->apsFrame->profileId);
        return false;
    }
    else if ((cmd->type == EMBER_INCOMING_MULTICAST || cmd->type == EMBER_INCOMING_MULTICAST_LOOPBACK) &&
             !emberAfGroupsClusterEndpointInGroupCallback(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->groupId))
    {
        emberAfDebugPrint("Drop cluster 0x%2x command 0x%x", cmd->apsFrame->clusterId, cmd->commandId);
        emberAfDebugPrint(" for endpoint 0x%x due to wrong %s: ", cmd->apsFrame->destinationEndpoint, "group");
        emberAfDebugPrintln("0x%02x", cmd->apsFrame->groupId);
        return false;
    }
    else
    {
        return (cmd->clusterSpecific ? emAfProcessClusterSpecificCommand(cmd) : emAfProcessGlobalCommand(cmd));
    }
}

bool emberAfProcessMessageIntoZclCmd(EmberApsFrame * apsFrame, EmberIncomingMessageType type, uint8_t * message,
                                     uint16_t messageLength, ChipResponseDestination * source, InterPanHeader * interPanHeader,
                                     EmberAfClusterCommand * returnCmd)
{
    uint8_t minLength =
        (message[0] & ZCL_MANUFACTURER_SPECIFIC_MASK ? EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD : EMBER_AF_ZCL_OVERHEAD);

    if (messageLength < minLength)
    {
        emberAfAppPrintln("%pRX pkt too short: %d < %d", "ERROR: ", messageLength, minLength);
        return false;
    }

    // Populate the cluster command struct for processing.
    returnCmd->apsFrame        = apsFrame;
    returnCmd->type            = type;
    returnCmd->source          = source;
    returnCmd->buffer          = message;
    returnCmd->bufLen          = messageLength;
    returnCmd->clusterSpecific = (message[0] & ZCL_CLUSTER_SPECIFIC_COMMAND);
    returnCmd->mfgSpecific     = (message[0] & ZCL_MANUFACTURER_SPECIFIC_MASK);
    returnCmd->direction =
        ((message[0] & ZCL_FRAME_CONTROL_DIRECTION_MASK) ? ZCL_DIRECTION_SERVER_TO_CLIENT : ZCL_DIRECTION_CLIENT_TO_SERVER);
    returnCmd->payloadStartIndex = 1;
    if (returnCmd->mfgSpecific)
    {
        returnCmd->mfgCode = emberAfGetInt16u(message, returnCmd->payloadStartIndex, messageLength);
        returnCmd->payloadStartIndex += 2;
    }
    else
    {
        returnCmd->mfgCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    }
    returnCmd->seqNum    = message[returnCmd->payloadStartIndex++];
    returnCmd->commandId = message[returnCmd->payloadStartIndex++];
    if (returnCmd->payloadStartIndex > returnCmd->bufLen)
    {
        emberAfAppPrintln("%pRX pkt malformed: %d < %d", "ERROR: ", returnCmd->bufLen, returnCmd->payloadStartIndex);
        return false;
    }
    returnCmd->interPanHeader = interPanHeader;
    // returnCmd->networkIndex   = emberGetCurrentNetwork();
    return true;
}

// a single call to process global and cluster-specific messages and callbacks.
bool emberAfProcessMessage(EmberApsFrame * apsFrame, EmberIncomingMessageType type, uint8_t * message, uint16_t msgLen,
                           ChipResponseDestination * source, InterPanHeader * interPanHeader)
{
    EmberStatus sendStatus;
    bool msgHandled = false;
    // reset/reinitialize curCmd
    curCmd = staticCmd;
    if (!emberAfProcessMessageIntoZclCmd(apsFrame, type, message, msgLen, source, interPanHeader, &curCmd))
    {
        goto kickout;
    }

    emAfCurrentCommand = &curCmd;

    // All of these should be covered by the EmberAfClusterCommand but are
    // still here until all the code is moved over to use the cmd. -WEH
    emberAfIncomingZclSequenceNumber = curCmd.seqNum;

    printIncomingZclMessage(&curCmd);
    prepareForResponse(&curCmd);

    if (emAfPreCommandReceived(&curCmd))
    {
        msgHandled = true;
        goto kickout;
    }

    if (interPanHeader == NULL)
    {
        bool broadcast = (type == EMBER_INCOMING_BROADCAST || type == EMBER_INCOMING_BROADCAST_LOOPBACK ||
                          type == EMBER_INCOMING_MULTICAST || type == EMBER_INCOMING_MULTICAST_LOOPBACK);

        // if the cluster for the incoming message requires security and
        // doesnt have it return default response STATUS_FAILURE
        if (emberAfDetermineIfLinkSecurityIsRequired(curCmd.commandId,
                                                     true, // incoming
                                                     broadcast, curCmd.apsFrame->profileId, curCmd.apsFrame->clusterId,
                                                     curCmd.source) &&
            (!(curCmd.apsFrame->options & EMBER_APS_OPTION_ENCRYPTION)))
        {
            emberAfDebugPrintln("Drop clus %2x due to no aps security", curCmd.apsFrame->clusterId);
            afNoSecurityForDefaultResponse = true;
            sendStatus                     = emberAfSendDefaultResponse(&curCmd, EMBER_ZCL_STATUS_NOT_AUTHORIZED);
            if (EMBER_SUCCESS != sendStatus)
            {
                emberAfDebugPrintln("Util: failed to send %s response: 0x%x", "default", sendStatus);
            }
            afNoSecurityForDefaultResponse = false;

            // Mark the message as processed.  It failed security processing, so no
            // other parts of the code should act upon it.
            msgHandled = true;
            goto kickout;
        }
    }
    else if (!(interPanHeader->options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS))
    {
        // For safety, dump all interpan messages that don't have a long
        // source in the MAC layer.  In theory they should not get past
        // the MAC filters but this is insures they will not get processed.
        goto kickout;
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }

    if (curCmd.apsFrame->destinationEndpoint == EMBER_BROADCAST_ENDPOINT)
    {
        uint8_t i;
        for (i = 0; i < emberAfEndpointCount(); i++)
        {
            uint8_t endpoint = emberAfEndpointFromIndex(i);
            if (!emberAfEndpointIndexIsEnabled(i) ||
                !emberAfContainsClusterWithMfgCode(endpoint, curCmd.apsFrame->clusterId, curCmd.mfgCode))
            {
                continue;
            }
            // Since the APS frame is cleared after each sending,
            // we must reinitialize it.  It is cleared to prevent
            // data from leaking out and being sent inadvertently.
            prepareForResponse(&curCmd);

            // Change the destination endpoint of the incoming command and the source
            // source endpoint of the response so they both reflect the endpoint the
            // message is actually being passed to in this iteration of the loop.
            curCmd.apsFrame->destinationEndpoint   = endpoint;
            emberAfResponseApsFrame.sourceEndpoint = endpoint;
            if (dispatchZclMessage(&curCmd))
            {
                msgHandled = true;
            }
        }
    }
    else
    {
        msgHandled = dispatchZclMessage(&curCmd);
    }

kickout:
    emberAfClearResponseData();
    memset(&interpanResponseHeader, 0, sizeof(EmberAfInterpanHeader));
    emAfCurrentCommand = NULL;
    return msgHandled;
}

uint8_t emberAfNextSequence(void)
{
    return ((++emberAfSequenceNumber) & EMBER_AF_ZCL_SEQUENCE_MASK);
}

uint8_t emberAfGetLastSequenceNumber(void)
{
    return (emberAfSequenceNumber & EMBER_AF_ZCL_SEQUENCE_MASK);
}

// the caller to the library can set a flag to say do not respond to the
// next ZCL message passed in to the library. Passing true means disable
// the reply for the next ZCL message. Setting to false re-enables the
// reply (in the case where the app disables it and then doesnt send a
// message that gets parsed).
void emberAfSetNoReplyForNextMessage(bool set)
{
    if (set)
    {
        emberAfResponseType |= ZCL_UTIL_RESP_NONE;
    }
    else
    {
        emberAfResponseType &= ~ZCL_UTIL_RESP_NONE;
    }
}

void emberAfSetRetryOverride(EmberAfRetryOverride value)
{
    emberAfApsRetryOverride = value;
}

EmberAfRetryOverride emberAfGetRetryOverride(void)
{
    return (EmberAfRetryOverride) emberAfApsRetryOverride;
}

void emAfApplyRetryOverride(EmberApsOption * options)
{
    if (options == NULL)
    {
        return;
    }
    else if (emberAfApsRetryOverride == EMBER_AF_RETRY_OVERRIDE_SET)
    {
        *options |= EMBER_APS_OPTION_RETRY;
    }
    else if (emberAfApsRetryOverride == EMBER_AF_RETRY_OVERRIDE_UNSET)
    {
        *options &= ~EMBER_APS_OPTION_RETRY;
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }
}

void emberAfSetDisableDefaultResponse(EmberAfDisableDefaultResponse value)
{
    emAfDisableDefaultResponse = value;
    if (value != EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT)
    {
        emAfSavedDisableDefaultResponseVale = value;
    }
}

EmberAfDisableDefaultResponse emberAfGetDisableDefaultResponse(void)
{
    return (EmberAfDisableDefaultResponse) emAfDisableDefaultResponse;
}

void emAfApplyDisableDefaultResponse(uint8_t * frame_control)
{
    if (frame_control == NULL)
    {
        return;
    }
    else if (emAfDisableDefaultResponse == EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT)
    {
        emAfDisableDefaultResponse = emAfSavedDisableDefaultResponseVale;
        *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
    }
    else if (emAfDisableDefaultResponse == EMBER_AF_DISABLE_DEFAULT_RESPONSE_PERMANENT)
    {
        *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }
}

static bool isBroadcastDestination(ChipResponseDestination * responseDestination)
{
    // FIXME: Will need to actually figure out how to test for this!
    return false;
}

EmberStatus emberAfSendResponseWithCallback(EmberAfMessageSentFunction callback)
{
    EmberStatus status;
    uint8_t label;

    // If the no-response flag is set, don't send anything.
    if ((emberAfResponseType & ZCL_UTIL_RESP_NONE) != 0U)
    {
        emberAfDebugPrintln("ZCL Util: no response at user request");
        return EMBER_SUCCESS;
    }

    // Make sure we are respecting the request APS options
    // there are seemingly some calls to emberAfSendResponse
    //  that occur outside of the emberAfProcessMessage context,
    //  which leads to a bad memory reference - AHilton
    if (emberAfCurrentCommand() != NULL)
    {
        if ((emberAfCurrentCommand()->apsFrame->options & EMBER_APS_OPTION_ENCRYPTION) != 0U)
        {
            emberAfResponseApsFrame.options |= EMBER_APS_OPTION_ENCRYPTION;
        }
        if ((emberAfCurrentCommand()->apsFrame->options & EMBER_APS_OPTION_RETRY) != 0U)
        {
            emberAfResponseApsFrame.options |= EMBER_APS_OPTION_RETRY;
        }
    }

    // Fill commands may increase the sequence.  For responses, we want to make
    // sure the sequence is reset to that of the request.
    if ((appResponseData[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U)
    {
        appResponseData[3] = emberAfIncomingZclSequenceNumber;
    }
    else
    {
        appResponseData[1] = emberAfIncomingZclSequenceNumber;
    }

    // The manner in which the message is sent depends on the response flags and
    // the destination of the message.
    if ((emberAfResponseType & ZCL_UTIL_RESP_INTERPAN) != 0U)
    {
        label  = 'I';
        status = emberAfInterpanSendMessageCallback(&interpanResponseHeader, appResponseLength, appResponseData);
        emberAfResponseType &= ~ZCL_UTIL_RESP_INTERPAN;
    }
    else if (!isBroadcastDestination(emberAfResponseDestination))
    {
        label  = 'U';
        status = chipSendResponse(emberAfResponseDestination, &emberAfResponseApsFrame, appResponseLength, appResponseData);
    }
    else
    {
        label = 'B';
#if 0
    status = emberAfSendBroadcastWithCallback(emberAfResponseDestination,
                                              &emberAfResponseApsFrame,
                                              appResponseLength,
                                              appResponseData,
                                              callback);
#else
        status = EMBER_SUCCESS;
#endif
    }
    UNUSED_VAR(label);
    emberAfDebugPrintln("T%4x:TX (%p) %ccast 0x%x%p", 0, "resp", label, status,
                        ((emberAfResponseApsFrame.options & EMBER_APS_OPTION_ENCRYPTION) ? " w/ link key" : ""));
    emberAfDebugPrint("TX buffer: [");
    emberAfDebugFlush();
    emberAfDebugPrintBuffer(appResponseData, appResponseLength, true);
    emberAfDebugPrintln("]");
    emberAfDebugFlush();

#ifdef EMBER_AF_ENABLE_STATISTICS
    if (status == EMBER_SUCCESS)
    {
        afNumPktsSent++;
    }
#endif

    return status;
}

EmberStatus emberAfSendResponse(void)
{
    return emberAfSendResponseWithCallback(NULL);
}

EmberStatus emberAfSendImmediateDefaultResponseWithCallback(EmberAfStatus status, EmberAfMessageSentFunction callback)
{
    return emberAfSendDefaultResponseWithCallback(emberAfCurrentCommand(), status, callback);
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
    return emberAfSendImmediateDefaultResponseWithCallback(status, NULL);
}

EmberStatus emberAfSendDefaultResponseWithCallback(const EmberAfClusterCommand * cmd, EmberAfStatus status,
                                                   EmberAfMessageSentFunction callback)
{
    uint8_t frameControl;

    // Default Response commands are only sent in response to unicast commands.
    if (cmd->type != EMBER_INCOMING_UNICAST && cmd->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return EMBER_SUCCESS;
    }

    // If the Disable Default Response sub-field is set, Default Response commands
    // are only sent if there was an error.
    if ((cmd->buffer[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK) && status == EMBER_ZCL_STATUS_SUCCESS)
    {
        return EMBER_SUCCESS;
    }

    // Default Response commands are never sent in response to other Default
    // Response commands.
    if (!cmd->clusterSpecific && cmd->commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID)
    {
        return EMBER_SUCCESS;
    }

    appResponseLength = 0;
    frameControl      = (ZCL_GLOBAL_COMMAND |
                    (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                                                      : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER));

    if (!cmd->mfgSpecific)
    {
        emberAfPutInt8uInResp(frameControl & (uint8_t) ~ZCL_MANUFACTURER_SPECIFIC_MASK);
    }
    else
    {
        emberAfPutInt8uInResp(frameControl | ZCL_MANUFACTURER_SPECIFIC_MASK);
        emberAfPutInt16uInResp(cmd->mfgCode);
    }
    emberAfPutInt8uInResp(cmd->seqNum);
    emberAfPutInt8uInResp(ZCL_DEFAULT_RESPONSE_COMMAND_ID);
    emberAfPutInt8uInResp(cmd->commandId);
    emberAfPutInt8uInResp(status);

    prepareForResponse(cmd);
    return emberAfSendResponseWithCallback(callback);
}

EmberStatus emberAfSendDefaultResponse(const EmberAfClusterCommand * cmd, EmberAfStatus status)
{
    return emberAfSendDefaultResponseWithCallback(cmd, status, NULL);
}

bool emberAfDetermineIfLinkSecurityIsRequired(uint8_t commandId, bool incoming, bool broadcast, EmberAfProfileId profileId,
                                              EmberAfClusterId clusterId, ChipResponseDestination * remoteNodeId)
{
    (void) afNoSecurityForDefaultResponse; // remove warning if not used

    // If we have turned off all APS security (needed for testing), then just
    // always return false.
    if ((emAfTestApsSecurityOverride == APS_TEST_SECURITY_DISABLED) || afNoSecurityForDefaultResponse)
    {
        afNoSecurityForDefaultResponse = false;
        return false;
    }

    // NOTE: In general if it is a unicast, and one of the SE clusters, it
    // requires APS encryption.  A few special cases exists that we allow for
    // but those must be explicitly spelled out here.

    // Assume that if the local device is broadcasting, even if it is using one
    // of the SE clusters, this is okay.
    if (!incoming && broadcast)
    {
        return false;
    }

    // At this point if the CLI command has been issued, it's safe to over any other settings
    // and return.
    // This change allows HA applications to use the CLI option to enable APS security.
    if (emAfTestApsSecurityOverride == APS_TEST_SECURITY_ENABLED)
    {
        return true;
    }
    else if (emAfTestApsSecurityOverride == APS_TEST_SECURITY_DISABLED)
    {
        // The default return value before this change.
        return false;
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }

#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
    if (emberAfIsCurrentSecurityProfileSmartEnergy())
    {
        // Check against profile IDs that use APS security on these clusters.
        if (profileId != SE_PROFILE_ID && profileId != EMBER_WILDCARD_PROFILE_ID)
        {
            return false;
        }

        // Loopback packets do not require security
        if (emberGetNodeId() == remoteNodeId)
        {
            return false;
        }

        // This list comes from Section 5.4.6 of the SE spec.
        switch (clusterId)
        {
        case ZCL_TIME_CLUSTER_ID:
        case ZCL_COMMISSIONING_CLUSTER_ID:
        case ZCL_PRICE_CLUSTER_ID:
        case ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID:
        case ZCL_SIMPLE_METERING_CLUSTER_ID:
        case ZCL_MESSAGING_CLUSTER_ID:
        case ZCL_TUNNELING_CLUSTER_ID:
        case ZCL_GENERIC_TUNNEL_CLUSTER_ID:
        case ZCL_PREPAYMENT_CLUSTER_ID:
        case ZCL_CALENDAR_CLUSTER_ID:
        case ZCL_DEVICE_MANAGEMENT_CLUSTER_ID:
        case ZCL_EVENTS_CLUSTER_ID:
        case ZCL_MDU_PAIRING_CLUSTER_ID:
        case ZCL_ENERGY_MANAGEMENT_CLUSTER_ID:
        case ZCL_SUB_GHZ_CLUSTER_ID:
            return true;
        case ZCL_OTA_BOOTLOAD_CLUSTER_ID:
            if (commandId == ZCL_IMAGE_NOTIFY_COMMAND_ID && broadcast)
            {
                return false;
            }
            else
            {
                return true;
            }
        default:
            break;
        }
    }
#endif // EMBER_AF_HAS_SECURITY_PROFILE_SE

    // All works with all hubs commands require aps link key authorization
    if (clusterId == ZCL_SL_WWAH_CLUSTER_ID)
    {
        return true;
    }

    if (emberAfClusterSecurityCustomCallback(profileId, clusterId, incoming, commandId))
    {
        return true;
    }

    // APS_TEST_SECURITY_DEFAULT at this point returns false.
    return false;
}

uint8_t emberAfMaximumApsPayloadLength(EmberOutgoingMessageType type, uint16_t indexOrDestination, EmberApsFrame * apsFrame)
{
    EmberNodeId destination = EMBER_UNKNOWN_NODE_ID;
    uint8_t max             = EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH;

    if ((apsFrame->options & EMBER_APS_OPTION_ENCRYPTION) != 0U)
    {
        max -= EMBER_AF_APS_ENCRYPTION_OVERHEAD;
    }
    if ((apsFrame->options & EMBER_APS_OPTION_SOURCE_EUI64) != 0U)
    {
        max -= EUI64_SIZE;
    }
    if ((apsFrame->options & EMBER_APS_OPTION_DESTINATION_EUI64) != 0U)
    {
        max -= EUI64_SIZE;
    }
    if ((apsFrame->options & EMBER_APS_OPTION_FRAGMENT) != 0U)
    {
        max -= EMBER_AF_APS_FRAGMENTATION_OVERHEAD;
    }

    switch (type)
    {
    case EMBER_OUTGOING_DIRECT:
        destination = indexOrDestination;
        break;
    case EMBER_OUTGOING_VIA_ADDRESS_TABLE:
        // destination = emberGetAddressTableRemoteNodeId(indexOrDestination);
        break;
    case EMBER_OUTGOING_VIA_BINDING:
        // destination = emberGetBindingRemoteNodeId(indexOrDestination);
        break;
    case EMBER_OUTGOING_MULTICAST:
        // APS multicast messages include the two-byte group id and exclude the
        // one-byte destination endpoint, for a net loss of an extra byte.
        max--;
        break;
    case EMBER_OUTGOING_BROADCAST:
        break;
    default:
        // MISRA requires default case.
        break;
    }

    max -= emberAfGetSourceRouteOverheadCallback(destination);

    return max;
}

void emberAfCopyInt16u(uint8_t * data, uint16_t index, uint16_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
}

void emberAfCopyInt24u(uint8_t * data, uint16_t index, uint32_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
    data[index + 2] = (uint8_t)(((x) >> 16) & 0xFF);
}

void emberAfCopyInt32u(uint8_t * data, uint16_t index, uint32_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
    data[index + 2] = (uint8_t)(((x) >> 16) & 0xFF);
    data[index + 3] = (uint8_t)(((x) >> 24) & 0xFF);
}

void emberAfCopyString(uint8_t * dest, uint8_t * src, uint8_t size)
{
    if (src == NULL)
    {
        dest[0] = 0; // Zero out the length of string
    }
    else if (src[0] == 0xFF)
    {
        dest[0] = src[0];
    }
    else
    {
        uint8_t length = emberAfStringLength(src);
        if (size < length)
        {
            length = size;
        }
        memmove(dest + 1, src + 1, length);
        dest[0] = length;
    }
}

void emberAfCopyLongString(uint8_t * dest, uint8_t * src, uint16_t size)
{
    if (src == NULL)
    {
        dest[0] = dest[1] = 0; // Zero out the length of string
    }
    else if ((src[0] == 0xFF) && (src[1] == 0xFF))
    {
        dest[0] = 0xFF;
        dest[1] = 0xFF;
    }
    else
    {
        uint16_t length = emberAfLongStringLength(src);
        if (size < length)
        {
            length = size;
        }
        memmove(dest + 2, src + 2, length);
        dest[0] = LOW_BYTE(length);
        dest[1] = HIGH_BYTE(length);
    }
}

#if (BIGENDIAN_CPU)
#define EM_BIG_ENDIAN true
#else
#define EM_BIG_ENDIAN false
#endif

// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
int8_t emberAfCompareValues(uint8_t * val1, uint8_t * val2, uint8_t len, bool signedNumber)
{
    uint8_t i, j, k;
    if (signedNumber)
    { // signed number comparison
        if (len <= 4)
        { // only number with 32-bits or less is supported
            int32_t accum1 = 0x0;
            int32_t accum2 = 0x0;
            int32_t all1s  = -1;

            for (i = 0; i < len; i++)
            {
                j = (val1 == NULL ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
                accum1 |= j << (8 * (len - 1 - i));

                k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);
                accum2 |= k << (8 * (len - 1 - i));
            }

            // sign extending, no need for 32-bits numbers
            if (len < 4)
            {
                if ((accum1 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum1 |= all1s - ((1 << (len * 8)) - 1);
                }
                if ((accum2 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum2 |= all1s - ((1 << (len * 8)) - 1);
                }
            }

            if (accum1 > accum2)
            {
                return 1;
            }
            else if (accum1 < accum2)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
        else
        { // not supported
            return 0;
        }
    }
    else
    { // regular unsigned number comparison
        for (i = 0; i < len; i++)
        {
            j = (val1 == NULL ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
            k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);

            if (j > k)
            {
                return 1;
            }
            else if (k > j)
            {
                return -1;
            }
            else
            {
                // MISRA requires ..else if.. to have terminating else.
            }
        }
        return 0;
    }
}

#if 0
// Moving to time-util.c
int8_t emberAfCompareDates(EmberAfDate* date1, EmberAfDate* date2)
{
  uint32_t val1 = emberAfEncodeDate(date1);
  uint32_t val2 = emberAfEncodeDate(date2);
  return (val1 == val2) ? 0 : ((val1 < val2) ? -1 : 1);
}
#endif

// returns the type that the attribute is, either EMBER_AF_DATA_TYPE_ANALOG,
// EMBER_AF_DATA_TYPE_DISCRETE, or EMBER_AF_DATA_TYPE_NONE. This is based on table
// 2.15 from the ZCL spec 075123r02
uint8_t emberAfGetAttributeAnalogOrDiscreteType(uint8_t dataType)
{
    uint8_t index = 0;

    while (emberAfAnalogDiscreteThresholds[index] < dataType)
    {
        index += 2;
    }
    return emberAfAnalogDiscreteThresholds[index + 1];
}

// Zigbee spec says types between signed 8 bit and signed 64 bit
bool emberAfIsTypeSigned(EmberAfAttributeType dataType)
{
    return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

EmberStatus emberAfEndpointEventControlSetInactive(EmberEventControl * controls, uint8_t endpoint)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFF)
    {
        return EMBER_INVALID_ENDPOINT;
    }
    // emberEventControlSetInactive(controls[index]);
    return EMBER_SUCCESS;
}

bool emberAfEndpointEventControlGetActive(EmberEventControl * controls, uint8_t endpoint)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    return (index != 0xFF && false /*emberEventControlGetActive(controls[index])*/);
}

EmberStatus emberAfEndpointEventControlSetActive(EmberEventControl * controls, uint8_t endpoint)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFF)
    {
        return EMBER_INVALID_ENDPOINT;
    }
    // emberEventControlSetActive(controls[index]);
    return EMBER_SUCCESS;
}

// EmberStatus emberAfEndpointEventControlSetDelayMS(EmberEventControl * controls, uint8_t endpoint, uint32_t delayMs)
// {
//     uint8_t index = emberAfIndexFromEndpoint(endpoint);
//     if (index == 0xFF)
//     {
//         return EMBER_INVALID_ENDPOINT;
//     }
//     return emberAfEventControlSetDelayMS(&controls[index], delayMs);
// }

// EmberStatus emberAfEndpointEventControlSetDelayQS(EmberEventControl * controls, uint8_t endpoint, uint32_t delayQs)
// {
//     uint8_t index = emberAfIndexFromEndpoint(endpoint);
//     if (index == 0xFF)
//     {
//         return EMBER_INVALID_ENDPOINT;
//     }
//     return emberAfEventControlSetDelayQS(&controls[index], delayQs);
// }

// EmberStatus emberAfEndpointEventControlSetDelayMinutes(EmberEventControl * controls, uint8_t endpoint, uint16_t delayM)
// {
//     uint8_t index = emberAfIndexFromEndpoint(endpoint);
//     if (index == 0xFF)
//     {
//         return EMBER_INVALID_ENDPOINT;
//     }
//     return emberAfEventControlSetDelayMinutes(&controls[index], delayM);
// }

// bool emberAfIsThisMyEui64(EmberEUI64 eui64)
// {
//     EmberEUI64 myEui64;
//     emberAfGetEui64(myEui64);
//     return (0 == MEMCOMPARE(eui64, myEui64, EUI64_SIZE) ? true : false);
// }

uint8_t emberAfAppendCharacters(uint8_t * zclString, uint8_t zclStringMaxLen, const uint8_t * appendingChars,
                                uint8_t appendingCharsLen)
{
    uint8_t freeChars;
    uint8_t curLen;
    uint8_t charsToWrite;

    if ((zclString == NULL) || (zclStringMaxLen == 0) || (appendingChars == NULL) || (appendingCharsLen == 0))
    {
        return 0;
    }

    curLen = emberAfStringLength(zclString);

    if ((zclString[0] == 0xFF) || (curLen >= zclStringMaxLen))
    {
        return 0;
    }

    freeChars    = zclStringMaxLen - curLen;
    charsToWrite = (freeChars > appendingCharsLen) ? appendingCharsLen : freeChars;

    memcpy(&zclString[1 + curLen], // 1 is to account for zcl's length byte
           appendingChars, charsToWrite);
    zclString[0] = curLen + charsToWrite;
    return charsToWrite;
}

/*
   On each page, first channel maps to channel number zero and so on.
   Example:
   page    Band      Rage of 90 channels    Per page channel mapping
   28     863 MHz        0-26                    0-26
   29     863 MHz        27-34,62                0-8 (Here 7th channel maps to 34 and 8th to 62)
   30     863 MHz        35 - 61                 0-26
   31     915            0-26                    0-26

 */
EmberStatus emAfValidateChannelPages(uint8_t page, uint8_t channel)
{
    switch (page)
    {
    case 0:
        if (!((channel <= EMBER_MAX_802_15_4_CHANNEL_NUMBER) &&
              ((EMBER_MIN_802_15_4_CHANNEL_NUMBER == 0) || (channel >= EMBER_MIN_802_15_4_CHANNEL_NUMBER))))
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    case 28:
    case 30:
    case 31:
        if (channel > EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGES_28_30_31)
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    case 29:
        if (channel > EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGE_29)
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    default:
        return EMBER_PHY_INVALID_CHANNEL;
        break;
    }
    return EMBER_SUCCESS;
}

void slabAssert(const char * file, int line)
{
    (void) file; // Unused parameter
    (void) line; // Unused parameter
    // Wait forever until the watchdog fires
    while (true)
    {
    }
}

#define ENCODED_8BIT_CHANPG_PAGE_MASK 0xE0         // top 3 bits
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0 0x00  // 0b000xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28 0x80 // 0b100xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29 0xA0 // 0b101xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30 0xC0 // 0b110xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31 0xE0 // 0b111xxxxx

#define ENCODED_8BIT_CHANPG_CHANNEL_MASK 0x1F // bottom 5 bits

uint8_t emberAfGetPageFrom8bitEncodedChanPg(uint8_t chanPg)
{
    switch (chanPg & ENCODED_8BIT_CHANPG_PAGE_MASK)
    {
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0:
        return 0;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28:
        return 28;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29:
        return 29;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30:
        return 30;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31:
        return 31;
    default:
        return 0xFF;
    }
}

uint8_t emberAfGetChannelFrom8bitEncodedChanPg(uint8_t chanPg)
{
    return chanPg & ENCODED_8BIT_CHANPG_CHANNEL_MASK;
}

uint8_t emberAfMake8bitEncodedChanPg(uint8_t page, uint8_t channel)
{
    if (emAfValidateChannelPages(page, channel) != EMBER_SUCCESS)
    {
        return 0xFF;
    }

    switch (page)
    {
    case 28:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28;
    case 29:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29;
    case 30:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30;
    case 31:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31;
    default:
        // Strictly speaking, we only need case 0 here, but MISRA in its infinite
        // wisdom requires a default case. Since we have validated the arguments
        // already, and 0 is the only remaining case, we simply treat the default
        // as case 0 to make MISRA happy.
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0;
    }
}

EmberStatus emberAfScheduleServerTick(uint8_t endpoint, EmberAfClusterId clusterId, uint32_t delayMs)
{
    return EMBER_SUCCESS; // Stub for now.
}

EmberStatus emberAfScheduleServerTickExtended(uint8_t endpoint, EmberAfClusterId clusterId, uint32_t delayMs,
                                              EmberAfEventPollControl pollControl, EmberAfEventSleepControl sleepControl)
{
    return EMBER_SUCCESS; // Stub for now.
}

EmberStatus emberAfDeactivateServerTick(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return EMBER_SUCCESS; // Stub for now.
}
