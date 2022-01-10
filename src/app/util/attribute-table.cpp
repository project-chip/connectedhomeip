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
 * @brief This file contains the code to manipulate
 *the Smart Energy attribute table.  This handles
 *external calls to read/write the table, as well as
 *internal ones.
 *******************************************************************************
 ******************************************************************************/

// this file contains all the common includes for clusters in the zcl-util

#include <app/util/attribute-storage.h>

// for pulling in defines dealing with EITHER server or client
#include "app/util/common.h"
#include <app-common/zap-generated/callback.h>
#include <app/util/af-main.h>
#include <app/util/error-mapping.h>
#include <app/util/odd-sized-integers.h>

#include <app/reporting/reporting.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

EmberAfStatus emberAfWriteAttributeExternal(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask,
                                            uint16_t manufacturerCode, uint8_t * dataPtr, EmberAfAttributeType dataType)
{
    EmberAfAttributeWritePermission extWritePermission =
        emberAfAllowNetworkWriteAttributeCallback(endpoint, cluster, attributeID, mask, manufacturerCode, dataPtr, dataType);
    switch (extWritePermission)
    {
    case EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_DENY_WRITE:
        return EMBER_ZCL_STATUS_FAILURE;
    case EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_NORMAL:
    case EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_OF_READ_ONLY:
        return emAfWriteAttribute(endpoint, cluster, attributeID, mask, manufacturerCode, dataPtr, dataType,
                                  (extWritePermission == EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_OF_READ_ONLY), false);
    default:
        return (EmberAfStatus) extWritePermission;
    }
}

//@deprecated use emberAfWriteServerAttribute or emberAfWriteClientAttribute
EmberAfStatus emberAfWriteAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask,
                                    uint8_t * dataPtr, EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, mask, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr, dataType,
                              true,   // override read-only?
                              false); // just test?
}

EmberAfStatus emberAfWriteClientAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                          EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_CLIENT, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr,
                              dataType,
                              true,   // override read-only?
                              false); // just test?
}

EmberAfStatus emberAfWriteServerAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                          EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr,
                              dataType,
                              true,   // override read-only?
                              false); // just test?
}

EmberAfStatus emberAfWriteManufacturerSpecificClientAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID,
                                                              uint16_t manufacturerCode, uint8_t * dataPtr,
                                                              EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_CLIENT, manufacturerCode, dataPtr, dataType,
                              true,   // override read-only?
                              false); // just test?
}

EmberAfStatus emberAfWriteManufacturerSpecificServerAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID,
                                                              uint16_t manufacturerCode, uint8_t * dataPtr,
                                                              EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_SERVER, manufacturerCode, dataPtr, dataType,
                              true,   // override read-only?
                              false); // just test?
}

EmberAfStatus emberAfVerifyAttributeWrite(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask,
                                          uint16_t manufacturerCode, uint8_t * dataPtr, EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, mask, manufacturerCode, dataPtr, dataType,
                              false, // override read-only?
                              true); // just test?
}

EmberAfStatus emberAfReadAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask, uint8_t * dataPtr,
                                   uint16_t readLength, EmberAfAttributeType * dataType)
{
    return emAfReadAttribute(endpoint, cluster, attributeID, mask, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr, readLength, dataType);
}

EmberAfStatus emberAfReadServerAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                         uint16_t readLength)
{
    return emAfReadAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr,
                             readLength, NULL);
}

EmberAfStatus emberAfReadClientAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                         uint16_t readLength)
{
    return emAfReadAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_CLIENT, EMBER_AF_NULL_MANUFACTURER_CODE, dataPtr,
                             readLength, NULL);
}

EmberAfStatus emberAfReadManufacturerSpecificServerAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID,
                                                             uint16_t manufacturerCode, uint8_t * dataPtr, uint16_t readLength)
{
    return emAfReadAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_SERVER, manufacturerCode, dataPtr, readLength, NULL);
}

