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
 * @brief Routines for the Reporting plugin, which
 *sends asynchronous reports when a ZCL attribute's
 *value has changed.
 *******************************************************************************
 ******************************************************************************/

#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/reporting/reporting.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/binding-table.h>
#include <app/util/common.h>
#include <support/SafeInt.h>
#include <system/SystemLayer.h>

using namespace chip;

// TODO: Need to figure out what needs to happen wrt HAL tokens here, but for
// now define ESZP_HOST to disable it.  See
// https://github.com/project-chip/connectedhomeip/issues/3275
#define EZSP_HOST

#ifdef ATTRIBUTE_LARGEST
#define READ_DATA_SIZE ATTRIBUTE_LARGEST
#else
#define READ_DATA_SIZE 8 // max size if attributes aren't present
#endif

#define NULL_INDEX 0xFF

static void conditionallySendReport(EndpointId endpoint, ClusterId clusterId);
static void scheduleTick(void);
static void removeConfiguration(uint8_t index);
static void removeConfigurationAndScheduleTick(uint8_t index);
static EmberAfStatus configureReceivedAttribute(const EmberAfClusterCommand * cmd, AttributeId attributeId, uint8_t mask,
                                                uint16_t timeout);
static void putReportableChangeInResp(const EmberAfPluginReportingEntry * entry, EmberAfAttributeType dataType);
static void retrySendReport(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t msgLen,
                            uint8_t * message, EmberStatus status);
static uint32_t computeStringHash(uint8_t * data, uint8_t length);

EmberEventControl emberAfPluginReportingTickEventControl;

EmAfPluginReportVolatileData emAfPluginReportVolatileData[REPORT_TABLE_SIZE];

/** @brief Configured
 *
 * This callback is called by the Reporting plugin whenever a reporting entry
 * is configured, including when entries are deleted or updated. The
 * application can use this callback for scheduling readings or measurements
 * based on the minimum and maximum reporting interval for the entry. The
 * application should return EMBER_ZCL_STATUS_SUCCESS if it can support the
 * configuration or an error status otherwise. Note: attribute reporting is
 * required for many clusters and attributes, so rejecting a reporting
 * configuration may violate ZigBee specifications.
 *
 * @param entry   Ver.: always
 */
EmberAfStatus emberAfPluginReportingConfiguredCallback(const EmberAfPluginReportingEntry * entry)
{
    return EMBER_ZCL_STATUS_SUCCESS;
}

static void retrySendReport(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t msgLen,
                            uint8_t * message, EmberStatus status)
{
    // Retry once, and do so by unicasting without a pointer to this callback
    if (status != EMBER_SUCCESS)
    {
        emberAfSendUnicast(destination, apsFrame, msgLen, message);
    }
}

// Implementation based on public domain Fowler/Noll/Vo FNV-1a hash function:
// http://isthe.com/chongo/tech/comp/fnv/
// https://tools.ietf.org/html/draft-eastlake-fnv-14
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//
// Used to save and compare hashes of CHAR and OCTET string values in order to detect
// reportable changes. The strings themselves are longer than the storage size.
#define FNV1_OFFSET_BASIS (2166136261)
#define FNV1_PRIME (16777619)
static uint32_t computeStringHash(uint8_t * data, uint8_t length)
{
    // FNV-1a, 32-bit hash
    uint32_t hash = FNV1_OFFSET_BASIS;
    for (int i = 0; i < length; ++i)
    {
        hash ^= data[i];
        hash *= FNV1_PRIME; // Or, hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
    }
    return hash;
}

#ifdef EZSP_HOST
#if REPORT_TABLE_SIZE != 0
static EmberAfPluginReportingEntry table[REPORT_TABLE_SIZE];
#endif
void emAfPluginReportingGetEntry(uint8_t index, EmberAfPluginReportingEntry * result)
{
#if REPORT_TABLE_SIZE != 0
    memmove(result, &table[index], sizeof(EmberAfPluginReportingEntry));
#endif
}
void emAfPluginReportingSetEntry(uint8_t index, EmberAfPluginReportingEntry * value)
{
#if REPORT_TABLE_SIZE != 0
    memmove(&table[index], value, sizeof(EmberAfPluginReportingEntry));
#endif
}
#else
void emAfPluginReportingGetEntry(uint8_t index, EmberAfPluginReportingEntry * result)
{
    // TODO: Once https://github.com/project-chip/connectedhomeip/issues/2470 is
    // fixed this manual marking of the entry as unused can probably be removed.
    result->endpoint = EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID;
    halCommonGetIndexedToken(result, TOKEN_REPORT_TABLE, index);
}
void emAfPluginReportingSetEntry(uint8_t index, EmberAfPluginReportingEntry * value)
{
    halCommonSetIndexedToken(TOKEN_REPORT_TABLE, index, value);
}
#endif

void emberAfPluginReportingStackStatusCallback(EmberStatus status)
{
    if (status == EMBER_NETWORK_UP)
    {
        // Load default reporting configurations
        emberAfPluginReportingLoadReportingConfigDefaults();

        scheduleTick();
    }
}

