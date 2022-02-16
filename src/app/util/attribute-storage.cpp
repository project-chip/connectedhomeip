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

#include "app/util/common.h"
#include <app/AttributePersistenceProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/LockTracker.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/callbacks/PluginCallbacks.h>
#include <app-common/zap-generated/ids/Attributes.h>

using namespace chip;

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

namespace {

#if (!defined(ATTRIBUTE_SINGLETONS_SIZE)) || (ATTRIBUTE_SINGLETONS_SIZE == 0)
#define ACTUAL_SINGLETONS_SIZE 1
#else
#define ACTUAL_SINGLETONS_SIZE ATTRIBUTE_SINGLETONS_SIZE
#endif
uint8_t singletonAttributeData[ACTUAL_SINGLETONS_SIZE];

uint16_t emberEndpointCount = 0;

// If we have attributes that are more than 2 bytes, then
// we need this data block for the defaults
#if (defined(GENERATED_DEFAULTS) && GENERATED_DEFAULTS_COUNT)
constexpr const uint8_t generatedDefaults[] = GENERATED_DEFAULTS;
#endif // GENERATED_DEFAULTS

#if (defined(GENERATED_MIN_MAX_DEFAULTS) && GENERATED_MIN_MAX_DEFAULT_COUNT)
constexpr const EmberAfAttributeMinMaxValue minMaxDefaults[] = GENERATED_MIN_MAX_DEFAULTS;
#endif // GENERATED_MIN_MAX_DEFAULTS

#ifdef GENERATED_FUNCTION_ARRAYS
GENERATED_FUNCTION_ARRAYS
#endif

#ifdef GENERATED_COMMANDS
constexpr const chip::CommandId generatedCommands[] = GENERATED_COMMANDS;
#endif // GENERATED_COMMANDS

constexpr const EmberAfAttributeMetadata generatedAttributes[]      = GENERATED_ATTRIBUTES;
constexpr const EmberAfCluster generatedClusters[]                  = GENERATED_CLUSTERS;
constexpr const EmberAfEndpointType generatedEmberAfEndpointTypes[] = GENERATED_ENDPOINT_TYPES;
// Not const, because these need to mutate.
DataVersion fixedEndpointDataVersions[ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT];

#if !defined(EMBER_SCRIPTED_TEST)
#define endpointNumber(x) fixedEndpoints[x]
#define endpointDeviceId(x) fixedDeviceIds[x]
#define endpointDeviceVersion(x) fixedDeviceVersions[x]
// Added 'Macro' to silence MISRA warning about conflict with synonymous vars.
#define endpointTypeMacro(x) (&(generatedEmberAfEndpointTypes[fixedEmberAfEndpointTypes[x]]))
#endif

app::AttributeAccessInterface * gAttributeAccessOverrides = nullptr;
} // anonymous namespace

//------------------------------------------------------------------------------
// Forward declarations

// Returns endpoint index within a given cluster
static uint16_t findClusterEndpointIndex(EndpointId endpoint, ClusterId clusterId, uint8_t mask);

//------------------------------------------------------------------------------

// Initial configuration
void emberAfEndpointConfigure(void)
{
    uint8_t ep;

#if !defined(EMBER_SCRIPTED_TEST)
    uint16_t fixedEndpoints[]           = FIXED_ENDPOINT_ARRAY;
    uint16_t fixedDeviceIds[]           = FIXED_DEVICE_IDS;
    uint8_t fixedDeviceVersions[]       = FIXED_DEVICE_VERSIONS;
    uint8_t fixedEmberAfEndpointTypes[] = FIXED_ENDPOINT_TYPES;
#endif

#if ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT > 0
    // Initialize our data version storage.  If
    // ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT == 0, gcc complains about a memset
    // with size equal to number of elements without multiplication by element
    // size, because the sizeof() is also 0 in that case...
    if (Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(fixedEndpointDataVersions), sizeof(fixedEndpointDataVersions)) !=
        CHIP_NO_ERROR)
    {
        // Now what?  At least 0-init it.
        memset(fixedEndpointDataVersions, 0, sizeof(fixedEndpointDataVersions));
    }
#endif // ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT > 0

    emberEndpointCount                = FIXED_ENDPOINT_COUNT;
    DataVersion * currentDataVersions = fixedEndpointDataVersions;
    for (ep = 0; ep < FIXED_ENDPOINT_COUNT; ep++)
    {
        emAfEndpoints[ep].endpoint      = endpointNumber(ep);
        emAfEndpoints[ep].deviceId      = endpointDeviceId(ep);
        emAfEndpoints[ep].deviceVersion = endpointDeviceVersion(ep);
        emAfEndpoints[ep].endpointType  = endpointTypeMacro(ep);
        emAfEndpoints[ep].dataVersions  = currentDataVersions;
        emAfEndpoints[ep].bitmask       = EMBER_AF_ENDPOINT_ENABLED;

        // Increment currentDataVersions by 1 (slot) for every server cluster
        // this endpoint has.
        currentDataVersions += emberAfClusterCountByIndex(ep, /* server = */ true);
    }

