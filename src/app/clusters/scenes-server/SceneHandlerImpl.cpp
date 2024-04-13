/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/util/endpoint-config-api.h>

namespace chip {
namespace scenes {

namespace {

using ConcreteAttributePath  = app::ConcreteAttributePath;
using AttributeValuePairType = app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type;

/// IsAttributeTypeValid
/// @brief Check if the attribute type is valid for a scene extension field value according to the spec.
/// @param type Type of the attribute's metadata
/// @return
bool IsSceneValidAttributeType(EmberAfAttributeType type)
{
    return (type == ZCL_INT8U_ATTRIBUTE_TYPE || type == ZCL_INT16U_ATTRIBUTE_TYPE || type == ZCL_INT32U_ATTRIBUTE_TYPE ||
            type == ZCL_INT64U_ATTRIBUTE_TYPE || type == ZCL_INT8S_ATTRIBUTE_TYPE || type == ZCL_INT16S_ATTRIBUTE_TYPE ||
            type == ZCL_INT32S_ATTRIBUTE_TYPE || type == ZCL_INT64S_ATTRIBUTE_TYPE);
}

/// IsSignedAttribute
/// @brief Compare the attribute type to the signed attribute types in ZCL and return true if it is signed.
/// @param attributeType metadata's attribute type
/// @return
bool IsSignedAttribute(EmberAfAttributeType attributeType)
{
    return (attributeType >= ZCL_INT8S_ATTRIBUTE_TYPE && attributeType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

/// ConvertByteArrayToUInt64
/// @brief Helper function to convert a byte array to a uint64_t value
/// @param EmberAfDefaultAttributeValue & defaultValue
/// @param len Length of the byte array
/// @return uint64_t Value
/// @note The attribute table supports 8 bytes only for unsigned integers, 4 bytes is the maximum for signed integers
uint64_t ConvertByteArrayToUInt64(const EmberAfDefaultAttributeValue & defaultValue, uint16_t len)
{
    if (len <= 2)
    {
        return static_cast<uint64_t>(defaultValue.defaultValue);
    }
    else
    {
        uint64_t result     = 0;
        const uint8_t * val = defaultValue.ptrToDefaultValue;
        for (size_t i = 0; i < len; i++)
        {
            result = (result << 8) | val[(CHIP_CONFIG_BIG_ENDIAN_TARGET ? i : (len - 1) - i)];
        }
        return result;
    }
}

/// CapAttributeID
/// Cap the attribute value based on the attribute's min and max if they are defined,
/// or based on the attribute's size if they are not.
/// @param[in] aVPair   AttributeValuePairType
/// @param[in] metadata  EmberAfAttributeMetadata
///
void CapAttributeID(AttributeValuePairType & aVPair, const EmberAfAttributeMetadata * metadata)
{
    // Calculate the maximum value that can be represented with the given number of bytes
    uint64_t maxValue = 0;

    // Check if the attribute type is signed
    if (IsSignedAttribute(metadata->attributeType))
    {
        maxValue = (1ULL << ((emberAfAttributeSize(metadata) * 8) - 1)) - 1;
    }
    else
    {
        maxValue = (1ULL << (emberAfAttributeSize(metadata) * 8)) - 1;
    }

    // Check metadata for min and max values
    if ((metadata->mask & ATTRIBUTE_MASK_MIN_MAX) && metadata->defaultValue.ptrToMinMaxValue)
    {
        const EmberAfAttributeMinMaxValue * minMaxValue = metadata->defaultValue.ptrToMinMaxValue;
        uint64_t minVal = ConvertByteArrayToUInt64(minMaxValue->minValue, emberAfAttributeSize(metadata));
        uint64_t maxVal = ConvertByteArrayToUInt64(minMaxValue->maxValue, emberAfAttributeSize(metadata));

        // Cap based on minValue
        if (minVal > aVPair.attributeValue)
        {
            aVPair.attributeValue = minVal;
        }

        // Adjust maxValue if greater than the meta data's max value
        if (maxVal < maxValue)
        {
            maxValue = maxVal;
        }
    }

    // Cap based on maximum value
    if (aVPair.attributeValue > maxValue)
    {
        aVPair.attributeValue = maxValue;
    }
}

/// @brief  Validate the attribute exists for a given cluster
/// @param[in] endpoint   Endpoint ID
/// @param[in] clusterID  Cluster ID
/// @param[in] aVPair     AttributeValuePairType, will be mutated to cap the value if it is out of range
/// @return CHIP_ERROR_UNSUPPORTED_ATTRIBUTE if the attribute does not exist for a given cluster or is not scenable
/// @note This will allways fail for global list attributes. If we do want to make them scenable someday, we will need to
///       use a different validation method.
// TODO: Add check for "S" quality to determine if the attribute is scenable once suported :
// https://github.com/project-chip/connectedhomeip/issues/24177
CHIP_ERROR ValidateAttributePath(EndpointId endpoint, ClusterId cluster, AttributeValuePairType & aVPair)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpoint, cluster, aVPair.attributeID);

    if (nullptr == metadata)
    {
        return CHIP_ERROR_UNSUPPORTED_ATTRIBUTE;
    }

    if (!IsSceneValidAttributeType(metadata->attributeType))
    {
        return CHIP_ERROR_UNSUPPORTED_ATTRIBUTE;
    }

    // Cap value based on the attribute type size
    CapAttributeID(aVPair, metadata);

    return CHIP_NO_ERROR;
}
} // namespace

CHIP_ERROR
DefaultSceneHandlerImpl::EncodeAttributeValueList(const List<AttributeValuePairType> & aVlist, MutableByteSpan & serializedBytes)
{
    TLV::TLVWriter writer;
    writer.Init(serializedBytes);
    ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), aVlist));
    serializedBytes.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneHandlerImpl::DecodeAttributeValueList(const ByteSpan & serializedBytes,
                                                             DecodableList<AttributeValuePairDecodableType> & aVlist)
{
    TLV::TLVReader reader;

    reader.Init(serializedBytes);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(aVlist.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DefaultSceneHandlerImpl::SerializeAdd(EndpointId endpoint, const ExtensionFieldSetDecodableType & extensionFieldSet,
                                      MutableByteSpan & serializedBytes)
{
    AttributeValuePairType aVPairs[kMaxAvPair];

    size_t pairTotal = 0;
    // Verify size of list
    ReturnErrorOnFailure(extensionFieldSet.attributeValueList.ComputeSize(&pairTotal));
    VerifyOrReturnError(pairTotal <= ArraySize(aVPairs), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t pairCount  = 0;
    auto pair_iterator = extensionFieldSet.attributeValueList.begin();
    while (pair_iterator.Next())
    {
        AttributeValuePairType currentPair = pair_iterator.GetValue();
        ReturnErrorOnFailure(ValidateAttributePath(endpoint, extensionFieldSet.clusterID, currentPair));
        aVPairs[pairCount] = currentPair;
        pairCount++;
    }
    ReturnErrorOnFailure(pair_iterator.GetStatus());
    List<AttributeValuePairType> attributeValueList(aVPairs, pairCount);

    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR DefaultSceneHandlerImpl::Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                                ExtensionFieldSetType & extensionFieldSet)
{
    DecodableList<AttributeValuePairDecodableType> attributeValueList;

    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    // Verify size of list
    size_t pairTotal = 0;
    ReturnErrorOnFailure(attributeValueList.ComputeSize(&pairTotal));
    VerifyOrReturnError(pairTotal <= ArraySize(mAVPairs), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t pairCount  = 0;
    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        mAVPairs[pairCount] = pair_iterator.GetValue();
        pairCount++;
    };
    ReturnErrorOnFailure(pair_iterator.GetStatus());

    extensionFieldSet.clusterID          = cluster;
    extensionFieldSet.attributeValueList = mAVPairs;
    extensionFieldSet.attributeValueList.reduce_size(pairCount);

    return CHIP_NO_ERROR;
}

} // namespace scenes
} // namespace chip