void emberAfPluginReportingInitCallback(void)
{
    // On device initialization, any attributes that have been set up to report
    // should generate an attribute report.
    for (uint8_t i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        EmberAfPluginReportingEntry entry;
        emAfPluginReportingGetEntry(i, &entry);
        if (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID &&
            entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        {
            emAfPluginReportVolatileData[i].reportableChange = true;
        }
    }

    scheduleTick();
}

void emberAfPluginReportingTickEventHandler(void)
{
    EmberApsFrame * apsFrame = NULL;
    EmberAfStatus status;
    EmberAfAttributeType dataType;
    uint16_t manufacturerCode = 0;
    uint8_t readData[READ_DATA_SIZE];
    uint8_t i;
    uint16_t dataSize;
    bool clientToServer = false;
    EmberBindingTableEntry bindingEntry;
    // reportSize needs to be able to fit a sum of dataSize and some other stuff
    // without overflowing.
    uint32_t reportSize;
    uint8_t index;
    uint16_t currentPayloadMaxLength = 0, smallestPayloadMaxLength = 0;

    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        EmberAfPluginReportingEntry entry;
        // Not initializing entry.mask causes errors even if wrapped with GCC diagnostic ignored
        entry.mask = CLUSTER_MASK_SERVER;
        uint32_t elapsedMs;
        emAfPluginReportingGetEntry(i, &entry);
        // We will only send reports for active reported attributes and only if a
        // reportable change has occurred and the minimum interval has elapsed or
        // if the maximum interval is set and has elapsed.
        elapsedMs =
            elapsedTimeInt32u(emAfPluginReportVolatileData[i].lastReportTimeMs, chip::System::Clock::GetMonotonicMilliseconds());
        if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID ||
            entry.direction != EMBER_ZCL_REPORTING_DIRECTION_REPORTED ||
            (elapsedMs < entry.data.reported.minInterval * MILLISECOND_TICKS_PER_SECOND) ||
            (!emAfPluginReportVolatileData[i].reportableChange &&
             (entry.data.reported.maxInterval == 0 ||
              (elapsedMs < (entry.data.reported.maxInterval * MILLISECOND_TICKS_PER_SECOND)))))
        {
            continue;
        }

        status = emAfReadAttribute(entry.endpoint, entry.clusterId, entry.attributeId, entry.mask, entry.manufacturerCode,
                                   (uint8_t *) &readData, READ_DATA_SIZE, &dataType);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfReportingPrintln("ERR: reading cluster 0x%2x attribute 0x%2x: 0x%x", entry.clusterId, entry.attributeId, status);
            continue;
        }
        if (emberAfIsLongStringAttributeType(dataType))
        {
            // LONG string types are rarely used and even more rarely (never?)
            // reported; ignore and leave ensuing handling of other types unchanged.
            emberAfReportingPrintln("ERR: reporting of LONG string attribute type not supported: cluster 0x%2x attribute 0x%2x",
                                    entry.clusterId, entry.attributeId);
            continue;
        }

        // find size of current report
        dataSize   = emberAfAttributeValueSize(entry.clusterId, entry.attributeId, dataType, readData);
        reportSize = static_cast<uint32_t>(sizeof(entry.attributeId) + sizeof(dataType) + dataSize);

        // If we have already started a report for a different attribute or
        // destination, or if the current entry is too big for current report, send it and create a new one.
        if (apsFrame != NULL &&
            (!(entry.endpoint == apsFrame->sourceEndpoint && entry.clusterId == apsFrame->clusterId &&
               emberAfClusterIsClient(&entry) == clientToServer && entry.manufacturerCode == manufacturerCode) ||
             (appResponseLength + reportSize > smallestPayloadMaxLength)))
        {
            if (appResponseLength + reportSize > smallestPayloadMaxLength)
            {
                emberAfReportingPrintln("Reporting Entry Full - creating new report");
            }
            conditionallySendReport(apsFrame->sourceEndpoint, apsFrame->clusterId);
            apsFrame = NULL;
        }

        // If we haven't made the message header, make it.
        if (apsFrame == NULL)
        {
            apsFrame       = emberAfGetCommandApsFrame();
            clientToServer = emberAfClusterIsClient(&entry);
            // The manufacturer-specfic version of the fill API only creates a
            // manufacturer-specfic command if the manufacturer code is set.  For
            // non-manufacturer-specfic reports, the manufacturer code is unset, so
            // we can get away with using this API for both cases.
            emberAfFillExternalManufacturerSpecificBuffer(
                (clientToServer
                     ? (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS)
                     : (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS)),
                entry.clusterId, entry.manufacturerCode, ZCL_REPORT_ATTRIBUTES_COMMAND_ID, "");
            apsFrame->sourceEndpoint = entry.endpoint;
            apsFrame->options        = EMBER_AF_DEFAULT_APS_OPTIONS;
            manufacturerCode         = entry.manufacturerCode;

            // EMAPPFWKV2-1327: Reporting plugin does not account for reporting too many attributes
            //                  in the same ZCL:ReportAttributes message

            // find smallest maximum payload that the destination can receive for this cluster and source endpoint
            smallestPayloadMaxLength = MAX_INT8U_VALUE;
            for (index = 0; index < EMBER_BINDING_TABLE_SIZE; index++)
            {
                status = (EmberAfStatus) emberGetBinding(index, &bindingEntry);
                if (status == (EmberAfStatus) EMBER_SUCCESS && bindingEntry.local == entry.endpoint &&
                    bindingEntry.clusterId == entry.clusterId)
                {
                    currentPayloadMaxLength = EMBER_AF_RESPONSE_BUFFER_LEN;
                    if (currentPayloadMaxLength < smallestPayloadMaxLength)
                    {
                        smallestPayloadMaxLength = currentPayloadMaxLength;
                    }
                }
            }
        }

        // Payload is [attribute id:4] [type:1] [data:N].
        emberAfPutInt32uInResp(entry.attributeId);
        emberAfPutInt8uInResp(dataType);