EmberAfStatus emberAfReadManufacturerSpecificClientAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID,
                                                             uint16_t manufacturerCode, uint8_t * dataPtr, uint16_t readLength)
{
    return emAfReadAttribute(endpoint, cluster, attributeID, CLUSTER_MASK_CLIENT, manufacturerCode, dataPtr, readLength, NULL);
}

bool emberAfReadSequentialAttributesAddToResponse(EndpointId endpoint, ClusterId clusterId, AttributeId startAttributeId,
                                                  uint8_t mask, uint16_t manufacturerCode, uint8_t maxAttributeIds,
                                                  bool includeAccessControl)
{
    uint16_t i;
    uint16_t discovered = 0;
    uint16_t skipped    = 0;
    uint16_t total      = 0;

    EmberAfCluster * cluster = emberAfFindClusterWithMfgCode(endpoint, clusterId, mask, manufacturerCode);

    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = clusterId;
    record.clusterMask      = mask;
    record.attributeId      = startAttributeId;
    record.manufacturerCode = manufacturerCode;

    // If we don't have the cluster or it doesn't match the search, we're done.
    if (cluster == NULL || !emAfMatchCluster(cluster, &record))
    {
        return true;
    }

    for (i = 0; i < cluster->attributeCount; i++)
    {
        EmberAfAttributeMetadata * metadata = &cluster->attributes[i];

        // If the cluster is not manufacturer-specific, an attribute is considered
        // only if its manufacturer code matches that of the command (which may be
        // unset).
        if (!emberAfClusterIsManufacturerSpecific(cluster))
        {
            record.attributeId = metadata->attributeId;
            if (!emAfMatchAttribute(cluster, metadata, &record))
            {
                continue;
            }
        }

        if (metadata->attributeId < startAttributeId)
        {
            skipped++;
        }
        else if (discovered < maxAttributeIds)
        {
            emberAfPutInt32uInResp(metadata->attributeId);
            emberAfPutInt8uInResp(metadata->attributeType);
            if (includeAccessControl)
            {
                // bit 0 : Readable <-- All our attributes are readable
                // bit 1 : Writable <-- The only thing we track in the attribute metadata mask
                // bit 2 : Reportable <-- All our attributes are reportable
                emberAfPutInt8uInResp((metadata->mask & ATTRIBUTE_MASK_WRITABLE) ? 0x07 : 0x05);
            }
            discovered++;
        }
        else
        {
            // MISRA requires ..else if.. to have terminating else.
        }
        total++;
    }

    // We are finished if there are no more attributes to find, which means the
    // number of attributes discovered plus the number skipped equals the total
    // attributes in the cluster.  For manufacturer-specific clusters, the total
    // includes all attributes in the cluster.  For standard ZCL clusters, if the
    // the manufacturer code is set, the total is the number of attributes that
    // match the manufacturer code.  Otherwise, the total is the number of
    // standard ZCL attributes in the cluster.
    return (discovered + skipped == total);
}

static void emberAfAttributeDecodeAndPrintCluster(ClusterId cluster, uint16_t mfgCode)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_ATTRIBUTES)
    uint16_t index = emberAfFindClusterNameIndexWithMfgCode(cluster, mfgCode);
    if (index != 0xFFFF)
    {
        emberAfAttributesPrintln("(%p)", zclClusterNames[index].name);
    }
    emberAfAttributesFlush();
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_ATTRIBUTES)
}

