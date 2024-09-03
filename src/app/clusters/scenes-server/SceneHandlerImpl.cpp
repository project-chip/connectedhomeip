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
#include <app/util/ember-io-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/odd-sized-integers.h>

namespace chip {
namespace scenes {

namespace {

template <int ByteSize, bool IsSigned>
using OddSizedInteger        = app::OddSizedInteger<ByteSize, IsSigned>;
using ConcreteAttributePath  = app::ConcreteAttributePath;
using AttributeValuePairType = app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type;

/// ConvertDefaultValueToWorkingValue
/// @brief Helper function to convert a byte array to a value of the given type.
/// @param EmberAfDefaultAttributeValue & defaultValue
/// @return Value converted to the given working type
template <typename Type>
typename app::NumericAttributeTraits<Type>::WorkingType
ConvertDefaultValueToWorkingValue(const EmberAfDefaultAttributeValue & defaultValue)
{
    if constexpr (sizeof(typename app::NumericAttributeTraits<Type>::WorkingType) <= 2)
    {
        return static_cast<typename app::NumericAttributeTraits<Type>::WorkingType>(defaultValue.defaultValue);
    }

    typename app::NumericAttributeTraits<Type>::StorageType sValue;
    memcpy(&sValue, defaultValue.ptrToDefaultValue, sizeof(sValue));
    return app::NumericAttributeTraits<Type>::StorageToWorking(sValue);
}

/// IsExactlyOneValuePopulated
/// @brief Helper function to verify that exactly one value is populated in a given AttributeValuePairType
/// @param AttributeValuePairType & type AttributeValuePairType to verify
/// @return bool true if only one value is populated, false otherwise
bool IsExactlyOneValuePopulated(const AttributeValuePairType & type)
{
    int count = 0;
    if (type.valueUnsigned8.HasValue())
        count++;
    if (type.valueSigned8.HasValue())
        count++;
    if (type.valueUnsigned16.HasValue())
        count++;
    if (type.valueSigned16.HasValue())
        count++;
    if (type.valueUnsigned32.HasValue())
        count++;
    if (type.valueSigned32.HasValue())
        count++;
    if (type.valueUnsigned64.HasValue())
        count++;
    if (type.valueSigned64.HasValue())
        count++;
    return count == 1;
}

/// CapAttributeValue
/// Cap the attribute value based on the attribute's min and max if they are defined,
/// or based on the attribute's size if they are not.
/// @param[in] aVPair   AttributeValuePairType
/// @param[in] metadata  EmberAfAttributeMetadata
///
template <typename Type>
void CapAttributeValue(typename app::NumericAttributeTraits<Type>::WorkingType & value, const EmberAfAttributeMetadata * metadata)
{
    using IntType     = app::NumericAttributeTraits<Type>;
    using WorkingType = typename IntType::WorkingType;

    WorkingType maxValue;
    WorkingType minValue;
    uint16_t bitWidth = static_cast<uint16_t>(emberAfAttributeSize(metadata) * 8);

    // TODO Use min/max values from Type to obtain min/max instead of relying on metadata. See:
    // https://github.com/project-chip/connectedhomeip/issues/35328

    // Min/Max Value caps for the OddSize integers
    if (metadata->IsSignedIntegerAttribute())
    {
        // We use emberAfAttributeSize for cases like INT24S, INT40S, INT48S, INT56S where numeric_limits<WorkingType>::max()
        // wouldn't work
        maxValue = static_cast<WorkingType>((1ULL << (bitWidth - 1)) - 1);
        minValue = static_cast<WorkingType>(-(1ULL << (bitWidth - 1)));
    }
    else
    {
        // We use emberAfAttributeSize for cases like INT24U, INT40U, INT48U, INT56U where numeric_limits<WorkingType>::max()
        // wouldn't work
        if (ZCL_INT64U_ATTRIBUTE_TYPE == app::Compatibility::Internal::AttributeBaseType(metadata->attributeType))
        {
            maxValue = static_cast<WorkingType>(UINT64_MAX); // Bit shift of 64 is undefined so we use UINT64_MAX
        }
        else
        {
            maxValue = static_cast<WorkingType>((1ULL << bitWidth) - 1);
        }
        minValue = static_cast<WorkingType>(0);
    }

    // Ensure that the metadata's signedness matches the working type's signedness
    VerifyOrDie(metadata->IsSignedIntegerAttribute() == std::is_signed<WorkingType>::value);

    if (metadata->IsBoolean())
    {
        if (metadata->IsNullable() && (value != 1 && value != 0))
        {
            // If the attribute is nullable, the value can be set to NULL
            app::NumericAttributeTraits<WorkingType>::SetNull(value);
        }
        else
        {
            // Caping the value to 1 in case values greater than 1 are set
            value = value ? 1 : 0;
        }
        return;
    }

    // Check metadata for min and max values
    if (metadata->HasMinMax())
    {
        const EmberAfAttributeMinMaxValue * minMaxValue = metadata->defaultValue.ptrToMinMaxValue;
        minValue                                        = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->minValue);
        maxValue                                        = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->maxValue);
    }

    // If the attribute is nullable, the min and max values calculated for types will not be valid, however this does not
    // change the behavior here as the value will already be NULL if it is out of range. E.g. a nullable INT8U has a minValue of
    // -127. The code above determin minValue = -128, so an input value of -128 would not enter the condition block below, but would
    // be considered NULL nonetheless.
    if (metadata->IsNullable() && (minValue > value || maxValue < value))
    {
        // If the attribute is nullable, the value can be set to NULL
        app::NumericAttributeTraits<WorkingType>::SetNull(value);
        return;
    }

    if (minValue > value)
    {
        value = minValue;
    }
    else if (maxValue < value)
    {
        value = maxValue;
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

    // There should never be more than one populated value in an ExtensionFieldSet
    VerifyOrReturnError(IsExactlyOneValuePopulated(aVPair), CHIP_ERROR_INVALID_ARGUMENT);

    switch (app::Compatibility::Internal::AttributeBaseType(metadata->attributeType))
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
    case ZCL_INT8U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint8_t>(aVPair.valueUnsigned8.Value(), metadata);
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint16_t>(aVPair.valueUnsigned16.Value(), metadata);
        break;
    case ZCL_INT24U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<3, false>>(aVPair.valueUnsigned32.Value(), metadata);
        break;
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint32_t>(aVPair.valueUnsigned32.Value(), metadata);
        break;
    case ZCL_INT40U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<5, false>>(aVPair.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT48U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<6, false>>(aVPair.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT56U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<7, false>>(aVPair.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint64_t>(aVPair.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int8_t>(aVPair.valueSigned8.Value(), metadata);
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int16_t>(aVPair.valueSigned16.Value(), metadata);
        break;
    case ZCL_INT24S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<3, true>>(aVPair.valueSigned32.Value(), metadata);
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int32_t>(aVPair.valueSigned32.Value(), metadata);
        break;
    case ZCL_INT40S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<5, true>>(aVPair.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT48S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<6, true>>(aVPair.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT56S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<7, true>>(aVPair.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(aVPair.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int64_t>(aVPair.valueSigned64.Value(), metadata);
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