#if (BIGENDIAN_CPU)
        if (isThisDataTypeSentLittleEndianOTA(dataType))
        {
            uint8_t i;
            for (i = 0; i < dataSize; i++)
            {
                emberAfPutInt8uInResp(readData[dataSize - i - 1]);
            }
        }
        else
        {
            emberAfPutBlockInResp(readData, dataSize);
        }
#else
        emberAfPutBlockInResp(readData, dataSize);
#endif

        // Store the last reported time and value so that we can track intervals
        // and changes.  We only track changes for data types that are small enough
        // for us to compare. For CHAR and OCTET strings, we substitute a 32-bit hash.
        emAfPluginReportVolatileData[i].reportableChange = false;
        emAfPluginReportVolatileData[i].lastReportTimeMs = static_cast<uint32_t>(chip::System::Clock::GetMonotonicMilliseconds());
        uint32_t stringHash                              = 0;
        uint8_t * copyData                               = readData;
        uint16_t copySize                                = dataSize;
        if (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE)
        {
            // dataSize was set above to count the string's length byte, in addition to string length.
            // Compute hash on string value only.  Note that string length fits
            // in one byte, so dataSize can't be larger than 256 right now.
            stringHash = computeStringHash(readData + 1, static_cast<uint8_t>(dataSize - 1));
            copyData   = (uint8_t *) &stringHash;
            copySize   = sizeof(stringHash);
        }
        if (copySize <= sizeof(emAfPluginReportVolatileData[i].lastReportValue))
        {
            emAfPluginReportVolatileData[i].lastReportValue = 0;
#if (BIGENDIAN_CPU)
            memmove(((uint8_t *) &emAfPluginReportVolatileData[i].lastReportValue +
                     sizeof(emAfPluginReportVolatileData[i].lastReportValue) - copySize),
                    copyData, copySize);
#else
            memmove(&emAfPluginReportVolatileData[i].lastReportValue, copyData, copySize);
#endif
        }
    }

    if (apsFrame != NULL)
    {
        conditionallySendReport(apsFrame->sourceEndpoint, apsFrame->clusterId);
    }
    scheduleTick();
}

static void conditionallySendReport(EndpointId endpoint, ClusterId clusterId)
{
    EmberStatus status;
    if (emberAfIsDeviceEnabled(endpoint) || clusterId == ZCL_IDENTIFY_CLUSTER_ID)
    {
        status = emberAfSendCommandUnicastToBindingsWithCallback(&retrySendReport);

        // If the callback table is full, attempt to send the message with no
        // callback.  Note that this could lead to a message failing to transmit
        // with no notification to the user for any number of reasons (ex: hitting
        // the message queue limit), but is better than not sending the message at
        // all because the system hits its callback queue limit.
        if (status == EMBER_TABLE_FULL)
        {
            emberAfSendCommandUnicastToBindings();
        }

#ifdef EMBER_AF_PLUGIN_REPORTING_ENABLE_GROUP_BOUND_REPORTS
        emberAfSendCommandMulticastToBindings();
#endif // EMBER_AF_PLUGIN_REPORTING_ENABLE_GROUP_BOUND_REPORTS
    }
}