void emberAfPrintAttributeTable(void)
{
    uint8_t data[ATTRIBUTE_LARGEST];
    decltype(emberAfEndpointCount()) endpointIndex;
    decltype(EmberAfEndpointType::clusterCount) clusterIndex;
    uint16_t attributeIndex;
    EmberAfStatus status;
    for (endpointIndex = 0; endpointIndex < emberAfEndpointCount(); endpointIndex++)
    {
        EmberAfDefinedEndpoint * ep = &(emAfEndpoints[endpointIndex]);
        emberAfAttributesPrintln("ENDPOINT %x", ep->endpoint);
        emberAfAttributesPrintln("clus / side / attr / mfg  /type(len)/ rw / storage / data (raw)");
        emberAfAttributesFlush();
        for (clusterIndex = 0; clusterIndex < ep->endpointType->clusterCount; clusterIndex++)
        {
            EmberAfCluster * cluster = &(ep->endpointType->cluster[clusterIndex]);

            for (attributeIndex = 0; attributeIndex < cluster->attributeCount; attributeIndex++)
            {
                EmberAfAttributeMetadata * metaData = &(cluster->attributes[attributeIndex]);

                // Depending on user config, this loop can take a very long time to
                // run and watchdog reset will  kick in. As a workaround, we'll
                // manually reset the watchdog.
                //        halResetWatchdog();

                emberAfAttributesPrint(ChipLogFormatMEI " / %p / " ChipLogFormatMEI " / ", ChipLogValueMEI(cluster->clusterId),
                                       (emberAfAttributeIsClient(metaData) ? "clnt" : "srvr"),
                                       ChipLogValueMEI(metaData->attributeId));
                emberAfAttributesPrint("----");
                emberAfAttributesPrint(
                    " / %x (%x) / %p / %p / ", metaData->attributeType, emberAfAttributeSize(metaData),
                    (metaData->IsReadOnly() ? "RO" : "RW"),
                    (metaData->IsNonVolatile() ? " nonvolatile " : (metaData->IsExternal() ? " extern " : "  RAM  ")));
                emberAfAttributesFlush();
                status = emAfReadAttribute(ep->endpoint, cluster->clusterId, metaData->attributeId,
                                           (emberAfAttributeIsClient(metaData) ? CLUSTER_MASK_CLIENT : CLUSTER_MASK_SERVER),
                                           EMBER_AF_NULL_MANUFACTURER_CODE, data, ATTRIBUTE_LARGEST, NULL);
                if (status == EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE)
                {
                    emberAfAttributesPrintln("Unsupported");
                }
                else
                {
                    uint16_t length;
                    if (emberAfIsStringAttributeType(metaData->attributeType))
                    {
                        length = static_cast<uint16_t>(emberAfStringLength(data) + 1);
                    }
                    else if (emberAfIsLongStringAttributeType(metaData->attributeType))
                    {
                        length = static_cast<uint16_t>(emberAfLongStringLength(data) + 2);
                    }
                    else
                    {
                        length = emberAfAttributeSize(metaData);
                    }
                    UNUSED_VAR(length);
                    emberAfAttributesPrintBuffer(data, length, true);
                    emberAfAttributesFlush();
                    emberAfAttributeDecodeAndPrintCluster(cluster->clusterId, EMBER_AF_NULL_MANUFACTURER_CODE);
                }
            }
        }
        emberAfAttributesFlush();
    }
}

