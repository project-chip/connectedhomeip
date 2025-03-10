/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include <app/util/attribute-storage.h>

#include <app/util/attribute-storage-detail.h>

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <app/util/ember-io-storage.h>
#include <app/util/ember-strings.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/generic-callbacks.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

// Attribute storage depends on knowing the current layout/setup of attributes
// and corresponding callbacks. Specifically:
//   - zap-generated/callback.h is needed because endpoint_config will call the
//     corresponding callbacks (via GENERATED_FUNCTION_ARRAYS) and the include
//     for it is:
//     util/config.h -> zap-generated/endpoint_config.h
#include <app-common/zap-generated/callback.h>

using namespace chip;
using namespace chip::app;

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

// ----- internal-only methods, not part of the external API -----

// Loads the attributes from built-in default and storage.
static void emAfLoadAttributeDefaults(EndpointId endpoint, Optional<ClusterId> = NullOptional);

static bool emAfMatchCluster(const EmberAfCluster * cluster, const EmberAfAttributeSearchRecord * attRecord);
static bool emAfMatchAttribute(const EmberAfCluster * cluster, const EmberAfAttributeMetadata * am,
                               const EmberAfAttributeSearchRecord * attRecord);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on the endpoint at the given index.
static uint8_t emberAfClusterCountByIndex(uint16_t endpointIndex, bool server);

// Check whether there is an endpoint defined with the given endpoint id that is
// enabled.
static bool emberAfEndpointIsEnabled(EndpointId endpoint);

namespace {

#if (!defined(ATTRIBUTE_SINGLETONS_SIZE)) || (ATTRIBUTE_SINGLETONS_SIZE == 0)
#define ACTUAL_SINGLETONS_SIZE 1
#else
#define ACTUAL_SINGLETONS_SIZE ATTRIBUTE_SINGLETONS_SIZE
#endif
uint8_t singletonAttributeData[ACTUAL_SINGLETONS_SIZE];

uint16_t emberEndpointCount = 0;

/// Determines a incremental unique index for ember
/// metadata that is increased whenever a structural change is made to the
/// ember metadata (e.g. changing dynamic endpoints or enabling/disabling endpoints)
unsigned emberMetadataStructureGeneration = 0;

// If we have attributes that are more than 4 bytes, then
// we need this data block for the defaults
#if (defined(GENERATED_DEFAULTS) && GENERATED_DEFAULTS_COUNT)
constexpr const uint8_t generatedDefaults[] = GENERATED_DEFAULTS;
#define ZAP_LONG_DEFAULTS_INDEX(index)                                                                                             \
    {                                                                                                                              \
        &generatedDefaults[index]                                                                                                  \
    }
#endif // GENERATED_DEFAULTS

#if (defined(GENERATED_MIN_MAX_DEFAULTS) && GENERATED_MIN_MAX_DEFAULT_COUNT)
constexpr const EmberAfAttributeMinMaxValue minMaxDefaults[] = GENERATED_MIN_MAX_DEFAULTS;
#define ZAP_MIN_MAX_DEFAULTS_INDEX(index)                                                                                          \
    {                                                                                                                              \
        &minMaxDefaults[index]                                                                                                     \
    }
#endif // GENERATED_MIN_MAX_DEFAULTS

#ifdef GENERATED_FUNCTION_ARRAYS
GENERATED_FUNCTION_ARRAYS
#endif

#ifdef GENERATED_COMMANDS
constexpr const CommandId generatedCommands[] = GENERATED_COMMANDS;
#define ZAP_GENERATED_COMMANDS_INDEX(index) (&generatedCommands[index])
#endif // GENERATED_COMMANDS

#if (defined(GENERATED_EVENTS) && (GENERATED_EVENT_COUNT > 0))
constexpr const EventId generatedEvents[] = GENERATED_EVENTS;
#define ZAP_GENERATED_EVENTS_INDEX(index) (&generatedEvents[index])
#endif // GENERATED_EVENTS

constexpr const EmberAfAttributeMetadata generatedAttributes[] = GENERATED_ATTRIBUTES;
#define ZAP_ATTRIBUTE_INDEX(index) (&generatedAttributes[index])

#ifdef GENERATED_CLUSTERS
constexpr const EmberAfCluster generatedClusters[] = GENERATED_CLUSTERS;
#define ZAP_CLUSTER_INDEX(index) (&generatedClusters[index])
#endif

#if FIXED_ENDPOINT_COUNT > 0
constexpr const EmberAfEndpointType generatedEmberAfEndpointTypes[] = GENERATED_ENDPOINT_TYPES;
constexpr const EmberAfDeviceType fixedDeviceTypeList[]             = FIXED_DEVICE_TYPES;

// Not const, because these need to mutate.
DataVersion fixedEndpointDataVersions[ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT];
#endif // FIXED_ENDPOINT_COUNT > 0

bool emberAfIsThisDataTypeAListType(EmberAfAttributeType dataType)
{
    return dataType == ZCL_ARRAY_ATTRIBUTE_TYPE;
}

uint16_t findIndexFromEndpoint(EndpointId endpoint, bool ignoreDisabledEndpoints)
{
    if (endpoint == kInvalidEndpointId)
    {
        return kEmberInvalidEndpointIndex;
    }

    uint16_t epi;
    for (epi = 0; epi < emberAfEndpointCount(); epi++)
    {
        if (emAfEndpoints[epi].endpoint == endpoint &&
            (!ignoreDisabledEndpoints || emAfEndpoints[epi].bitmask.Has(EmberAfEndpointOptions::isEnabled)))
        {
            return epi;
        }
    }
    return kEmberInvalidEndpointIndex;
}

// Returns the index of a given endpoint.  Considers disabled endpoints.
uint16_t emberAfIndexFromEndpointIncludingDisabledEndpoints(EndpointId endpoint)
{
    return findIndexFromEndpoint(endpoint, false /* ignoreDisabledEndpoints */);
}

} // anonymous namespace