bool emberAfConfigureReportingCommandCallback(const EmberAfClusterCommand * cmd)
{
    EmberStatus sendStatus;
    uint16_t bufIndex = cmd->payloadStartIndex;
    uint8_t frameControl, mask;
    bool failures = false;

    emberAfReportingPrint("%p: ", "CFG_RPT");
    emberAfReportingDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(cmd->apsFrame->clusterId, cmd->mfgCode));
    emberAfReportingPrintln("");
    emberAfReportingFlush();

    if (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)
    {
        frameControl = (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
        mask         = CLUSTER_MASK_SERVER;
    }
    else
    {
        frameControl = (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
        mask         = CLUSTER_MASK_CLIENT;
    }

    // The manufacturer-specfic version of the fill API only creates a
    // manufacturer-specfic command if the manufacturer code is set.  For non-
    // manufacturer-specfic reports, the manufacturer code is unset, so we can
    // get away with using this API for both cases.
    emberAfFillExternalManufacturerSpecificBuffer(frameControl, cmd->apsFrame->clusterId, cmd->mfgCode,
                                                  ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID, "");

    // Each record in the command has at least a one-byte direction and a four-
    // byte attribute id.  Additional fields are present depending on the value
    // of the direction field.
    while (bufIndex + 5 < cmd->bufLen)
    {
        AttributeId attributeId;
        EmberAfReportingDirection direction;
        EmberAfStatus status;

        direction = (EmberAfReportingDirection) emberAfGetInt8u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex++;
        attributeId = (AttributeId) emberAfGetInt32u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex    = static_cast<uint16_t>(bufIndex + 4);

        emberAfReportingPrintln(" - direction:%x, attr:%2x", direction, attributeId);

        switch (direction)
        {
        case EMBER_ZCL_REPORTING_DIRECTION_REPORTED: {
            EmberAfAttributeMetadata * metadata;
            EmberAfAttributeType dataType;
            uint16_t minInterval, maxInterval;
            uint32_t reportableChange = 0;
            EmberAfPluginReportingEntry newEntry;

            dataType = (EmberAfAttributeType) emberAfGetInt8u(cmd->buffer, bufIndex, cmd->bufLen);
            bufIndex++;
            minInterval = emberAfGetInt16u(cmd->buffer, bufIndex, cmd->bufLen);
            bufIndex    = static_cast<uint16_t>(bufIndex + 2);
            maxInterval = emberAfGetInt16u(cmd->buffer, bufIndex, cmd->bufLen);
            bufIndex    = static_cast<uint16_t>(bufIndex + 2);

            emberAfReportingPrintln("   type:%x, min:%2x, max:%2x", dataType, minInterval, maxInterval);
            emberAfReportingFlush();

            if (emberAfGetAttributeAnalogOrDiscreteType(dataType) == EMBER_AF_DATA_TYPE_ANALOG)
            {
                uint8_t dataSize       = emberAfGetDataSize(dataType);
                uint64_t currentChange = emberAfGetInt(cmd->buffer, bufIndex, cmd->bufLen, dataSize);
                if (chip::CanCastTo<uint32_t>(currentChange))
                {
                    reportableChange = static_cast<uint32_t>(emberAfGetInt(cmd->buffer, bufIndex, cmd->bufLen, dataSize));
                }
                else
                {
                    status = EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
                    break;
                }

                emberAfReportingPrint("   change:");
                emberAfReportingPrintBuffer(cmd->buffer + bufIndex, dataSize, false);
                emberAfReportingPrintln("");

                bufIndex = static_cast<uint16_t>(bufIndex + dataSize);
            }

            // emberAfPluginReportingConfigureReportedAttribute handles non-
            // existent attributes, but does not verify the attribute data type, so
            // we need to check it here.
            metadata = emberAfLocateAttributeMetadata(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, attributeId,
                                                      mask, cmd->mfgCode);
            if (metadata != NULL && metadata->attributeType != dataType)
            {
                status = EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
            }
            else
            {
                // Add a reporting entry for a reported attribute.  The reports will
                // be sent from us to the source of the Configure Reporting command.
                newEntry.endpoint                       = cmd->apsFrame->destinationEndpoint;
                newEntry.clusterId                      = cmd->apsFrame->clusterId;
                newEntry.attributeId                    = attributeId;
                newEntry.mask                           = mask;
                newEntry.manufacturerCode               = cmd->mfgCode;
                newEntry.data.reported.minInterval      = minInterval;
                newEntry.data.reported.maxInterval      = maxInterval;
                newEntry.data.reported.reportableChange = reportableChange;
                status                                  = emberAfPluginReportingConfigureReportedAttribute(&newEntry);
            }
            break;
        }
        case EMBER_ZCL_REPORTING_DIRECTION_RECEIVED: {
            uint16_t timeout = emberAfGetInt16u(cmd->buffer, bufIndex, cmd->bufLen);
            bufIndex         = static_cast<uint16_t>(bufIndex + 2);

            emberAfReportingPrintln("   timeout:%2x", timeout);

            // Add a reporting entry from a received attribute.  The reports
            // will be sent to us from the source of the Configure Reporting
            // command.
            status = configureReceivedAttribute(cmd, attributeId, mask, timeout);
            break;
        }
        default:
            // This will abort the processing (see below).
            status = EMBER_ZCL_STATUS_INVALID_FIELD;
            break;
        }

        // If a report cannot be configured, the status, direction, and
        // attribute are added to the response.  If the failure was due to an
        // invalid field, we have to abort after this record because we don't
        // know how to interpret the rest of the data in the request.
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfPutInt8uInResp(status);
            emberAfPutInt8uInResp(direction);
            emberAfPutInt32uInResp(attributeId);
            failures = true;
            if (status == EMBER_ZCL_STATUS_INVALID_FIELD)
            {
                break;
            }
        }
    }

    // We just respond with SUCCESS if we made it through without failures.
    if (!failures)
    {
        emberAfPutInt8uInResp(EMBER_ZCL_STATUS_SUCCESS);
    }

    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfReportingPrintln("Reporting: failed to send %s response: 0x%x", "configure_reporting", sendStatus);
    }
    return true;
}

