/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "thread-bookkeeping.h"
#include "thread-callbacks.h"
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN

typedef struct {
  EmberZclAttributeContext_t context;
  EmberZclReadAttributeResponseHandler readHandler;
  EmberZclWriteAttributeResponseHandler writeHandler;
} Response;

typedef struct {
  uint16_t count;
  uint16_t usedCounts;
} FilterState;

static void *attributeDataLocation(EmberZclEndpointIndex_t endpointIndex,
                                   const EmZclAttributeEntry_t *attribute);
static const void *attributeDefaultMinMaxLocation(const EmZclAttributeEntry_t *attribute,
                                                  EmZclAttributeMask_t dataBit);
static bool isValueLocIndirect(const EmZclAttributeEntry_t *attribute);
static bool isLessThan(const uint8_t *dataA,
                       size_t dataALength,
                       const uint8_t *dataB,
                       size_t dataBLength,
                       const EmZclAttributeEntry_t *attribute);
static bool encodeAttributeResponseMap(CborState *state,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       EmZclMetadata_t metadata,
                                       bool makeAttrIdMap);
static EmberZclStatus_t getAttributeIdsHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data);
static EmberZclStatus_t getAttributeHandler(const EmZclContext_t *context,
                                            CborState *state,
                                            void *data);
static EmberZclStatus_t updateAttributeHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data);
static bool filterAttribute(const EmZclContext_t *context,
                            FilterState *state,
                            const EmZclAttributeEntry_t *attribute);
static void readWriteResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info);
static void handleRead(EmberZclMessageStatus_t status,
                       const Response *response);
static void handleWrite(EmberZclMessageStatus_t status,
                        const Response *response);
static EmberZclStatus_t writeAttribute(EmberZclEndpointIndex_t index,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       const void *data,
                                       size_t dataLength);
static void callPostAttributeChange(EmberZclEndpointId_t endpointId,
                                    const EmZclAttributeEntry_t *attribute,
                                    const void *data,
                                    size_t dataLength);
static bool callPreAttributeChange(EmberZclEndpointId_t endpointId,
                                   const EmZclAttributeEntry_t *attribute,
                                   const void *data,
                                   size_t dataLength);
static size_t tokenize(const EmZclContext_t *context,
                       void *skipData,
                       uint8_t depth,
                       const uint8_t **tokens,
                       size_t *tokenLengths);
static void convertBufferToTwosComplement(uint8_t *buffer, size_t size);
static char* makeMetadataTypeString(const EmZclAttributeEntry_t *attribute);
static uint16_t makeMetadataAccessValue(const EmZclAttributeEntry_t *attribute);

#define oneBitSet(mask) ((mask) != 0 && (mask) == ((mask) & - (mask)))

#define attributeDefaultLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_DEFAULT)
#define attributeMinimumLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_MINIMUM)
#define attributeMaximumLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_MAXIMUM)

// This limit is copied from MAX_ENCODED_URI in coap.c.
#define MAX_ATTRIBUTE_URI_LENGTH 64

// Bit definitions for metadata access value.
#define METADATA_ACCESS_READABLE_BIT (1 << 0)    // bit 0
#define METADATA_ACCESS_WRITEABLE_BIT (1 << 1)   // bit 1
#define METADATA_ACCESS_REPORTABLE_BIT (1 << 5)  // bit 5

void emberZclResetAttributes(EmberZclEndpointId_t endpointId)
{
  // TODO: Handle tokens.
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    EmberZclEndpointIndex_t index
      = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
    if (index != EMBER_ZCL_ENDPOINT_INDEX_NULL
        && emZclIsAttributeLocal(attribute)) {
      const void *dephault = attributeDefaultLocation(attribute);
      writeAttribute(index, endpointId, attribute, dephault, attribute->size);
      callPostAttributeChange(endpointId, attribute, dephault, attribute->size);
    }
  }
}

EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  return emZclReadAttributeEntry(endpointId,
                                 emZclFindAttribute(clusterSpec,
                                                    attributeId,
                                                    false), // exclude remote
                                 buffer,
                                 bufferLength);
}

EmberZclStatus_t emberZclWriteAttribute(EmberZclEndpointId_t endpointId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclAttributeId_t attributeId,
                                        const void *buffer,
                                        size_t bufferLength)
{
  return emZclWriteAttributeEntry(endpointId,
                                  emZclFindAttribute(clusterSpec,
                                                     attributeId,
                                                     false), // exclude remote
                                  buffer,
                                  bufferLength,
                                  true); // enable update
}