// given a clusterId and an attribute to read, this crafts the response
// and places it in the response buffer. Response is one of two items:
// 1) unsupported: [attrId:2] [status:1]
// 2) supported:   [attrId:2] [status:1] [type:1] [data:n]
//
void emberAfRetrieveAttributeAndCraftResponse(EndpointId endpoint, ClusterId clusterId, AttributeId attrId, uint8_t mask,
                                              uint16_t manufacturerCode, uint16_t readLength)
{
    EmberAfStatus status;
    uint8_t data[ATTRIBUTE_LARGEST];
    uint8_t dataType;
    uint16_t dataLen;

    // account for at least one byte of data
    if (readLength < 5)
    {
        return;
    }

    emberAfAttributesPrintln("OTA READ: ep:%" PRIx16 " cid:" ChipLogFormatMEI " attid:" ChipLogFormatMEI " msk:%x mfcode:%2x",
                             endpoint, ChipLogValueMEI(clusterId), ChipLogValueMEI(attrId), mask, manufacturerCode);

    // lookup the attribute in our table
    status = emAfReadAttribute(endpoint, clusterId, attrId, mask, manufacturerCode, data, ATTRIBUTE_LARGEST, &dataType);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        dataLen = emberAfAttributeValueSize(clusterId, attrId, dataType, data);
        if ((readLength - 4) < dataLen)
        { // Not enough space for attribute.
            return;
        }
    }
    else
    {
        emberAfPutInt32uInResp(attrId);
        emberAfPutStatusInResp(status);
        emberAfAttributesPrintln("READ: clus " ChipLogFormatMEI ", attr " ChipLogFormatMEI " failed %x", ChipLogValueMEI(clusterId),
                                 ChipLogValueMEI(attrId), status);
        emberAfAttributesFlush();
        return;
    }

    // put attribute in least sig byte first
    emberAfPutInt32uInResp(attrId);

    // attribute is found, so copy in the status and the data type
    emberAfPutInt8uInResp(EMBER_ZCL_STATUS_SUCCESS);
    emberAfPutInt8uInResp(dataType);

    if (dataLen < (EMBER_AF_RESPONSE_BUFFER_LEN - appResponseLength))
    {
#if (BIGENDIAN_CPU)
        // strings go over the air as length byte and then in human
        // readable format. These should not be flipped. Other attributes
        // need to be flipped so they go little endian OTA
        if (isThisDataTypeSentLittleEndianOTA(dataType))
        {
            uint8_t i;
            for (i = 0; i < dataLen; i++)
            {
                appResponseData[appResponseLength + i] = data[dataLen - i - 1];
            }
        }
        else
        {
            memmove(&(appResponseData[appResponseLength]), data, dataLen);
        }
#else  //(BIGENDIAN_CPU)
        memmove(&(appResponseData[appResponseLength]), data, dataLen);
#endif //(BIGENDIAN_CPU)
       // TODO: How do we know this does not overflow?
        appResponseLength = static_cast<uint16_t>(appResponseLength + dataLen);
    }

    emberAfAttributesPrintln("READ: clus " ChipLogFormatMEI ", attr " ChipLogFormatMEI ", dataLen: %x, OK",
                             ChipLogValueMEI(clusterId), ChipLogValueMEI(attrId), dataLen);
    emberAfAttributesFlush();
}

// This function appends the attribute report fields for the given endpoint,
// cluster, and attribute to the buffer starting at the index.  If there is
// insufficient space in the buffer or an error occurs, buffer and bufIndex will
// remain unchanged.  Otherwise, bufIndex will be incremented appropriately and
// the fields will be written to the buffer.
EmberAfStatus emberAfAppendAttributeReportFields(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                                 uint8_t * buffer, uint8_t bufLen, uint8_t * bufIndex)
{
    EmberAfStatus status;
    EmberAfAttributeType type;
    uint16_t size;
    uint16_t bufLen16 = (uint16_t) bufLen;
    uint8_t data[ATTRIBUTE_LARGEST];

    status = emberAfReadAttribute(endpoint, clusterId, attributeId, mask, data, sizeof(data), &type);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }

    size = emberAfAttributeValueSize(clusterId, attributeId, type, data);
    if (bufLen16 - *bufIndex < 3 || size > bufLen16 - (*bufIndex + 3))
    {
        status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        goto kickout;
    }

    buffer[(*bufIndex)++] = EMBER_LOW_BYTE(attributeId);
    buffer[(*bufIndex)++] = EMBER_HIGH_BYTE(attributeId);
    buffer[(*bufIndex)++] = type;
#if (BIGENDIAN_CPU)
    if (isThisDataTypeSentLittleEndianOTA(type))
    {
        emberReverseMemCopy(buffer + *bufIndex, data, size);
    }
    else
    {
        memmove(buffer + *bufIndex, data, size);
    }
#else
    memmove(buffer + *bufIndex, data, size);
#endif
    *bufIndex = static_cast<uint8_t>(*bufIndex + size);

kickout:
    emberAfAttributesPrintln("REPORT: clus " ChipLogFormatMEI ", attr " ChipLogFormatMEI ": 0x%x", ChipLogValueMEI(clusterId),
                             ChipLogValueMEI(attributeId), status);
    emberAfAttributesFlush();

    return status;
}

//------------------------------------------------------------------------------
// Internal Functions

