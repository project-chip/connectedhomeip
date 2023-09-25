/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/util/util.h>
#include <cstdint>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

// max of 20 characters as defined by the constraint on the ProductIdentifierValue in the specification
static constexpr size_t kProductIdentifierValueMaxNameLength = 20u;

// Enum for ChangeIndicationEnum
enum class ChangeIndicationEnum : uint8_t
{
    kOk       = 0x00,
    kWarning  = 0x01,
    kCritical = 0x02,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = UINT8_MAX,
};

// Enum for DegradationDirectionEnum
enum class DegradationDirectionEnum : uint8_t
{
    kUp   = 0x00,
    kDown = 0x01,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = UINT8_MAX,
};

// Bitmap for Feature
enum class Feature : uint32_t
{
    kCondition              = 0x1,
    kWarning                = 0x2,
    kReplacementProductList = 0x4
};

// Enum for ProductIdentifierTypeEnum
enum class ProductIdentifierTypeEnum : uint8_t
{
    kUpc    = 0x00,
    kGtin8  = 0x01,
    kEan    = 0x02,
    kGtin14 = 0x03,
    kOem    = 0x04
};

// A struct used during reads of the ReplacementProductList to store a single list instance we request
// from the application.
//
// Inherit from an auto-generated struct to pick up the implementation bits, but make
// it private inheritance so people can't accidentally use this struct where the other
// is expected.
struct ReplacementProductStruct : private HepaFilterMonitoring::Structs::ReplacementProductStruct::Type
{
private:
    char productIdentifierValueBuffer[kProductIdentifierValueMaxNameLength];

public:
    static constexpr bool kIsFabricScoped = false;
    virtual ~ReplacementProductStruct()   = default;
    ReplacementProductStruct() {}
    ReplacementProductStruct(ResourceMonitoring::ProductIdentifierTypeEnum aProductIdentifierType,
                             chip::CharSpan aProductIdentifierValue)
    {
        SetProductIdentifierType(aProductIdentifierType);
        SetProductIdentifierValue(aProductIdentifierValue);
    }

    ReplacementProductStruct & operator=(const ReplacementProductStruct & aReplacementProductStruct)
    {
        SetProductIdentifierType(aReplacementProductStruct.GetProductIdentifierType());
        SetProductIdentifierValue(aReplacementProductStruct.GetProductIdentifierValue());
        return *this;
    }

    using HepaFilterMonitoring::Structs::ReplacementProductStruct::Type::Encode;

    /**
     * Sets the product identifier type.
     *
     * @param aProductIdentifierType The product identifier type.
     */
    void SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum aProductIdentifierType)
    {
        productIdentifierType = static_cast<HepaFilterMonitoring::ProductIdentifierTypeEnum>(aProductIdentifierType);
    }

    /**
     * Sets the product identifier value.
     * This implementation will copy the argument into this struct's buffer.
     *
     * @param aProductIdentifierValue The value of the product identifier to set.
     * @return CHIP_ERROR_INVALID_ARGUMENT when aProductIdentifierValue is invalid
     * or the size exceeds kProductIdentifierValueMaxNameLength, returns CHIP_NO_ERROR
     * otherwise.
     */
    CHIP_ERROR SetProductIdentifierValue(chip::CharSpan aProductIdentifierValue)
    {
        VerifyOrReturnError(!aProductIdentifierValue.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(aProductIdentifierValue.size() <= sizeof(productIdentifierValueBuffer), CHIP_ERROR_INVALID_ARGUMENT);

        memcpy(productIdentifierValueBuffer, aProductIdentifierValue.data(), aProductIdentifierValue.size());
        productIdentifierValue = CharSpan(productIdentifierValueBuffer, aProductIdentifierValue.size());

        return CHIP_NO_ERROR;
    }

    ProductIdentifierTypeEnum GetProductIdentifierType() const
    {
        return static_cast<ProductIdentifierTypeEnum>(productIdentifierType);
    };
    chip::CharSpan GetProductIdentifierValue() const { return productIdentifierValue; };
};

namespace Attributes {

namespace Condition {
static constexpr AttributeId Id = 0x00000000;
struct TypeInfo
{
    using Type             = chip::Percent;
    using DecodableType    = chip::Percent;
    using DecodableArgType = chip::Percent;

    static constexpr AttributeId GetAttributeId() { return Attributes::Condition::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace Condition

namespace DegradationDirection {
static constexpr AttributeId Id = 0x00000001;
struct TypeInfo
{
    using Type             = DegradationDirectionEnum;
    using DecodableType    = DegradationDirectionEnum;
    using DecodableArgType = DegradationDirectionEnum;

    static constexpr AttributeId GetAttributeId() { return Attributes::DegradationDirection::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace DegradationDirection

namespace ChangeIndication {
static constexpr AttributeId Id = 0x00000002;
struct TypeInfo
{
    using Type             = ChangeIndicationEnum;
    using DecodableType    = ChangeIndicationEnum;
    using DecodableArgType = ChangeIndicationEnum;

    static constexpr AttributeId GetAttributeId() { return Attributes::ChangeIndication::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace ChangeIndication

namespace InPlaceIndicator {
static constexpr AttributeId Id = 0x00000003;
struct TypeInfo
{
    using Type             = bool;
    using DecodableType    = bool;
    using DecodableArgType = bool;

    static constexpr AttributeId GetAttributeId() { return Attributes::InPlaceIndicator::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace InPlaceIndicator

namespace LastChangedTime {
static constexpr AttributeId Id = 0x00000004;
struct TypeInfo
{
    using Type             = chip::app::DataModel::Nullable<uint32_t>;
    using DecodableType    = chip::app::DataModel::Nullable<uint32_t>;
    using DecodableArgType = const chip::app::DataModel::Nullable<uint32_t> &;

    static constexpr AttributeId GetAttributeId() { return Attributes::LastChangedTime::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace LastChangedTime

namespace ReplacementProductList {
static constexpr AttributeId Id = 0x00000005;
}

namespace GeneratedCommandList {
static constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
struct TypeInfo : public Clusters::Globals::Attributes::GeneratedCommandList::TypeInfo
{
};
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
static constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;

struct TypeInfo : public Clusters::Globals::Attributes::AcceptedCommandList::TypeInfo
{
};
} // namespace AcceptedCommandList

namespace EventList {
static constexpr AttributeId Id = Globals::Attributes::EventList::Id;

struct TypeInfo : public Clusters::Globals::Attributes::EventList::TypeInfo
{
};
} // namespace EventList

namespace AttributeList {
static constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
struct TypeInfo : public Clusters::Globals::Attributes::AttributeList::TypeInfo
{
};
} // namespace AttributeList

namespace FeatureMap {
static constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
struct TypeInfo : public Clusters::Globals::Attributes::FeatureMap::TypeInfo
{
};
} // namespace FeatureMap

namespace ClusterRevision {
static constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;

struct TypeInfo : public Clusters::Globals::Attributes::ClusterRevision::TypeInfo
{
};
} // namespace ClusterRevision

} // namespace Attributes

namespace Commands {
namespace ResetCondition {
static constexpr CommandId Id = 0x00000000;

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ResetCondition::Id; }

    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace ResetCondition
} // namespace Commands

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