EmberZclStatus_t emberZclExternalAttributeChanged(EmberZclEndpointId_t endpointId,
                                                  const EmberZclClusterSpec_t *clusterSpec,
                                                  EmberZclAttributeId_t attributeId,
                                                  const void *buffer,
                                                  size_t bufferLength)
{
  if (!emZclEndpointHasCluster(endpointId, clusterSpec)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  const EmZclAttributeEntry_t *attribute
    = emZclFindAttribute(clusterSpec, attributeId, false); // exclude remote
  if (attribute == NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  if (!emZclIsAttributeExternal(attribute)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  callPostAttributeChange(endpointId, attribute, buffer, bufferLength);

  return EMBER_ZCL_STATUS_SUCCESS;
}

const EmZclAttributeEntry_t *emZclFindAttribute(const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                bool includeRemote)
{
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec, clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && attributeId == attribute->attributeId
               && (includeRemote
                   || emZclIsAttributeLocal(attribute))) {
      return attribute;
    }
  }
  return NULL;
}

EmberZclStatus_t emZclReadAttributeEntry(EmberZclEndpointId_t endpointId,
                                         const EmZclAttributeEntry_t *attribute,
                                         void *buffer,
                                         size_t bufferLength)
{
  if (attribute == NULL || emZclIsAttributeRemote(attribute)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  EmberZclEndpointIndex_t index
    = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
  if (index == EMBER_ZCL_ENDPOINT_INDEX_NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  if (emZclIsAttributeExternal(attribute)) {
    return emberZclReadExternalAttributeCallback(endpointId,
                                                 attribute->clusterSpec,
                                                 attribute->attributeId,
                                                 buffer,
                                                 bufferLength);
  }

  // For variable-length attributes, we are a little flexible for buffer sizes.
  // As long as there is enough space in the buffer to store the current value
  // of the attribute, we permit the read, even if the buffer is smaller than
  // the maximum possible size of the attribute.
  void *data = attributeDataLocation(index, attribute);
  size_t size = emZclAttributeSize(attribute, data);
  if (bufferLength < size) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  MEMCOPY(buffer, data, size);
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberZclStatus_t emZclWriteAttributeEntry(EmberZclEndpointId_t endpointId,
                                          const EmZclAttributeEntry_t *attribute,
                                          const void *buffer,
                                          size_t bufferLength,
                                          bool enableUpdate)
{
  if ((attribute == NULL) || (emZclIsAttributeRemote(attribute))) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  EmberZclEndpointIndex_t index
    = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
  if (index == EMBER_ZCL_ENDPOINT_INDEX_NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  // For variable-length attributes, we are a little flexible for buffer sizes.
  // As long as there is enough space in the table to store the new value of
  // the attribute, we permit the write, even if the actual length of buffer
  // containing the new value is larger than what we have space for.
  size_t size = emZclAttributeSize(attribute, buffer);

  if (attribute->size < size) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  if ((bufferLength == 0)
      || (bufferLength > EMBER_ZCL_ATTRIBUTE_MAX_SIZE)) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  // Check new attribute value against appbuilder bounded min and max values.
  // (Attribute must be an integer numeric type for the bound check to be
  // valid).
  if ((emZclIsAttributeBounded(attribute))
      && ((attribute->type == EMBER_ZCLIP_TYPE_INTEGER)
          || (attribute->type == EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER)
          || (attribute->type == EMBER_ZCLIP_TYPE_BOOLEAN))) {
    if ((isLessThan(buffer,
                    bufferLength,
                    attributeMinimumLocation(attribute),
                    attribute->size,
                    attribute))
        || (isLessThan(attributeMaximumLocation(attribute),
                       attribute->size,
                       buffer,
                       bufferLength,
                       attribute))) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }

  if (!enableUpdate) {
    // If we are pre-write error checking return before the actual write.
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  if (!callPreAttributeChange(endpointId, attribute, buffer, size)) {
    return EMBER_ZCL_STATUS_FAILURE;
  }

  EmberZclStatus_t status = writeAttribute(index,
                                           endpointId,
                                           attribute,
                                           buffer,
                                           size);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    callPostAttributeChange(endpointId, attribute, buffer, size);
  }
  return status;
}

bool emZclReadEncodeAttributeKeyValue(CborState *state,
                                      EmberZclEndpointId_t endpointId,
                                      const EmZclAttributeEntry_t *attribute,
                                      void *buffer,
                                      size_t bufferLength)
{
  if (attribute == NULL) {
    return true;
  } else {
    return ((emZclReadAttributeEntry(endpointId,
                                     attribute,
                                     buffer,
                                     attribute->size)
             == EMBER_ZCL_STATUS_SUCCESS)
            && emCborEncodeMapEntry(state,
                                    attribute->attributeId,
                                    emZclDirectBufferedZclipType(attribute->type),
                                    attribute->size,
                                    buffer));
  }
}

size_t emZclAttributeSize(const EmZclAttributeEntry_t *attribute,
                          const void *data)
{
  if (attribute->type == EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING) {
    return emberZclStringSize(data);
  } else if (attribute->type == EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING) {
    return emberZclLongStringSize(data);
  } else {
    return attribute->size;
  }
}

static void *attributeDataLocation(EmberZclEndpointIndex_t endpointIndex,
                                   const EmZclAttributeEntry_t *attribute)
{
  // AppBuilder generates the maximum size for all of the attribute data, so
  // that the app can create a buffer to hold all of the runtime attribute
  // values. This maximum size factors in attributes that need multiple data
  // instances since they exist on multiple endpoints and are not singleton.
  // When an attribute has multiple data instances, the values are stored
  // sequentially in the buffer. AppBuilder also generates the per-attribute
  // offset into the buffer so that it is easy to go from attribute to value.
  static uint8_t attributeData[EM_ZCL_ATTRIBUTE_DATA_SIZE] = EM_ZCL_ATTRIBUTE_DEFAULTS;
  return (attributeData
          + attribute->dataOffset
          + (attribute->size
             * (emZclIsAttributeSingleton(attribute)
                ? 0
                : endpointIndex)));
}

const void *attributeDefaultMinMaxLocation(const EmZclAttributeEntry_t *attribute,
                                           EmZclAttributeMask_t dataBit)
{
  // AppBuilder generates a table of attribute "constants" that are all possible
  // values of defaults, minimums, and maximums that the user has configured
  // their app to use. We don't generate attribute constants that are zero. We
  // always assume that if an attribute doesn't have a bit set for a
  // default/min/max value in its mask, then the value is all zeros.
  //
  // Each attribute uses an index into a lookup table to figure out where each
  // of their default/min/max constants are located in the constant table. The
  // lookup table stores up to 3 indices per attribute (always in this order):
  // a default value index, a minimum value index, and a maximum value index.
  // However, all of these indices are optional. If an attribute does not have
  // a default/min/max value, or the value is all 0's, or the app was
  // configured not to include that constant through AppBuilder (in the case of
  // min/max values), then an index will not be generated.
  assert(READBITS(dataBit, EM_ZCL_ATTRIBUTE_DATA_MASK) && oneBitSet(dataBit));
  if (!READBITS(attribute->mask, dataBit)) {
    const static uint8_t zeros[EMBER_ZCL_ATTRIBUTE_MAX_SIZE] = { 0 };
    return zeros;
  }

  const size_t *lookupLocation = (emZclAttributeDefaultMinMaxLookupTable
                                  + attribute->defaultMinMaxLookupOffset);
  for (EmZclAttributeMask_t mask = EM_ZCL_ATTRIBUTE_DATA_DEFAULT;
       mask < dataBit;
       mask <<= 1) {
    if (READBITS(attribute->mask, mask)) {
      lookupLocation++;
    }
  }

  return emZclAttributeDefaultMinMaxData + *lookupLocation;
}

static bool isValueLocIndirect(const EmZclAttributeEntry_t *attribute)
{
  // For the CBOR encoder and decoder, in most cases, valueLoc is a pointer to
  // some data.  For strings, valueLoc is a pointer to a pointer to some data.
  // For commands, this is handled automatically by the structs and specs.  For
  // attributes, it must be done manually.
  switch (attribute->type) {
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
      return true;
    default:
      return false;
  }
}

static bool isLessThan(const uint8_t *dataA,
                       size_t dataALength,
                       const uint8_t *dataB,
                       size_t dataBLength,
                       const EmZclAttributeEntry_t *attribute)
{
  // Compares two integer type attribute values in buffers A and B and
  // returns true if dataB < dataA.
  // This function assumes a few things:-
  // - The data* arrays follow the native machine endianness.
  // - The data* arrays represent the same (ZCL) data types (as defined by *attribute).
  // - The data* arrays represent numeric data types.
  // - The data*Length values are not 0.
  // - The data*Length values are no greater than EMBER_ZCL_ATTRIBUTE_MAX_SIZE.

  // Use the largest size of dataALength, dataBLength or attributeSize.
  size_t size = (dataALength > dataBLength) ? dataALength : dataBLength;
  if (attribute->size > size) {
    size = attribute->size;
  }
  if (size > EMBER_ZCL_ATTRIBUTE_MAX_SIZE) {
    return false; // Something has gone badly wrong.
  }

  // Allocate local storage for processing numeric integer attribute types-
  // (Buffer SIZE+1 allows us to add a sign-extension byte which is necessary
  // for absolute difference caculation to return the correct result).
  uint8_t dataABuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
  uint8_t dataBBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
  uint8_t diffBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];

  ++size; // incr size to include one sign-extension byte.

  emZclSignExtendAttributeBuffer(dataABuffer,
                                 size,
                                 dataA,
                                 dataALength,
                                 attribute->type);
  emZclSignExtendAttributeBuffer(dataBBuffer,
                                 size,
                                 dataB,
                                 dataBLength,
                                 attribute->type);

  return emZclGetAbsDifference(dataABuffer,
                               dataBBuffer,
                               diffBuffer,
                               size);
}

// Encode response map of attributes, where key is "v" and value is attribute's
// value for a successful read, or key is "s" and value is failure status for
// unsuccessful read. [ZCLIP 16-07008-026 spec 3.6.4, 3.7.2]
// e.g. {0: {"v": 1}, 6: {"s": 0x86}}
//
// If the metadata value is non-zero we retrieve & encode the appropriate
// attribute metadata value without reading the attribute.
// e.g. {0: {"$base":"uint16"}, 6: {"$base":"uint16"}}
// The makeAttrIdMap parameter is applicable only in the metadata case. If true
// we construct the outer map keyed by attribute IDs as described above, if false
// we encode just the inner {<$metadata_name>:<metadata_value>} map.
static bool encodeAttributeResponseMap(CborState *state,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       EmZclMetadata_t metadata,
                                       bool makeAttrIdMap)
{
  char key[8] = { 0 };
  uint8_t *value;
  uint8_t valueType;
  size_t valueSize;

  // 16-07008-071 Section 2.8.1.5.4: For a single metadata item query
  // don't encode the attribute ID map, in all other cases do.
  if (metadata == EM_ZCL_METADATA_NONE || makeAttrIdMap) {
    if ((!emCborEncodeValue(state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(attribute->attributeId),
                            (const uint8_t *)&attribute->attributeId))
        || (!emCborEncodeIndefiniteMap(state))) {
      return false;
    }
  }

  if (metadata == EM_ZCL_METADATA_NONE) {
    // Read the attribute value and encode the result.
    uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
    EmberZclStatus_t status
      = emZclReadAttributeEntry(endpointId,
                                attribute,
                                buffer,
                                attribute->size);

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      key[0] = 'v';
      value = buffer;
      valueType = emZclDirectBufferedZclipType(attribute->type);
      valueSize = attribute->size;
    } else {
      key[0] = 's';
      value = &status;
      valueType = EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER;
      valueSize = sizeof(status);
    }
    if ((!emCborEncodeValue(state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)&key))
        || (!emCborEncodeValue(state,
                               valueType,
                               valueSize,
                               (const uint8_t *)value))) {
      return false;
    }
  } else {
    // metadata query...
    if ((metadata == EM_ZCL_METADATA_WILDCARD)
        || (metadata == EM_ZCL_METADATA_BASE)) {
      sprintf(key, "%s", EM_ZCL_URI_METADATA_BASE);
      if ((!emCborEncodeValue(state,
                              EMBER_ZCLIP_TYPE_STRING,
                              0, // size - ignored
                              (const uint8_t *)&key))
          || (!emCborEncodeValue(state,
                                 EMBER_ZCLIP_TYPE_STRING,
                                 0, // size - ignored
                                 (const uint8_t *)makeMetadataTypeString(attribute)))) {
        return false;
      }
    }
    if ((metadata == EM_ZCL_METADATA_WILDCARD)
        || (metadata == EM_ZCL_METADATA_ACCESS)) {
      sprintf(key, "%s", EM_ZCL_URI_METADATA_ACCESS);
      uint16_t access = makeMetadataAccessValue(attribute);
      if ((!emCborEncodeValue(state,
                              EMBER_ZCLIP_TYPE_STRING,
                              0, // size - ignored
                              (const uint8_t *)&key))
          || (!emCborEncodeValue(state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(access),
                                 (const uint8_t *)&access))) {
        return false;
      }
    }
  }

  // If we are adding the outer map completete it here
  if (metadata == EM_ZCL_METADATA_NONE || makeAttrIdMap) {
    return (emCborEncodeBreak(state));
  } else {
    return true;
  }
}

static char* makeMetadataTypeString(const EmZclAttributeEntry_t *attribute)
{
  static char metaString[12] = { 0 };
  char *finger = metaString;

  // Some of the ZCL types are not implemented in Studio, e.g. enum8. Until it's
  // fixed use special logic for some of the clusters and attributes that are known
  // to be using the wrong type. Add more as needed. ZCLIP test 18.9 relies on this.
  if (attribute->clusterSpec->id == 0x00 && attribute->attributeId == 0x07) {
    // Basic cluster, PowerSource attribute
    finger += sprintf(finger, "%s", "enum8");
    return metaString;
  }

  switch (attribute->type) {
    case EMBER_ZCLIP_TYPE_BOOLEAN:
      finger += sprintf(finger, "%s", "bool");
      return metaString;
    case EMBER_ZCLIP_TYPE_INTEGER:
      finger += sprintf(finger, "%s", "int");
      break;
    case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      finger += sprintf(finger, "%s", "uint");
      break;
    //TODO- Include these when Appbuilder support is available.
    //case EMBER_ZCLIP_TYPE_ENUM:
    //  finger += sprintf(finger, "%s", "enum");
    //  break;
    //case EMBER_ZCLIP_TYPE_BITMAP:
    //  finger += sprintf(finger, "%s", "map");
    //  break;
    //case EMBER_ZCLIP_TYPE_UTC_TIME:
    //  finger += sprintf(finger, "%s", "UTC");
    //  return metaString;
    case EMBER_ZCLIP_TYPE_STRING:
    case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING:
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
      finger += sprintf(finger, "%s", "string");
      return metaString;
    default:
      finger += sprintf(finger, "%s", "unknown");
      return metaString;
  }

  // Add a bit-length char.
  finger += sprintf(finger, "%d", (int)attribute->size << 3);  // Multiply by 8
  return metaString;
}

static uint16_t makeMetadataAccessValue(const EmZclAttributeEntry_t *attribute)
{
  // Builds the metadata attribute access value.

  uint16_t value = 0;

  if (emZclIsAttributeReadable(attribute)) {
    value |= METADATA_ACCESS_READABLE_BIT;
  }
  if (emZclIsAttributeWritable(attribute)) {
    value |= METADATA_ACCESS_WRITEABLE_BIT;
  }
  if (emZclIsAttributeReportable(attribute)) {
    value |= METADATA_ACCESS_REPORTABLE_BIT;
  }

  return value;
}

static EmberZclStatus_t getAttributeIdsHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data)
{
  // If there are no queries, then we return an array of attribute ids.
  // Otherwise, we return a map from the filtered attribute ids to
  // their values.
  const EmZclAttributeQuery_t *query = &context->attributeQuery;
  bool array = (query->filterCount == 0);

  if (query->filterCount == 0 && query->metadata == EM_ZCL_METADATA_WILDCARD) {
    // This is a ../a?meta=* query. We don't support any metatdata for ../a .See ZCLIP test 18.11
    return EMBER_ZCL_STATUS_NOT_FOUND;
  }

  if (array) {
    emCborEncodeIndefiniteArray(state);
  } else {
    emCborEncodeIndefiniteMap(state);
  }

  FilterState filterState = { 0 };
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = emZclAttributeTable + i;
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &context->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0) {
      if (emZclIsAttributeLocal(attribute)
          && filterAttribute(context, &filterState, attribute)) {
        if (array) {
          if (!emCborEncodeValue(state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(attribute->attributeId),
                                 (const uint8_t *)&attribute->attributeId)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        } else {
          // Return NotFound if an unsupported metadata query was requested.
          if (context->attributeQuery.metadata == EM_ZCL_METADATA_NOT_SUPPORTED) {
            return EMBER_ZCL_STATUS_NOT_FOUND;
          }
          // Call encodeAttributeResponseMap() with "true": For metadata queries encode
          // the (outer) attribute ID map
          if (!encodeAttributeResponseMap(state,
                                          context->endpoint->endpointId,
                                          attribute,
                                          query->metadata,
                                          true)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        }
      }
    }
  }
  if (!emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_FAILURE;
  }

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberZclStatus_t getAttributeHandler(const EmZclContext_t *context,
                                            CborState *state,
                                            void *data)
{
  // Call encodeAttributeResponseMap() with "false": For metadata queries don't encode
  // the (outer) attribute ID map
  if (emCborEncodeIndefiniteMap(state)
      && encodeAttributeResponseMap(state,
                                    context->endpoint->endpointId,
                                    context->attribute,
                                    context->attributeQuery.metadata,
                                    false)
      && emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  return EMBER_ZCL_STATUS_FAILURE;
}

static EmberZclStatus_t updateAttributeHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data)
{
  EmberZclStatus_t result = EMBER_ZCL_STATUS_MALFORMED_COMMAND; // Init overall result status.
  uint8_t numberOfPasses = 1;
  bool precheckFailed = false;
  bool failureMapCreated = false;

  // If POST+Undivided Write mode is selected attributes are only updated if the
  // precheck completes with no errors (i.e. a check of all attributes for
  // potential write errors, without updating any attributes).
  // Note- We also do a precheck for the PUT type access to verify that the
  // supplied attribute map payload contains only a single attributeId (which
  // must also match the attributeId specified in the Uri).
  if ((context->attributeQuery.undivided)  // ==POST+Undivided Write.
      || (context->attribute != NULL)) {   // ==PUT
    numberOfPasses = 2; // First pass is precheck only.
  }

  for (uint8_t i = 0; (i < numberOfPasses) && (!precheckFailed); ++i) {
    bool prechecking = ((numberOfPasses > 1)
                        && (i == 0));
    uint8_t     numDecodedAttributeIds = 0;
    CborState inState;
    emCborDecodeStart(&inState, context->payload, context->payloadLength); // Reset cbor decode state on each loop.
    if (emCborDecodeMap(&inState)) {
      EmberZclAttributeId_t attributeId;
      while (emCborDecodeValue(&inState,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(attributeId),
                               (uint8_t *)&attributeId)) {
        ++numDecodedAttributeIds;

        EmberZclStatus_t status;
        const EmZclAttributeEntry_t *attribute = NULL;

        if (context->attribute != NULL) {
          //==PUT, check PUT access request map is correctly formed.
          if ((numDecodedAttributeIds == 1)
              && (context->attribute->attributeId == attributeId)) {
            attribute = context->attribute;
          }
          if (attribute == NULL) {
            status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
          }
        } else {
          //==POST
          attribute = emZclFindAttribute(&context->clusterSpec,
                                         attributeId,
                                         false); // (exclude remote).
          if (attribute == NULL) {
            status = EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
          }
        }

        if (attribute == NULL) {
          if (!emCborDecodeSkipValue(&inState)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        } else if (!emZclIsAttributeWritable(attribute)) {
          status = EMBER_ZCL_STATUS_READ_ONLY;
          if (!emCborDecodeSkipValue(&inState)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        } else {
          uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
          if (!emCborDecodeValue(&inState,
                                 emZclDirectBufferedZclipType(attribute->type),
                                 attribute->size,
                                 buffer)) {
            status = emZclCborValueReadStatusToEmberStatus(inState.readStatus);
          } else {
            status = emZclWriteAttributeEntry(context->endpoint->endpointId,
                                              attribute,
                                              buffer,
                                              attribute->size,
                                              !prechecking); // no attribute update if prechecking.
          }
        }

        if (!failureMapCreated) {
          result = status;
        }

        // BDB spec 16-07008-069, Section 3.5.2, second bullet. Skip unsupported attributes unless
        // this is undivided write
        if ((status != EMBER_ZCL_STATUS_SUCCESS)
            && (status != EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE || prechecking)) {
          if (prechecking) {
            // Set precheck failed flag but continue looping for other attribute
            // writes (rsp payload is populated with attr fail status values).
            precheckFailed = true;
          }
          if (!failureMapCreated) {
            // start failure map encoding.
            failureMapCreated = true;
            emCborEncodeIndefiniteMap(state);
          }

          // Encode the attribute write fail status into the response map e.g. {1: {"s":0x88}}
          if (!emCborEncodeKey(state, attributeId)
              || !emZclEncodeDefaultResponse(state, status)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        }
      } // while
    }
  } // for

  if ((failureMapCreated && !emCborEncodeBreak(state)) // if any failures, end cbor fail status map.
      || ((result == EMBER_ZCL_STATUS_SUCCESS)
          && !emZclEncodeDefaultResponse(state, result))) { // no failures, encode default rsp success status.
    return EMBER_ZCL_STATUS_FAILURE;
  }

  if (context->attributeQuery.undivided  // ==POST+Undivided Write.
      && (precheckFailed)) {
    // We don't have a specific ZCL_STATUS enum for precheck failed
    // so return ZCL_STATUS_NULL value here.
    return EMBER_ZCL_STATUS_NULL;
  }

  return result;
}

static bool filterAttribute(const EmZclContext_t *context,
                            FilterState *state,
                            const EmZclAttributeEntry_t *attribute)
{
  EmberZclAttributeId_t attributeId = attribute->attributeId;
  for (size_t i = 0; i < context->attributeQuery.filterCount; i++) {
    const EmZclAttributeQueryFilter_t *filter = &context->attributeQuery.filters[i];
    switch (filter->type) {
      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_ID:
        if (attributeId == filter->data.attributeId) {
          return true;
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_COUNT:
        if (attributeId >= filter->data.countData.start) {
          if (filter->data.countData.count > state->count
              && !READBIT(state->usedCounts, i)) {
            state->count = filter->data.countData.count;
          }
          SETBIT(state->usedCounts, i);
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_RANGE:
        if (attributeId >= filter->data.rangeData.start
            && attributeId <= filter->data.rangeData.end) {
          return true;
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_WILDCARD:
        return true;

      default:
        assert(false);
    }
  }

  bool ret = (context->attributeQuery.filterCount == 0 || state->count > 0);
  if (state->count > 0) {
    state->count--;
  }
  return ret;
}

EmberStatus emberZclSendAttributeRead(const EmberZclDestination_t *destination,
                                      const EmberZclClusterSpec_t *clusterSpec,
                                      const EmberZclAttributeId_t *attributeIds,
                                      size_t attributeIdsCount,
                                      const EmberZclReadAttributeResponseHandler handler)
{
  // The size of this array is the maximum number of filter range data
  // structures that could possibly be encoded into a URI string of length 64
  // that has been optimized with range formatting. This helps us protect from
  // writing off the end of the filterRangeData array in the second for loop
  // below.
  EmZclAttributeQueryFilterRangeData_t filterRangeData[19];
  EmZclAttributeQueryFilterRangeData_t *filterRangeDatum = filterRangeData;
  for (size_t i = 0; i < COUNTOF(filterRangeData); i++) {
    filterRangeData[i].start = filterRangeData[i].end = EMBER_ZCL_ATTRIBUTE_NULL;
  }
  for (size_t i = 0; i < attributeIdsCount; i++) {
    EmberZclAttributeId_t attributeId = attributeIds[i];
    if (attributeId == filterRangeDatum->end + 1) {
      filterRangeDatum->end = attributeId;
    } else {
      if (filterRangeDatum->start != EMBER_ZCL_ATTRIBUTE_NULL) {
        filterRangeDatum++;
        if (filterRangeDatum - filterRangeData > sizeof(filterRangeData)) {
          return EMBER_BAD_ARGUMENT;
        }
      }
      filterRangeDatum->start = filterRangeDatum->end = attributeId;
    }
  }

  size_t filterRangeDataCount = filterRangeDatum - filterRangeData + 1;
  uint8_t uri[MAX_ATTRIBUTE_URI_LENGTH];
  uint8_t *uriFinger = uri;
  uriFinger += emZclAttributeToUriPath(&destination->application,
                                       clusterSpec,
                                       uriFinger);
  *uriFinger++ = '?';
  *uriFinger++ = 'f';
  *uriFinger++ = '=';
  for (size_t i = 0; i < filterRangeDataCount; i++) {
    uint8_t buffer[10];
    uint8_t *bufferFinger = buffer;
    if (i != 0) {
      *bufferFinger++ = ',';
    }
    bufferFinger += emZclIntToHexString(filterRangeData[i].start,
                                        sizeof(EmberZclAttributeId_t),
                                        bufferFinger);
    if (filterRangeData[i].start != filterRangeData[i].end) {
      *bufferFinger++ = '-';
      bufferFinger += emZclIntToHexString(filterRangeData[i].end,
                                          sizeof(EmberZclAttributeId_t),
                                          bufferFinger);
    }

    size_t bufferLength = bufferFinger - buffer;
    if ((uriFinger + bufferLength + 1) - uri > sizeof(uri)) { // +1 for the nul
      return EMBER_BAD_ARGUMENT;
    }
    MEMMOVE(uriFinger, buffer, bufferLength);
    uriFinger += bufferLength;
    *uriFinger = '\0';
  }

  Response response = {
    .context = {
      .code = EMBER_COAP_CODE_EMPTY, // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : EMBER_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : EMBER_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .attributeId = EMBER_ZCL_ATTRIBUTE_NULL, // filled in when the response arrives
      .status = EMBER_ZCL_STATUS_NULL, // filled in when the response arrives
      .state = NULL, // filled in when the response arrives
    },
    .readHandler = handler,
    .writeHandler = NULL, // unused
  };

  return emZclSend(&destination->network,
                   EMBER_COAP_CODE_GET,
                   uri,
                   NULL, // payload
                   0,    // payload length
                   (handler == NULL ? NULL : readWriteResponseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

EmberStatus emberZclSendAttributeWrite(const EmberZclDestination_t *destination,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       const EmberZclAttributeWriteData_t *attributeWriteData,
                                       size_t attributeWriteDataCount,
                                       const EmberZclWriteAttributeResponseHandler handler)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteMapStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < attributeWriteDataCount; i++) {
    const EmZclAttributeEntry_t *attribute
      = emZclFindAttribute(clusterSpec,
                           attributeWriteData[i].attributeId,
                           true); // include remote
    if (attribute == NULL) {
      return EMBER_BAD_ARGUMENT;
    } else if (!emCborEncodeMapEntry(&state,
                                     attribute->attributeId,
                                     attribute->type,
                                     attribute->size,
                                     (isValueLocIndirect(attribute)
                                      ? (const uint8_t *)&attributeWriteData[i].buffer
                                      : attributeWriteData[i].buffer))) {
      return EMBER_ERR_FATAL;
    }
  }
  emCborEncodeBreak(&state);

  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  emZclAttributeToUriPath(&destination->application, clusterSpec, uriPath);

  Response response = {
    .context = {
      .code = EMBER_COAP_CODE_EMPTY, // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : EMBER_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : EMBER_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .attributeId = EMBER_ZCL_ATTRIBUTE_NULL, // filled in when the response arrives
      .status = EMBER_ZCL_STATUS_NULL, // filled in when the response arrives
      .state = NULL, // filled in when the response arrives
    },
    .readHandler = NULL, // unused
    .writeHandler = handler,
  };

  return emZclSend(&destination->network,
                   EMBER_COAP_CODE_POST,
                   uriPath,
                   buffer,
                   emCborEncodeSize(&state),
                   (handler == NULL ? NULL : readWriteResponseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

static void readWriteResponseHandler(EmberCoapStatus coapStatus,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  const Response *response = info->applicationData;
  bool isRead = (*response->readHandler != NULL);
  bool isWrite = (*response->writeHandler != NULL);
  assert(isRead != isWrite);
  EmberZclMessageStatus_t status = (EmberZclMessageStatus_t) coapStatus;

  emZclCoapStatusHandler(coapStatus, info);
  ((Response *)response)->context.code = code;

  // TODO: What should happen if the overall payload is missing or malformed?
  // Note that this is a separate issue from how missing or malformed responses
  // from the individual endpoints should be handled.
  if (status == EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE) {
    // Note- all coap rsp codes are now checked here.
    CborState state;
    ((Response *)response)->context.state = &state;
    emCborDecodeStart(&state, payload, payloadLength);
    if (response->context.groupId == EMBER_ZCL_GROUP_NULL) {
      (isRead ? handleRead : handleWrite)(status, response);
      return;
    } else if (emCborDecodeMap(&state)) {
      while (emCborDecodeValue(&state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(response->context.endpointId),
                               (uint8_t *)&response->context.endpointId)) {
        (isRead ? handleRead : handleWrite)(status, response);
      }
      return;
    }
  }

  if (isRead) {
    (*response->readHandler)(status, &response->context, NULL, 0);
  } else {
    (*response->writeHandler)(status, &response->context);
  }
}

static void handleRead(EmberZclMessageStatus_t status,
                       const Response *response)
{
  // TODO: If we expect an attribute but it is missing, or it is present but
  // malformed, would should we do?
  if (emCborDecodeMap(response->context.state)) {
    while (emCborDecodeValue(response->context.state,
                             EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(response->context.attributeId),
                             (uint8_t *)&response->context.attributeId)) {
      const EmZclAttributeEntry_t *attribute
        = emZclFindAttribute(response->context.clusterSpec,
                             response->context.attributeId,
                             true); // include remote
      if (attribute == NULL) {
        emCborDecodeMap(response->context.state);
        emCborDecodeSkipValue(response->context.state); // key
        emCborDecodeSkipValue(response->context.state); // value
        emCborDecodeSkipValue(response->context.state); // break
      } else {
        uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
        uint8_t key[2]; // 'v' or 's' plus a NUL
        if (emCborDecodeMap(response->context.state)
            && emCborDecodeValue(response->context.state,
                                 EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                                 sizeof(key),
                                 key)
            && emCborDecodeValue(response->context.state,
                                 emZclDirectBufferedZclipType(attribute->type),
                                 attribute->size,
                                 buffer)) {
          ((Response *)response)->context.status = (key[0] == 'v'
                                                    ? EMBER_ZCL_STATUS_SUCCESS
                                                    : buffer[0]);
          (*response->readHandler)(status,
                                   &response->context,
                                   buffer,
                                   attribute->size);
        }
        emCborDecodeSkipValue(response->context.state); // break
      }
    }
  }
  (*response->readHandler)(status, &response->context, NULL, 0);
}

static void handleWrite(EmberZclMessageStatus_t status,
                        const Response *response)
{
  // Handle attribute write response.

  (*response->writeHandler)(status, &response->context);
}

static EmberZclStatus_t writeAttribute(EmberZclEndpointIndex_t index,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       const void *data,
                                       size_t dataLength)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_SUCCESS;
  EmZclAttributeMask_t storageType
    = READBITS(attribute->mask, EM_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK);

  assert(emZclIsAttributeLocal(attribute));

  switch (storageType) {
    case EM_ZCL_ATTRIBUTE_STORAGE_TYPE_EXTERNAL:
      status = emberZclWriteExternalAttributeCallback(endpointId,
                                                      attribute->clusterSpec,
                                                      attribute->attributeId,
                                                      data,
                                                      dataLength);
      break;

    case EM_ZCL_ATTRIBUTE_STORAGE_TYPE_RAM:
      MEMMOVE(attributeDataLocation(index, attribute), data, dataLength);
      break;

    default:
      assert(false);
  }

  return status;
}

static void callPostAttributeChange(EmberZclEndpointId_t endpointId,
                                    const EmZclAttributeEntry_t *attribute,
                                    const void *data,
                                    size_t dataLength)
{
  if (emZclIsAttributeSingleton(attribute)) {
    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                  attribute->clusterSpec)) {
        emZclPostAttributeChange(emZclEndpointTable[i].endpointId,
                                 attribute->clusterSpec,
                                 attribute->attributeId,
                                 data,
                                 dataLength);
      }
    }
  } else {
    emZclPostAttributeChange(endpointId,
                             attribute->clusterSpec,
                             attribute->attributeId,
                             data,
                             dataLength);
  }
}

static bool callPreAttributeChange(EmberZclEndpointId_t endpointId,
                                   const EmZclAttributeEntry_t *attribute,
                                   const void *data,
                                   size_t dataLength)
{
  if (emZclIsAttributeSingleton(attribute)) {
    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                  attribute->clusterSpec)
          && !emZclPreAttributeChange(emZclEndpointTable[i].endpointId,
                                      attribute->clusterSpec,
                                      attribute->attributeId,
                                      data,
                                      dataLength)) {
        return false;
      }
    }
    return true;
  } else {
    return emZclPreAttributeChange(endpointId,
                                   attribute->clusterSpec,
                                   attribute->attributeId,
                                   data,
                                   dataLength);
  }
}

static size_t tokenize(const EmZclContext_t *context,
                       void *skipData,
                       uint8_t depth,
                       const uint8_t **tokens,
                       size_t *tokenLengths)
{
  uint8_t skipLength = strlen((const char *)skipData);
  const uint8_t *bytes = context->uriQuery[depth] + skipLength;
  size_t bytesLength = context->uriQueryLength[depth] - skipLength;
  size_t count = 0;
  const uint8_t *next = NULL, *end = bytes + bytesLength;

  for (; bytes < end; bytes = next + 1) {
    next = memchr(bytes, ',', end - bytes);
    if (next == NULL) {
      next = end;
    } else if (next == bytes || next == end - 1) {
      return 0; // 0 length strings are no good.
    }

    tokens[count] = bytes;
    tokenLengths[count] = next - bytes;
    count++;
  }

  return count;
}

static void convertBufferToTwosComplement(uint8_t *buffer, size_t size)
{
  // Converts the input buffer to 2S complement format.

  // Invert all bytes in buffer.
  for (uint8_t i = 0; i < size; i++) {
    buffer[i] = ~buffer[i];
  }

  // Add +1 to number in buffer.
  for (uint8_t i = 0; i < size; i++) {
    if (buffer[i] < 0xFF) {
      buffer[i] += 1;
      break;
    } else {
      buffer[i] = 0;
    }
  }
}

// .../a?meta=$base&f=xxxx
// .../a?meta=$acc&f=xxxx
// .../a?meta=*&f=xxxx
bool emZclAttributeUriMetadataQueryParse(EmZclContext_t *context,
                                         void *data,
                                         uint8_t depth)
{
  context->attributeQuery.metadata = EM_ZCL_METADATA_NONE;
  const uint8_t *finger = context->uriQuery[depth];

  // Check for metadata Query prefix-
  // "meta="
  size_t metaPrefixLength = strlen(EM_ZCL_URI_METADATA_QUERY);
  if (MEMCOMPARE(finger, EM_ZCL_URI_METADATA_QUERY, metaPrefixLength) == 0) {
    // Check for any of the supported metadata query types.
    //TODO- Add support for other metadata types here.
    finger += metaPrefixLength;
    if (MEMCOMPARE(finger, EM_ZCL_URI_METADATA_WILDCARD, strlen(EM_ZCL_URI_METADATA_WILDCARD)) == 0) {
      context->attributeQuery.metadata = EM_ZCL_METADATA_WILDCARD;
    } else if (MEMCOMPARE(finger, EM_ZCL_URI_METADATA_BASE, strlen(EM_ZCL_URI_METADATA_BASE)) == 0) {
      context->attributeQuery.metadata = EM_ZCL_METADATA_BASE;
    } else if (MEMCOMPARE(finger, EM_ZCL_URI_METADATA_ACCESS, strlen(EM_ZCL_URI_METADATA_ACCESS)) == 0) {
      context->attributeQuery.metadata = EM_ZCL_METADATA_ACCESS;
    } else {
      context->attributeQuery.metadata = EM_ZCL_METADATA_NOT_SUPPORTED;
    }
  }

  return true; // Always returns true.
}

// .../a?f=
bool emZclAttributeUriQueryFilterParse(EmZclContext_t *context,
                                       void *data,
                                       uint8_t depth)
{
  const uint8_t *tokens[MAX_URI_QUERY_SEGMENTS];
  size_t tokenLengths[MAX_URI_QUERY_SEGMENTS];
  size_t tokenCount = tokenize(context, data, depth, tokens, tokenLengths);
  if (tokenCount == 0) {
    return false;
  }

  for (size_t i = 0; i < tokenCount; i++) {
    EmZclAttributeQueryFilter_t *filter
      = &context->attributeQuery.filters[context->attributeQuery.filterCount++];

    if (tokenLengths[i] == 1 && tokens[i][0] == '*') {
      // f=*
      filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_WILDCARD;
    } else {
      const uint8_t *operator = NULL;
      const uint8_t *now = tokens[i];
      const uint8_t *next = now + tokenLengths[i];
      uintmax_t first, second, length;
      if (((operator = memchr(now, '-', tokenLengths[i])) != NULL
           || (operator = memchr(now, '+', tokenLengths[i])) != NULL)
          && (length = operator - now) > 0
          && length <= sizeof(EmberZclAttributeId_t) * 2 // nibbles
          && emHexStringToInt(now, length, &first)
          && (length = next - operator - 1) > 0
          && length <= sizeof(EmberZclAttributeId_t) * 2 // nibbles
          && emHexStringToInt(operator + 1, length, &second)) {
        // f=1-2
        // f=3+4
        if (*operator == '-') {
          filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_RANGE;
          filter->data.rangeData.start = first;
          filter->data.rangeData.end = second;
          if (filter->data.rangeData.end <= filter->data.rangeData.start) {
            return false;
          }
        } else {
          filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_COUNT;
          filter->data.countData.start = first;
          filter->data.countData.count = second;
        }
      } else if (tokenLengths[i] <= sizeof(EmberZclAttributeId_t) * 2
                 && emHexStringToInt(now, tokenLengths[i], &first)) {
        // f=5
        filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_ID;
        filter->data.attributeId = first;
      } else {
        return false;
      }
    }
  }

  // Finally, check that there are no duplicate filters.
  for (size_t j = 0; j < context->attributeQuery.filterCount; j++) {
    EmZclAttributeQueryFilter_t *filter1 = &context->attributeQuery.filters[j];
    for (size_t k = j + 1; k < context->attributeQuery.filterCount; k++) {
      EmZclAttributeQueryFilter_t *filter2 = &context->attributeQuery.filters[k];
      if (MEMCOMPARE(filter1, filter2, sizeof(EmZclAttributeQueryFilter_t)) == 0) {
        return false;
      }
    }
  }

  return true;
}

// ...a/?u
bool emZclAttributeUriQueryUndividedParse(EmZclContext_t *context,
                                          void *data,
                                          uint8_t depth)
{
  // Only accept 'u'.
  return (context->attributeQuery.undivided
            = (context->uriQueryLength[depth] == 1));
}

// zcl/[eg]/XX/<cluster>/a:
//   GET:
//     w/ query: read multiple attributes.
//     w/o query: list attributes in cluster.
//   POST:
//     w/ query: update attributes undivided.
//     w/o query: write multiple attributes.
//   OTHER: not allowed.
void emZclUriClusterAttributeHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      status = emZclMultiEndpointDispatch(context,
                                          getAttributeIdsHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespond205ContentCborState(context->info, &state);
          break;
        case EMBER_ZCL_STATUS_NOT_FOUND:
          emZclRespond404NotFound(context->info);
          break;
        default:
          emZclRespond500InternalServerError(context->info);
          break;
      }
      break;

    case EMBER_COAP_CODE_POST:
      status = emZclMultiEndpointDispatch(context,
                                          updateAttributeHandler,
                                          &state,
                                          NULL);

      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          if (context->groupId == EMBER_ZCL_GROUP_NULL) {
            emZclRespond204Changed(context->info);
          } else {
            // 16-07008-069 Section 3.16.6 is not 100% clear on whether we should send an empty map or No Content
            // but No Content seems more logical and that's what is specified in 3.16.5 and test 4.10.
            emZclRespond204Changed(context->info);
          }
          break;

        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond500InternalServerError(context->info);
          break;
        case EMBER_ZCL_STATUS_MALFORMED_COMMAND:
          emZclRespond400BadRequestWithStatus(context->info, status);
          break;
        case EMBER_ZCL_STATUS_NULL:
          emZclRespond412PreconditionFailedCborState(context->info, &state); // Rsp payload is a map of failed writes.
          break;
        default:
          emZclRespond204ChangedCborState(context->info, &state); // Rsp payload is a map of all failed writes (attId/status).
          break;
      }
      break;

    default:
      assert(false);
  }
}

// zcl/[eg]/XX/<cluster>/a/XXXX:
//   GET: read one attribute,
//        or one attribute metadata item (e.g. ../a/XXXX/$base)
//   PUT: write one attribute.
//   OTHER: not allowed.
void emZclUriClusterAttributeIdHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      status = emZclMultiEndpointDispatch(context,
                                          getAttributeHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespond205ContentCborState(context->info, &state);
          break;
        default:
          emZclRespond500InternalServerError(context->info);
          break;
      }
      break;

    case EMBER_COAP_CODE_PUT:
      status = emZclMultiEndpointDispatch(context,
                                          updateAttributeHandler,
                                          &state,
                                          NULL);

      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespond204Changed(context->info); // BDB spec 16-07008-069, Section 3.6.5: No payload
          break;
        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond500InternalServerError(context->info);
          break;
        case EMBER_ZCL_STATUS_MALFORMED_COMMAND:
          emZclRespond400BadRequestWithStatus(context->info, status);   // BDB spec 16-07008-069, Section 3.5.2, 3rd bullet
          break;
        default:
          emZclRespond400BadRequestCborState(context->info, &state);  // Rsp payload is a map of failed writes (attId/status)

          break;
      }
      break;

    default:
      assert(false);
  }
}