// Initial configuration
void emberAfEndpointConfigure()
{
    uint16_t ep;

    static_assert(FIXED_ENDPOINT_COUNT <= std::numeric_limits<decltype(ep)>::max(),
                  "FIXED_ENDPOINT_COUNT must not exceed the size of the endpoint data type");

    emberEndpointCount = FIXED_ENDPOINT_COUNT;

#if FIXED_ENDPOINT_COUNT > 0

    constexpr uint16_t fixedEndpoints[]             = FIXED_ENDPOINT_ARRAY;
    constexpr uint16_t fixedDeviceTypeListLengths[] = FIXED_DEVICE_TYPE_LENGTHS;
    constexpr uint16_t fixedDeviceTypeListOffsets[] = FIXED_DEVICE_TYPE_OFFSETS;
    constexpr uint8_t fixedEmberAfEndpointTypes[]   = FIXED_ENDPOINT_TYPES;
    constexpr EndpointId fixedParentEndpoints[]     = FIXED_PARENT_ENDPOINTS;

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

    DataVersion * currentDataVersions = fixedEndpointDataVersions;
    for (ep = 0; ep < FIXED_ENDPOINT_COUNT; ep++)
    {
        emAfEndpoints[ep].endpoint = fixedEndpoints[ep];
        emAfEndpoints[ep].deviceTypeList =
            Span<const EmberAfDeviceType>(&fixedDeviceTypeList[fixedDeviceTypeListOffsets[ep]], fixedDeviceTypeListLengths[ep]);
        emAfEndpoints[ep].endpointType     = &generatedEmberAfEndpointTypes[fixedEmberAfEndpointTypes[ep]];
        emAfEndpoints[ep].dataVersions     = currentDataVersions;
        emAfEndpoints[ep].parentEndpointId = fixedParentEndpoints[ep];

        emAfEndpoints[ep].bitmask.Set(EmberAfEndpointOptions::isEnabled);
        emAfEndpoints[ep].bitmask.Set(EmberAfEndpointOptions::isFlatComposition);

        // Increment currentDataVersions by 1 (slot) for every server cluster
        // this endpoint has.
        currentDataVersions += emberAfClusterCountByIndex(ep, /* server = */ true);
    }

#endif // FIXED_ENDPOINT_COUNT > 0

#if CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
    if (MAX_ENDPOINT_COUNT > FIXED_ENDPOINT_COUNT)
    {
        //
        // Reset instances tracking dynamic endpoints to safe defaults.
        //
        for (ep = FIXED_ENDPOINT_COUNT; ep < MAX_ENDPOINT_COUNT; ep++)
        {
            emAfEndpoints[ep] = EmberAfDefinedEndpoint();
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
    if (id == kInvalidEndpointId)
    {
        return kEmberInvalidEndpointIndex;
    }

    uint16_t index;
    for (index = FIXED_ENDPOINT_COUNT; index < MAX_ENDPOINT_COUNT; index++)
    {
        if (emAfEndpoints[index].endpoint == id)
        {
            return static_cast<uint16_t>(index - FIXED_ENDPOINT_COUNT);
        }
    }
    return kEmberInvalidEndpointIndex;
}

CHIP_ERROR emberAfSetDynamicEndpoint(uint16_t index, EndpointId id, const EmberAfEndpointType * ep,
                                     const Span<DataVersion> & dataVersionStorage, Span<const EmberAfDeviceType> deviceTypeList,
                                     EndpointId parentEndpointId)
{
    auto realIndex = index + FIXED_ENDPOINT_COUNT;

    if (realIndex >= MAX_ENDPOINT_COUNT)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (id == kInvalidEndpointId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto serverClusterCount = emberAfClusterCountForEndpointType(ep, /* server = */ true);
    if (dataVersionStorage.size() < serverClusterCount)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    index = static_cast<uint16_t>(realIndex);
    for (uint16_t i = FIXED_ENDPOINT_COUNT; i < MAX_ENDPOINT_COUNT; i++)
    {
        if (emAfEndpoints[i].endpoint == id)
        {
            return CHIP_ERROR_ENDPOINT_EXISTS;
        }
    }

    const size_t bufferSize = Compatibility::Internal::gEmberAttributeIOBufferSpan.size();
    for (uint8_t i = 0; i < ep->clusterCount; i++)
    {
        const EmberAfCluster * cluster = &(ep->cluster[i]);
        if (!cluster->attributes)
        {
            continue;
        }

        for (uint16_t j = 0; j < cluster->attributeCount; j++)
        {
            const EmberAfAttributeMetadata * attr = &(cluster->attributes[j]);
            uint16_t attrSize                     = emberAfAttributeSize(attr);
            if (attrSize > bufferSize)
            {
                ChipLogError(DataManagement,
                             "Attribute size %u exceeds max size %lu, (attrId=" ChipLogFormatMEI ", clusterId=" ChipLogFormatMEI
                             ")",
                             attrSize, static_cast<unsigned long>(bufferSize), ChipLogValueMEI(attr->attributeId),
                             ChipLogValueMEI(cluster->clusterId));
                return CHIP_ERROR_NO_MEMORY;
            }
        }
    }
    emAfEndpoints[index].endpoint       = id;
    emAfEndpoints[index].deviceTypeList = deviceTypeList;
    emAfEndpoints[index].endpointType   = ep;
    emAfEndpoints[index].dataVersions   = dataVersionStorage.data();
    // Start the endpoint off as disabled.
    emAfEndpoints[index].bitmask.Clear(EmberAfEndpointOptions::isEnabled);
    emAfEndpoints[index].parentEndpointId = parentEndpointId;

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

    emberMetadataStructureGeneration++;
    return CHIP_NO_ERROR;
}

EndpointId emberAfClearDynamicEndpoint(uint16_t index)
{
    EndpointId ep = 0;

    index = static_cast<uint16_t>(index + FIXED_ENDPOINT_COUNT);

    if ((index < MAX_ENDPOINT_COUNT) && (emAfEndpoints[index].endpoint != kInvalidEndpointId) &&
        (emberAfEndpointIndexIsEnabled(index)))
    {
        ep = emAfEndpoints[index].endpoint;
        emberAfEndpointEnableDisable(ep, false);
        emAfEndpoints[index].endpoint = kInvalidEndpointId;
    }

    emberMetadataStructureGeneration++;
    return ep;
}

uint16_t emberAfFixedEndpointCount()
{
    return FIXED_ENDPOINT_COUNT;
}

uint16_t emberAfEndpointCount()
{
    return emberEndpointCount;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    return (emAfEndpoints[index].bitmask.Has(EmberAfEndpointOptions::isEnabled));
}

// This function is used to call the per-cluster attribute changed callback
void emAfClusterAttributeChangedCallback(const ConcreteAttributePath & attributePath)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(attributePath.mEndpointId, attributePath.mClusterId);
    if (cluster != nullptr)
    {
        EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, MATTER_CLUSTER_FLAG_ATTRIBUTE_CHANGED_FUNCTION);
        if (f != nullptr)
        {
            ((EmberAfClusterAttributeChangedCallback) f)(attributePath);
        }
    }
}

// This function is used to call the per-cluster pre-attribute changed callback
Status emAfClusterPreAttributeChangedCallback(const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType,
                                              uint16_t size, uint8_t * value)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(attributePath.mEndpointId, attributePath.mClusterId);
    if (cluster == nullptr)
    {
        if (!emberAfEndpointIsEnabled(attributePath.mEndpointId))
        {
            return Status::UnsupportedEndpoint;
        }
        return Status::UnsupportedCluster;
    }

    Status status = Status::Success;
    // Casting and calling a function pointer on the same line results in ignoring the return
    // of the call on gcc-arm-none-eabi-9-2019-q4-major
    EmberAfClusterPreAttributeChangedCallback f = (EmberAfClusterPreAttributeChangedCallback) (emberAfFindClusterFunction(
        cluster, MATTER_CLUSTER_FLAG_PRE_ATTRIBUTE_CHANGED_FUNCTION));
    if (f != nullptr)
    {
        status = f(attributePath, attributeType, size, value);
    }
    return status;
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
        f = emberAfFindClusterFunction(cluster, MATTER_CLUSTER_FLAG_INIT_FUNCTION);
        if (f != nullptr)
        {
            ((EmberAfInitFunction) f)(definedEndpoint->endpoint);
        }
    }
}