#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
    if (MAX_ENDPOINT_COUNT > FIXED_ENDPOINT_COUNT)
    {
        // This is assuming that EMBER_AF_ENDPOINT_DISABLED is 0
        static_assert(EMBER_AF_ENDPOINT_DISABLED == 0, "We are creating enabled dynamic endpoints!");
        memset(&emAfEndpoints[FIXED_ENDPOINT_COUNT], 0,
               sizeof(EmberAfDefinedEndpoint) * (MAX_ENDPOINT_COUNT - FIXED_ENDPOINT_COUNT));
        for (ep = FIXED_ENDPOINT_COUNT; ep < MAX_ENDPOINT_COUNT; ep++)
        {
            emAfEndpoints[ep].endpoint = kInvalidEndpointId;
        }
    }
#endif
}

void emberAfSetDynamicEndpointCount(uint16_t dynamicEndpointCount)
{
    emberEndpointCount = static_cast<uint16_t>(FIXED_ENDPOINT_COUNT + dynamicEndpointCount);
}

uint16_t emberAfGetDynamicIndexFromEndpoint(EndpointId id)
{
    uint16_t index;
    for (index = FIXED_ENDPOINT_COUNT; index < MAX_ENDPOINT_COUNT; index++)
    {
        if (emAfEndpoints[index].endpoint == id)
        {
            return static_cast<uint8_t>(index - FIXED_ENDPOINT_COUNT);
        }
    }
    return 0xFFFF;
}

EmberAfStatus emberAfSetDynamicEndpoint(uint16_t index, EndpointId id, EmberAfEndpointType * ep, uint16_t deviceId,
                                        uint8_t deviceVersion, const Span<DataVersion> & dataVersionStorage)
{
    auto realIndex = index + FIXED_ENDPOINT_COUNT;

    if (realIndex >= MAX_ENDPOINT_COUNT)
    {
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    if (id == kInvalidEndpointId)
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }

    auto serverClusterCount = emberAfClusterCountForEndpointType(ep, /* server = */ true);
    if (dataVersionStorage.size() < serverClusterCount)
    {
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }

    index = static_cast<uint16_t>(realIndex);
    for (uint16_t i = FIXED_ENDPOINT_COUNT; i < MAX_ENDPOINT_COUNT; i++)
    {
        if (emAfEndpoints[i].endpoint == id)
        {
            return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
        }
    }

    emAfEndpoints[index].endpoint      = id;
    emAfEndpoints[index].deviceId      = deviceId;
    emAfEndpoints[index].deviceVersion = deviceVersion;
    emAfEndpoints[index].endpointType  = ep;
    emAfEndpoints[index].dataVersions  = dataVersionStorage.data();
    // Start the endpoint off as disabled.
    emAfEndpoints[index].bitmask = EMBER_AF_ENDPOINT_DISABLED;

    emberAfSetDynamicEndpointCount(MAX_ENDPOINT_COUNT - FIXED_ENDPOINT_COUNT);

    // Initialize the data versions.
    size_t dataSize = sizeof(DataVersion) * serverClusterCount;
    if (dataSize != 0)
    {
        if (Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(dataVersionStorage.data()), dataSize) != CHIP_NO_ERROR)
        {
            // Now what?  At least 0-init it.
            memset(dataVersionStorage.data(), 0, dataSize);
        }
    }

    // Now enable the endpoint.
    emberAfEndpointEnableDisable(id, true);
    emberAfSetDeviceEnabled(id, true);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EndpointId emberAfClearDynamicEndpoint(uint16_t index)
{
    EndpointId ep = 0;

    index = static_cast<uint8_t>(index + FIXED_ENDPOINT_COUNT);

    if ((index < MAX_ENDPOINT_COUNT) && (emAfEndpoints[index].endpoint != kInvalidEndpointId) &&
        (emberAfEndpointIndexIsEnabled(index)))
    {
        ep = emAfEndpoints[index].endpoint;
        emberAfSetDeviceEnabled(ep, false);
        emberAfEndpointEnableDisable(ep, false);
        emAfEndpoints[index].endpoint = kInvalidEndpointId;
    }

    return ep;
}

uint16_t emberAfFixedEndpointCount(void)
{
    return FIXED_ENDPOINT_COUNT;
}

uint16_t emberAfEndpointCount(void)
{
    return emberEndpointCount;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
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

bool emberAfIsThisDataTypeAListType(EmberAfAttributeType dataType)
{
    return dataType == ZCL_ARRAY_ATTRIBUTE_TYPE;
}

// This function is used to call the per-cluster default response callback
void emberAfClusterDefaultResponseCallback(EndpointId endpoint, ClusterId clusterId, CommandId commandId, EmberAfStatus status,
                                           uint8_t clientServerMask)
{
    const EmberAfCluster * cluster = emberAfFindCluster(endpoint, clusterId, clientServerMask);
    if (cluster != NULL)
    {
        EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION);
        if (f != NULL)
        {
            ((EmberAfDefaultResponseFunction) f)(endpoint, commandId, status);
        }
    }
}

