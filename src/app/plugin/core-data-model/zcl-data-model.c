/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *    @file
 *      This file provides the attribute database implementation
 *      used by the CHIP ZCL Application Layer
 *
 */

#include "chip-zcl.h"

#include "gen.h"

//------------------------------------------------------------------------------
// Globals
// This is not declared CONST in order to handle dynamic endpoint information
// retrieved from tokens.
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

uint8_t chipZclEndpointCounter = 0;
ChipZclDefinedEndpoint chipZclEndpointArray[FIXED_ENDPOINT_COUNT];

uint8_t singletonAttributeData[ACTUAL_SINGLETONS_SIZE];

const uint8_t generatedDefaults[] = GENERATED_DEFAULTS;
#ifdef GENERATED_MIN_MAX_DEFAULTS
const ChipZclAttributeMinMaxValue minMaxDefaults[] = GENERATED_MIN_MAX_DEFAULTS;
#endif
const ChipZclAttributeMetadata generatedAttributes[]      = GENERATED_ATTRIBUTES;
const ChipZclCluster generatedClusters[]                  = GENERATED_CLUSTERS;
const ChipZclEndpointType generatedChipZclEndpointTypes[] = GENERATED_ENDPOINT_TYPES;

const ChipZclManufacturerCodeEntry clusterManufacturerCodes[]   = GENERATED_CLUSTER_MANUFACTURER_CODES;
const uint16_t clusterManufacturerCodeCount                     = GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT;
const ChipZclManufacturerCodeEntry attributeManufacturerCodes[] = GENERATED_ATTRIBUTE_MANUFACTURER_CODES;
const uint16_t attributeManufacturerCodeCount                   = GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT;

uint8_t chipZclEndpointCount(void)
{
    return chipZclEndpointCounter;
}

uint8_t chipZclStringLength(const uint8_t * buffer)
{
    return (buffer[0] == 0xFF ? 0 : buffer[0]);
}
uint16_t chipZclLongStringLength(const uint8_t * buffer)
{
    uint16_t length = buffer[0] + (buffer[1] << 8);
    return (length == 0xFFFF ? 0 : length);
}

void chipZclCopyString(uint8_t * dest, uint8_t * src, uint8_t size)
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
        uint8_t length = chipZclStringLength(src);
        if (size < length)
        {
            length = size;
        }
        MEMMOVE(dest + 1, src + 1, length);
        dest[0] = length;
    }
}

static uint8_t * chipZclSingletonAttributeLocation(ChipZclAttributeMetadata * am)
{
    ChipZclAttributeMetadata * m = (ChipZclAttributeMetadata *) &(generatedAttributes[0]);
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

void chipZclCopyLongString(uint8_t * dest, uint8_t * src, uint16_t size)
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
        uint16_t length = chipZclLongStringLength(src);
        if (size < length)
        {
            length = size;
        }
        MEMMOVE(dest + 2, src + 2, length);
        dest[0] = LOW_BYTE(length);
        dest[1] = HIGH_BYTE(length);
    }
}
// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
int8_t chipZclCompareValues(uint8_t * val1, uint8_t * val2, uint8_t len, bool signedNumber)
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
                j = (val1 == NULL ? 0 : (IS_BIG_ENDIAN() ? val1[i] : val1[(len - 1) - i]));
                accum1 |= j << (8 * (len - 1 - i));

                k = (IS_BIG_ENDIAN() ? val2[i] : val2[(len - 1) - i]);
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
            j = (val1 == NULL ? 0 : (IS_BIG_ENDIAN() ? val1[i] : val1[(len - 1) - i]));
            k = (IS_BIG_ENDIAN() ? val2[i] : val2[(len - 1) - i]);

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

