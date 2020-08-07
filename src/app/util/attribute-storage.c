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
 * @brief Contains the per-endpoint configuration of
 *attribute tables.
 *******************************************************************************
 ******************************************************************************/

#include "attribute-storage.h"
#include "af.h"
#include "common.h"

#include "gen/znet-bookkeeping.h"

//------------------------------------------------------------------------------
// Globals
// This is not declared CONST in order to handle dynamic endpoint information
// retrieved from tokens.
EmberAfDefinedEndpoint emAfEndpoints[MAX_ENDPOINT_COUNT];

#if (ATTRIBUTE_MAX_SIZE == 0)
#define ACTUAL_ATTRIBUTE_SIZE 1
#else
#define ACTUAL_ATTRIBUTE_SIZE ATTRIBUTE_MAX_SIZE
#endif

uint8_t attributeData[ACTUAL_ATTRIBUTE_SIZE];

#if (!defined(ATTRIBUTE_SINGLETONS_SIZE)) || (ATTRIBUTE_SINGLETONS_SIZE == 0)
#define ACTUAL_SINGLETONS_SIZE 1
#else
#define ACTUAL_SINGLETONS_SIZE ATTRIBUTE_SINGLETONS_SIZE
#endif
uint8_t singletonAttributeData[ACTUAL_SINGLETONS_SIZE];

uint8_t emberEndpointCount = 0;

// If we have attributes that are more than 2 bytes, then
// we need this data block for the defaults
#ifdef GENERATED_DEFAULTS
const uint8_t generatedDefaults[] = GENERATED_DEFAULTS;
#endif // GENERATED_DEFAULTS

#ifdef GENERATED_MIN_MAX_DEFAULTS
const EmberAfAttributeMinMaxValue minMaxDefaults[] = GENERATED_MIN_MAX_DEFAULTS;
#endif // GENERATED_MIN_MAX_DEFAULTS

#ifdef GENERATED_FUNCTION_ARRAYS
GENERATED_FUNCTION_ARRAYS
#endif

#ifdef EMBER_AF_SUPPORT_COMMAND_DISCOVERY
const EmberAfCommandMetadata generatedCommands[]              = GENERATED_COMMANDS;
const EmberAfManufacturerCodeEntry commandManufacturerCodes[] = GENERATED_COMMAND_MANUFACTURER_CODES;
const uint16_t commandManufacturerCodeCount                   = GENERATED_COMMAND_MANUFACTURER_CODE_COUNT;
#endif

const EmberAfAttributeMetadata generatedAttributes[]      = GENERATED_ATTRIBUTES;
const EmberAfCluster generatedClusters[]                  = GENERATED_CLUSTERS;
const EmberAfEndpointType generatedEmberAfEndpointTypes[] = GENERATED_ENDPOINT_TYPES;
const EmAfZigbeeProNetwork emAfZigbeeProNetworks[]        = EM_AF_GENERATED_ZIGBEE_PRO_NETWORKS;

const EmberAfManufacturerCodeEntry clusterManufacturerCodes[]   = GENERATED_CLUSTER_MANUFACTURER_CODES;
const uint16_t clusterManufacturerCodeCount                     = GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT;
const EmberAfManufacturerCodeEntry attributeManufacturerCodes[] = GENERATED_ATTRIBUTE_MANUFACTURER_CODES;
const uint16_t attributeManufacturerCodeCount                   = GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT;

#if !defined(EMBER_SCRIPTED_TEST)
#define endpointNumber(x) fixedEndpoints[x]
#define endpointProfileId(x) fixedProfileIds[x]
#define endpointDeviceId(x) fixedDeviceIds[x]
#define endpointDeviceVersion(x) fixedDeviceVersions[x]
// Added 'Macro' to silence MISRA warning about conflict with synonymous vars.
#define endpointTypeMacro(x) (EmberAfEndpointType *) &(generatedEmberAfEndpointTypes[fixedEmberAfEndpointTypes[x]])
#define endpointNetworkIndex(x) fixedNetworks[x]
#endif

//------------------------------------------------------------------------------
// Forward declarations

// Returns endpoint index within a given cluster
static uint8_t findClusterEndpointIndex(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t mask, uint16_t manufacturerCode);

//------------------------------------------------------------------------------

// Initial configuration
void emberAfEndpointConfigure(void)
{
    uint8_t ep;

#if !defined(EMBER_SCRIPTED_TEST)
    uint8_t fixedEndpoints[]            = FIXED_ENDPOINT_ARRAY;
    uint16_t fixedProfileIds[]          = FIXED_PROFILE_IDS;
    uint16_t fixedDeviceIds[]           = FIXED_DEVICE_IDS;
    uint8_t fixedDeviceVersions[]       = FIXED_DEVICE_VERSIONS;
    uint8_t fixedEmberAfEndpointTypes[] = FIXED_ENDPOINT_TYPES;
    uint8_t fixedNetworks[]             = FIXED_NETWORKS;
#endif

    emberEndpointCount = FIXED_ENDPOINT_COUNT;
    for (ep = 0; ep < FIXED_ENDPOINT_COUNT; ep++)
    {
        emAfEndpoints[ep].endpoint      = endpointNumber(ep);
        emAfEndpoints[ep].profileId     = endpointProfileId(ep);
        emAfEndpoints[ep].deviceId      = endpointDeviceId(ep);
        emAfEndpoints[ep].deviceVersion = endpointDeviceVersion(ep);
        emAfEndpoints[ep].endpointType  = endpointTypeMacro(ep);
        emAfEndpoints[ep].networkIndex  = endpointNetworkIndex(ep);
        emAfEndpoints[ep].bitmask       = EMBER_AF_ENDPOINT_ENABLED;
    }
}