// Helper for determining whether a value is a null value.
template <typename T>
static bool IsNullValue(const uint8_t * data)
{
    using Traits = app::NumericAttributeTraits<T>;
    // We don't know how data is aligned, so safely copy it over to the relevant
    // StorageType value.
    typename Traits::StorageType val;
    memcpy(&val, data, sizeof(val));
    return Traits::IsNullValue(val);
}

static bool IsNullValue(const uint8_t * data, uint16_t dataLen, bool isAttributeSigned)
{
    if (dataLen > 4)
    {
        // We don't support this, just like emberAfCompareValues does not.
        return false;
    }

    switch (dataLen)
    {
    case 1: {
        if (isAttributeSigned)
        {
            return IsNullValue<int8_t>(data);
        }
        return IsNullValue<uint8_t>(data);
    }
    case 2: {
        if (isAttributeSigned)
        {
            return IsNullValue<int16_t>(data);
        }
        return IsNullValue<uint16_t>(data);
    }
    case 3: {
        if (isAttributeSigned)
        {
            return IsNullValue<app::OddSizedInteger<3, true>>(data);
        }
        return IsNullValue<app::OddSizedInteger<3, false>>(data);
    }
    case 4: {
        if (isAttributeSigned)
        {
            return IsNullValue<int32_t>(data);
        }
        return IsNullValue<uint32_t>(data);
    }
    }

    // Not reached.
    return false;
}