static void shutdownEndpoint(EmberAfDefinedEndpoint * definedEndpoint)
{
    // Call shutdown callbacks from clusters, mainly for canceling pending timers
    uint8_t clusterIndex;
    const EmberAfEndpointType * epType = definedEndpoint->endpointType;
    for (clusterIndex = 0; clusterIndex < epType->clusterCount; clusterIndex++)
    {
        const EmberAfCluster * cluster  = &(epType->cluster[clusterIndex]);
        EmberAfGenericClusterFunction f = emberAfFindClusterFunction(cluster, MATTER_CLUSTER_FLAG_SHUTDOWN_FUNCTION);
        if (f != nullptr)
        {
            ((EmberAfShutdownFunction) f)(definedEndpoint->endpoint);
        }
    }

    CommandHandlerInterfaceRegistry::Instance().UnregisterAllCommandHandlersForEndpoint(definedEndpoint->endpoint);
    AttributeAccessInterfaceRegistry::Instance().UnregisterAllForEndpoint(definedEndpoint->endpoint);
}

// Calls the init functions.
void emAfCallInits()
{
    uint16_t index;
    for (index = 0; index < emberAfEndpointCount(); index++)
    {
        if (emberAfEndpointIndexIsEnabled(index))
        {
            initializeEndpoint(&(emAfEndpoints[index]));
        }
    }
}