void emberAfSetEndpointCount(uint8_t dynamicEndpointCount)
{
    emberEndpointCount = FIXED_ENDPOINT_COUNT + dynamicEndpointCount;
}

uint8_t emberAfFixedEndpointCount(void)
{
    return FIXED_ENDPOINT_COUNT;
}

uint8_t emberAfEndpointCount(void)
{
    return emberEndpointCount;
}

bool emberAfEndpointIndexIsEnabled(uint8_t index)
{
    return (emAfEndpoints[index].bitmask & EMBER_AF_ENDPOINT_ENABLED);
}

// some data types (like strings) are sent OTA in human readable order
// (how they are read) instead of little endian as the data types are.
bool emberAfIsThisDataTypeAStringType(EmberAfAttributeType dataType)
{
    return (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE ||
            dataType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE || dataType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

// This function is used to call the per-cluster default response callback
void emberAfClusterDefaultResponseWithMfgCodeCallback(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t commandId,
                                                      EmberAfStatus status, uint8_t clientServerMask, uint16_t manufacturerCode)
{
    EmberAfCluster * cluster = emberAfFindClusterWithMfgCode(endpoint, clusterId, clientServerMask, manufacturerCode);
    if (cluster != NULL)
    {
        EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION);
        if (f != NULL)
        {
            // emberAfPushEndpointNetworkIndex(endpoint);
            ((EmberAfDefaultResponseFunction) f)(endpoint, commandId, status);
            // emberAfPopNetworkIndex();
        }
    }
}

// This function is used to call the per-cluster default response callback, and
// wraps the emberAfClusterDefaultResponseWithMfgCodeCallback with a
// EMBER_AF_NULL_MANUFACTURER_CODE.
void emberAfClusterDefaultResponseCallback(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t commandId, EmberAfStatus status,
                                           uint8_t clientServerMask)
{
    emberAfClusterDefaultResponseWithMfgCodeCallback(endpoint, clusterId, commandId, status, clientServerMask,
                                                     EMBER_AF_NULL_MANUFACTURER_CODE);
}

// This function is used to call the per-cluster message sent callback
void emberAfClusterMessageSentWithMfgCodeCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status,
                                                  uint16_t mfgCode)
{
    if (apsFrame != NULL && message != NULL && msgLen != 0)
    {
        EmberAfCluster * cluster = emberAfFindClusterWithMfgCode(
            apsFrame->sourceEndpoint, apsFrame->clusterId,
            (((message[0] & ZCL_FRAME_CONTROL_DIRECTION_MASK) == ZCL_FRAME_CONTROL_SERVER_TO_CLIENT) ? CLUSTER_MASK_SERVER
                                                                                                     : CLUSTER_MASK_CLIENT),
            mfgCode);
        if (cluster != NULL)
        {
            EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_MESSAGE_SENT_FUNCTION);
            if (f != NULL)
            {
                // emberAfPushEndpointNetworkIndex(apsFrame->sourceEndpoint);
                ((EmberAfMessageSentFunction) f)(type, indexOrDestination, apsFrame, msgLen, message, status);
                // emberAfPopNetworkIndex();
            }
        }
    }
}

// This function is used to call the per-cluster message sent callback, and
// wraps the emberAfClusterMessageSentWithMfgCodeCallback with a
// EMBER_AF_NULL_MANUFACTURER_CODE.
void emberAfClusterMessageSentCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination, EmberApsFrame * apsFrame,
                                       uint16_t msgLen, uint8_t * message, EmberStatus status)
{
    emberAfClusterMessageSentWithMfgCodeCallback(type, indexOrDestination, apsFrame, msgLen, message, status,
                                                 EMBER_AF_NULL_MANUFACTURER_CODE);
}

// This function is used to call the per-cluster attribute changed callback
void emAfClusterAttributeChangedCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                         uint8_t clientServerMask, uint16_t manufacturerCode)
{
    EmberAfCluster * cluster = emberAfFindClusterWithMfgCode(endpoint, clusterId, clientServerMask, manufacturerCode);
    if (cluster != NULL)
    {
        if (manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)
        {
            EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION);
            if (f != NULL)
            {
                // emberAfPushEndpointNetworkIndex(endpoint);
                ((EmberAfClusterAttributeChangedCallback) f)(endpoint, attributeId);
                // emberAfPopNetworkIndex();
            }
        }
        else
        {
            EmberAfGenericClusterFunction f =
                emberAfFindClusterFunction(cluster, CLUSTER_MASK_MANUFACTURER_SPECIFIC_ATTRIBUTE_CHANGED_FUNCTION);
            if (f != NULL)
            {
                // emberAfPushEndpointNetworkIndex(endpoint);
                ((EmberAfManufacturerSpecificClusterAttributeChangedCallback) f)(endpoint, attributeId, manufacturerCode);
                // emberAfPopNetworkIndex();
            }
        }
    }
}

// This function is used to call the per-cluster pre-attribute changed callback
EmberAfStatus emAfClusterPreAttributeChangedCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                                     uint8_t clientServerMask, uint16_t manufacturerCode,
                                                     EmberAfAttributeType attributeType, uint8_t size, uint8_t * value)
{
    EmberAfCluster * cluster = emberAfFindClusterWithMfgCode(endpoint, clusterId, clientServerMask, manufacturerCode);
    if (cluster == NULL)
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
    else
    {
        EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
        if (manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)
        {
            EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_PRE_ATTRIBUTE_CHANGED_FUNCTION);
            if (f != NULL)
            {
                // emberAfPushEndpointNetworkIndex(endpoint);
                status = ((EmberAfClusterPreAttributeChangedCallback) f)(endpoint, attributeId, attributeType, size, value);
                // emberAfPopNetworkIndex();
            }
        }
        return status;
    }
}

