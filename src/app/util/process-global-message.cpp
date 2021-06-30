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
 * @brief This file contains function that processes
 *global ZCL message.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

#include <app/clusters/ias-zone-client/ias-zone-client.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/reporting/reporting.h>
#include <app/util/common.h>

#include "gen/callback.h"

#ifdef EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_SUB_GHZ
#include "app/framework/plugin/comms-hub-function-sub-ghz/comms-hub-function-sub-ghz.h"
#endif

#include <support/CodeUtils.h>

using namespace chip;

// flag to keep track of the fact that we just sent a read attr for time and
// we should set our time to the result of the read attr response.
bool emAfSyncingTime = false;

#ifdef EMBER_AF_GBCS_COMPATIBLE
// Some GBCS use cases (e.g. GCS15e, GCS21f) require that ReadAttributesResponse
// should be send back with Disable Default Response flag set. The only pattern
// is that the decision is based on the cluster and attribute IDs requested.
// To reduce the possibility of false positives, we disable default response
// only for responses containing at least the specified minimum of attributes.
#define MIN_MATCHING_ATTR_IDS_TO_DISABLE_DEFAULT_RESPONSE 3
#endif

#define DISC_ATTR_RSP_MAX_ATTRIBUTES                                                                                               \
    (((EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH - EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD /* max ZCL header size */                \
       - 1)                                                                               /* discovery is complete boolean */      \
      / 5)        /* size of one discover attributes response entry */                                                             \
     % UINT8_MAX) /* make count fit in an 8 bit integer */
#define DISC_ATTR_EXT_RSP_MAX_ATTRIBUTES                                                                                           \
    (((EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH - EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD /* max ZCL header size */                \
       - 1)                                                                               /* discovery is complete boolean */      \
      / 5)        /* size of one discover attributes extended response entry */                                                    \
     % UINT8_MAX) /* make count fit in an 8 bit integer */

#if defined(EMBER_AF_SUPPORT_COMMAND_DISCOVERY)
static void printDiscoverCommandsResponse(bool generated, ClusterId clusterId, bool discoveryComplete, uint8_t * buffer,
                                          uint16_t length)
{
    uint16_t i;
    emberAfServiceDiscoveryPrint("Discover Commands response (complete: %c), %p IDs: ", (discoveryComplete ? 'y' : 'n'),
                                 (generated ? "Generated" : "Received"));
    for (i = 0; i < length; i++)
    {
        emberAfServiceDiscoveryPrint("0x%X ", buffer[i]);
    }
    emberAfServiceDiscoveryPrintln("");
}
#endif