bool emberAfReadReportingConfigurationCommandCallback(const EmberAfClusterCommand * cmd)
{
    EmberStatus sendStatus;
    uint16_t bufIndex = cmd->payloadStartIndex;
    uint8_t frameControl, mask;

    emberAfReportingPrint("%p: ", "READ_RPT_CFG");
    emberAfReportingDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(cmd->apsFrame->clusterId, cmd->mfgCode));
    emberAfReportingPrintln("");
    emberAfReportingFlush();

    if (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)
    {
        frameControl = (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
        mask         = CLUSTER_MASK_SERVER;
    }
    else
    {
        frameControl = (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
        mask         = CLUSTER_MASK_CLIENT;
    }

    // The manufacturer-specfic version of the fill API only creates a
    // manufacturer-specfic command if the manufacturer code is set.  For non-
    // manufacturer-specfic reports, the manufacturer code is unset, so we can
    // get away with using this API for both cases.
    emberAfFillExternalManufacturerSpecificBuffer(frameControl, cmd->apsFrame->clusterId, cmd->mfgCode,
                                                  ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID, "");

    // Each record in the command has a one-byte direction and a four-byte
    // attribute id.
    while (bufIndex + 5 <= cmd->bufLen)
    {
        AttributeId attributeId;
        EmberAfAttributeMetadata * metadata = NULL;
        EmberAfPluginReportingEntry entry;
        EmberAfReportingDirection direction;
        uint8_t i;
        bool found = false;

        direction = (EmberAfReportingDirection) emberAfGetInt8u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex++;
        attributeId = (AttributeId) emberAfGetInt32u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex    = static_cast<uint16_t>(bufIndex + 4);

        switch (direction)
        {
        case EMBER_ZCL_REPORTING_DIRECTION_REPORTED:
        case EMBER_ZCL_REPORTING_DIRECTION_RECEIVED:
            metadata = emberAfLocateAttributeMetadata(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, attributeId,
                                                      mask, cmd->mfgCode);
            if (metadata == NULL)
            {
                emberAfPutInt8uInResp(EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);
                emberAfPutInt8uInResp(direction);
                emberAfPutInt32uInResp(attributeId);
                continue;
            }
            break;
        default:
            emberAfPutInt8uInResp(EMBER_ZCL_STATUS_INVALID_FIELD);
            emberAfPutInt8uInResp(direction);
            emberAfPutInt32uInResp(attributeId);
            continue;
        }

        // 075123r03 seems to suggest that SUCCESS is returned even if reporting
        // isn't configured for the requested attribute.  The individual fields
        // of the response for this attribute get populated with defaults.
        // CCB 1854 removes the ambiguity and requires NOT_FOUND to be returned in
        // the status field and all fields except direction and attribute identifier
        // to be omitted if there is no report configuration found.
        for (i = 0; i < REPORT_TABLE_SIZE; i++)
        {
            emAfPluginReportingGetEntry(i, &entry);
            if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
            {
                continue;
            }
            if (entry.direction == direction && entry.endpoint == cmd->apsFrame->destinationEndpoint &&
                entry.clusterId == cmd->apsFrame->clusterId && entry.attributeId == attributeId && entry.mask == mask &&
                entry.manufacturerCode == cmd->mfgCode &&
                (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED ||
                 (entry.data.received.source == cmd->SourceNodeId() &&
                  entry.data.received.endpoint == cmd->apsFrame->sourceEndpoint)))
            {
                found = true;
                break;
            }
        }
        // Attribute supported, reportable, no report configuration was found.
        if (found == false)
        {
            emberAfPutInt8uInResp(EMBER_ZCL_STATUS_NOT_FOUND);
            emberAfPutInt8uInResp(direction);
            emberAfPutInt32uInResp(attributeId);
            continue;
        }
        // Attribute supported, reportable, report configuration was found.
        emberAfPutInt8uInResp(EMBER_ZCL_STATUS_SUCCESS);
        emberAfPutInt8uInResp(direction);
        emberAfPutInt32uInResp(attributeId);
        switch (direction)
        {
        case EMBER_ZCL_REPORTING_DIRECTION_REPORTED:
            if (metadata != NULL)
            {
                emberAfPutInt8uInResp(metadata->attributeType);
                emberAfPutInt16uInResp(entry.data.reported.minInterval);
                emberAfPutInt16uInResp(entry.data.reported.maxInterval);
                if (emberAfGetAttributeAnalogOrDiscreteType(metadata->attributeType) == EMBER_AF_DATA_TYPE_ANALOG)
                {
                    putReportableChangeInResp(&entry, metadata->attributeType);
                }
            }
            break;
        case EMBER_ZCL_REPORTING_DIRECTION_RECEIVED:
            emberAfPutInt16uInResp(entry.data.received.timeout);
            break;
        }
    }

    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfReportingPrintln("Reporting: failed to send %s response: 0x%x", "read_reporting_configuration", sendStatus);
    }
    return true;
}