static void initializeEndpoint(EmberAfDefinedEndpoint * definedEndpoint)
{
    uint8_t clusterIndex;
    EmberAfEndpointType * epType = definedEndpoint->endpointType;
    // emberAfPushEndpointNetworkIndex(definedEndpoint->endpoint);
    for (clusterIndex = 0; clusterIndex < epType->clusterCount; clusterIndex++)
    {
        EmberAfCluster * cluster = &(epType->cluster[clusterIndex]);
        EmberAfGenericClusterFunction f;
        emberAfClusterInitCallback(definedEndpoint->endpoint, cluster->clusterId);
        f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_INIT_FUNCTION);
        if (f != NULL)
        {
            ((EmberAfInitFunction) f)(definedEndpoint->endpoint);
        }
    }
    // emberAfPopNetworkIndex();
}

// Calls the init functions.
void emAfCallInits(void)
{
    uint8_t index;
    for (index = 0; index < emberAfEndpointCount(); index++)
    {
        if (emberAfEndpointIndexIsEnabled(index))
        {
            initializeEndpoint(&(emAfEndpoints[index]));
        }
    }
}

// Returns the pointer to metadata, or null if it is not found
EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(uint8_t endpoint, EmberAfClusterId clusterId,
                                                          EmberAfAttributeId attributeId, uint8_t mask, uint16_t manufacturerCode)
{
    EmberAfAttributeMetadata * metadata = NULL;
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = clusterId;
    record.clusterMask      = mask;
    record.attributeId      = attributeId;
    record.manufacturerCode = manufacturerCode;
    emAfReadOrWriteAttribute(&record, &metadata,
                             NULL,   // buffer
                             0,      // buffer size
                             false); // write?
    return metadata;
}

static uint8_t * singletonAttributeLocation(EmberAfAttributeMetadata * am)
{
    EmberAfAttributeMetadata * m = (EmberAfAttributeMetadata *) &(generatedAttributes[0]);
    uint16_t index               = 0;
    while (m < am)
    {
        if ((m->mask & ATTRIBUTE_MASK_SINGLETON) != 0U)
        {
            index += m->size;
        }
        m++;
    }
    return (uint8_t *) (singletonAttributeData + index);
}