// Returns the pointer to metadata, or null if it is not found
const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = nullptr;
    EmberAfAttributeSearchRecord record;
    record.endpoint    = endpoint;
    record.clusterId   = clusterId;
    record.attributeId = attributeId;
    emAfReadOrWriteAttribute(&record, &metadata,
                             nullptr, // buffer
                             0,       // buffer size
                             false);  // write?
    return metadata;
}

static uint8_t * singletonAttributeLocation(const EmberAfAttributeMetadata * am)
{
    const EmberAfAttributeMetadata * m = &(generatedAttributes[0]);
    uint16_t index                     = 0;
    while (m < am)
    {
        if (m->IsSingleton() && !m->IsExternal())
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
static Status typeSensitiveMemCopy(ClusterId clusterId, uint8_t * dest, uint8_t * src, const EmberAfAttributeMetadata * am,
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
            return Status::ResourceExhausted;
        }
        emberAfCopyString(dest, src, bufferSize - 1);
    }
    else if (emberAfIsLongStringAttributeType(attributeType))
    {
        if (bufferSize < 2)
        {
            return Status::ResourceExhausted;
        }
        emberAfCopyLongString(dest, src, bufferSize - 2);
    }
    else if (emberAfIsThisDataTypeAListType(attributeType))
    {
        if (bufferSize < 2)
        {
            return Status::ResourceExhausted;
        }

        // Just copy the length.
        memmove(dest, src, 2);
    }
    else
    {
        if (!ignoreReadLength && readLength < am->size)
        {
            return Status::ResourceExhausted;
        }
        if (src == nullptr)
        {
            memset(dest, 0, am->size);
        }
        else
        {
            memmove(dest, src, am->size);
        }
    }
    return Status::Success;
}

/**
 * @brief Matches a cluster based on cluster id and direction.
 *
 *   This function assumes that the passed cluster's endpoint already
 *   matches the endpoint of the EmberAfAttributeSearchRecord.
 *
 * Clusters match if:
 *   1. Cluster ids match AND
 *   2. Cluster is a server cluster (because there are no client attributes).
 */
bool emAfMatchCluster(const EmberAfCluster * cluster, const EmberAfAttributeSearchRecord * attRecord)
{
    return (cluster->clusterId == attRecord->clusterId && (cluster->mask & MATTER_CLUSTER_FLAG_SERVER));
}

/**
 * @brief Matches an attribute based on attribute id.
 *   This function assumes that the passed cluster already matches the
 *   clusterId and direction of the passed EmberAfAttributeSearchRecord.
 *
 * Attributes match if attr ids match.
 */
bool emAfMatchAttribute(const EmberAfCluster * cluster, const EmberAfAttributeMetadata * am,
                        const EmberAfAttributeSearchRecord * attRecord)
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
Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                uint8_t * buffer, uint16_t readLength, bool write)
{
    assertChipStackLockedByCurrentThread();

    uint16_t attributeOffsetIndex = 0;

    for (uint16_t ep = 0; ep < emberAfEndpointCount(); ep++)
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
                            if (metadata != nullptr)
                            {
                                *metadata = am;
                            }

                            {
                                uint8_t * attributeLocation =
                                    (am->mask & MATTER_ATTRIBUTE_FLAG_SINGLETON ? singletonAttributeLocation(am)
                                                                                : attributeData + attributeOffsetIndex);
                                uint8_t *src, *dst;
                                if (write)
                                {
                                    src = buffer;
                                    dst = attributeLocation;
                                    if (!emberAfAttributeWriteAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                             am->attributeId))
                                    {
                                        return Status::UnsupportedAccess;
                                    }
                                }
                                else
                                {
                                    if (buffer == nullptr)
                                    {
                                        return Status::Success;
                                    }

                                    src = attributeLocation;
                                    dst = buffer;
                                    if (!emberAfAttributeReadAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                            am->attributeId))
                                    {
                                        return Status::UnsupportedAccess;
                                    }
                                }

                                // Is the attribute externally stored?
                                if (am->mask & MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE)
                                {
                                    if (write)
                                    {
                                        return emberAfExternalAttributeWriteCallback(attRecord->endpoint, attRecord->clusterId, am,
                                                                                     buffer);
                                    }

                                    if (readLength < emberAfAttributeSize(am))
                                    {
                                        // Prevent a potential buffer overflow
                                        return Status::ResourceExhausted;
                                    }

                                    return emberAfExternalAttributeReadCallback(attRecord->endpoint, attRecord->clusterId, am,
                                                                                buffer, emberAfAttributeSize(am));
                                }

                                // Internal storage is only supported for fixed endpoints
                                if (!isDynamicEndpoint)
                                {
                                    return typeSensitiveMemCopy(attRecord->clusterId, dst, src, am, write, readLength);
                                }

                                return Status::Failure;
                            }
                        }
                        else
                        { // Not the attribute we are looking for
                            // Increase the index if attribute is not externally stored
                            if (!(am->mask & MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE) &&
                                !(am->mask & MATTER_ATTRIBUTE_FLAG_SINGLETON))
                            {
                                attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + emberAfAttributeSize(am));
                            }
                        }
                    }

                    // Attribute is not in the cluster.
                    return Status::UnsupportedAttribute;
                }

                // Not the cluster we are looking for
                attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + cluster->clusterSize);
            }

            // Cluster is not in the endpoint.
            return Status::UnsupportedCluster;
        }

        // Not the endpoint we are looking for
        // Dynamic endpoints are external and don't factor into storage size
        if (!isDynamicEndpoint)
        {
            attributeOffsetIndex = static_cast<uint16_t>(attributeOffsetIndex + emAfEndpoints[ep].endpointType->endpointSize);
        }
    }
    return Status::UnsupportedEndpoint; // Sorry, endpoint was not found.
}