EmberStatus emberAfClearReportTableCallback(void)
{
    uint8_t i;
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        removeConfiguration(i);
    }
    emberEventControlSetInactive(&emberAfPluginReportingTickEventControl);
    return EMBER_SUCCESS;
}

EmberStatus emAfPluginReportingRemoveEntry(uint8_t index)
{
    EmberStatus status = EMBER_INDEX_OUT_OF_RANGE;
    if (index < REPORT_TABLE_SIZE)
    {
        removeConfigurationAndScheduleTick(index);
        status = EMBER_SUCCESS;
    }
    return status;
}

void emberAfReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                             uint16_t manufacturerCode, EmberAfAttributeType type, uint8_t * data)
{
    uint8_t i;
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        EmberAfPluginReportingEntry entry;
        emAfPluginReportingGetEntry(i, &entry);
        if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        {
            continue;
        }
        if (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED && entry.endpoint == endpoint &&
            entry.clusterId == clusterId && entry.attributeId == attributeId && entry.mask == mask &&
            entry.manufacturerCode == manufacturerCode)
        {
            // For CHAR and OCTET strings, the string value may be too long to fit into the
            // lastReportValue field (EmberAfDifferenceType), so instead we save the string's
            // hash, and detect changes in string value based on unequal hash.
            uint32_t stringHash = 0;
            uint8_t dataSize    = emberAfGetDataSize(type);
            uint8_t * dataRef   = data;
            if (type == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || type == ZCL_CHAR_STRING_ATTRIBUTE_TYPE)
            {
                stringHash = computeStringHash(data + 1, emberAfStringLength(data));
                dataRef    = (uint8_t *) &stringHash;
                dataSize   = sizeof(stringHash);
            }
            // If we are reporting this particular attribute, we only care whether
            // the new value meets the reportable change criteria.  If it does, we
            // mark the entry as ready to report and reschedule the tick.  Whether
            // the tick will be scheduled for immediate or delayed execution depends
            // on the minimum reporting interval.  This is handled in the scheduler.
            EmberAfDifferenceType difference =
                emberAfGetDifference(dataRef, emAfPluginReportVolatileData[i].lastReportValue, dataSize);
            uint8_t analogOrDiscrete = emberAfGetAttributeAnalogOrDiscreteType(type);
            if ((analogOrDiscrete == EMBER_AF_DATA_TYPE_DISCRETE && difference != 0) ||
                (analogOrDiscrete == EMBER_AF_DATA_TYPE_ANALOG && entry.data.reported.reportableChange <= difference))
            {
                emAfPluginReportVolatileData[i].reportableChange = true;
                scheduleTick();
            }
            break;
        }
    }
}

bool emAfPluginReportingDoEntriesMatch(const EmberAfPluginReportingEntry * const entry1,
                                       const EmberAfPluginReportingEntry * const entry2)
{
    // Verify that the reporting parameters of both entries match.
    // If the entries are for EMBER_ZCL_REPORTING_DIRECTION_REPORTED, the
    // reporting configurations do not need to match.  If the direction is
    // EMBER_ZCL_REPORTING_DIRECTION_RECEIVED, then the source and destination
    // endpoints need to match.
    if ((entry1->endpoint == entry2->endpoint) && (entry1->clusterId == entry2->clusterId) &&
        (entry1->attributeId == entry2->attributeId) && (entry1->mask == entry2->mask) &&
        (entry1->manufacturerCode == entry2->manufacturerCode) && (entry1->direction == entry2->direction) &&
        ((entry1->direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED) ||
         ((entry1->data.received.source == entry2->data.received.source) &&
          (entry1->data.received.endpoint == entry2->data.received.endpoint))))
    {
        return true;
    }
    return false;
}

uint8_t emAfPluginReportingAddEntry(EmberAfPluginReportingEntry * newEntry)
{
    uint8_t i;
    EmberAfPluginReportingEntry oldEntry;

    // If an entry already exists, or exists but with different parameters,
    // overwrite it with the new entry to prevent pollution of the report table
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportingGetEntry(i, &oldEntry);
        if (emAfPluginReportingDoEntriesMatch(&oldEntry, newEntry))
        {
            emAfPluginReportingSetEntry(i, newEntry);
            return i;
        }
    }

    // If no pre-existing entries were found, copy the new entry into the lowest
    // indexed free spot in the reporting table
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportingGetEntry(i, &oldEntry);
        if (oldEntry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        {
            emAfPluginReportingSetEntry(i, newEntry);
            return i;
        }
    }

    // If no free spots were found, return the failure indicator
    return 0xFF;
}