// This function does mem copy, but smartly, which means that if the type is a
// string, it will copy as much as it can.
// If src == NULL, then this method will set memory to zeroes
static EmberAfStatus typeSensitiveMemCopy(uint8_t * dest, uint8_t * src, EmberAfAttributeMetadata * am, bool write,
                                          uint16_t readLength)
{
    EmberAfAttributeType attributeType = am->attributeType;
    uint16_t size                      = (readLength == 0) ? am->size : readLength;

    if (emberAfIsStringAttributeType(attributeType))
    {
        emberAfCopyString(dest, src, size - 1);
    }
    else if (emberAfIsLongStringAttributeType(attributeType))
    {
        emberAfCopyLongString(dest, src, size - 2);
    }
    else
    {
        if (!write && readLength != 0 && readLength < am->size)
        {
            return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        }
        if (src == NULL)
        {
            MEMSET(dest, 0, size);
        }
        else
        {
            MEMMOVE(dest, src, size);
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

// Returns the manufacturer code or ::EMBER_AF_NULL_MANUFACTURER_CODE if none
// could be found.
static uint16_t getManufacturerCode(EmberAfManufacturerCodeEntry * codes, uint16_t codeTableSize, uint16_t tableIndex)
{
    uint16_t i;
    for (i = 0; i < codeTableSize; i++)
    {
        if (codes->index == tableIndex)
        {
            return codes->manufacturerCode;
        }
        codes++;
    }
    return EMBER_AF_NULL_MANUFACTURER_CODE;
}

// This function basically wraps getManufacturerCode with the parameters
// associating an attributes metadata with its code.
uint16_t emberAfGetMfgCode(EmberAfAttributeMetadata * metadata)
{
    return getManufacturerCode((EmberAfManufacturerCodeEntry *) attributeManufacturerCodes, attributeManufacturerCodeCount,
                               (metadata - generatedAttributes));
}

uint16_t emAfGetManufacturerCodeForAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * attMetaData)
{
    return (emberAfClusterIsManufacturerSpecific(cluster) ? emAfGetManufacturerCodeForCluster(cluster)
                                                          : emberAfGetMfgCode(attMetaData));
}

uint16_t emAfGetManufacturerCodeForCluster(EmberAfCluster * cluster)
{
    return getManufacturerCode((EmberAfManufacturerCodeEntry *) clusterManufacturerCodes, clusterManufacturerCodeCount,
                               (cluster - generatedClusters));
}

/**
 * @brief Matches a cluster based on cluster id, direction and manufacturer code.
 *   This function assumes that the passed cluster's endpoint already
 *   matches the endpoint of the EmberAfAttributeSearchRecord.
 *
 * Cluster's match if:
 *   1. Cluster ids match AND
 *   2. Cluster directions match as defined by cluster->mask
 *        and attRecord->clusterMask AND
 *   3. If the clusters are mf specific, their mfg codes match.
 */
bool emAfMatchCluster(EmberAfCluster * cluster, EmberAfAttributeSearchRecord * attRecord)
{
    return (cluster->clusterId == attRecord->clusterId && cluster->mask & attRecord->clusterMask &&
            (!emberAfClusterIsManufacturerSpecific(cluster) ||
             (emAfGetManufacturerCodeForCluster(cluster) == attRecord->manufacturerCode)));
}

/**
 * @brief Matches an attribute based on attribute id and manufacturer code.
 *   This function assumes that the passed cluster already matches the
 *   clusterId, direction and mf specificity of the passed
 *   EmberAfAttributeSearchRecord.
 *
 * Note: If both the attribute and cluster are manufacturer specific,
 *   the cluster's mf code gets precedence.
 *
 * Attributes match if:
 *   1. Att ids match AND
 *      a. cluster IS mf specific OR
 *      b. both stored and saught attributes are NOT mf specific OR
 *      c. stored att IS mf specific AND mfg codes match.
 */
bool emAfMatchAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * am, EmberAfAttributeSearchRecord * attRecord)
{
    return (am->attributeId == attRecord->attributeId &&
            (emberAfClusterIsManufacturerSpecific(cluster) ||
             (emAfGetManufacturerCodeForAttribute(cluster, am) == attRecord->manufacturerCode)));
}

// When reading non-string attributes, this function returns an error when destination
// buffer isn't large enough to accommodate the attribute type.  For strings, the
// function will copy at most readLength bytes.  This means the resulting string
// may be truncated.  The length byte(s) in the resulting string will reflect
// any truncation.  If readLength is zero, we are working with backwards-
// compatibility wrapper functions and we just cross our fingers and hope for
// the best.
//
// When writing attributes, readLength is ignored.  For non-string attributes,
// this function assumes the source buffer is the same size as the attribute
// type.  For strings, the function will copy as many bytes as will fit in the
// attribute.  This means the resulting string may be truncated.  The length
// byte(s) in the resulting string will reflect any truncated.
EmberAfStatus emAfReadOrWriteAttribute(EmberAfAttributeSearchRecord * attRecord, EmberAfAttributeMetadata ** metadata,
                                       uint8_t * buffer, uint16_t readLength, bool write)
{
    uint8_t i;
    uint16_t attributeOffsetIndex = 0;

    for (i = 0; i < emberAfEndpointCount(); i++)
    {
        if (emAfEndpoints[i].endpoint == attRecord->endpoint)
        {
            EmberAfEndpointType * endpointType = emAfEndpoints[i].endpointType;
            uint8_t clusterIndex;
            if (!emberAfEndpointIndexIsEnabled(i))
            {
                continue;
            }
            for (clusterIndex = 0; clusterIndex < endpointType->clusterCount; clusterIndex++)
            {
                EmberAfCluster * cluster = &(endpointType->cluster[clusterIndex]);
                if (emAfMatchCluster(cluster, attRecord))
                { // Got the cluster
                    uint16_t attrIndex;
                    for (attrIndex = 0; attrIndex < cluster->attributeCount; attrIndex++)
                    {
                        EmberAfAttributeMetadata * am = &(cluster->attributes[attrIndex]);
                        if (emAfMatchAttribute(cluster, am, attRecord))
                        { // Got the attribute
                            // If passed metadata location is not null, populate
                            if (metadata != NULL)
                            {
                                *metadata = am;
                            }

                            {
                                uint8_t * attributeLocation =
                                    (am->mask & ATTRIBUTE_MASK_SINGLETON ? singletonAttributeLocation(am)
                                                                         : attributeData + attributeOffsetIndex);
                                uint8_t *src, *dst;
                                if (write)
                                {
                                    src = buffer;
                                    dst = attributeLocation;
                                    if (!emberAfAttributeWriteAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                             emAfGetManufacturerCodeForAttribute(cluster, am),
                                                                             am->attributeId))
                                    {
                                        return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
                                    }
                                }
                                else
                                {
                                    if (buffer == NULL)
                                    {
                                        return EMBER_ZCL_STATUS_SUCCESS;
                                    }

                                    src = attributeLocation;
                                    dst = buffer;
                                    if (!emberAfAttributeReadAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                            emAfGetManufacturerCodeForAttribute(cluster, am),
                                                                            am->attributeId))
                                    {
                                        return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
                                    }
                                }

                                return (am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE
                                            ? (write) ? emberAfExternalAttributeWriteCallback(
                                                            attRecord->endpoint, attRecord->clusterId, am,
                                                            emAfGetManufacturerCodeForAttribute(cluster, am), buffer)
                                                      : emberAfExternalAttributeReadCallback(
                                                            attRecord->endpoint, attRecord->clusterId, am,
                                                            emAfGetManufacturerCodeForAttribute(cluster, am), buffer,
                                                            emberAfAttributeSize(am))
                                            : typeSensitiveMemCopy(dst, src, am, write, readLength));
                            }
                        }
                        else
                        { // Not the attribute we are looking for
                            // Increase the index if attribute is not externally stored
                            if (!(am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) && !(am->mask & ATTRIBUTE_MASK_SINGLETON))
                            {
                                attributeOffsetIndex += emberAfAttributeSize(am);
                            }
                        }
                    }
                }
                else
                { // Not the cluster we are looking for
                    attributeOffsetIndex += cluster->clusterSize;
                }
            }
        }
        else
        { // Not the endpoint we are looking for
            attributeOffsetIndex += emAfEndpoints[i].endpointType->endpointSize;
        }
    }
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // Sorry, attribute was not found.
}