bool emAfProcessGlobalCommand(EmberAfClusterCommand * cmd)
{
    AttributeId attrId;
    uint8_t frameControl;
    // This is a little clumsy but easier to read and port
    // from earlier implementation.
    ClusterId clusterId      = cmd->apsFrame->clusterId;
    CommandId zclCmd         = cmd->commandId;
    uint8_t * message        = cmd->buffer;
    uint16_t msgLen          = cmd->bufLen;
    uint16_t msgIndex        = cmd->payloadStartIndex;
    uint8_t clientServerMask = (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER ? CLUSTER_MASK_SERVER : CLUSTER_MASK_CLIENT);

    // If we are disabled then we can only respond to read or write commands
    // or identify cluster (see device enabled attr of basic cluster)
    if (!emberAfIsDeviceEnabled(cmd->apsFrame->destinationEndpoint) && zclCmd != ZCL_READ_ATTRIBUTES_COMMAND_ID &&
        zclCmd != ZCL_WRITE_ATTRIBUTES_COMMAND_ID && zclCmd != ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID &&
        zclCmd != ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID && clusterId != ZCL_IDENTIFY_CLUSTER_ID)
    {
        emberAfCorePrintln("disabled");
        emberAfDebugPrintln("%pd, dropping global cmd:%x", "disable", zclCmd);
        emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    // If a manufacturer-specific command arrives using our special internal "not
    // manufacturer specific" code, we need to reject it outright without letting
    // it pass through to the rest of the code.  The internal read and write APIs
    // would interpret it as a standard attribute or cluster and return incorrect
    // results.
    if (cmd->mfgSpecific && cmd->mfgCode == EMBER_AF_NULL_MANUFACTURER_CODE)
    {
        goto kickout;
    }

    // Clear out the response buffer by setting its length to zero
    appResponseLength = 0;

    // Make the ZCL header for the response
    // note: cmd byte is set below
    frameControl = static_cast<uint8_t>(ZCL_GLOBAL_COMMAND |
                                        (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER
                                             ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES
                                             : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES));
    if (cmd->mfgSpecific)
    {
        frameControl |= ZCL_MANUFACTURER_SPECIFIC_MASK;
    }
    emberAfPutInt8uInResp(frameControl);
    if (cmd->mfgSpecific)
    {
        emberAfPutInt16uInResp(cmd->mfgCode);
    }
    emberAfPutInt8uInResp(cmd->seqNum);

    switch (zclCmd)
    {
    // Write undivided means all attributes must be written in order to write
    // any of them. So first do a check. If the check fails, send back a fail
    // response. If it works, fall through to the normal write attr code.
    // write attr responses are the same for undivided and normal writes.
    case ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID: {
        uint8_t numFailures = 0;
        uint8_t dataType;
        uint16_t dataSize;
        EmberAfStatus status;

        emberAfPutInt32uInResp(ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID);

        // Go through the message until there are no more attrID/type/data
        while (msgIndex < msgLen - 5)
        {
            attrId   = emberAfGetInt32u(message, msgIndex, msgLen);
            dataType = emberAfGetInt8u(message, msgIndex + 4, msgLen);

            dataSize = emberAfAttributeValueSize(clusterId, attrId, dataType, message + msgIndex + 5);

            // Check to see if there are dataSize bytes left in the message if it is a string
            if (emberAfIsThisDataTypeAStringType(dataType) && (dataSize < msgLen - (msgIndex + 5)))
            {
                // This command is malformed
                status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            }
            else
            {
                status = emberAfVerifyAttributeWrite(cmd->apsFrame->destinationEndpoint, clusterId, attrId, clientServerMask,
                                                     cmd->mfgCode, &(message[msgIndex + 5]), dataType);
            }

            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                numFailures++;
                // Write to the response buffer - status and then attrID
                emberAfPutStatusInResp(status);
                emberAfPutInt32uInResp(attrId);

                emberAfAttributesPrintln("WRITE: clus %2x attr %2x ", clusterId, attrId);
                emberAfAttributesPrintln("FAIL %x", status);
                emberAfCoreFlush();
                if (status == EMBER_ZCL_STATUS_MALFORMED_COMMAND)
                {
                    // this attribute is malformed, terminate attribute processing.
                    break;
                }
            }

            // Increment past the attribute id (four bytes), the type (one byte), and
            // the data (N bytes, including the length byte for strings).
            msgIndex = static_cast<uint16_t>(msgIndex + 5 + dataSize);
        }
        // If there are any failures, send the response and exit
        if (numFailures > 0)
        {
            emberAfSendResponse();
            return true;
        }
    }
        // Reset message back to start
        msgIndex          = cmd->payloadStartIndex;
        appResponseLength = (cmd->mfgSpecific ? 4 : 2);
        FALLTHROUGH;
        /* fall through */
    // DO NOT BREAK from this case

    // the format of the write attributes cmd is:
    // ([attr ID:4] [data type:1] [data:N]) * N
    // the format of the write attributes response is:
    // ([status 1] [attr ID 2]) * n
    // ONLY errors are reported unless all are successful then a single success
    // is sent. write attr no response is handled by just executing the same
    // code but not setting the flag that sends the response at the end.
    case ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID:
    case ZCL_WRITE_ATTRIBUTES_COMMAND_ID: {
        uint8_t numFailures = 0;
        uint8_t numSuccess  = 0;
        uint8_t dataType;
        uint16_t dataSize;
#if (BIGENDIAN_CPU)
        uint8_t writeData[ATTRIBUTE_LARGEST];
#endif //(BIGENDIAN_CPU)
        EmberAfStatus status;

        // set the cmd byte - this is byte 3 index 2, but since we have
        // already incremented past the 3 byte ZCL header (our index is at 3),
        // this gets written to "-1" since 3 - 1 = 2.
        emberAfPutInt32uInResp(ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID);

        // go through the message until there are no more attrID/type/data
        while (msgLen > msgIndex + 5)
        {
            attrId   = emberAfGetInt32u(message, msgIndex, msgLen);
            dataType = emberAfGetInt8u(message, msgIndex + 4, msgLen);

            dataSize = emberAfAttributeValueSize(clusterId, attrId, dataType, message + msgIndex + 5);

            // the data is sent little endian over-the-air, it needs to be
            // inserted into the table big endian for the EM250 and little
            // endian for the EZSP hosts. This means for the EM250 the data
            // needs to be reversed before sending to writeAttributes
#if (BIGENDIAN_CPU)
            if (dataSize <= msgLen - (msgIndex + 5) && dataSize <= ATTRIBUTE_LARGEST)
            {
                // strings go over the air as length byte and then in human
                // readable format. These should not be flipped.
                if (emberAfIsThisDataTypeAStringType(dataType))
                {
                    memmove(writeData, message + msgIndex + 5, dataSize);
                }
                else
                {
                    // the data is sent little endian over-the-air, it needs to be
                    // inserted into the table big endian
                    uint16_t i;
                    for (i = 0; i < dataSize; i++)
                    {
                        writeData[i] = message[msgIndex + 5 + dataSize - i - 1];
                    }
                }
#else  //(BIGENDIAN_CPU)
            if (dataSize <= msgLen - (msgIndex + 5))
            {
#endif //(BIGENDIAN_CPU)

                status = emberAfWriteAttributeExternal(cmd->apsFrame->destinationEndpoint, clusterId, attrId, clientServerMask,
                                                       cmd->mfgCode,
#if (BIGENDIAN_CPU)
                                                       writeData,
#else  //(BIGENDIAN_CPU)
                                                       &(message[msgIndex + 5]),
#endif //(BIGENDIAN_CPU)
                                                       dataType);
                emberAfAttributesPrint("WRITE: clus %2x attr %2x ", clusterId, attrId);
                if (status == EMBER_ZCL_STATUS_SUCCESS)
                {
                    numSuccess++;
                    emberAfAttributesPrintln("OK");
                }
                else
                {
                    numFailures++;
                    // write to the response buffer - status and then attrID
                    emberAfPutStatusInResp(status);
                    emberAfPutInt32uInResp(attrId);
                    emberAfAttributesPrintln("FAIL %x", status);
                }
                emberAfCoreFlush();

                // Increment past the attribute id (four bytes), the type (one byte), and
                // the data (N bytes, including the length byte for strings).
                msgIndex = static_cast<uint16_t>(msgIndex + 5 + dataSize);
            }
            else
            {
                numFailures++;
                status = EMBER_ZCL_STATUS_INVALID_VALUE;
                // write to the response buffer - status and then attrID
                emberAfPutStatusInResp(status);
                emberAfPutInt32uInResp(attrId);
                emberAfAttributesPrintln("FAIL %x", status);
                // size exceeds buffer, terminate loop
                break;
            }
        }

        // always send a response unless the cmd requested no response
        if (zclCmd == ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID)
        {
            return true;
        }

        if (numFailures == 0)
        {
            // if no failures and no success this means the packet was too short
            // print an error message but still return true as we consumed the
            // message
            if (numSuccess == 0)
            {
                emberAfAttributesPrintln("WRITE: too short");
                emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
                return true;
            }
            // if no failures and at least one success, write a success status
            // that means everything worked
            else
            {
                emberAfPutInt8uInResp(EMBER_ZCL_STATUS_SUCCESS);
            }
        }
        emberAfSendResponse();
        return true;
    }

    // the format of discover is: [start attr ID:4] [max attr IDs:1]
    // the format of the response is: [done:1] ([attrID:4] [type:1]) * N
    case ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID:
    case ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID: {
        AttributeId startingAttributeId;
        uint8_t numberAttributes;
        uint8_t * complete;

        emberAfAttributesPrintln("%p%p: clus %2x", "DISC_ATTR",
                                 (zclCmd == ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID ? "_EXT" : ""), clusterId);

        // set the cmd byte - this is byte 3 index 2, but since we have
        // already incremented past the 3 byte ZCL header (our index is at 3),
        // this gets written to "-1" since 3 - 1 = 2.
        emberAfPutInt32uInResp((zclCmd == ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID
                                    ? ZCL_DISCOVER_ATTRIBUTES_RESPONSE_COMMAND_ID
                                    : ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID));

        // get the attrId to start on and the max count
        startingAttributeId = emberAfGetInt32u(message, msgIndex, msgLen);
        numberAttributes    = emberAfGetInt8u(message, msgIndex + 4, msgLen);

        // BUGZID: EMAPPFWKV2-828, EMAPPFWKV2-1401
        if (zclCmd == ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID && numberAttributes > DISC_ATTR_RSP_MAX_ATTRIBUTES)
        {
            numberAttributes = DISC_ATTR_RSP_MAX_ATTRIBUTES;
        }
        else if (zclCmd == ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID && numberAttributes > DISC_ATTR_EXT_RSP_MAX_ATTRIBUTES)
        {
            numberAttributes = DISC_ATTR_EXT_RSP_MAX_ATTRIBUTES;
        }
        else
        {
            // MISRA requires ..else if.. to have terminating else.
        }

        // The response has a one-byte field indicating whether discovery is
        // complete.  We can't populate that field until we've finished going
        // through all the attributes, so save a placeholder, write a temporary
        // value for now (so that the offset moves forward), and write the real
        // value when we're done.
        complete = &(appResponseData[appResponseLength]);
        emberAfPutInt8uInResp(false);
        *complete = emberAfReadSequentialAttributesAddToResponse(cmd->apsFrame->destinationEndpoint, clusterId, startingAttributeId,
                                                                 clientServerMask, cmd->mfgCode, numberAttributes,
                                                                 (zclCmd == ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID));
        emberAfSendResponse();
        return true;
    }

    case ZCL_CONFIGURE_REPORTING_COMMAND_ID:
        if (emberAfConfigureReportingCommandCallback(cmd))
        {
            return true;
        }
        break;

    case ZCL_READ_REPORTING_CONFIGURATION_COMMAND_ID:
        if (emberAfReadReportingConfigurationCommandCallback(cmd))
        {
            return true;
        }
        break;

    // ([attribute id:4] [status:1] [type:0/1] [value:0/V])+
    case ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID:
        // The "timesync" command in the CLI sends a Read Attributes command for the
        // Time attribute on another device and then sets a flag.  If that flag is
        // set and a Read Attributes Response command for the time comes in, we set
        // the time to the value in the message.
        if (clusterId == ZCL_TIME_CLUSTER_ID)
        {
            if (emAfSyncingTime && !cmd->mfgSpecific && msgLen - msgIndex == 10 // attr:4 status:1 type:1 data:4
                && (emberAfGetInt32u(message, msgIndex, msgLen) == ZCL_TIME_ATTRIBUTE_ID) &&
                (emberAfGetInt8u(message, msgIndex + 4, msgLen) == EMBER_ZCL_STATUS_SUCCESS) &&
                (emberAfGetInt8u(message, msgIndex + 5, msgLen) == ZCL_UTC_ATTRIBUTE_TYPE))
            {
                // emberAfSetTime(emberAfGetInt32u(message, msgIndex + 4, msgLen));
                // emberAfDebugPrintln("time sync ok, time: %4x", emberAfGetCurrentTime());
                emAfSyncingTime = false;
            }
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
            emAfPluginSmartEnergyRegistrationReadAttributesResponseCallback(message + msgIndex, msgLen - msgIndex);
#endif // EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
#ifdef EMBER_AF_PLUGIN_WWAH_SERVER_SILABS
            emAfPluginSlWwahReadAttributesResponseCallback(clusterId, message, msgLen);
#endif
        }

#ifdef EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE
        if (clusterId == ZCL_KEEPALIVE_CLUSTER_ID && !cmd->mfgSpecific)
        {
            emAfPluginTrustCenterKeepaliveReadAttributesResponseCallback(message + msgIndex, msgLen - msgIndex);
        }
#endif // EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE

#if defined(EMBER_AF_PLUGIN_KEY_ESTABLISHMENT)
        if (clusterId == ZCL_KEY_ESTABLISHMENT_CLUSTER_ID && !cmd->mfgSpecific &&
            msgLen - msgIndex == 8 // attr:4 status:1 type:1 data:2
            && (emberAfGetInt32u(message, msgIndex, msgLen) == ZCL_KEY_ESTABLISHMENT_SUITE_CLIENT_ATTRIBUTE_ID) &&
            (emberAfGetInt8u(message, msgIndex + 4, msgLen) == EMBER_ZCL_STATUS_SUCCESS) &&
            ((emberAfGetInt8u(message, msgIndex + 5, msgLen) == ZCL_ENUM16_ATTRIBUTE_TYPE) ||
             (emberAfGetInt8u(message, msgIndex + 5, msgLen) == ZCL_BITMAP16_ATTRIBUTE_TYPE)))
        {
            uint16_t suite = emberAfGetInt16u(message, msgIndex + 6, msgLen);
            emberAfPluginKeyEstablishmentReadAttributesCallback(suite);
        }
#endif

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS)
        emberAfPluginTestHarnessReadAttributesResponseCallback(clusterId, message + msgIndex, msgLen - msgIndex);
#endif

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_CLIENT)
        emberAfPluginIasZoneClientReadAttributesResponseCallback(clusterId, message + msgIndex,
                                                                 static_cast<uint16_t>(msgLen - msgIndex));
#endif

#if defined(EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER)
        emberAfPluginSimpleMeteringClusterReadAttributesResponseCallback(clusterId, message + msgIndex,
                                                                         static_cast<uint16_t>(msgLen - msgIndex));
#endif

        return true;

    // ([status:1] [attribute id:4])+
    case ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID:

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS)
        emberAfPluginTestHarnessWriteAttributesResponseCallback(clusterId, message + msgIndex,
                                                                static_cast<uint16_t>(msgLen - msgIndex));
