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

static constexpr std::array<ClusterId, 2> AliasedClusters = { HepaFilterMonitoring::Id, ActivatedCarbonFilterMonitoring::Id };

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
    kReplacementProductList = 0x3
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

namespace ReplacementProductStruct {
enum class Fields : uint8_t
{
    kProductIdentifierType  = 0,
    kProductIdentifierValue = 1,
};

struct Type
{
public:
    ProductIdentifierTypeEnum productIdentifierType = static_cast<ProductIdentifierTypeEnum>(0);
    chip::CharSpan productIdentifierValue;

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const;
};

using DecodableType = Type;

} // namespace ReplacementProductStruct

namespace ReplacementProductList {
static constexpr AttributeId Id = 0x00000005;
struct TypeInfo
{
    using Type             = chip::app::DataModel::List<const ReplacementProductStruct::Type>;
    using DecodableType    = chip::app::DataModel::DecodableList<ReplacementProductStruct::Type>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<ReplacementProductStruct::Type> &;

    static constexpr AttributeId GetAttributeId() { return Attributes::ReplacementProductList::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace ReplacementProductList

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
