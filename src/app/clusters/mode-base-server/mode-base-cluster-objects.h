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
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

namespace Attributes {

namespace SupportedModes {
static constexpr AttributeId Id = 0x00000000;
struct TypeInfo
{
    using Type             = DataModel::List<const detail::Structs::ModeOptionStruct::Type>;
    using DecodableType    = DataModel::DecodableList<detail::Structs::ModeOptionStruct::DecodableType>;
    using DecodableArgType = DecodableType &;

    static constexpr AttributeId GetAttributeId() { return Attributes::SupportedModes::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace SupportedModes

namespace CurrentMode {
static constexpr AttributeId Id = 0x00000001;
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr AttributeId GetAttributeId() { return Attributes::CurrentMode::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace CurrentMode

namespace StartUpMode {
static constexpr AttributeId Id = 0x00000002;
struct TypeInfo
{
    using Type             = DataModel::Nullable<uint8_t>;
    using DecodableType    = DataModel::Nullable<uint8_t>;
    using DecodableArgType = const DecodableType &;

    static constexpr AttributeId GetAttributeId() { return Attributes::StartUpMode::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace StartUpMode

namespace OnMode {
static constexpr AttributeId Id = 0x00000003;
struct TypeInfo
{
    using Type             = DataModel::Nullable<uint8_t>;
    using DecodableType    = DataModel::Nullable<uint8_t>;
    using DecodableArgType = const DecodableType &;

    static constexpr AttributeId GetAttributeId() { return Attributes::OnMode::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace OnMode

namespace GeneratedCommandList {
static constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
static constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace EventList {
static constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList

namespace AttributeList {
static constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
static constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
static constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes

enum class Feature : uint32_t
{
    kOnOff = 0x1,
};

namespace Commands {

namespace ChangeToModeResponse {

static constexpr CommandId Id = 0x00000001;

enum class Fields : uint8_t
{
    kStatus     = 0,
    kStatusText = 1,
};

struct Type
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ChangeToModeResponse::Id; }

    uint8_t status = static_cast<uint8_t>(0);
    Optional<CharSpan> statusText;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ChangeToModeResponse::Id; }

    uint8_t status = static_cast<uint8_t>(0);
    Optional<CharSpan> statusText;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace ChangeToModeResponse

namespace ChangeToMode {

static constexpr CommandId Id = 0x00000000;

enum class Fields : uint8_t
{
    kNewMode = 0,
};

struct Type
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ChangeToMode::Id; }

    uint8_t newMode = static_cast<uint8_t>(0);

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const;

    using ResponseType = Clusters::ModeBase::Commands::ChangeToModeResponse::DecodableType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ChangeToMode::Id; }
    uint8_t newMode = static_cast<uint8_t>(0);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace ChangeToMode

} // namespace Commands

// enums
// Enum for ModeTag
enum class ModeTag : uint16_t
{
    kAuto      = 0x0,
    kQuick     = 0x1,
    kQuiet     = 0x2,
    kLowNoise  = 0x3,
    kLowEnergy = 0x4,
    kVacation  = 0x5,
    kMin       = 0x6,
    kMax       = 0x7,
    kNight     = 0x8,
    kDay       = 0x9,
};

// Enum for StatusCode
enum class StatusCode : uint8_t
{
    kSuccess         = 0x0,
    kUnsupportedMode = 0x1,
    kGenericFailure  = 0x2,
    kInvalidInMode   = 0x3,
};

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