#endif

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_CLIENT)
        emberAfPluginIasZoneClientWriteAttributesResponseCallback(clusterId, message + msgIndex,
                                                                  static_cast<uint16_t>(msgLen - msgIndex));
#endif

        if (!emberAfWriteAttributesResponseCallback(clusterId, message + msgIndex, static_cast<uint16_t>(msgLen - msgIndex)))
        {
            emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
        }
        return true;

    // ([status:1] [direction:1] [attribute id:4])+
    case ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID:
        if (!emberAfConfigureReportingResponseCallback(clusterId, message + msgIndex, static_cast<uint16_t>(msgLen - msgIndex)))
        {
            emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
        }
        return true;

    // ([status:1] [direction:1] [attribute id:4] [type:0/1] ...
    // ... [min interval:0/2] [max interval:0/2] [reportable change:0/V] ...
    // ... [timeout:0/2])+
    case ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID:
        if (!emberAfReadReportingConfigurationResponseCallback(clusterId, message + msgIndex,
                                                               static_cast<uint16_t>(msgLen - msgIndex)))
        {
            emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
        }
        return true;

    // ([attribute id:4] [type:1] [data:V])+
    case ZCL_REPORT_ATTRIBUTES_COMMAND_ID:
        if (!emberAfReportAttributesCallback(clusterId, message + msgIndex, static_cast<uint16_t>(msgLen - msgIndex)))
        {
            emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
        }
        return true;

    // [command id:4] [status:1]
    case ZCL_DEFAULT_RESPONSE_COMMAND_ID: {
        EmberAfStatus status;
        CommandId commandId;
        commandId = emberAfGetInt32u(message, msgIndex, msgLen);
        status    = (EmberAfStatus) emberAfGetInt8u(message, msgIndex + 4, msgLen);

        emberAfClusterDefaultResponseWithMfgCodeCallback(cmd->apsFrame->destinationEndpoint, clusterId, commandId, status,
                                                         clientServerMask, cmd->mfgCode);
        emberAfDefaultResponseCallback(clusterId, commandId, status);
        return true;
    }

    // [discovery complete:1] ([attribute id:4] [type:1])*
    case ZCL_DISCOVER_ATTRIBUTES_RESPONSE_COMMAND_ID:
    case ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID: {
        bool discoveryComplete = emberAfGetInt8u(message, msgIndex, msgLen);
        msgIndex++;
        if (!emberAfDiscoverAttributesResponseCallback(clusterId, discoveryComplete, message + msgIndex,
                                                       static_cast<uint16_t>(msgLen - msgIndex),
                                                       (zclCmd == ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID)))
        {
            emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
        }
        return true;
    }

