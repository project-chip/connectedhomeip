/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/scenes-server/CodegenAttributeValuePairValidator.h>
#include <app/util/attribute-storage.h>
#include <app/util/ember-io-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/odd-sized-integers.h>

namespace chip::scenes {
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
///
/// The TypeForMinMax template parameter determines the type to use for the
/// min/max computation.  The Type template parameter determines how the
/// resulting value is actually represented, because for booleans we
/// unfortunately end up using uint8, not an actual boolean.
///
/// @param[in] value   The value from the AttributeValuePairType that we were given.
/// @param[in] metadata  The metadata for the attribute the value is for.
///
///
///
template <typename Type, typename TypeForMinMax = Type>
void CapAttributeValue(typename app::NumericAttributeTraits<Type>::WorkingType & value, const EmberAfAttributeMetadata * metadata)
{
    using IntType     = app::NumericAttributeTraits<TypeForMinMax>;
    using WorkingType = std::remove_reference_t<decltype(value)>;

    WorkingType minValue = IntType::MinValue(metadata->IsNullable());
    WorkingType maxValue = IntType::MaxValue(metadata->IsNullable());

    // Check metadata for min and max values
    if (metadata->HasMinMax())
    {
        const EmberAfAttributeMinMaxValue * minMaxValue = metadata->defaultValue.ptrToMinMaxValue;
        minValue                                        = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->minValue);
        maxValue                                        = ConvertDefaultValueToWorkingValue<Type>(minMaxValue->maxValue);
    }

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

} // namespace

CHIP_ERROR CodegenAttributeValuePairValidator::Validate(const app::ConcreteClusterPath & clusterPath,
                                                        AttributeValuePairType & value)
{
    const EmberAfAttributeMetadata * metadata =
        emberAfLocateAttributeMetadata(clusterPath.mEndpointId, clusterPath.mClusterId, value.attributeID);

    if (nullptr == metadata)
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    // There should never be more than one populated value in an ExtensionFieldSet
    VerifyOrReturnError(IsExactlyOneValuePopulated(value), CHIP_ERROR_INVALID_ARGUMENT);

    switch (app::Compatibility::Internal::AttributeBaseType(metadata->attributeType))
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint8_t, bool>(value.valueUnsigned8.Value(), metadata);
        break;
    case ZCL_INT8U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint8_t>(value.valueUnsigned8.Value(), metadata);
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint16_t>(value.valueUnsigned16.Value(), metadata);
        break;
    case ZCL_INT24U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<3, false>>(value.valueUnsigned32.Value(), metadata);
        break;
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint32_t>(value.valueUnsigned32.Value(), metadata);
        break;
    case ZCL_INT40U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<5, false>>(value.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT48U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<6, false>>(value.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT56U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<7, false>>(value.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueUnsigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<uint64_t>(value.valueUnsigned64.Value(), metadata);
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int8_t>(value.valueSigned8.Value(), metadata);
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int16_t>(value.valueSigned16.Value(), metadata);
        break;
    case ZCL_INT24S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<3, true>>(value.valueSigned32.Value(), metadata);
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned32.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int32_t>(value.valueSigned32.Value(), metadata);
        break;
    case ZCL_INT40S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<5, true>>(value.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT48S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<6, true>>(value.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT56S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<OddSizedInteger<7, true>>(value.valueSigned64.Value(), metadata);
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        VerifyOrReturnError(value.valueSigned64.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        CapAttributeValue<int64_t>(value.valueSigned64.Value(), metadata);
        break;
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::scenes
