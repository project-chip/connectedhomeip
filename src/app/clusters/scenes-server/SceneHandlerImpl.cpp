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
#include <app/util/odd-sized-integers.h>

namespace chip {
namespace scenes {

namespace {

template <int ByteSize, bool IsSigned>
using OddSizedInteger        = app::OddSizedInteger<ByteSize, IsSigned>;
using ConcreteAttributePath  = app::ConcreteAttributePath;
using AttributeValuePairType = app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type;

/// ConvertDefaultValueToWorkingValue
/// @brief Helper function to convert a byte array to a value of the given type.
/// @param EmberAfDefaultAttributeValue & defaultValue
/// @return Value converted to the given working type
template <typename Type>
Type ConvertDefaultValueToWorkingValue(const EmberAfDefaultAttributeValue & defaultValue)
{
    if (sizeof(Type) <= 2)
    {
        return static_cast<Type>(defaultValue.defaultValue);
    }

    Type sValue = 0;
    memcpy(&sValue, defaultValue.ptrToDefaultValue, sizeof(Type));
    return app::NumericAttributeTraits<Type>::StorageToWorking(sValue);
}

/// CapAttributeID
/// Cap the attribute value based on the attribute's min and max if they are defined,
/// or based on the attribute's size if they are not.
/// @param[in] aVPair   AttributeValuePairType
/// @param[in] metadata  EmberAfAttributeMetadata
///
template <typename Type>
void CapAttributeID(AttributeValuePairType & aVPair, const EmberAfAttributeMetadata * metadata)
{
    using IntType     = app::NumericAttributeTraits<Type>;
    using WorkingType = typename IntType::WorkingType;

    WorkingType maxValue;

    if (metadata->IsBoolean())
    {
        aVPair.attributeValue = aVPair.attributeValue ? 1 : 0;
        return;
    }

    // Check if the attribute type is signed
    if (metadata->IsSignedIntegerAttribute())
    {
        maxValue = static_cast<WorkingType>((1ULL << (emberAfAttributeSize(metadata) * 8 - 1)) - 1);
    }
    else
    {
        maxValue = static_cast<WorkingType>((1ULL << (emberAfAttributeSize(metadata) * 8)) - 1);
    }

    // Check metadata for min and max values
    if (metadata->HasMinMax())
    {
        const EmberAfAttributeMinMaxValue * minMaxValue = metadata->defaultValue.ptrToMinMaxValue;
        WorkingType minVal                              = ConvertDefaultValueToWorkingValue<WorkingType>(minMaxValue->minValue);
        WorkingType maxVal                              = ConvertDefaultValueToWorkingValue<WorkingType>(minMaxValue->maxValue);

        // Cap based on minimum value
        if (minVal > static_cast<WorkingType>(aVPair.attributeValue))
        {
            aVPair.attributeValue = static_cast<std::make_unsigned_t<WorkingType>>(minVal);
            // We assume the max is >= min therefore we can return
            return;
        }

        // Adjust maxValue if greater than the meta data's max value
        if (maxVal < maxValue)
        {
            maxValue = maxVal;
        }
    }

    // Cap based on maximum value
    if (metadata->IsSignedIntegerAttribute())
    {
        if (static_cast<int64_t>(aVPair.attributeValue) > static_cast<int64_t>(maxValue))
        {
            aVPair.attributeValue = static_cast<std::make_unsigned_t<WorkingType>>(maxValue);
        }
    }
    else
    {
        if (aVPair.attributeValue > static_cast<uint64_t>(maxValue))
        {
            aVPair.attributeValue = static_cast<std::make_unsigned_t<WorkingType>>(maxValue);
        }
    }
}

/// @brief  Validate the attribute exists for a given cluster
/// @param[in] endpoint   Endpoint ID
/// @param[in] clusterID  Cluster ID
/// @param[in] aVPair     AttributeValuePairType, will be mutated to cap the value if it is out of range
/// @return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute) if the attribute does not exist for a given cluster or is not scenable
/// @note This will allways fail for global list attributes. If we do want to make them scenable someday, we will need to
///       use a different validation method.
// TODO: Add check for "S" quality to determine if the attribute is scenable once suported :
// https://github.com/project-chip/connectedhomeip/issues/24177
CHIP_ERROR ValidateAttributePath(EndpointId endpoint, ClusterId cluster, AttributeValuePairType & aVPair)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpoint, cluster, aVPair.attributeID);

    if (nullptr == metadata)
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    switch (metadata->attributeType)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
    case ZCL_BITMAP8_ATTRIBUTE_TYPE:
    case ZCL_INT8U_ATTRIBUTE_TYPE:
        CapAttributeID<uint8_t>(aVPair, metadata);
        break;
    case ZCL_BITMAP16_ATTRIBUTE_TYPE:
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        CapAttributeID<uint16_t>(aVPair, metadata);
        break;
    case ZCL_INT24U_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<3, false>>(aVPair, metadata);
        break;
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        CapAttributeID<uint32_t>(aVPair, metadata);
        break;
    case ZCL_INT40U_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<5, false>>(aVPair, metadata);
        break;
    case ZCL_INT48U_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<6, false>>(aVPair, metadata);
        break;
    case ZCL_INT56U_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<7, false>>(aVPair, metadata);
        break;
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        CapAttributeID<uint64_t>(aVPair, metadata);
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        CapAttributeID<int8_t>(aVPair, metadata);
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE: // fallthrough
        CapAttributeID<int16_t>(aVPair, metadata);
        break;
    case ZCL_INT24S_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<3, true>>(aVPair, metadata);
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        CapAttributeID<int32_t>(aVPair, metadata);
        break;
    case ZCL_INT40S_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<5, true>>(aVPair, metadata);
        break;
    case ZCL_INT48S_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<6, true>>(aVPair, metadata);
        break;
    case ZCL_INT56S_ATTRIBUTE_TYPE:
        CapAttributeID<OddSizedInteger<7, true>>(aVPair, metadata);
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        CapAttributeID<int64_t>(aVPair, metadata);
        break;
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

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
