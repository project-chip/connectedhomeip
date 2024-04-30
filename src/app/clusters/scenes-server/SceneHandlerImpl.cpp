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
typename app::NumericAttributeTraits<Type>::WorkingType
ConvertDefaultValueToWorkingValue(const EmberAfDefaultAttributeValue & defaultValue)
{
    if (sizeof(typename app::NumericAttributeTraits<Type>::WorkingType) <= 2)
    {
        return static_cast<typename app::NumericAttributeTraits<Type>::WorkingType>(defaultValue.defaultValue);
    }

    typename app::NumericAttributeTraits<Type>::StorageType sValue;
    memcpy(&sValue, defaultValue.ptrToDefaultValue, sizeof(sValue));
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

    // Check if the attribute type is signed
    if (metadata->IsSignedIntegerAttribute())
    {
        // We use emberAfAttributeSize for cases like INT24S, INT40S, INT48S, INT56S where numeric_limits<WorkingType>::max()
        // wouldn't work
        maxValue = static_cast<WorkingType>((1ULL << (emberAfAttributeSize(metadata) * 8 - 1)) - 1);
    }
    else
    {
        // We use emberAfAttributeSize for cases like INT24U, INT40U, INT48U, INT56U where numeric_limits<WorkingType>::max()
        // wouldn't work
        maxValue = static_cast<WorkingType>((1ULL << (emberAfAttributeSize(metadata) * 8)) - 1);
    }

    // Ensure that the metadata's signedness matches the working type's signedness
    VerifyOrDie(metadata->IsSignedIntegerAttribute() == std::is_signed<WorkingType>::value);

    if (metadata->IsBoolean())
    {
        // Caping the value to 1 in case values greater than 1 are set
        aVPair.attributeValue = aVPair.attributeValue ? 1 : 0;
        return;
    }

    // Check metadata for min and max values
    if (metadata->HasMinMax())
    {
        const EmberAfAttributeMinMaxValue * minMaxValue = metadata->defaultValue.ptrToMinMaxValue;
        WorkingType rangeMin                            = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->minValue);
        WorkingType rangeMax                            = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->maxValue);

        // Cap based on minimum value
        if (rangeMin > static_cast<WorkingType>(aVPair.attributeValue))
        {
            aVPair.attributeValue = static_cast<std::make_unsigned_t<WorkingType>>(rangeMin);
            // We assume the max is >= min therefore we can return
            return;
        }

        // Adjust maxValue if greater than the meta data's max value
        if (rangeMax < static_cast<WorkingType>(aVPair.attributeValue))
        {
            maxValue = rangeMax;
        }
    }

    // Cap based on type-enforced maximum value (and minnimum for signed types)
    if constexpr (std::is_signed<WorkingType>::value)
    {
        // Cap on max value, check for comparison to the WorkingType size and check for overflow
        if (static_cast<WorkingType>(aVPair.attributeValue) > maxValue || aVPair.attributeValue & ~static_cast<uint64_t>(maxValue))
        {
            // We treat overflows as > Max value since the wrong type is being used and we cannot determine the actual value
            aVPair.attributeValue = static_cast<std::make_unsigned_t<WorkingType>>(maxValue);
        }
    }
    else
    {
        if (aVPair.attributeValue > maxValue)
        {
            aVPair.attributeValue = maxValue;
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