bool emZclGetAbsDifference(uint8_t *bufferA,
                           uint8_t *bufferB,
                           uint8_t *diffBuffer,
                           size_t size)
{
  // -Assumes that bufferA and bufferB represent valid sign-extended numeric
  // integer type attribute values.
  // -The resulting abs (i.e. +ve) difference is placed in the diffBuffer.
  // -Returns TRUE if the difference is negative.

  // Convert the bufferB value to 2S complement (i.e. -bufferB).
  convertBufferToTwosComplement(bufferB, size);

  // Do a bytewise add of bufferA to -bufferB. The result in the
  // diffBuffer is the signed difference of bufferA and bufferB.
  int8u carry = 0;
  for (size_t i = 0; i < size; i++) {
    uint16_t tmp = (bufferA[i] + bufferB[i] + carry);
    if (tmp >= 256) {
      diffBuffer[i] = tmp - 256;
      carry = 1;
    } else {
      diffBuffer[i] = tmp;
      carry = 0;
    }
  }

  // Restore bufferB to its original contents
  convertBufferToTwosComplement(bufferB, size);

  // Check if the value in the diffBuffer is negative.
  bool diffIsNegative = (diffBuffer[size - 1] & 0x80);

  if (diffIsNegative) {
    // Convert the diff buffer value to a positive number by running
    // a 2S complement on it.
    convertBufferToTwosComplement(diffBuffer, size);
  }

  return diffIsNegative;
}