const EmberAfEndpointType * emberAfFindEndpointType(EndpointId endpointId)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpointId);
    if (ep == kEmberInvalidEndpointIndex)
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

        if (mask == 0 || ((cluster->mask & mask) != 0))
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

    return nullptr;
}

uint8_t emberAfClusterIndex(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask)
{
    for (uint16_t ep = 0; ep < emberAfEndpointCount(); ep++)
    {
        // Check the endpoint id first, because that way we avoid examining the
        // endpoint type for endpoints that are not actually defined.
        if (emAfEndpoints[ep].endpoint == endpoint)
        {
            const EmberAfEndpointType * endpointType = emAfEndpoints[ep].endpointType;
            uint8_t index                            = 0xFF;
            if (emberAfFindClusterInType(endpointType, clusterId, mask, &index) != nullptr)
            {
                return index;
            }
        }
    }
    return 0xFF;
}

// Returns whether the given endpoint has the server of the given cluster on it.
bool emberAfContainsServer(EndpointId endpoint, ClusterId clusterId)
{
    return (emberAfFindServerCluster(endpoint, clusterId) != nullptr);
}

// Returns whether the given endpoint has the client of the given cluster on it.
bool emberAfContainsClient(EndpointId endpoint, ClusterId clusterId)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpoint);
    if (ep == kEmberInvalidEndpointIndex)
    {
        return false;
    }

    return (emberAfFindClusterInType(emAfEndpoints[ep].endpointType, clusterId, MATTER_CLUSTER_FLAG_CLIENT) != nullptr);
}

// This will find the first server that has the clusterId given from the index of endpoint.
bool emberAfContainsServerFromIndex(uint16_t index, ClusterId clusterId)
{
    if (index == kEmberInvalidEndpointIndex)
    {
        return false;
    }

    return emberAfFindClusterInType(emAfEndpoints[index].endpointType, clusterId, MATTER_CLUSTER_FLAG_SERVER);
}

namespace chip {
namespace app {

EnabledEndpointsWithServerCluster::EnabledEndpointsWithServerCluster(ClusterId clusterId) :
    mEndpointCount(emberAfEndpointCount()), mClusterId(clusterId)
{
    EnsureMatchingEndpoint();
}

EndpointId EnabledEndpointsWithServerCluster::operator*() const
{
    return emberAfEndpointFromIndex(mEndpointIndex);
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
const EmberAfCluster * emberAfFindServerCluster(EndpointId endpoint, ClusterId clusterId)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpoint);
    if (ep == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    return emberAfFindClusterInType(emAfEndpoints[ep].endpointType, clusterId, MATTER_CLUSTER_FLAG_SERVER);
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
    return nullptr;
}

uint16_t emberAfGetClusterServerEndpointIndex(EndpointId endpoint, ClusterId cluster, uint16_t fixedClusterServerEndpointCount)
{
    VerifyOrDie(fixedClusterServerEndpointCount <= FIXED_ENDPOINT_COUNT);
    uint16_t epIndex = findIndexFromEndpoint(endpoint, true /*ignoreDisabledEndpoints*/);

    // Endpoint must be configured and enabled
    if (epIndex == kEmberInvalidEndpointIndex)
    {
        return kEmberInvalidEndpointIndex;
    }

    if (emberAfFindClusterInType(emAfEndpoints[epIndex].endpointType, cluster, MATTER_CLUSTER_FLAG_SERVER) == nullptr)
    {
        // The provided endpoint does not contain the given cluster server.
        return kEmberInvalidEndpointIndex;
    }

    if (epIndex < FIXED_ENDPOINT_COUNT)
    {
        // This endpoint is a fixed one.
        // Return the index of this endpoint in the list of fixed endpoints that support the given cluster.
        uint16_t adjustedEndpointIndex = 0;
        for (uint16_t i = 0; i < epIndex; i++)
        {
            // Increase adjustedEndpointIndex for every endpoint containing the cluster server
            // before our endpoint of interest
            if (emAfEndpoints[i].endpoint != kInvalidEndpointId &&
                (emberAfFindClusterInType(emAfEndpoints[i].endpointType, cluster, MATTER_CLUSTER_FLAG_SERVER) != nullptr))
            {
                adjustedEndpointIndex++;
            }
        }

        // If this asserts, the provided fixedClusterServerEndpointCount doesn't match the app data model.
        VerifyOrDie(adjustedEndpointIndex < fixedClusterServerEndpointCount);
        epIndex = adjustedEndpointIndex;
    }
    else
    {
        // This is a dynamic endpoint.
        // Its index is just its index in the dynamic endpoint list, offset by fixedClusterServerEndpointCount.
        epIndex = static_cast<uint16_t>(fixedClusterServerEndpointCount + (epIndex - FIXED_ENDPOINT_COUNT));
    }

    return epIndex;
}

bool emberAfEndpointIsEnabled(EndpointId endpoint)
{
    uint16_t index = findIndexFromEndpoint(endpoint, false /* ignoreDisabledEndpoints */);

    if (kEmberInvalidEndpointIndex == index)
    {
        return false;
    }

    return emberAfEndpointIndexIsEnabled(index);
}

bool emberAfEndpointEnableDisable(EndpointId endpoint, bool enable)
{
    uint16_t index = findIndexFromEndpoint(endpoint, false /* ignoreDisabledEndpoints */);
    bool currentlyEnabled;

    if (kEmberInvalidEndpointIndex == index)
    {
        return false;
    }

    currentlyEnabled = emAfEndpoints[index].bitmask.Has(EmberAfEndpointOptions::isEnabled);

    if (enable)
    {
        emAfEndpoints[index].bitmask.Set(EmberAfEndpointOptions::isEnabled);
    }

    if (currentlyEnabled != enable)
    {
        if (enable)
        {
            initializeEndpoint(&(emAfEndpoints[index]));
            emberAfEndpointChanged(endpoint, emberAfGlobalInteractionModelAttributesChangedListener());
        }
        else
        {
            shutdownEndpoint(&(emAfEndpoints[index]));
            emAfEndpoints[index].bitmask.Clear(EmberAfEndpointOptions::isEnabled);
        }

        EndpointId parentEndpointId = emberAfParentEndpointFromIndex(index);
        while (parentEndpointId != kInvalidEndpointId)
        {
            emberAfAttributeChanged(parentEndpointId, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id,
                                    emberAfGlobalInteractionModelAttributesChangedListener());
            uint16_t parentIndex = emberAfIndexFromEndpoint(parentEndpointId);
            if (parentIndex == kEmberInvalidEndpointIndex)
            {
                // Something has gone wrong.
                break;
            }
            parentEndpointId = emberAfParentEndpointFromIndex(parentIndex);
        }

        emberAfAttributeChanged(/* endpoint = */ 0, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id,
                                emberAfGlobalInteractionModelAttributesChangedListener());
    }

    emberMetadataStructureGeneration++;
    return true;
}

unsigned emberAfMetadataStructureGeneration()
{
    return emberMetadataStructureGeneration;
}

// Returns the index of a given endpoint.  Does not consider disabled endpoints.
uint16_t emberAfIndexFromEndpoint(EndpointId endpoint)
{
    return findIndexFromEndpoint(endpoint, true /* ignoreDisabledEndpoints */);
}

EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    return emAfEndpoints[index].endpoint;
}

EndpointId emberAfParentEndpointFromIndex(uint16_t index)
{
    return emAfEndpoints[index].parentEndpointId;
}

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(EndpointId endpoint, bool server)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return 0;
    }

    return emberAfClusterCountByIndex(index, server);
}