// Check if a cluster is implemented or not. If yes, the cluster is returned.
// If the cluster is not manufacturerSpecific [ClusterId < FC00] then
// manufacturerCode argument is ignored otherwise checked.
//
// mask = 0 -> find either client or server
// mask = CLUSTER_MASK_CLIENT -> find client
// mask = CLUSTER_MASK_SERVER -> find server
EmberAfCluster * emberAfFindClusterInTypeWithMfgCode(EmberAfEndpointType * endpointType, EmberAfClusterId clusterId,
                                                     EmberAfClusterMask mask, uint16_t manufacturerCode)
{
    uint8_t i;
    for (i = 0; i < endpointType->clusterCount; i++)
    {
        EmberAfCluster * cluster = &(endpointType->cluster[i]);
        if (cluster->clusterId == clusterId &&
            (mask == 0 || (mask == CLUSTER_MASK_CLIENT && emberAfClusterIsClient(cluster)) ||
             (mask == CLUSTER_MASK_SERVER && emberAfClusterIsServer(cluster))) &&
            (!emberAfClusterIsManufacturerSpecific(cluster) ||
             (emAfGetManufacturerCodeForCluster(cluster) == manufacturerCode)
             // For compatibility with older stack api, we ignore manufacturer code here
             // if the manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE
             || manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE))
        {
            return cluster;
        }
    }
    return NULL;
}

// This functions wraps emberAfFindClusterInTypeWithMfgCode with
// a manufacturerCode of EMBER_AF_NULL_MANUFACTURER_CODE.
EmberAfCluster * emberAfFindClusterInType(EmberAfEndpointType * endpointType, EmberAfClusterId clusterId, EmberAfClusterMask mask)
{
    return emberAfFindClusterInTypeWithMfgCode(endpointType, clusterId, mask, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// This code is used during unit tests for clusters that do not involve manufacturer code.
// Should this code be used in other locations, manufacturerCode should be added.
uint8_t emberAfClusterIndex(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask)
{
    uint8_t ep;
    uint8_t index = 0xFF;
    for (ep = 0; ep < emberAfEndpointCount(); ep++)
    {
        EmberAfEndpointType * endpointType = emAfEndpoints[ep].endpointType;
        if (emberAfFindClusterInTypeWithMfgCode(endpointType, clusterId, mask, EMBER_AF_NULL_MANUFACTURER_CODE) != NULL)
        {
            index++;
            if (emAfEndpoints[ep].endpoint == endpoint)
            {
                return index;
            }
        }
    }
    return 0xFF;
}

// Returns true uf endpoint contains passed cluster
bool emberAfContainsClusterWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, uint16_t manufacturerCode)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, 0, manufacturerCode) != NULL);
}

// Returns true if endpoint contains passed cluster as a server
bool emberAfContainsServerWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, uint16_t manufacturerCode)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, CLUSTER_MASK_SERVER, manufacturerCode) != NULL);
}

// Returns true if endpoint contains passed cluster as a client
bool emberAfContainsClientWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, uint16_t manufacturerCode)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, CLUSTER_MASK_CLIENT, manufacturerCode) != NULL);
}