// This function is used to call the per-cluster message sent callback
void emberAfClusterMessageSentCallback(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t msgLen,
                                       uint8_t * message, EmberStatus status)
{
    if (apsFrame != NULL && message != NULL && msgLen != 0)
    {
        const EmberAfCluster * cluster = emberAfFindCluster(
            apsFrame->sourceEndpoint, apsFrame->clusterId,
            (((message[0] & ZCL_FRAME_CONTROL_DIRECTION_MASK) == ZCL_FRAME_CONTROL_SERVER_TO_CLIENT) ? CLUSTER_MASK_SERVER
                                                                                                     : CLUSTER_MASK_CLIENT));
        if (cluster != NULL)
        {
            EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_MESSAGE_SENT_FUNCTION);
            if (f != NULL)
            {
                ((EmberAfMessageSentFunction) f)(destination, apsFrame, msgLen, message, status);
            }
        }
    }
}

// This function is used to call the per-cluster attribute changed callback
void emAfClusterAttributeChangedCallback(const app::ConcreteAttributePath & attributePath, uint8_t clientServerMask)
{
    const EmberAfCluster * cluster = emberAfFindCluster(attributePath.mEndpointId, attributePath.mClusterId, clientServerMask);
    if (cluster != NULL)
    {
        EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION);
        if (f != NULL)
        {
            ((EmberAfClusterAttributeChangedCallback) f)(attributePath);
        }
    }
}

// This function is used to call the per-cluster pre-attribute changed callback
EmberAfStatus emAfClusterPreAttributeChangedCallback(const app::ConcreteAttributePath & attributePath, uint8_t clientServerMask,
                                                     EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    const EmberAfCluster * cluster = emberAfFindCluster(attributePath.mEndpointId, attributePath.mClusterId, clientServerMask);
    if (cluster == NULL)
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
    else
    {
        EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
        // Casting and calling a function pointer on the same line results in ignoring the return
        // of the call on gcc-arm-none-eabi-9-2019-q4-major
        EmberAfClusterPreAttributeChangedCallback f = (EmberAfClusterPreAttributeChangedCallback)(
            emberAfFindClusterFunction(cluster, CLUSTER_MASK_PRE_ATTRIBUTE_CHANGED_FUNCTION));
        if (f != NULL)
        {
            status = f(attributePath, attributeType, size, value);
        }
        return status;
    }
}

static void initializeEndpoint(EmberAfDefinedEndpoint * definedEndpoint)
{
    uint8_t clusterIndex;
    const EmberAfEndpointType * epType = definedEndpoint->endpointType;
    for (clusterIndex = 0; clusterIndex < epType->clusterCount; clusterIndex++)
    {
        const EmberAfCluster * cluster = &(epType->cluster[clusterIndex]);
        EmberAfGenericClusterFunction f;
        emberAfClusterInitCallback(definedEndpoint->endpoint, cluster->clusterId);
        f = emberAfFindClusterFunction(cluster, CLUSTER_MASK_INIT_FUNCTION);
        if (f != NULL)
        {
            ((EmberAfInitFunction) f)(definedEndpoint->endpoint);
        }
    }
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
const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                                                                uint8_t mask)
{
    const EmberAfAttributeMetadata * metadata = NULL;
    EmberAfAttributeSearchRecord record;
    record.endpoint    = endpoint;
    record.clusterId   = clusterId;
    record.clusterMask = mask;
    record.attributeId = attributeId;
    emAfReadOrWriteAttribute(&record, &metadata,
                             NULL,   // buffer
                             0,      // buffer size
                             false); // write?
    return metadata;
}

static uint8_t * singletonAttributeLocation(const EmberAfAttributeMetadata * am)
{
    const EmberAfAttributeMetadata * m = &(generatedAttributes[0]);
    uint16_t index                     = 0;
    while (m < am)
    {
        if ((m->mask & ATTRIBUTE_MASK_SINGLETON) != 0U)
        {
            index = static_cast<uint16_t>(index + m->size);
        }
        m++;
    }
    return (uint8_t *) (singletonAttributeData + index);
}