uint8_t emberAfClusterCountByIndex(uint16_t endpointIndex, bool server)
{
    const EmberAfDefinedEndpoint * de = &(emAfEndpoints[endpointIndex]);
    if (de->endpointType == nullptr)
    {
        return 0;
    }

    return emberAfClusterCountForEndpointType(de->endpointType, server);
}

uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * type, bool server)
{
    const EmberAfClusterMask cluster_mask = server ? MATTER_CLUSTER_FLAG_SERVER : MATTER_CLUSTER_FLAG_CLIENT;

    return static_cast<uint8_t>(std::count_if(type->cluster, type->cluster + type->clusterCount,
                                              [=](const EmberAfCluster & cluster) { return (cluster.mask & cluster_mask) != 0; }));
}

uint8_t emberAfGetClusterCountForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return 0;
    }
    return emAfEndpoints[index].endpointType->clusterCount;
}

Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpoint(EndpointId endpoint, CHIP_ERROR & err)
{
    return emberAfDeviceTypeListFromEndpointIndex(emberAfIndexFromEndpoint(endpoint), err);
}

chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpointIndex(unsigned endpointIndex, CHIP_ERROR & err)
{
    if (endpointIndex == 0xFFFF)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
        return Span<const EmberAfDeviceType>();
    }

    err = CHIP_NO_ERROR;
    return emAfEndpoints[endpointIndex].deviceTypeList;
}

