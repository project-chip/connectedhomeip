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

// this file contains all the common includes for clusters in the zcl-util

#include <app/util/attribute-storage.h>

// for pulling in defines dealing with EITHER server or client
#include "app/util/common.h"
#include <app/util/config.h>
#include <app/util/error-mapping.h>
#include <app/util/generic-callbacks.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPConfig.h>

#include <app/reporting/reporting.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

EmberAfStatus emAfWriteAttributeExternal(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                         EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, dataPtr, dataType, false /* override read-only */);
}

EmberAfStatus emberAfWriteAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                    EmberAfAttributeType dataType)
{
    return emAfWriteAttribute(endpoint, cluster, attributeID, dataPtr, dataType, true /* override read-only */);
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

EmberAfStatus emAfWriteAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * data,
                                 EmberAfAttributeType dataType, bool overrideReadOnlyAndDataType)
{
    const EmberAfAttributeMetadata * metadata = nullptr;
    EmberAfAttributeSearchRecord record;
    record.endpoint      = endpoint;
    record.clusterId     = cluster;
    record.attributeId   = attributeID;
    EmberAfStatus status = emAfReadOrWriteAttribute(&record, &metadata,
                                                    nullptr, // buffer
                                                    0,       // buffer size
                                                    false);  // write?

    // if we dont support that attribute
    if (metadata == nullptr)
    {
        ChipLogProgress(Zcl, "%p ep %x clus " ChipLogFormatMEI " attr " ChipLogFormatMEI " not supported", "WRITE ERR: ", endpoint,
                        ChipLogValueMEI(cluster), ChipLogValueMEI(attributeID));
        return status;
    }

    // if the data type specified by the caller is incorrect
    if (!(overrideReadOnlyAndDataType))
    {
        if (dataType != metadata->attributeType)
        {
            ChipLogProgress(Zcl, "%p invalid data type", "WRITE ERR: ");
            return EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
        }

        if (metadata->IsReadOnly())
        {
            ChipLogProgress(Zcl, "%p attr not writable", "WRITE ERR: ");
            return EMBER_ZCL_STATUS_UNSUPPORTED_WRITE;
        }
    }

    // if the value the attribute is being set to is out of range
    // return EMBER_ZCL_STATUS_CONSTRAINT_ERROR
    if ((metadata->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U)
    {
        EmberAfDefaultAttributeValue minv = metadata->defaultValue.ptrToMinMaxValue->minValue;
        EmberAfDefaultAttributeValue maxv = metadata->defaultValue.ptrToMinMaxValue->maxValue;
        uint16_t dataLen                  = emberAfAttributeSize(metadata);
        const uint8_t * minBytes;
        const uint8_t * maxBytes;
        if (dataLen <= 2)
        {
            static_assert(sizeof(minv.defaultValue) == 2, "if statement relies on size of minv.defaultValue being 2");
            static_assert(sizeof(maxv.defaultValue) == 2, "if statement relies on size of maxv.defaultValue being 2");
            minBytes = reinterpret_cast<const uint8_t *>(&(minv.defaultValue));
            maxBytes = reinterpret_cast<const uint8_t *>(&(maxv.defaultValue));
// On big endian cpu with length 1 only the second byte counts
#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
            if (dataLen == 1)
            {
                minBytes++;
                maxBytes++;
            }
#endif // CHIP_CONFIG_BIG_ENDIAN_TARGET
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
            return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
        }
    }

    const app::ConcreteAttributePath attributePath(endpoint, cluster, attributeID);

    // Pre write attribute callback for all attribute changes,
    // regardless of cluster.
    Protocols::InteractionModel::Status imStatus =
        MatterPreAttributeChangeCallback(attributePath, dataType, emberAfAttributeSize(metadata), data);
    if (imStatus != Protocols::InteractionModel::Status::Success)
    {
        return app::ToEmberAfStatus(imStatus);
    }

    // Pre-write attribute callback specific
    // to the cluster that the attribute lives in.
    status = emAfClusterPreAttributeChangedCallback(attributePath, dataType, emberAfAttributeSize(metadata), data);

    // Ignore the following write operation and return success
    if (status == EMBER_ZCL_STATUS_WRITE_IGNORED)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    // write the attribute
    status = emAfReadOrWriteAttribute(&record,
                                      nullptr, // metadata
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

    MatterReportingAttributeChangeCallback(endpoint, cluster, attributeID);

    // Post write attribute callback for all attributes changes, regardless
    // of cluster.
    MatterPostAttributeChangeCallback(attributePath, dataType, emberAfAttributeSize(metadata), data);

    // Post-write attribute callback specific
    // to the cluster that the attribute lives in.
    emAfClusterAttributeChangedCallback(attributePath);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfReadAttribute(EndpointId endpoint, ClusterId cluster, AttributeId attributeID, uint8_t * dataPtr,
                                   uint16_t readLength)
{
    const EmberAfAttributeMetadata * metadata = nullptr;
    EmberAfAttributeSearchRecord record;
    EmberAfStatus status;
    record.endpoint    = endpoint;
    record.clusterId   = cluster;
    record.attributeId = attributeID;
    status             = emAfReadOrWriteAttribute(&record, &metadata, dataPtr, readLength,
                                                  false); // write?

    // failed, print debug info
    if (status == EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED)
    {
        ChipLogProgress(Zcl, "READ: attribute size too large for caller");
    }

    return status;
}