// This function does mem copy, but smartly, which means that if the type is a
// string, it will copy as much as it can.
// If src == NULL, then this method will set memory to zeroes
// See documentation for emAfReadOrWriteAttribute for the semantics of
// readLength when reading and writing.
static EmberAfStatus typeSensitiveMemCopy(ClusterId clusterId, uint8_t * dest, uint8_t * src, const EmberAfAttributeMetadata * am,
                                          bool write, uint16_t readLength)
{
    EmberAfAttributeType attributeType = am->attributeType;
    // readLength == 0 for a read indicates that we should just trust that the
    // caller has enough space for an attribute...
    bool ignoreReadLength = write || (readLength == 0);
    uint16_t bufferSize   = ignoreReadLength ? am->size : readLength;

    if (emberAfIsStringAttributeType(attributeType))
    {
        if (bufferSize < 1)
        {
            return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        }
        emberAfCopyString(dest, src, bufferSize - 1);
    }
    else if (emberAfIsLongStringAttributeType(attributeType))
    {
        if (bufferSize < 2)
        {
            return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        }
        emberAfCopyLongString(dest, src, bufferSize - 2);
    }
    else if (emberAfIsThisDataTypeAListType(attributeType))
    {
        if (bufferSize < 2)
        {
            return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        }

        // Just copy the length.
        memmove(dest, src, 2);
    }
    else
    {
        if (!ignoreReadLength && readLength < am->size)
        {
            return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        }
        if (src == NULL)
        {
            memset(dest, 0, am->size);
        }
        else
        {
            memmove(dest, src, am->size);
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Matches a cluster based on cluster id and direction.
 *
 *   This function assumes that the passed cluster's endpoint already
 *   matches the endpoint of the EmberAfAttributeSearchRecord.
 *
 * Clusters match if:
 *   1. Cluster ids match AND
 *   2. Cluster directions match as defined by cluster->mask
 *        and attRecord->clusterMask
 */
bool emAfMatchCluster(const EmberAfCluster * cluster, EmberAfAttributeSearchRecord * attRecord)
{
    return (cluster->clusterId == attRecord->clusterId && cluster->mask & attRecord->clusterMask);
}

/**
 * @brief Matches an attribute based on attribute id.
 *   This function assumes that the passed cluster already matches the
 *   clusterId and direction of the passed EmberAfAttributeSearchRecord.
 *
 * Attributes match if attr ids match.
 */
bool emAfMatchAttribute(const EmberAfCluster * cluster, const EmberAfAttributeMetadata * am,
                        EmberAfAttributeSearchRecord * attRecord)
{
    return (am->attributeId == attRecord->attributeId);
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
EmberAfStatus emAfReadOrWriteAttribute(EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                       uint8_t * buffer, uint16_t readLength, bool write)
{
    assertChipStackLockedByCurrentThread();

    uint16_t attributeOffsetIndex = 0;

    for (uint8_t ep = 0; ep < emberAfEndpointCount(); ep++)
    {
        // Is this a dynamic endpoint?
        bool isDynamicEndpoint = (ep >= emberAfFixedEndpointCount());

        if (emAfEndpoints[ep].endpoint == attRecord->endpoint)
        {
            const EmberAfEndpointType * endpointType = emAfEndpoints[ep].endpointType;
            uint8_t clusterIndex;
            if (!emberAfEndpointIndexIsEnabled(ep))
            {
                continue;
            }
            for (clusterIndex = 0; clusterIndex < endpointType->clusterCount; clusterIndex++)
            {
                const EmberAfCluster * cluster = &(endpointType->cluster[clusterIndex]);
                if (emAfMatchCluster(cluster, attRecord))
                { // Got the cluster
                    uint16_t attrIndex;
                    for (attrIndex = 0; attrIndex < cluster->attributeCount; attrIndex++)
                    {
                        const EmberAfAttributeMetadata * am = &(cluster->attributes[attrIndex]);
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
                                                                            am->attributeId))
                                    {
                                        return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
                                    }
                                }

                                // Is the attribute externally stored?
                                if (am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE)
                                {
                                    return (write ? emberAfExternalAttributeWriteCallback(attRecord->endpoint, attRecord->clusterId,
                                                                                          am, buffer)
                                                  : emberAfExternalAttributeReadCallback(attRecord->endpoint, attRecord->clusterId,
                                                                                         am, buffer, emberAfAttributeSize(am)));
                                }
                                else
                                {
                                    // Internal storage is only supported for fixed endpoints
                                    if (!isDynamicEndpoint)
                                    {
                                        return typeSensitiveMemCopy(attRecord->clusterId, dst, src, am, write, readLength);
                                    }
                                    else
                                    {
                                        return EMBER_ZCL_STATUS_FAILURE;
                                    }
                                }
                            }
                        }
                        else
                        { // Not the attribute we are looking for
                            // Increase the index if attribute is not externally stored
                            if (!(am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) && !(am->mask & ATTRIBUTE_MASK_SINGLETON))
                            {
                                attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + emberAfAttributeSize(am));
                            }
                        }
                    }
                }
                else
                { // Not the cluster we are looking for
                    attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + cluster->clusterSize);
                }
            }
        }
        else
        { // Not the endpoint we are looking for
            // Dynamic endpoints are external and don't factor into storage size
            if (!isDynamicEndpoint)
            {
                attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + emAfEndpoints[ep].endpointType->endpointSize);
            }
        }
    }
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // Sorry, attribute was not found.
}