#ifdef EMBER_AF_SUPPORT_COMMAND_DISCOVERY
    // Command discovery takes a bit of flash because we need to add structs
    // for commands into the generated hader. Hence it's all configurable.
    case ZCL_DISCOVER_COMMANDS_RECEIVED_COMMAND_ID:
    case ZCL_DISCOVER_COMMANDS_GENERATED_COMMAND_ID: {
        uint8_t startCommandIdentifier    = emberAfGetInt8u(message, msgIndex, msgLen);
        uint8_t maximumCommandIdentifiers = emberAfGetInt8u(message, msgIndex + 1, msgLen);
        uint16_t savedIndex;
        bool flag;

        // Ok. This is the command that matters.
        if (zclCmd == ZCL_DISCOVER_COMMANDS_RECEIVED_COMMAND_ID)
        {
            emberAfPutInt32uInResp(ZCL_DISCOVER_COMMANDS_RECEIVED_RESPONSE_COMMAND_ID);
            flag = false;
        }
        else
        {
            emberAfPutInt32uInResp(ZCL_DISCOVER_COMMANDS_GENERATED_RESPONSE_COMMAND_ID);
            flag = true;
        }
        savedIndex                  = appResponseLength;
        flag                        = emberAfExtractCommandIds(flag, cmd, clusterId, appResponseData + appResponseLength + 1,
                                        static_cast<uint16_t>(EMBER_AF_RESPONSE_BUFFER_LEN - appResponseLength - 1),
                                        &appResponseLength, startCommandIdentifier, maximumCommandIdentifiers);
        appResponseData[savedIndex] = (flag ? 1 : 0);
        appResponseLength++;
        emberAfSendResponse();
        return true;
    }
    case ZCL_DISCOVER_COMMANDS_RECEIVED_RESPONSE_COMMAND_ID: {
        bool discoveryComplete = emberAfGetInt8u(message, msgIndex, msgLen);
        msgIndex++;
        if (msgIndex <= msgLen)
        {
            printDiscoverCommandsResponse(false, // is ZCL command generated?
                                          clusterId, discoveryComplete, message + msgIndex,
                                          static_cast<uint16_t>(msgLen - msgIndex));
            if (!emberAfDiscoverCommandsReceivedResponseCallback(clusterId, cmd->mfgCode, discoveryComplete, message + msgIndex,
                                                                 static_cast<uint16_t>(msgLen - msgIndex)))
            {
                emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    case ZCL_DISCOVER_COMMANDS_GENERATED_RESPONSE_COMMAND_ID: {
        bool discoveryComplete = emberAfGetInt8u(message, msgIndex, msgLen);
        msgIndex++;
        if (msgIndex <= msgLen)
        {
            printDiscoverCommandsResponse(true, // is ZCL command generated?
                                          clusterId, discoveryComplete, message + msgIndex,
                                          static_cast<uint16_t>(msgLen - msgIndex));
            if (!emberAfDiscoverCommandsGeneratedResponseCallback(clusterId, cmd->mfgCode, discoveryComplete, message + msgIndex,
                                                                  static_cast<uint16_t>(msgLen - msgIndex)))
            {
                emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

#endif
    default:
        // MISRA requires default case.
        break;
    }

kickout:
    emberAfSendDefaultResponse(
        cmd, (cmd->mfgSpecific ? EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND : EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND));
    return true;
}