// writes an attribute (identified by clusterID and attrID to the given value.
// this returns:
// - EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE: if attribute isnt supported by the device (the
//           device is not found in the attribute table)
// - EMBER_ZCL_STATUS_INVALID_DATA_TYPE: if the data type passed in doesnt match the type
//           stored in the attribute table
// - EMBER_ZCL_STATUS_READ_ONLY: if the attribute isnt writable
// - EMBER_ZCL_STATUS_INVALID_VALUE: if the value is set out of the allowable range for
//           the attribute
// - EMBER_ZCL_STATUS_SUCCESS: if the attribute was found and successfully written
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
EmberAfStatus emAfWriteAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask,
                                 uint16_t manufacturerCode, uint8_t * data, EmberAfAttributeType dataType,
                                 bool overrideReadOnlyAndDataType, bool justTest)
{
    EmberAfAttributeMetadata * metadata = NULL;
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = cluster;
    record.clusterMask      = mask;
    record.attributeId      = attributeID;
    record.manufacturerCode = manufacturerCode;
    emAfReadOrWriteAttribute(&record, &metadata,
                             NULL,   // buffer
                             0,      // buffer size
                             false); // write?

    // if we dont support that attribute
    if (metadata == NULL)
    {
        emberAfAttributesPrintln("%pep %x clus " ChipLogFormatMEI " attr " ChipLogFormatMEI " not supported",
                                 "WRITE ERR: ", endpoint, ChipLogValueMEI(cluster), ChipLogValueMEI(attributeID));
        emberAfAttributesFlush();
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    // if the data type specified by the caller is incorrect
    if (!(overrideReadOnlyAndDataType))
    {
        if (dataType != metadata->attributeType)
        {
            emberAfAttributesPrintln("%pinvalid data type", "WRITE ERR: ");
            emberAfAttributesFlush();
            return EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
        }

        if (metadata->IsReadOnly())
        {
            emberAfAttributesPrintln("%pattr not writable", "WRITE ERR: ");
            emberAfAttributesFlush();
            return EMBER_ZCL_STATUS_READ_ONLY;
        }
    }

    // if the value the attribute is being set to is out of range
    // return EMBER_ZCL_STATUS_INVALID_VALUE
    if ((metadata->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U)
    {
        EmberAfDefaultAttributeValue minv = metadata->defaultValue.ptrToMinMaxValue->minValue;
        EmberAfDefaultAttributeValue maxv = metadata->defaultValue.ptrToMinMaxValue->maxValue;
        uint16_t dataLen                  = emberAfAttributeSize(metadata);
        const uint8_t * minBytes;
        const uint8_t * maxBytes;
        if (dataLen <= 2)
        {
            minBytes = reinterpret_cast<const uint8_t *>(&(minv.defaultValue));
            maxBytes = reinterpret_cast<const uint8_t *>(&(maxv.defaultValue));
// On big endian cpu with length 1 only the second byte counts
#if (BIGENDIAN_CPU)
            if (dataLen == 1)
            {
                minBytes++;
                maxBytes++;
            }
#endif // BIGENDIAN_CPU
        }
        else
        {
            minBytes = minv.ptrToDefaultValue;
            maxBytes = maxv.ptrToDefaultValue;
        }

        bool isAttributeSigned = emberAfIsTypeSigned(metadata->attributeType);
        bool isOutOfRange      = emberAfCompareValues(minBytes, data, dataLen, isAttributeSigned) == 1 ||
            emberAfCompareValues(maxBytes, data, dataLen, isAttributeSigned) == -1;

        if (isOutOfRange &&
            // null value is always in-range for a nullable attribute.
            (!metadata->IsNullable() || !IsNullValue(data, dataLen, isAttributeSigned)))
        {
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
    }

    // write the data unless this is only a test
    if (!justTest)
    {
        const app::ConcreteAttributePath attributePath(endpoint, cluster, attributeID);

        // Pre write attribute callback for all attribute changes,
        // regardless of cluster.
        Protocols::InteractionModel::Status imStatus =
            MatterPreAttributeChangeCallback(attributePath, mask, dataType, emberAfAttributeSize(metadata), data);
        if (imStatus != Protocols::InteractionModel::Status::Success)
        {
            return app::ToEmberAfStatus(imStatus);
        }

        // Pre-write attribute callback specific
        // to the cluster that the attribute lives in.
        EmberAfStatus status =
            emAfClusterPreAttributeChangedCallback(attributePath, mask, dataType, emberAfAttributeSize(metadata), data);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return status;
        }

        // write the attribute
        status = emAfReadOrWriteAttribute(&record,
                                          NULL, // metadata
                                          data,
                                          0,     // buffer size - unused
                                          true); // write?

        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return status;
        }

        // Save the attribute to persistent storage if needed
        // The callee will weed out attributes that do not need to be stored.
        emAfSaveAttributeToStorageIfNeeded(data, endpoint, cluster, metadata);

        MatterReportingAttributeChangeCallback(endpoint, cluster, attributeID, mask, manufacturerCode, dataType, data);

        // Post write attribute callback for all attributes changes, regardless
        // of cluster.
        MatterPostAttributeChangeCallback(attributePath, mask, dataType, emberAfAttributeSize(metadata), data);

        // Post-write attribute callback specific
        // to the cluster that the attribute lives in.
        emAfClusterAttributeChangedCallback(attributePath, mask);
    }
    else
    {
        // bug: 11618, we are not handling properly external attributes
        // in this case... We need to do something. We don't really
        // know if it will succeed.
        emberAfAttributesPrintln("WRITE: no write, just a test");
        emberAfAttributesFlush();
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

// If dataPtr is NULL, no data is copied to the caller.
// readLength should be 0 in that case.

EmberAfStatus emAfReadAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t mask,
                                uint16_t manufacturerCode, uint8_t * dataPtr, uint16_t readLength, EmberAfAttributeType * dataType)
{
    EmberAfAttributeMetadata * metadata = NULL;
    EmberAfAttributeSearchRecord record;
    EmberAfStatus status;
    record.endpoint         = endpoint;
    record.clusterId        = cluster;
    record.clusterMask      = mask;
    record.attributeId      = attributeID;
    record.manufacturerCode = manufacturerCode;
    status                  = emAfReadOrWriteAttribute(&record, &metadata, dataPtr, readLength,
                                      false); // write?

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // It worked!  If the user asked for the type, set it before returning.
        if (dataType != NULL)
        {
            (*dataType) = metadata->attributeType;
        }
    }
    else
    { // failed, print debug info
        if (status == EMBER_ZCL_STATUS_INSUFFICIENT_SPACE)
        {
            emberAfAttributesPrintln("READ: attribute size too large for caller");
            emberAfAttributesFlush();
        }
    }

    return status;
}