const EmberAfEndpointType * emberAfFindEndpointType(chip::EndpointId endpointId)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpointId);
    if (ep == 0xFFFF)
    {
        return nullptr;
    }
    return emAfEndpoints[ep].endpointType;
}

const EmberAfCluster * emberAfFindClusterInType(const EmberAfEndpointType * endpointType, ClusterId clusterId,
                                                EmberAfClusterMask mask, uint8_t * index)
{
    uint8_t i;
    uint8_t scopedIndex = 0;

    for (i = 0; i < endpointType->clusterCount; i++)
    {
        const EmberAfCluster * cluster = &(endpointType->cluster[i]);

        if ((mask == 0 || (mask == CLUSTER_MASK_CLIENT && emberAfClusterIsClient(cluster)) ||
             (mask == CLUSTER_MASK_SERVER && emberAfClusterIsServer(cluster))))
        {
            if (cluster->clusterId == clusterId)
            {
                if (index)
                {
                    *index = scopedIndex;
                }

                return cluster;
            }

            scopedIndex++;
        }
    }

    return NULL;
}

uint8_t emberAfClusterIndex(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask)
{
    for (uint8_t ep = 0; ep < emberAfEndpointCount(); ep++)
    {
        // Check the endpoint id first, because that way we avoid examining the
        // endpoint type for endpoints that are not actually defined.
        if (emAfEndpoints[ep].endpoint == endpoint)
        {
            const EmberAfEndpointType * endpointType = emAfEndpoints[ep].endpointType;
            uint8_t index                            = 0xFF;
            if (emberAfFindClusterInType(endpointType, clusterId, mask, &index) != NULL)
            {
                return index;
            }
        }
    }
    return 0xFF;
}

// Returns whether the given endpoint has the client or server of the given
// cluster on it.
bool emberAfContainsCluster(EndpointId endpoint, ClusterId clusterId)
{
    return (emberAfFindCluster(endpoint, clusterId, 0) != NULL);
}

// Returns whether the given endpoint has the server of the given cluster on it.
bool emberAfContainsServer(EndpointId endpoint, ClusterId clusterId)
{
    return (emberAfFindCluster(endpoint, clusterId, CLUSTER_MASK_SERVER) != NULL);
}

// Returns whether the given endpoint has the client of the given cluster on it.
bool emberAfContainsClient(EndpointId endpoint, ClusterId clusterId)
{
    return (emberAfFindCluster(endpoint, clusterId, CLUSTER_MASK_CLIENT) != NULL);
}

// This will find the first server that has the clusterId given from the index of endpoint.
bool emberAfContainsServerFromIndex(uint16_t index, ClusterId clusterId)
{
    if (index == 0xFFFF)
    {
        return false;
    }
    else
    {
        return emberAfFindClusterInType(emAfEndpoints[index].endpointType, clusterId, CLUSTER_MASK_SERVER);
    }
}

namespace chip {
namespace app {

EnabledEndpointsWithServerCluster::EnabledEndpointsWithServerCluster(ClusterId clusterId) : mClusterId(clusterId)
{
    EnsureMatchingEndpoint();
}
EnabledEndpointsWithServerCluster & EnabledEndpointsWithServerCluster::operator++()
{
    ++mEndpointIndex;
    EnsureMatchingEndpoint();
    return *this;
}

void EnabledEndpointsWithServerCluster::EnsureMatchingEndpoint()
{
    for (; mEndpointIndex < mEndpointCount; ++mEndpointIndex)
    {
        if (!emberAfEndpointIndexIsEnabled(mEndpointIndex))
        {
            continue;
        }

        if (emberAfContainsServerFromIndex(mEndpointIndex, mClusterId))
        {
            break;
        }
    }
}

} // namespace app
} // namespace chip

// Finds the cluster that matches endpoint, clusterId, direction.
const EmberAfCluster * emberAfFindCluster(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpoint);
    if (ep == 0xFFFF)
    {
        return NULL;
    }
    else
    {
        return emberAfFindClusterInType(emAfEndpoints[ep].endpointType, clusterId, mask);
    }
}

// Returns cluster within the endpoint; Does not ignore disabled endpoints
const EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(EndpointId endpoint, ClusterId clusterId,
                                                                    EmberAfClusterMask mask)
{
    uint16_t ep = emberAfIndexFromEndpointIncludingDisabledEndpoints(endpoint);
    if (ep < MAX_ENDPOINT_COUNT)
    {
        return emberAfFindClusterInType(emAfEndpoints[ep].endpointType, clusterId, mask);
    }
    return NULL;
}

// Server wrapper for findClusterEndpointIndex
uint16_t emberAfFindClusterServerEndpointIndex(EndpointId endpoint, ClusterId clusterId)
{
    return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_SERVER);
}