// Wraps emberAfContainsClusterWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
// This will find the first cluster that has the clusterId given, regardless of mfgCode.
bool emberAfContainsCluster(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, 0, EMBER_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Wraps emberAfContainsServerWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
// This will find the first server that has the clusterId given, regardless of mfgCode.
bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Wraps emberAfContainsClientWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
// This will find the first client that has the clusterId given, regardless of mfgCode.
bool emberAfContainsClient(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return (emberAfFindClusterWithMfgCode(endpoint, clusterId, CLUSTER_MASK_CLIENT, EMBER_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Finds the cluster that matches endpoint, clusterId, direction, and manufacturerCode.
EmberAfCluster * emberAfFindClusterWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask,
                                               uint16_t manufacturerCode)
{
    uint8_t ep = emberAfIndexFromEndpoint(endpoint);
    if (ep == 0xFF)
    {
        return NULL;
    }
    else
    {
        return emberAfFindClusterInTypeWithMfgCode(emAfEndpoints[ep].endpointType, clusterId, mask, manufacturerCode);
    }
}

// This function wraps emberAfFindClusterWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
// and will ignore the manufacturerCode when trying to find clusters.
// This will return the first cluster in the cluster table that matches the parameters given.
EmberAfCluster * emberAfFindCluster(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask)
{
    return emberAfFindClusterWithMfgCode(endpoint, clusterId, mask, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// Returns cluster within the endpoint; Does not ignore disabled endpoints
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpointsWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId,
                                                                         EmberAfClusterMask mask, uint16_t manufacturerCode)
{
    uint8_t ep = emberAfIndexFromEndpointIncludingDisabledEndpoints(endpoint);
    if (ep < MAX_ENDPOINT_COUNT)
    {
        return emberAfFindClusterInTypeWithMfgCode(emAfEndpoints[ep].endpointType, clusterId, mask, manufacturerCode);
    }
    return NULL;
}

// Returns cluster within the endpoint; Does not ignore disabled endpoints
// This will ignore manufacturerCode.
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask)
{
    return emberAfFindClusterIncludingDisabledEndpointsWithMfgCode(endpoint, clusterId, mask, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// Server wrapper for findClusterEndpointIndex.
uint8_t emberAfFindClusterServerEndpointIndexWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, uint16_t manufacturerCode)
{
    return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_SERVER, manufacturerCode);
}

// Client wrapper for findClusterEndpointIndex.
uint8_t emberAfFindClusterClientEndpointIndexWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, uint16_t manufacturerCode)
{
    return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_CLIENT, manufacturerCode);
}

// Server wrapper for findClusterEndpointIndex
// This will ignore manufacturerCode, and return the index for the first server that matches on clusterId
uint8_t emberAfFindClusterServerEndpointIndex(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return emberAfFindClusterServerEndpointIndexWithMfgCode(endpoint, clusterId, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// Client wrapper for findClusterEndpointIndex
// This will ignore manufacturerCode, and return the index for the first client that matches on clusterId
uint8_t emberAfFindClusterClientEndpointIndex(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return emberAfFindClusterClientEndpointIndexWithMfgCode(endpoint, clusterId, EMBER_AF_NULL_MANUFACTURER_CODE);
}

// Returns the endpoint index within a given cluster
static uint8_t findClusterEndpointIndex(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t mask, uint16_t manufacturerCode)
{
    uint8_t i, epi = 0;

    if (emberAfFindClusterWithMfgCode(endpoint, clusterId, mask, manufacturerCode) == NULL)
    {
        return 0xFF;
    }

    for (i = 0; i < emberAfEndpointCount(); i++)
    {
        if (emAfEndpoints[i].endpoint == endpoint)
        {
            break;
        }
        epi += (emberAfFindClusterIncludingDisabledEndpointsWithMfgCode(emAfEndpoints[i].endpoint, clusterId, mask,
                                                                        manufacturerCode) != NULL)
            ? 1
            : 0;
    }

    return epi;
}

static uint8_t findIndexFromEndpoint(uint8_t endpoint, bool ignoreDisabledEndpoints)
{
    uint8_t epi;
    for (epi = 0; epi < emberAfEndpointCount(); epi++)
    {
        if (emAfEndpoints[epi].endpoint == endpoint &&
            (!ignoreDisabledEndpoints || emAfEndpoints[epi].bitmask & EMBER_AF_ENDPOINT_ENABLED))
        {
            return epi;
        }
    }
    return 0xFF;
}

bool emberAfEndpointIsEnabled(uint8_t endpoint)
{
    uint8_t index = findIndexFromEndpoint(endpoint,
                                          false); // ignore disabled endpoints?

    EMBER_TEST_ASSERT(0xFF != index);

    if (0xFF == index)
    {
        return false;
    }

    return emberAfEndpointIndexIsEnabled(index);
}

bool emberAfEndpointEnableDisable(uint8_t endpoint, bool enable)
{
    uint8_t index = findIndexFromEndpoint(endpoint,
                                          false); // ignore disabled endpoints?
    bool currentlyEnabled;

    if (0xFF == index)
    {
        return false;
    }

    currentlyEnabled = emAfEndpoints[index].bitmask & EMBER_AF_ENDPOINT_ENABLED;

    if (enable)
    {
        emAfEndpoints[index].bitmask |= EMBER_AF_ENDPOINT_ENABLED;
    }
    else
    {
        emAfEndpoints[index].bitmask &= EMBER_AF_ENDPOINT_DISABLED;
    }

#if defined(EZSP_HOST)
    ezspSetEndpointFlags(endpoint, (enable ? EZSP_ENDPOINT_ENABLED : EZSP_ENDPOINT_DISABLED));
#endif

    if (currentlyEnabled != enable)
    {
        if (enable)
        {
            initializeEndpoint(&(emAfEndpoints[index]));
        }
        else
        {
            uint8_t i;
            for (i = 0; i < emAfEndpoints[index].endpointType->clusterCount; i++)
            {
                EmberAfCluster * cluster = &((emAfEndpoints[index].endpointType->cluster)[i]);
                //        emberAfCorePrintln("Disabling cluster tick for ep:%d, cluster:0x%2X, %p",
                //                           endpoint,
                //                           cluster->clusterId,
                //                           ((cluster->mask & CLUSTER_MASK_CLIENT)
                //                            ? "client"
                //                            : "server"));
                //        emberAfCoreFlush();
                emberAfDeactivateClusterTick(
                    endpoint, cluster->clusterId,
                    (cluster->mask & CLUSTER_MASK_CLIENT ? EMBER_AF_CLIENT_CLUSTER_TICK : EMBER_AF_SERVER_CLUSTER_TICK));
            }
        }
    }

    return true;
}

// Returns the index of a given endpoint.  Does not consider disabled endpoints.
uint8_t emberAfIndexFromEndpoint(uint8_t endpoint)
{
    return findIndexFromEndpoint(endpoint,
                                 true); // ignore disabled endpoints?
}

// Returns the index of a given endpoint.  Considers disabled endpoints.
uint8_t emberAfIndexFromEndpointIncludingDisabledEndpoints(uint8_t endpoint)
{
    return findIndexFromEndpoint(endpoint,
                                 false); // ignore disabled endpoints?
}

uint8_t emberAfEndpointFromIndex(uint8_t index)
{
    return emAfEndpoints[index].endpoint;
}

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(uint8_t endpoint, bool server)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    uint8_t i, c = 0;
    EmberAfDefinedEndpoint * de;
    EmberAfCluster * cluster;

    if (index == 0xFF)
    {
        return 0;
    }
    de = &(emAfEndpoints[index]);
    if (de->endpointType == NULL)
    {
        return 0;
    }
    for (i = 0; i < de->endpointType->clusterCount; i++)
    {
        cluster = &(de->endpointType->cluster[i]);
        if (server && emberAfClusterIsServer(cluster))
        {
            c++;
        }
        if ((!server) && emberAfClusterIsClient(cluster))
        {
            c++;
        }
    }
    return c;
}

uint8_t emberAfGetClusterCountForEndpoint(uint8_t endpoint)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFF)
    {
        return 0;
    }
    return emAfEndpoints[index].endpointType->clusterCount;
}

// Note the difference in implementation from emberAfGetNthCluster().
// emberAfGetClusterByIndex() retrieves the cluster by index regardless of server/client
// and those indexes may be DIFFERENT than the indexes returned from
// emberAfGetNthCluster().  In other words:
//
//  - Use emberAfGetClustersFromEndpoint()  with emberAfGetNthCluster()
//  - Use emberAfGetClusterCountForEndpoint() with emberAfGetClusterByIndex()
//
// Don't mix them.
EmberAfCluster * emberAfGetClusterByIndex(uint8_t endpoint, uint8_t clusterIndex)
{
    uint8_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    EmberAfDefinedEndpoint * definedEndpoint;

    if (endpointIndex == 0xFF)
    {
        return NULL;
    }
    definedEndpoint = &(emAfEndpoints[endpointIndex]);

    if (clusterIndex >= definedEndpoint->endpointType->clusterCount)
    {
        return NULL;
    }
    return &(definedEndpoint->endpointType->cluster[clusterIndex]);
}