static void scheduleTick(void)
{
    uint32_t delayMs = MAX_INT32U_VALUE;
    uint8_t i;
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        EmberAfPluginReportingEntry entry;
        emAfPluginReportingGetEntry(i, &entry);

        if (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID &&
            entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        {
            uint32_t minIntervalMs = (entry.data.reported.minInterval * MILLISECOND_TICKS_PER_SECOND);
            uint32_t maxIntervalMs = (entry.data.reported.maxInterval * MILLISECOND_TICKS_PER_SECOND);
            uint32_t elapsedMs     = elapsedTimeInt32u(emAfPluginReportVolatileData[i].lastReportTimeMs,
                                                   chip::System::Clock::GetMonotonicMilliseconds());
            uint32_t remainingMs   = MAX_INT32U_VALUE;
            if (emAfPluginReportVolatileData[i].reportableChange)
            {
                remainingMs = (minIntervalMs < elapsedMs ? 0 : minIntervalMs - elapsedMs);
            }
            else if (maxIntervalMs)
            {
                remainingMs = (maxIntervalMs < elapsedMs ? 0 : maxIntervalMs - elapsedMs);
            }
            if (remainingMs < delayMs)
            {
                delayMs = remainingMs;
            }
        }
    }
    if (delayMs != MAX_INT32U_VALUE)
    {
        emberAfDebugPrintln("sched report event in %d ms", delayMs);
        emberEventControlSetDelayMS(&emberAfPluginReportingTickEventControl, delayMs);
    }
    else
    {
        emberAfDebugPrintln("deactivate report event");
        emberEventControlSetInactive(&emberAfPluginReportingTickEventControl);
    }
}

static void removeConfiguration(uint8_t index)
{
    EmberAfPluginReportingEntry entry;
    emAfPluginReportingGetEntry(index, &entry);
    entry.endpoint = EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID;
    emAfPluginReportingSetEntry(index, &entry);
    emberAfPluginReportingConfiguredCallback(&entry);
}

static void removeConfigurationAndScheduleTick(uint8_t index)
{
    removeConfiguration(index);
    scheduleTick();
}

EmberAfStatus emberAfPluginReportingConfigureReportedAttribute(const EmberAfPluginReportingEntry * newEntry)
{
    EmberAfAttributeMetadata * metadata;
    EmberAfPluginReportingEntry entry;
    EmberAfStatus status;
    uint8_t i, index = NULL_INDEX;
    bool initialize = true;

    // Verify that we support the attribute and that the data type matches.
    metadata = emberAfLocateAttributeMetadata(newEntry->endpoint, newEntry->clusterId, newEntry->attributeId, newEntry->mask,
                                              newEntry->manufacturerCode);
    if (metadata == NULL)
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    // Verify the minimum and maximum intervals make sense.
    if (newEntry->data.reported.maxInterval != 0 && (newEntry->data.reported.maxInterval < newEntry->data.reported.minInterval))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    // Check the table for an entry that matches this request and also watch for
    // empty slots along the way.  If a report exists, it will be overwritten
    // with the new configuration.  Otherwise, a new entry will be created and
    // initialized.
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportingGetEntry(i, &entry);
        if (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED && entry.endpoint == newEntry->endpoint &&
            entry.clusterId == newEntry->clusterId && entry.attributeId == newEntry->attributeId && entry.mask == newEntry->mask &&
            entry.manufacturerCode == newEntry->manufacturerCode)
        {
            initialize = false;
            index      = i;
            break;
        }
        else if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID && index == NULL_INDEX)
        {
            index = i;
        }
    }

    // If the maximum reporting interval is 0xFFFF, the device shall not issue
    // reports for the attribute and the configuration information for that
    // attribute need not be maintained.
    if (newEntry->data.reported.maxInterval == 0xFFFF)
    {
        if (!initialize)
        {
            removeConfigurationAndScheduleTick(index);
        }
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    // ZCL v6 Section 2.5.7.1.6 Maximum Reporting Interval Field
    // If this value is set to 0x0000 and the minimum reporting interval field
    // equals 0xffff, then the device SHALL revert back to its default reporting
    // configuration. The reportable change field, if present, SHALL be set to
    // zero.
    // Verify special condition to reset the reporting configuration to defaults
    // if the minimum == 0xFFFF and maximum == 0x0000
    bool reset = false;
    if ((newEntry->data.reported.maxInterval == 0x0000) && (newEntry->data.reported.minInterval == 0xFFFF))
    {
        // Get the configuration from the default configuration table for this
        memmove(&entry, newEntry, sizeof(EmberAfPluginReportingEntry));
        if (emberAfPluginReportingGetReportingConfigDefaults(&entry))
        {
            // Then it must be initialise with the default config - explicity
            initialize = true;
            reset      = true;
        }
    }

    if (index == NULL_INDEX)
    {
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    else if (initialize)
    {
        entry.direction        = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
        entry.endpoint         = newEntry->endpoint;
        entry.clusterId        = newEntry->clusterId;
        entry.attributeId      = newEntry->attributeId;
        entry.mask             = newEntry->mask;
        entry.manufacturerCode = newEntry->manufacturerCode;
        if (index < REPORT_TABLE_SIZE)
        {
            emAfPluginReportVolatileData[index].lastReportTimeMs =
                static_cast<uint32_t>(chip::System::Clock::GetMonotonicMilliseconds());
            emAfPluginReportVolatileData[index].lastReportValue = 0;
        }
    }

    // For new or updated entries, set the intervals and reportable change.
    // Updated entries will retain all other settings configured previously.
    if (false == reset)
    {
        entry.data.reported.minInterval      = newEntry->data.reported.minInterval;
        entry.data.reported.maxInterval      = newEntry->data.reported.maxInterval;
        entry.data.reported.reportableChange = newEntry->data.reported.reportableChange;
    }
    // Give the application a chance to review the configuration that we have
    // been building up.  If the application rejects it, we just do not save the
    // record.  If we were supposed to add a new configuration, it will not be
    // created.  If we were supposed to update an existing configuration, we will
    // keep the old one and just discard any changes.  So, in either case, life
    // continues unchanged if the application rejects the configuration.
    status = emberAfPluginReportingConfiguredCallback(&entry);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emAfPluginReportingSetEntry(index, &entry);
        scheduleTick();
    }
    return status;
}