// Client wrapper for findClusterEndpointIndex
uint16_t emberAfFindClusterClientEndpointIndex(EndpointId endpoint, ClusterId clusterId)
{
    return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_CLIENT);
}

// Returns the endpoint index within a given cluster
static uint16_t findClusterEndpointIndex(EndpointId endpoint, ClusterId clusterId, uint8_t mask)
{
    uint16_t i, epi = 0;

    if (emberAfFindCluster(endpoint, clusterId, mask) == NULL)
    {
        return 0xFFFF;
    }

    for (i = 0; i < emberAfEndpointCount(); i++)
    {
        if (emAfEndpoints[i].endpoint == endpoint)
        {
            break;
        }
        epi = static_cast<uint16_t>(
            epi + ((emberAfFindClusterIncludingDisabledEndpoints(emAfEndpoints[i].endpoint, clusterId, mask) != NULL) ? 1 : 0));
    }

    return epi;
}

static uint16_t findIndexFromEndpoint(EndpointId endpoint, bool ignoreDisabledEndpoints)
{
    uint16_t epi;
    for (epi = 0; epi < emberAfEndpointCount(); epi++)
    {
        if (emAfEndpoints[epi].endpoint == endpoint &&
            (!ignoreDisabledEndpoints || emAfEndpoints[epi].bitmask & EMBER_AF_ENDPOINT_ENABLED))
        {
            return epi;
        }
    }
    return 0xFFFF;
}

bool emberAfEndpointIsEnabled(EndpointId endpoint)
{
    uint16_t index = findIndexFromEndpoint(endpoint,
                                           false); // ignore disabled endpoints?

    EMBER_TEST_ASSERT(0xFFFF != index);

    if (0xFFFF == index)
    {
        return false;
    }

    return emberAfEndpointIndexIsEnabled(index);
}

bool emberAfEndpointEnableDisable(EndpointId endpoint, bool enable)
{
    uint16_t index = findIndexFromEndpoint(endpoint,
                                           false); // ignore disabled endpoints?
    bool currentlyEnabled;

    if (0xFFFF == index)
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
                const EmberAfCluster * cluster = &((emAfEndpoints[index].endpointType->cluster)[i]);
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

            // Clear out any command handler overrides registered for this
            // endpoint.
            chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandlers(endpoint);

            // Clear out any attribute access overrides registered for this
            // endpoint.
            app::AttributeAccessInterface * prev = nullptr;
            app::AttributeAccessInterface * cur  = gAttributeAccessOverrides;
            while (cur)
            {
                app::AttributeAccessInterface * next = cur->GetNext();
                if (cur->MatchesEndpoint(endpoint))
                {
                    // Remove it from the list
                    if (prev)
                    {
                        prev->SetNext(next);
                    }
                    else
                    {
                        gAttributeAccessOverrides = next;
                    }

                    cur->SetNext(nullptr);

                    // Do not change prev in this case.
                }
                else
                {
                    prev = cur;
                }
                cur = next;
            }
        }

        // TODO: We should notify about the fact that all the attributes for
        // this endpoint have appeared/disappeared, but the reporting engine has
        // no way to do that right now.

        // TODO: Once endpoints are in parts lists other than that of endpoint
        // 0, something more complicated might need to happen here.

        MatterReportingAttributeChangeCallback(/* EndpointId = */ 0, app::Clusters::Descriptor::Id,
                                               app::Clusters::Descriptor::Attributes::PartsList::Id);
    }

    return true;
}

// Returns the index of a given endpoint.  Does not consider disabled endpoints.
uint16_t emberAfIndexFromEndpoint(EndpointId endpoint)
{
    return findIndexFromEndpoint(endpoint,
                                 true); // ignore disabled endpoints?
}

// Returns the index of a given endpoint.  Considers disabled endpoints.
uint16_t emberAfIndexFromEndpointIncludingDisabledEndpoints(EndpointId endpoint)
{
    return findIndexFromEndpoint(endpoint,
                                 false); // ignore disabled endpoints?
}

EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    return emAfEndpoints[index].endpoint;
}

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(EndpointId endpoint, bool server)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFFFF)
    {
        return 0;
    }

    return emberAfClusterCountByIndex(index, server);
}

uint8_t emberAfClusterCountByIndex(uint16_t endpointIndex, bool server)
{
    const EmberAfDefinedEndpoint * de = &(emAfEndpoints[endpointIndex]);
    if (de->endpointType == NULL)
    {
        return 0;
    }

    return emberAfClusterCountForEndpointType(de->endpointType, server);
}

uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * type, bool server)
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < type->clusterCount; i++)
    {
        auto * cluster = &(type->cluster[i]);
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

uint8_t emberAfGetClusterCountForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFFFF)
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
const EmberAfCluster * emberAfGetClusterByIndex(EndpointId endpoint, uint8_t clusterIndex)
{
    uint16_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    EmberAfDefinedEndpoint * definedEndpoint;

    if (endpointIndex == 0xFFFF)
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

uint16_t emberAfGetDeviceIdForEndpoint(EndpointId endpoint)
{
    uint16_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    if (endpointIndex == 0xFFFF)
    {
        return 0xFFFF;
    }
    return emAfEndpoints[endpointIndex].deviceId;
}

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
const EmberAfCluster * emberAfGetNthCluster(EndpointId endpoint, uint8_t n, bool server)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    EmberAfDefinedEndpoint * de;
    uint8_t i, c = 0;
    const EmberAfCluster * cluster;

    if (index == 0xFFFF)
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

// Returns the cluster id of Nth server or client cluster,
// depending on server toggle.
// Returns Optional<ClusterId>::Missing() if cluster does not exist.
Optional<ClusterId> emberAfGetNthClusterId(EndpointId endpoint, uint8_t n, bool server)
{
    const EmberAfCluster * cluster = emberAfGetNthCluster(endpoint, n, server);
    if (cluster == nullptr)
    {
        return Optional<ClusterId>::Missing();
    }
    return Optional<ClusterId>(cluster->clusterId);
}

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(EndpointId endpoint, ClusterId * clusterList, uint8_t listLen, bool server)
{
    uint8_t clusterCount = emberAfClusterCount(endpoint, server);
    uint8_t i;
    const EmberAfCluster * cluster;
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

void emberAfInitializeAttributes(EndpointId endpoint)
{
    emAfLoadAttributeDefaults(endpoint, false);
}

void emberAfResetAttributes(EndpointId endpoint)
{
    emAfLoadAttributeDefaults(endpoint, true);
}

void emAfLoadAttributeDefaults(EndpointId endpoint, bool ignoreStorage, Optional<ClusterId> clusterId)
{
    uint16_t ep;
    uint8_t clusterI;
    uint16_t attr;
    uint8_t * ptr;
    uint16_t epCount = emberAfEndpointCount();
    uint8_t attrData[ATTRIBUTE_LARGEST];
    auto * attrStorage = ignoreStorage ? nullptr : app::GetAttributePersistenceProvider();
    // Don't check whether we actually have an attrStorage here, because it's OK
    // to have one if none of our attributes have NVM storage.

    for (ep = 0; ep < epCount; ep++)
    {
        EmberAfDefinedEndpoint * de;
        if (endpoint != EMBER_BROADCAST_ENDPOINT)
        {
            ep = emberAfIndexFromEndpoint(endpoint);
            if (ep == 0xFFFF)
            {
                return;
            }
        }
        de = &(emAfEndpoints[ep]);

        for (clusterI = 0; clusterI < de->endpointType->clusterCount; clusterI++)
        {
            const EmberAfCluster * cluster = &(de->endpointType->cluster[clusterI]);
            if (clusterId.HasValue())
            {
                if (clusterId.Value() != cluster->clusterId)
                {
                    continue;
                }
            }

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
                const EmberAfAttributeMetadata * am = &(cluster->attributes[attr]);
                ptr                                 = nullptr; // Will get set to the value to write, as needed.

                // First check for a persisted value.
                if (!ignoreStorage && am->IsNonVolatile())
                {
                    VerifyOrDie(attrStorage && "Attribute persistence needs a persistence provider");
                    MutableByteSpan bytes(attrData);
                    CHIP_ERROR err = attrStorage->ReadValue(
                        app::ConcreteAttributePath(de->endpoint, cluster->clusterId, am->attributeId), am, bytes);
                    if (err == CHIP_NO_ERROR)
                    {
                        ptr = attrData;
                    }
                    else
                    {
                        ChipLogDetail(DataManagement,
                                      "Failed to read stored attribute (%" PRIu16 ", " ChipLogFormatMEI ", " ChipLogFormatMEI
                                      ": %" CHIP_ERROR_FORMAT,
                                      de->endpoint, ChipLogValueMEI(cluster->clusterId), ChipLogValueMEI(am->attributeId),
                                      err.Format());
                        // Just fall back to default value.
                    }
                }

                if (!am->IsExternal())
                {
                    EmberAfAttributeSearchRecord record;
                    record.endpoint    = de->endpoint;
                    record.clusterId   = cluster->clusterId;
                    record.clusterMask = (emberAfAttributeIsClient(am) ? CLUSTER_MASK_CLIENT : CLUSTER_MASK_SERVER);
                    record.attributeId = am->attributeId;

                    if (ptr == nullptr)
                    {
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
                    }

                    emAfReadOrWriteAttribute(&record,
                                             NULL, // metadata - unused
                                             ptr,
                                             0,     // buffer size - unused
                                             true); // write?
                    if (ignoreStorage)
                    {
                        emAfSaveAttributeToStorageIfNeeded(ptr, de->endpoint, record.clusterId, am);
                    }
                }
            }
        }
        if (endpoint != EMBER_BROADCAST_ENDPOINT)
        {
            break;
        }
    }
}

// 'data' argument may be null, since we changed the ptrToDefaultValue
// to be null instead of pointing to all zeroes.
// This function has to be able to deal with that.
void emAfSaveAttributeToStorageIfNeeded(uint8_t * data, EndpointId endpoint, ClusterId clusterId,
                                        const EmberAfAttributeMetadata * metadata)
{
    // Get out of here if this attribute isn't marked non-volatile.
    if (!metadata->IsNonVolatile())
    {
        return;
    }

    // TODO: Maybe we should have a separate constant for the size of the
    // largest non-volatile attribute?
    uint8_t allZeroData[ATTRIBUTE_LARGEST] = { 0 };
    if (data == nullptr)
    {
        data = allZeroData;
    }

    size_t dataSize;
    EmberAfAttributeType type = metadata->attributeType;
    if (emberAfIsStringAttributeType(type))
    {
        dataSize = emberAfStringLength(data) + 1;
    }
    else if (emberAfIsLongStringAttributeType(type))
    {
        dataSize = emberAfLongStringLength(data) + 2;
    }
    else
    {
        dataSize = metadata->size;
    }

    auto * attrStorage = app::GetAttributePersistenceProvider();
    if (attrStorage)
    {
        attrStorage->WriteValue(app::ConcreteAttributePath(endpoint, clusterId, metadata->attributeId), metadata,
                                ByteSpan(data, dataSize));
    }
    else
    {
        ChipLogProgress(DataManagement, "Can't store attribute value: no persistence provider");
    }
}

// This function returns the actual function point from the array,
// iterating over the function bits.
EmberAfGenericClusterFunction emberAfFindClusterFunction(const EmberAfCluster * cluster, EmberAfClusterMask functionMask)
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
        mask = static_cast<EmberAfClusterMask>(mask << 1);
    }
    return cluster->functions[functionIndex];
}