EmberAfProfileId emberAfGetProfileIdForEndpoint(uint8_t endpoint)
{
    uint8_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    if (endpointIndex == 0xFF)
    {
        return EMBER_AF_INVALID_PROFILE_ID;
    }
    return emAfEndpoints[endpointIndex].profileId;
}

uint16_t emberAfGetDeviceIdForEndpoint(uint8_t endpoint)
{
    uint8_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    if (endpointIndex == 0xFF)
    {
        return EMBER_AF_INVALID_PROFILE_ID;
    }
    return emAfEndpoints[endpointIndex].deviceId;
}

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
EmberAfCluster * emberAfGetNthCluster(uint8_t endpoint, uint8_t n, bool server)
{
    uint8_t index = emberAfIndexFromEndpoint(endpoint);
    EmberAfDefinedEndpoint * de;
    uint8_t i, c = 0;
    EmberAfCluster * cluster;

    if (index == 0xFF)
    {
        return NULL;
    }
    de = &(emAfEndpoints[index]);

    for (i = 0; i < de->endpointType->clusterCount; i++)
    {
        cluster = &(de->endpointType->cluster[i]);

        if ((server && emberAfClusterIsServer(cluster)) || ((!server) && emberAfClusterIsClient(cluster)))
        {
            if (c == n)
            {
                return cluster;
            }
            c++;
        }
    }
    return NULL;
}

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(uint8_t endpoint, EmberAfClusterId * clusterList, uint8_t listLen, bool server)
{
    uint8_t clusterCount = emberAfClusterCount(endpoint, server);
    uint8_t i;
    EmberAfCluster * cluster;
    if (clusterCount > listLen)
    {
        clusterCount = listLen;
    }
    for (i = 0; i < clusterCount; i++)
    {
        cluster        = emberAfGetNthCluster(endpoint, i, server);
        clusterList[i] = (cluster == NULL ? 0xFFFF : cluster->clusterId);
    }
    return clusterCount;
}

void emberAfInitializeAttributes(uint8_t endpoint)
{
    emAfLoadAttributeDefaults(endpoint, false);
}

void emberAfResetAttributes(uint8_t endpoint)
{
    emAfLoadAttributeDefaults(endpoint, true);
    emAfResetAttributes(endpoint);
}