static EmberAfStatus configureReceivedAttribute(const EmberAfClusterCommand * cmd, AttributeId attributeId, uint8_t mask,
                                                uint16_t timeout)
{
    EmberAfPluginReportingEntry entry;
    EmberAfStatus status;
    uint8_t i, index = NULL_INDEX;
    bool initialize = true;

    // Check the table for an entry that matches this request and also watch for
    // empty slots along the way.  If a report exists, it will be overwritten
    // with the new configuration.  Otherwise, a new entry will be created and
    // initialized.
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportingGetEntry(i, &entry);
        if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        {
            continue;
        }
        if (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_RECEIVED && entry.endpoint == cmd->apsFrame->destinationEndpoint &&
            entry.clusterId == cmd->apsFrame->clusterId && entry.attributeId == attributeId && entry.mask == mask &&
            entry.manufacturerCode == cmd->mfgCode && entry.data.received.source == cmd->SourceNodeId() &&
            entry.data.received.endpoint == cmd->apsFrame->sourceEndpoint)
        {
            initialize = false;
            index      = i;
            break;
        }
        else if (entry.endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID && index == NULL_INDEX)
        {
            index = i;
        }
    }

    if (index == NULL_INDEX)
    {
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    else if (initialize)
    {
        entry.direction              = EMBER_ZCL_REPORTING_DIRECTION_RECEIVED;
        entry.endpoint               = cmd->apsFrame->destinationEndpoint;
        entry.clusterId              = cmd->apsFrame->clusterId;
        entry.attributeId            = attributeId;
        entry.mask                   = mask;
        entry.manufacturerCode       = cmd->mfgCode;
        entry.data.received.source   = cmd->SourceNodeId();
        entry.data.received.endpoint = cmd->apsFrame->sourceEndpoint;
    }

    // For new or updated entries, set the timeout.  Updated entries will retain
    // all other settings configured previously.
    entry.data.received.timeout = timeout;

    // Give the application a chance to review the configuration that we have
    // been building up.  If the application rejects it, we just do not save the
    // record.  If we were supposed to add a new configuration, it will not be
    // created.  If we were supposed to update an existing configuration, we will
    // keep the old one and just discard any changes.  So, in either case, life
    // continues unchanged if the application rejects the configuration.  If the
    // application accepts the change, the tick does not have to be rescheduled
    // here because we don't do anything with received reports.
    status = emberAfPluginReportingConfiguredCallback(&entry);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emAfPluginReportingSetEntry(index, &entry);
    }
    return status;
}

static void putReportableChangeInResp(const EmberAfPluginReportingEntry * entry, EmberAfAttributeType dataType)
{
    uint8_t bytes = emberAfGetDataSize(dataType);
    if (entry == NULL)
    { // default, 0xFF...UL or 0x80...L
        for (; bytes > 0; bytes--)
        {
            uint8_t b = 0xFF;
            if (emberAfIsTypeSigned(dataType))
            {
                b = (bytes == 1 ? 0x80 : 0x00);
            }
            emberAfPutInt8uInResp(b);
        }
    }
    else
    { // reportable change value
        uint32_t value = entry->data.reported.reportableChange;
        for (; bytes > 0; bytes--)
        {
            uint8_t b = EMBER_BYTE_0(value);
            emberAfPutInt8uInResp(b);
            value >>= 8;
        }
    }
}

// Conditionally add reporting entry.
// This is required to support setting up default reporting entries for
// reportable attributes.
static bool reportEntryDoesNotExist(const EmberAfPluginReportingEntry * newEntry)
{
    uint8_t i;
    EmberAfPluginReportingEntry entry;

    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportingGetEntry(i, &entry);
        if (emAfPluginReportingDoEntriesMatch(&entry, newEntry))
        {
            return false;
        }
    }

    return true;
}

uint8_t emAfPluginReportingConditionallyAddReportingEntry(EmberAfPluginReportingEntry * newEntry)
{
    if (reportEntryDoesNotExist(newEntry))
    {
        return emAfPluginReportingAddEntry(newEntry);
    }
    return 0;
}