void emZclSignExtendAttributeBuffer(uint8_t *outBuffer,
                                    size_t outBufferLength,
                                    const uint8_t *inBuffer,
                                    size_t inBufferLength,
                                    uint8_t attributeType)
{
  if ((attributeType != EMBER_ZCLIP_TYPE_INTEGER)
      && (attributeType != EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER)
      && (attributeType != EMBER_ZCLIP_TYPE_BOOLEAN)) {
    assert(false);
    return;
  }
  if (inBufferLength >= outBufferLength) {
    assert(false); // outBuffer length must be greater.
    return;
  }

  // Copy input buffer bytes to output buffer (convert to LE order).
  #if BIGENDIAN_CPU
  emberReverseMemCopy(outBuffer, inBuffer, inBufferLength);
  #else
  MEMCOPY(outBuffer, inBuffer, inBufferLength);
  #endif

  // Set all the remaining high end bytes of the outputBuffer to
  // the sign-extension byte value.
  uint8_t signExtensionByteValue = 0x00;  // default sign-extension value for positive numbers.
  uint8_t MSbyteOffset = inBufferLength - 1;
  if ((attributeType == EMBER_ZCLIP_TYPE_INTEGER)
      && (outBuffer[MSbyteOffset] & 0x80)) {
    signExtensionByteValue = 0xFF; // Sign bit is set so use sign-extension value for negative number.
  }
  MEMSET(&outBuffer[MSbyteOffset + 1],
         signExtensionByteValue,
         outBufferLength - inBufferLength);
}