void emAfLoadAttributeDefaults(uint8_t endpoint, bool writeTokens)
{
    uint8_t ep, clusterI, curNetwork = 0 /* emberGetCurrentNetwork() */;
    uint16_t attr;
    uint8_t * ptr;
    uint8_t epCount = emberAfEndpointCount();

    for (ep = 0; ep < epCount; ep++)
    {
        EmberAfDefinedEndpoint * de;
        if (endpoint != EMBER_BROADCAST_ENDPOINT)
        {
            ep = emberAfIndexFromEndpoint(endpoint);
            if (ep == 0xFF)
            {
                return;
            }
        }
        de = &(emAfEndpoints[ep]);

        // Ensure that the endpoint is on the current network
        if (endpoint == EMBER_BROADCAST_ENDPOINT && de->networkIndex != curNetwork)
        {
            continue;
        }
        for (clusterI = 0; clusterI < de->endpointType->clusterCount; clusterI++)
        {
            EmberAfCluster * cluster = &(de->endpointType->cluster[clusterI]);

            // when the attributeCount is high, the loop takes too long to run and a
            // watchdog kicks in causing a reset. As a workaround, we'll
            // conditionally manually reset the watchdog. 300 sounds like a good
            // magic number for now.
            if (cluster->attributeCount > 300)
            {
                // halResetWatchdog();
            }
            for (attr = 0; attr < cluster->attributeCount; attr++)
            {
                EmberAfAttributeMetadata * am = &(cluster->attributes[attr]);
                if (!(am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE))
                {
                    EmberAfAttributeSearchRecord record;
                    record.endpoint         = de->endpoint;
                    record.clusterId        = cluster->clusterId;
                    record.clusterMask      = (emberAfAttributeIsClient(am) ? CLUSTER_MASK_CLIENT : CLUSTER_MASK_SERVER);
                    record.attributeId      = am->attributeId;
                    record.manufacturerCode = emAfGetManufacturerCodeForAttribute(cluster, am);
                    if ((am->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U)
                    {
                        if (emberAfAttributeSize(am) <= 2)
                        {
                            ptr = (uint8_t *) &(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
                        }
                        else
                        {
                            ptr = (uint8_t *) am->defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
                        }
                    }
                    else
                    {
                        if (emberAfAttributeSize(am) <= 2)
                        {
                            ptr = (uint8_t *) &(am->defaultValue.defaultValue);
                        }
                        else
                        {
                            ptr = (uint8_t *) am->defaultValue.ptrToDefaultValue;
                        }
                    }
                    // At this point, ptr either points to a default value, or is NULL, in which case
                    // it should be treated as if it is pointing to an array of all zeroes.

#if (BIGENDIAN_CPU)
                    // The default value for one- and two-byte attributes is stored in an
                    // uint16_t.  On big-endian platforms, a pointer to the default value of
                    // a one-byte attribute will point to the wrong byte.  So, for those
                    // cases, nudge the pointer forward so it points to the correct byte.
                    if (emberAfAttributeSize(am) == 1 && ptr != NULL)
                    {
                        *ptr++;
                    }
#endif // BIGENDIAN
                    emAfReadOrWriteAttribute(&record,
                                             NULL, // metadata - unused
                                             ptr,
                                             0,     // buffer size - unused
                                             true); // write?
                    if (writeTokens)
                    {
                        emAfSaveAttributeToToken(ptr, de->endpoint, record.clusterId, am);
                    }
                }
            }
        }
        if (endpoint != EMBER_BROADCAST_ENDPOINT)
        {
            break;
        }
    }

    if (!writeTokens)
    {
        emAfLoadAttributesFromTokens(endpoint);
    }
}

void emAfLoadAttributesFromTokens(uint8_t endpoint)
{
    // On EZSP host we currently do not support this. We need to come up with some
    // callbacks.
#ifndef EZSP_HOST
    GENERATED_TOKEN_LOADER(endpoint);
#endif // EZSP_HOST
}

// 'data' argument may be null, since we changed the ptrToDefaultValue
// to be null instead of pointing to all zeroes.
// This function has to be able to deal with that.
void emAfSaveAttributeToToken(uint8_t * data, uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeMetadata * metadata)
{
    // Get out of here if this attribute doesn't have a token.
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return;
    }

// On EZSP host we currently do not support this. We need to come up with some
// callbacks.
#ifndef EZSP_HOST
    GENERATED_TOKEN_SAVER;
#endif // EZSP_HOST
}

// This function returns the actual function point from the array,
// iterating over the function bits.
EmberAfGenericClusterFunction emberAfFindClusterFunction(EmberAfCluster * cluster, EmberAfClusterMask functionMask)
{
    EmberAfClusterMask mask = 0x01;
    uint8_t functionIndex   = 0;

    if ((cluster->mask & functionMask) == 0)
    {
        return NULL;
    }

    while (mask < functionMask)
    {
        if ((cluster->mask & mask) != 0)
        {
            functionIndex++;
        }
        mask <<= 1;
    }
    return cluster->functions[functionIndex];
}

#ifdef EMBER_AF_SUPPORT_COMMAND_DISCOVERY

uint16_t emAfGetManufacturerCodeForCommand(EmberAfCommandMetadata * command)
{
    return getManufacturerCode((EmberAfManufacturerCodeEntry *) commandManufacturerCodes, commandManufacturerCodeCount,
                               (command - generatedCommands));
}

/**
 * This function populates command IDs into a given buffer.
 *
 * It returns true if commands are complete, meaning there are NO MORE
 * commands that would be returned after the last command.
 * It returns false, if there were more commands, but were not populated
 * because of maxIdCount limitation.
 */
bool emberAfExtractCommandIds(bool outgoing, EmberAfClusterCommand * cmd, uint16_t clusterId, uint8_t * buffer,
                              uint16_t bufferLength, uint16_t * bufferIndex, uint8_t startId, uint8_t maxIdCount)
{
    uint16_t i, count = 0;
    bool returnValue   = true;
    uint8_t cmdDirMask = 0;

    // determine the appropriate mask to match the request
    // discover commands generated, client is asking server what commands do you generate?
    if (outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER))
    {
        cmdDirMask = COMMAND_MASK_OUTGOING_SERVER;
        // discover commands generated server is asking client what commands do you generate?
    }
    else if (outgoing && (cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT))
    {
        cmdDirMask = COMMAND_MASK_OUTGOING_CLIENT;
        // discover commands received client is asking server what commands do you receive?
    }
    else if (!outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER))
    {
        cmdDirMask = COMMAND_MASK_INCOMING_SERVER;
        // discover commands received server is asking client what commands do you receive?
    }
    else
    {
        cmdDirMask = COMMAND_MASK_INCOMING_CLIENT;
    }

    for (i = 0; i < EMBER_AF_GENERATED_COMMAND_COUNT; i++)
    {
        if (generatedCommands[i].clusterId != clusterId)
        {
            continue;
        }

        if ((generatedCommands[i].mask & cmdDirMask) == 0)
        {
            continue;
        }

        // Only start from the passed command id
        if (generatedCommands[i].commandId < startId)
        {
            continue;
        }

        // According to spec: if cmd->mfgSpecific is set, then we ONLY return the
        // mfg specific commands. If it's not, then we ONLY return non-mfg specific.
        if (generatedCommands[i].mask & COMMAND_MASK_MANUFACTURER_SPECIFIC)
        {
            // Command is Mfg specific
            if (!cmd->mfgSpecific)
            {
                continue; // ignore if asking for not mfg specific
            }
            if (cmd->mfgCode != emAfGetManufacturerCodeForCommand((EmberAfCommandMetadata *) &(generatedCommands[i])))
            {
                continue; // Ignore if mfg code doesn't match the commands
            }
        }
        else
        {
            // Command is not mfg specific.
            if (cmd->mfgSpecific)
            {
                continue; // Ignore if asking for mfg specific
            }
        }

        // The one we are about to put in, is beyond the maxIdCount,
        // so instead of populating it in, we set the return flag to
        // false and get out of here.
        if (maxIdCount == count || count >= bufferLength)
        {
            returnValue = false;
            break;
        }
        buffer[count] = generatedCommands[i].commandId;
        (*bufferIndex)++;
        count++;
    }
    return returnValue;
}
#else
// We just need an empty stub if we don't support it
bool emberAfExtractCommandIds(bool outgoing, EmberAfClusterCommand * cmd, uint16_t clusterId, uint8_t * buffer,
                              uint16_t bufferLength, uint16_t * bufferIndex, uint8_t startId, uint8_t maxIdCount)
{
    return true;
}
#endif