bool registerAttributeAccessOverride(app::AttributeAccessInterface * attrOverride)
{
    for (auto * cur = gAttributeAccessOverrides; cur; cur = cur->GetNext())
    {
        if (cur->Matches(*attrOverride))
        {
            ChipLogError(Zcl, "Duplicate attribute override registration failed");
            return false;
        }
    }
    attrOverride->SetNext(gAttributeAccessOverrides);
    gAttributeAccessOverrides = attrOverride;
    return true;
}

app::AttributeAccessInterface * findAttributeAccessOverride(EndpointId endpointId, ClusterId clusterId)
{
    for (app::AttributeAccessInterface * cur = gAttributeAccessOverrides; cur; cur = cur->GetNext())
    {
        if (cur->Matches(endpointId, clusterId))
        {
            return cur;
        }
    }

    return nullptr;
}

uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster)
{
    const EmberAfCluster * clusterObj = emberAfFindCluster(endpoint, cluster, CLUSTER_MASK_SERVER);
    VerifyOrReturnError(clusterObj != nullptr, 0);
    return clusterObj->attributeCount;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                     chip::AttributeId attributeId)
{
    const EmberAfCluster * clusterObj = emberAfFindCluster(endpoint, cluster, CLUSTER_MASK_SERVER);
    VerifyOrReturnError(clusterObj != nullptr, UINT16_MAX);

    for (uint16_t i = 0; i < clusterObj->attributeCount; i++)
    {
        if (clusterObj->attributes[i].attributeId == attributeId)
        {
            return i;
        }
    }
    return UINT16_MAX;
}

Optional<AttributeId> emberAfGetServerAttributeIdByIndex(EndpointId endpoint, ClusterId cluster, uint16_t attributeIndex)
{
    const EmberAfCluster * clusterObj = emberAfFindCluster(endpoint, cluster, CLUSTER_MASK_SERVER);
    if (clusterObj == nullptr || clusterObj->attributeCount <= attributeIndex)
    {
        return Optional<AttributeId>::Missing();
    }
    return Optional<AttributeId>(clusterObj->attributes[attributeIndex].attributeId);
}

DataVersion * emberAfDataVersionStorage(const chip::app::ConcreteClusterPath & aConcreteClusterPath)
{
    uint16_t index = emberAfIndexFromEndpoint(aConcreteClusterPath.mEndpointId);
    if (index == 0xFFFF)
    {
        // Unknown endpoint.
        return nullptr;
    }
    const EmberAfDefinedEndpoint & ep = emAfEndpoints[index];
    if (!ep.dataVersions)
    {
        // No storage provided.
        return nullptr;
    }

    // This does a second walk over endpoints to find the right one, but
    // probably worth it to avoid duplicating code.
    auto clusterIndex = emberAfClusterIndex(aConcreteClusterPath.mEndpointId, aConcreteClusterPath.mClusterId, CLUSTER_MASK_SERVER);
    if (clusterIndex == 0xFF)
    {
        // No such cluster on this endpoint.
        return nullptr;
    }

    return ep.dataVersions + clusterIndex;
}