bool chipZclIsTypeSigned(ChipZclAttributeType dataType)
{
    return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

bool chipZclIsStringAttributeType(ChipZclAttributeType attributeType)
{
    return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool chipZclIsLongStringAttributeType(ChipZclAttributeType attributeType)
{
    return (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

// This function does mem copy, but smartly, which means that if the type is a
// string, it will copy as much as it can.
// If src == NULL, then this method will set memory to zeroes
static ChipZclStatus_t chipZclTypeSensitiveMemCopy(uint8_t * dest, uint8_t * src, ChipZclAttributeMetadata * am, bool write,
                                                   uint16_t readLength)
{
    ChipZclAttributeType attributeType = am->attributeType;
    uint16_t size                      = (readLength == 0) ? am->size : readLength;

    if (chipZclIsStringAttributeType(attributeType))
    {
        chipZclCopyString(dest, src, size - 1);
    }
    else if (chipZclIsLongStringAttributeType(attributeType))
    {
        chipZclCopyLongString(dest, src, size - 2);
    }
    else
    {
        if (!write && readLength != 0 && readLength < am->size)
        {
            return CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
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
    return CHIP_ZCL_STATUS_SUCCESS;
}

bool chipZclEndpointIndexIsEnabled(uint8_t index)
{
    return (chipZclEndpointArray[index].bitmask & CHIP_ZCL_ENDPOINT_ENABLED);
}

// Returns the manufacturer code or ::CHIP_ZCL_NULL_MANUFACTURER_CODE if none
// could be found.
static uint16_t chipZclGetManufacturerCode(ChipZclManufacturerCodeEntry * codes, uint16_t codeTableSize, uint16_t tableIndex)
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
    return CHIP_ZCL_NULL_MANUFACTURER_CODE;
}

uint16_t chipZclGetManufacturerCodeForCluster(ChipZclCluster * cluster)
{
    return chipZclGetManufacturerCode((ChipZclManufacturerCodeEntry *) clusterManufacturerCodes, clusterManufacturerCodeCount,
                                      (cluster - generatedClusters));
}

/**
 * @brief Matches a cluster based on cluster id, direction and manufacturer code.
 *   This function assumes that the passed cluster's endpoint already
 *   matches the endpoint of the ChipZclAttributeSearchRecord.
 *
 * Cluster's match if:
 *   1. Cluster ids match AND
 *   2. Cluster directions match as defined by cluster->mask
 *        and attRecord->clusterMask AND
 *   3. If the clusters are mf specific, their mfg codes match.
 */
bool chipZclMatchCluster(ChipZclCluster * cluster, ChipZclAttributeSearchRecord * attRecord)
{
    return (cluster->clusterId == attRecord->clusterId && cluster->mask & attRecord->clusterMask &&
            (!chipZclClusterIsManufacturerSpecific(cluster) ||
             (chipZclGetManufacturerCodeForCluster(cluster) == attRecord->manufacturerCode)));
}

// This function basically wraps chipZclGetManufacturerCode with the parameters
// associating an attributes metadata with its code.
static uint16_t chipZclGetMfgCode(ChipZclAttributeMetadata * metadata)
{
    return chipZclGetManufacturerCode((ChipZclManufacturerCodeEntry *) attributeManufacturerCodes, attributeManufacturerCodeCount,
                                      (metadata - generatedAttributes));
}

uint16_t chipZclGetManufacturerCodeForAttribute(ChipZclCluster * cluster, ChipZclAttributeMetadata * attMetaData)
{
    return (chipZclClusterIsManufacturerSpecific(cluster) ? chipZclGetManufacturerCodeForCluster(cluster)
                                                          : chipZclGetMfgCode(attMetaData));
}

// 'data' argument may be null, since we changed the ptrToDefaultValue
// to be null instead of pointing to all zeroes.
// This function has to be able to deal with that.
void chipZclSaveAttributeToToken(uint8_t * data, uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeMetadata * metadata)
{
    // Get out of here if this attribute doesn't have a token.
    if (!chipZclAttributeIsTokenized(metadata))
    {
        return;
    }

#ifdef GENERATED_TOKEN_SAVER
    GENERATED_TOKEN_SAVER;
#endif
}

/**
 * @brief Matches an attribute based on attribute id and manufacturer code.
 *   This function assumes that the passed cluster already matches the
 *   clusterId, direction and mf specificity of the passed
 *   ChipZclAttrib_tuteSearchRecord.
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
bool chipZclMatchAttribute(ChipZclCluster * cluster, ChipZclAttributeMetadata * am, ChipZclAttributeSearchRecord * attRecord)
{
    return (am->attributeId == attRecord->attributeId &&
            (chipZclClusterIsManufacturerSpecific(cluster) ||
             (chipZclGetManufacturerCodeForAttribute(cluster, am) == attRecord->manufacturerCode)));
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
static ChipZclStatus_t chipZclInternalReadOrWriteAttribute(ChipZclAttributeSearchRecord * attRecord,
                                                           ChipZclAttributeMetadata ** metadata, uint8_t * buffer,
                                                           uint16_t readLength, bool write)
{
    uint8_t i;
    uint16_t attributeOffsetIndex = 0;

    for (i = 0; i < chipZclEndpointCount(); i++)
    {
        if (chipZclEndpointArray[i].endpoint == attRecord->endpoint)
        {
            ChipZclEndpointType * endpointType = chipZclEndpointArray[i].endpointType;
            uint8_t clusterIndex;
            if (!chipZclEndpointIndexIsEnabled(i))
            {
                continue;
            }
            for (clusterIndex = 0; clusterIndex < endpointType->clusterCount; clusterIndex++)
            {
                ChipZclCluster * cluster = &(endpointType->cluster[clusterIndex]);
                if (chipZclMatchCluster(cluster, attRecord))
                { // Got the cluster
                    uint16_t attrIndex;
                    for (attrIndex = 0; attrIndex < cluster->attributeCount; attrIndex++)
                    {
                        ChipZclAttributeMetadata * am = &(cluster->attributes[attrIndex]);
                        if (chipZclMatchAttribute(cluster, am, attRecord))
                        { // Got the attribute
                            // If passed metadata location is not null, populate
                            if (metadata != NULL)
                            {
                                *metadata = am;
                            }

                            {
                                uint8_t * attributeLocation =
                                    (am->mask & ATTRIBUTE_MASK_SINGLETON ? chipZclSingletonAttributeLocation(am)
                                                                         : attributeData + attributeOffsetIndex);
                                uint8_t *src, *dst;
                                if (write)
                                {
                                    src = buffer;
                                    dst = attributeLocation;
                                    if (!chipZclAttributeWriteAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                             chipZclGetManufacturerCodeForAttribute(cluster, am),
                                                                             am->attributeId))
                                    {
                                        return CHIP_ZCL_STATUS_NOT_AUTHORIZED;
                                    }
                                }
                                else
                                {
                                    if (buffer == NULL)
                                    {
                                        return CHIP_ZCL_STATUS_SUCCESS;
                                    }

                                    src = attributeLocation;
                                    dst = buffer;
                                    if (!chipZclAttributeReadAccessCallback(attRecord->endpoint, attRecord->clusterId,
                                                                            chipZclGetManufacturerCodeForAttribute(cluster, am),
                                                                            am->attributeId))
                                    {
                                        return CHIP_ZCL_STATUS_NOT_AUTHORIZED;
                                    }
                                }

                                return (am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE
                                            ? (write) ? chipZclExternalAttributeWriteCallback(
                                                            attRecord->endpoint, attRecord->clusterId, am,
                                                            chipZclGetManufacturerCodeForAttribute(cluster, am), buffer)
                                                      : chipZclExternalAttributeReadCallback(
                                                            attRecord->endpoint, attRecord->clusterId, am,
                                                            chipZclGetManufacturerCodeForAttribute(cluster, am), buffer,
                                                            chipZclAttributeSize(am))
                                            : chipZclTypeSensitiveMemCopy(dst, src, am, write, readLength));
                            }
                        }
                        else
                        { // Not the attribute we are looking for
                            // Increase the index if attribute is not externally stored
                            if (!(am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) && !(am->mask & ATTRIBUTE_MASK_SINGLETON))
                            {
                                attributeOffsetIndex += chipZclAttributeSize(am);
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
            attributeOffsetIndex += chipZclEndpointArray[i].endpointType->endpointSize;
        }
    }
    return CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // Sorry, attribute was not found.
}

// Returns the pointer to metadata, or null if it is not found
ChipZclStatus_t chipZclLocateAttributeMetadata(ChipZclEndpointId_t endpoint, ChipZclClusterId clusterId,
                                               ChipZclAttributeId attributeId, uint8_t mask, uint16_t manufacturerCode,
                                               ChipZclAttributeMetadata ** metadata)
{
    ChipZclAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = clusterId;
    record.clusterMask      = mask;
    record.attributeId      = attributeId;
    record.manufacturerCode = manufacturerCode;
    *metadata               = NULL;
    return chipZclInternalReadOrWriteAttribute(&record,  // search record
                                               metadata, // where to write the result.
                                               NULL,     // buffer
                                               0,        // buffer size
                                               false);   // write?
}

// writes an attribute (identified by clusterID and attrID to the given value.
// this returns:
// - CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE: if attribute isnt supported by the device (the
//           device is not found in the attribute table)
// - CHIP_ZCL_STATUS_INVALID_DATA_TYPE: if the data type passed in doesnt match the type
//           stored in the attribute table
// - CHIP_ZCL_STATUS_READ_ONLY: if the attribute isnt writable
// - CHIP_ZCL_STATUS_INVALID_VALUE: if the value is set out of the allowable range for
//           the attribute
// - CHIP_ZCL_STATUS_SUCCESS: if the attribute was found and successfully written
//
// if true is passed in for overrideReadOnlyAndDataType then the data type is
// not checked and the read-only flag is ignored. This mode is meant for
// testing or setting the initial value of the attribute on the device.
//
// if true is passed for justTest, then the type is not written but all
// checks are done to see if the type could be written
// reads the attribute specified, returns false if the attribute is not in
// the table or the data is too large, returns true and writes to dataPtr
// if the attribute is supported and the readLength specified is less than
// the length of the data.
static ChipZclStatus_t chipZclInternalWriteAttribute(ChipZclEndpointId_t endpoint, ChipZclClusterId cluster,
                                                     ChipZclAttributeId attributeID, uint8_t mask, uint16_t manufacturerCode,
                                                     uint8_t * data, ChipZclAttributeType dataType,
                                                     bool overrideReadOnlyAndDataType, bool justTest)
{
    ChipZclAttributeMetadata * metadata = NULL;
    ChipZclAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = cluster;
    record.clusterMask      = mask;
    record.attributeId      = attributeID;
    record.manufacturerCode = manufacturerCode;
    chipZclInternalReadOrWriteAttribute(&record, &metadata,
                                        NULL,   // buffer
                                        0,      // buffer size
                                        false); // write?

    // if we dont support that attribute
    if (metadata == NULL)
    {
        chipZclCorePrintln("%pep %x clus %2x attr %2x not supported", "WRITE ERR: ", endpoint, cluster, attributeID);
        return CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    // if the data type specified by the caller is incorrect
    if (!(overrideReadOnlyAndDataType))
    {
        if (dataType != metadata->attributeType)
        {
            chipZclCorePrintln("%pinvalid data type", "WRITE ERR: ");
            return CHIP_ZCL_STATUS_INVALID_DATA_TYPE;
        }

        if (chipZclAttributeIsReadOnly(metadata))
        {
            chipZclCorePrintln("%pattr not writable", "WRITE ERR: ");
            return CHIP_ZCL_STATUS_READ_ONLY;
        }
    }

    // if the value the attribute is being set to is out of range
    // return CHIP_ZCL_STATUS_INVALID_VALUE
    if ((metadata->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U)
    {
        ChipZclDefaultAttributeValue minv = metadata->defaultValue.ptrToMinMaxValue->minValue;
        ChipZclDefaultAttributeValue maxv = metadata->defaultValue.ptrToMinMaxValue->maxValue;
        bool isAttributeSigned            = chipZclIsTypeSigned(metadata->attributeType);
        uint8_t dataLen                   = chipZclAttributeSize(metadata);
        if (dataLen <= 2)
        {
            int8_t minR, maxR;
            uint8_t * minI = (uint8_t *) &(minv.defaultValue);
            uint8_t * maxI = (uint8_t *) &(maxv.defaultValue);
// On big endian cpu with length 1 only the second byte counts
#if (BIGENDIAN_CPU)
            if (dataLen == 1)
            {
                minI++;
                maxI++;
            }
#endif // BIGENDIAN_CPU
            minR = chipZclCompareValues(minI, data, dataLen, isAttributeSigned);
            maxR = chipZclCompareValues(maxI, data, dataLen, isAttributeSigned);
            if ((minR == 1) || (maxR == -1))
            {
                return CHIP_ZCL_STATUS_INVALID_VALUE;
            }
        }
        else
        {
            if ((chipZclCompareValues(minv.ptrToDefaultValue, data, dataLen, isAttributeSigned) == 1) ||
                (chipZclCompareValues(maxv.ptrToDefaultValue, data, dataLen, isAttributeSigned) == -1))
            {
                return CHIP_ZCL_STATUS_INVALID_VALUE;
            }
        }
    }

    // write the data unless this is only a test
    if (!justTest)
    {
        // Pre write attribute callback for all attribute changes,
        // regardless of cluster.
        ChipZclStatus_t status = chipZclPreAttributeChangeCallback(endpoint, cluster, attributeID, mask, manufacturerCode, dataType,
                                                                   chipZclAttributeSize(metadata), data);
        if (status != CHIP_ZCL_STATUS_SUCCESS)
        {
            return status;
        }

        // write the attribute
        status = chipZclInternalReadOrWriteAttribute(&record,
                                                     NULL, // metadata
                                                     data,
                                                     0,     // buffer size - unused
                                                     true); // write?

        if (status != CHIP_ZCL_STATUS_SUCCESS)
        {
            return status;
        }

        // Save the attribute to token if needed
        // Function itself will weed out tokens that are not tokenized.
        chipZclSaveAttributeToToken(data, endpoint, cluster, metadata);

        // Post write attribute callback for all attributes changes, regardless
        // of cluster.
        chipZclPostAttributeChangeCallback(endpoint, cluster, attributeID, mask, manufacturerCode, dataType,
                                           chipZclAttributeSize(metadata), data);
    }
    else
    {
        // bug: 11618, we are not handling properly external attributes
        // in this case... We need to do something. We don't really
        // know if it will succeed.
        chipZclCorePrintln("WRITE: no write, just a test");
    }

    return CHIP_ZCL_STATUS_SUCCESS;
}

ChipZclStatus_t chipZclWriteAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                      ChipZclAttributeId_t attributeId, const void * buffer, size_t bufferLength)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

// If dataPtr is NULL, no data is copied to the caller.
// readLength should be 0 in that case.
//
// This function populates the dataPtr with the data from the attribute, and dataType (if not-NULL) with the correct type.
static ChipZclStatus_t chipZclInternalReadAttribute(ChipZclEndpointId_t endpoint, ChipZclClusterId cluster,
                                                    ChipZclAttributeId attributeID, uint8_t mask, uint16_t manufacturerCode,
                                                    uint8_t * dataPtr, uint16_t readLength, ChipZclAttributeType * dataType)
{
    ChipZclAttributeMetadata * metadata = NULL;

    ChipZclAttributeSearchRecord record;
    ChipZclStatus_t status;
    record.endpoint         = endpoint;
    record.clusterId        = cluster;
    record.clusterMask      = mask;
    record.attributeId      = attributeID;
    record.manufacturerCode = manufacturerCode;
    status                  = chipZclInternalReadOrWriteAttribute(&record, &metadata, dataPtr, readLength,
                                                 false); // write?

    if (status == CHIP_ZCL_STATUS_SUCCESS)
    {
        // It worked!  If the user asked for the type, set it before returning.
        if (dataType != NULL)
        {
            (*dataType) = metadata->attributeType;
        }
    }
    else
    { // failed, print debug info
        if (status == CHIP_ZCL_STATUS_INSUFFICIENT_SPACE)
        {
            chipZclCorePrintln("READ: attribute size too large for caller");
        }
    }

    return status;
}

ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                     ChipZclAttributeId_t attributeId, void * buffer, size_t bufferLength)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

// Initial configuration
void chipZclEndpointInit(void)
{
    uint8_t ep;

    uint8_t fixedEndpoints[]            = FIXED_ENDPOINT_ARRAY;
    uint16_t fixedProfileIds[]          = FIXED_PROFILE_IDS;
    uint16_t fixedDeviceIds[]           = FIXED_DEVICE_IDS;
    uint8_t fixedDeviceVersions[]       = FIXED_DEVICE_VERSIONS;
    uint8_t fixedChipZclEndpointTypes[] = FIXED_ENDPOINT_TYPES;
    uint8_t fixedNetworks[]             = FIXED_NETWORKS;

    chipZclEndpointCounter = FIXED_ENDPOINT_COUNT;
    for (ep = 0; ep < FIXED_ENDPOINT_COUNT; ep++)
    {
        chipZclEndpointArray[ep].endpoint      = fixedEndpoints[ep];
        chipZclEndpointArray[ep].profileId     = fixedProfileIds[ep];
        chipZclEndpointArray[ep].deviceId      = fixedDeviceIds[ep];
        chipZclEndpointArray[ep].deviceVersion = fixedDeviceVersions[ep];
        chipZclEndpointArray[ep].endpointType =
            (ChipZclEndpointType *) &(generatedChipZclEndpointTypes[fixedChipZclEndpointTypes[ep]]);
        chipZclEndpointArray[ep].networkIndex = fixedNetworks[ep];
        chipZclEndpointArray[ep].bitmask      = CHIP_ZCL_ENDPOINT_ENABLED;
    }
}