CHIP_ERROR GetSemanticTagForEndpointAtIndex(EndpointId endpoint, size_t index,
                                            Clusters::Descriptor::Structs::SemanticTagStruct::Type & tag)
{
    uint16_t endpointIndex = emberAfIndexFromEndpoint(endpoint);

    if (endpointIndex == 0xFFFF || index >= emAfEndpoints[endpointIndex].tagList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    tag = emAfEndpoints[endpointIndex].tagList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR emberAfSetDeviceTypeList(EndpointId endpoint, Span<const EmberAfDeviceType> deviceTypeList)
{
    uint16_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    if (endpointIndex == 0xFFFF)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    emAfEndpoints[endpointIndex].deviceTypeList = deviceTypeList;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetTagList(EndpointId endpoint, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type> tagList)
{
    uint16_t endpointIndex = emberAfIndexFromEndpoint(endpoint);
    if (endpointIndex == 0xFFFF)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    emAfEndpoints[endpointIndex].tagList = tagList;
    return CHIP_NO_ERROR;
}

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
const EmberAfCluster * emberAfGetNthCluster(EndpointId endpoint, uint8_t n, bool server)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    const EmberAfEndpointType * endpointType = emAfEndpoints[index].endpointType;
    const EmberAfClusterMask cluster_mask    = server ? MATTER_CLUSTER_FLAG_SERVER : MATTER_CLUSTER_FLAG_CLIENT;
    const uint8_t clusterCount               = endpointType->clusterCount;

    uint8_t c = 0;
    for (uint8_t i = 0; i < clusterCount; i++)
    {
        const EmberAfCluster * cluster = &(endpointType->cluster[i]);

        if ((cluster->mask & cluster_mask) == 0)
        {
            continue;
        }

        if (c == n)
        {
            return cluster;
        }

        c++;
    }
    return nullptr;
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
        clusterList[i] = (cluster == nullptr ? kEmberInvalidEndpointIndex : cluster->clusterId);
    }
    return clusterCount;
}

void emberAfInitializeAttributes(EndpointId endpoint)
{
    emAfLoadAttributeDefaults(endpoint);
}

void emAfLoadAttributeDefaults(EndpointId endpoint, Optional<ClusterId> clusterId)
{
    uint16_t ep;
    uint8_t clusterI;
    uint16_t attr;
    uint8_t * ptr;
    uint16_t epCount = emberAfEndpointCount();
    uint8_t attrData[ATTRIBUTE_LARGEST];
    auto * attrStorage = GetAttributePersistenceProvider();
    // Don't check whether we actually have an attrStorage here, because it's OK
    // to have one if none of our attributes have NVM storage.

    for (ep = 0; ep < epCount; ep++)
    {
        EmberAfDefinedEndpoint * de;
        if (endpoint != kInvalidEndpointId)
        {
            ep = emberAfIndexFromEndpoint(endpoint);
            if (ep == kEmberInvalidEndpointIndex)
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
                if (am->IsAutomaticallyPersisted())
                {
                    VerifyOrDieWithMsg(attrStorage != nullptr, Zcl, "Attribute persistence needs a persistence provider");
                    MutableByteSpan bytes(attrData);
                    CHIP_ERROR err =
                        attrStorage->ReadValue(ConcreteAttributePath(de->endpoint, cluster->clusterId, am->attributeId), am, bytes);
                    if (err == CHIP_NO_ERROR)
                    {
                        ptr = attrData;
                    }
                    else
                    {
                        ChipLogDetail(
                            DataManagement,
                            "Failed to read stored attribute (%u, " ChipLogFormatMEI ", " ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                            de->endpoint, ChipLogValueMEI(cluster->clusterId), ChipLogValueMEI(am->attributeId), err.Format());
                        // Just fall back to default value.
                    }
                }

                if (!am->IsExternal())
                {
                    EmberAfAttributeSearchRecord record;
                    record.endpoint    = de->endpoint;
                    record.clusterId   = cluster->clusterId;
                    record.attributeId = am->attributeId;

                    if (ptr == nullptr)
                    {
                        size_t defaultValueSizeForBigEndianNudger = 0;
                        // Bypasses compiler warning about unused variable for little endian platforms.
                        (void) defaultValueSizeForBigEndianNudger;
                        if ((am->mask & MATTER_ATTRIBUTE_FLAG_MIN_MAX) != 0U)
                        {
                            // This is intentionally 2 and not 4 bytes since defaultValue in min/max
                            // attributes is still uint16_t.
                            if (emberAfAttributeSize(am) <= 2)
                            {
                                static_assert(sizeof(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue) == 2,
                                              "if statement relies on size of max/min defaultValue being 2");
                                ptr = (uint8_t *) &(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
                                defaultValueSizeForBigEndianNudger =
                                    sizeof(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
                            }
                            else
                            {
                                ptr = (uint8_t *) am->defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
                            }
                        }
                        else
                        {
                            if ((emberAfAttributeSize(am) <= 4) && !emberAfIsStringAttributeType(am->attributeType))
                            {
                                ptr                                = (uint8_t *) &(am->defaultValue.defaultValue);
                                defaultValueSizeForBigEndianNudger = sizeof(am->defaultValue.defaultValue);
                            }
                            else
                            {
                                ptr = (uint8_t *) am->defaultValue.ptrToDefaultValue;
                            }
                        }
                        // At this point, ptr either points to a default value, or is NULL, in which case
                        // it should be treated as if it is pointing to an array of all zeroes.

#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
                        // The default values for attributes that are less than or equal to
                        // defaultValueSizeForBigEndianNudger in bytes are stored in an
                        // uint32_t.  On big-endian platforms, a pointer to the default value
                        // of size less than defaultValueSizeForBigEndianNudger will point to the wrong
                        // byte.  So, for those cases, nudge the pointer forward so it points
                        // to the correct byte.
                        if (emberAfAttributeSize(am) < defaultValueSizeForBigEndianNudger && ptr != NULL)
                        {
                            ptr += (defaultValueSizeForBigEndianNudger - emberAfAttributeSize(am));
                        }
#endif // BIGENDIAN
                    }

                    emAfReadOrWriteAttribute(&record,
                                             nullptr, // metadata - unused
                                             ptr,
                                             0,     // buffer size - unused
                                             true); // write?
                }
            }
        }
        if (endpoint != kInvalidEndpointId)
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
    if (!metadata->IsAutomaticallyPersisted())
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

    auto * attrStorage = GetAttributePersistenceProvider();
    if (attrStorage)
    {
        attrStorage->WriteValue(ConcreteAttributePath(endpoint, clusterId, metadata->attributeId), ByteSpan(data, dataSize));
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
        return nullptr;
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

namespace chip {
namespace app {

CHIP_ERROR SetParentEndpointForEndpoint(EndpointId childEndpoint, EndpointId parentEndpoint)
{
    uint16_t childIndex  = emberAfIndexFromEndpoint(childEndpoint);
    uint16_t parentIndex = emberAfIndexFromEndpoint(parentEndpoint);

    if (childIndex == kEmberInvalidEndpointIndex || parentIndex == kEmberInvalidEndpointIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    emAfEndpoints[childIndex].parentEndpointId = parentEndpoint;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetFlatCompositionForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    emAfEndpoints[index].bitmask.Clear(EmberAfEndpointOptions::isTreeComposition);
    emAfEndpoints[index].bitmask.Set(EmberAfEndpointOptions::isFlatComposition);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetTreeCompositionForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    emAfEndpoints[index].bitmask.Clear(EmberAfEndpointOptions::isFlatComposition);
    emAfEndpoints[index].bitmask.Set(EmberAfEndpointOptions::isTreeComposition);
    return CHIP_NO_ERROR;
}

bool IsFlatCompositionForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return false;
    }
    return emAfEndpoints[index].bitmask.Has(EmberAfEndpointOptions::isFlatComposition);
}

bool IsTreeCompositionForEndpoint(EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == kEmberInvalidEndpointIndex)
    {
        return false;
    }
    return emAfEndpoints[index].bitmask.Has(EmberAfEndpointOptions::isTreeComposition);
}

EndpointComposition GetCompositionForEndpointIndex(uint16_t endpointIndex)
{
    VerifyOrReturnValue(endpointIndex < MATTER_ARRAY_SIZE(emAfEndpoints), EndpointComposition::kInvalid);
    if (emAfEndpoints[endpointIndex].bitmask.Has(EmberAfEndpointOptions::isFlatComposition))
    {
        return EndpointComposition::kFullFamily;
    }
    if (emAfEndpoints[endpointIndex].bitmask.Has(EmberAfEndpointOptions::isTreeComposition))
    {
        return EndpointComposition::kTree;
    }
    return EndpointComposition::kInvalid;
}

} // namespace app
} // namespace chip

uint16_t emberAfGetServerAttributeCount(EndpointId endpoint, ClusterId cluster)
{
    const EmberAfCluster * clusterObj = emberAfFindServerCluster(endpoint, cluster);
    VerifyOrReturnError(clusterObj != nullptr, 0);
    return clusterObj->attributeCount;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(EndpointId endpoint, ClusterId cluster, AttributeId attributeId)
{
    const EmberAfCluster * clusterObj = emberAfFindServerCluster(endpoint, cluster);
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
    const EmberAfCluster * clusterObj = emberAfFindServerCluster(endpoint, cluster);
    if (clusterObj == nullptr || clusterObj->attributeCount <= attributeIndex)
    {
        return Optional<AttributeId>::Missing();
    }
    return Optional<AttributeId>(clusterObj->attributes[attributeIndex].attributeId);
}

DataVersion * emberAfDataVersionStorage(const ConcreteClusterPath & aConcreteClusterPath)
{
    uint16_t index = emberAfIndexFromEndpoint(aConcreteClusterPath.mEndpointId);
    if (index == kEmberInvalidEndpointIndex)
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
    auto clusterIndex =
        emberAfClusterIndex(aConcreteClusterPath.mEndpointId, aConcreteClusterPath.mClusterId, MATTER_CLUSTER_FLAG_SERVER);
    if (clusterIndex == 0xFF)
    {
        // No such cluster on this endpoint.
        return nullptr;
    }

    return ep.dataVersions + clusterIndex;
}

namespace {
class GlobalInteractionModelEngineChangedpathListener : public AttributesChangedListener
{
public:
    ~GlobalInteractionModelEngineChangedpathListener() = default;

    void MarkDirty(const AttributePathParams & path) override
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }
};

} // namespace

AttributesChangedListener * emberAfGlobalInteractionModelAttributesChangedListener()
{
    static GlobalInteractionModelEngineChangedpathListener listener;
    return &listener;
}

void emberAfAttributeChanged(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                             AttributesChangedListener * listener)
{
    // Increase cluster data path
    DataVersion * version = emberAfDataVersionStorage(ConcreteClusterPath(endpoint, clusterId));
    if (version == nullptr)
    {
        ChipLogError(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " not found in IncreaseClusterDataVersion!", endpoint,
                     ChipLogValueMEI(clusterId));
    }
    else
    {
        (*(version))++;
        ChipLogDetail(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " update version to %" PRIx32, endpoint,
                      ChipLogValueMEI(clusterId), *(version));
    }

    listener->MarkDirty(AttributePathParams(endpoint, clusterId, attributeId));
}

void emberAfEndpointChanged(EndpointId endpoint, AttributesChangedListener * listener)
{
    listener->MarkDirty(AttributePathParams(endpoint));
}
