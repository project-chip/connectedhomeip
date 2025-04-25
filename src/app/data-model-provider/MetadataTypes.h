/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>
#include <optional>

#include <access/Privilege.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TypeTraits.h>

#define StartBitFieldInit                                                                                                          \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wconversion\"")                                              \
        _Pragma("GCC diagnostic ignored \"-Wnarrowing\"")
#define EndBitFieldInit _Pragma("GCC diagnostic pop")

namespace chip {
namespace app {
namespace DataModel {

/// Represents various endpoint composition patters as defined in the spec
/// as `9.2.1. Endpoint Composition patterns`
enum class EndpointCompositionPattern : uint8_t
{
    // Tree pattern supports a general tree of endpoints. Commonly used for
    // device types that support physical device composition (e.g. Refrigerator)
    kTree = 0x1,

    // A full-family pattern is a list fo all descendant endpoints, with no
    // imposed hierarchy.
    //
    // For example the Root Node and Aggregator device types use the full-familiy
    // pattern, as defined in their device type specification
    kFullFamily = 0x2,
};

struct EndpointEntry
{
    EndpointId id;

    // kInvalidEndpointId if there is no explicit parent endpoint (which means the parent is endpoint 0,
    // for endpoints other than endpoint 0).
    EndpointId parentId;
    EndpointCompositionPattern compositionPattern;
};

enum class ClusterQualityFlags : uint32_t
{
    kDiagnosticsData = 0x0001, // `K` quality, may be filtered out in subscriptions
};

struct ServerClusterEntry
{
    ClusterId clusterId;
    DataVersion dataVersion; // current cluster data version
    BitFlags<ClusterQualityFlags> flags;
};

struct ClusterInfo
{
    /// Constructor that marks data version as mandatory
    /// for this structure.
    ClusterInfo(DataVersion version) : dataVersion(version) {}
    DataVersion dataVersion; // current cluster data version,
    // This cannot be compressed yet
    BitFlags<ClusterQualityFlags> flags;
};

enum class AttributeQualityFlags : uint32_t
{
    kListAttribute   = 0x0004, // This attribute is a list attribute
    kFabricScoped    = 0x0008, // 'F' quality on attributes
    kFabricSensitive = 0x0010, // 'S' quality on attributes
    kChangesOmitted  = 0x0020, // `C` quality on attributes
    kTimed           = 0x0040, // `T` quality on attributes (writes require timed interactions)
};
// We need 5 bits to handle it or 7 keeping the 2 zeroes
// kListAttribute   = 0b0000100
// kFabricScoped    = 0b0001000
// kFabricSensitive = 0b0010000
// kChangesOmitted  = 0b0100000
// kTimed           = 0b1000000

constexpr size_t kAttrQFlagsBits   = 5;
constexpr size_t kAttrQFlagsOffset = 2;

static_assert(CheckEnumBits<AttributeQualityFlags::kListAttribute, kAttrQFlagsBits, kAttrQFlagsOffset>(),
              "All AttributeQualityFlags must fit inside kAttrQFlagsBits when kAttrQFlagsOffset applied");
static_assert(CheckEnumBits<AttributeQualityFlags::kFabricScoped, kAttrQFlagsBits, kAttrQFlagsOffset>(),
              "All AttributeQualityFlags must fit inside kAttrQFlagsBits when kAttrQFlagsOffset applied");
static_assert(CheckEnumBits<AttributeQualityFlags::kFabricSensitive, kAttrQFlagsBits, kAttrQFlagsOffset>(),
              "All AttributeQualityFlags must fit inside kAttrQFlagsBits when kAttrQFlagsOffset applied");
static_assert(CheckEnumBits<AttributeQualityFlags::kChangesOmitted, kAttrQFlagsBits, kAttrQFlagsOffset>(),
              "All AttributeQualityFlags must fit inside kAttrQFlagsBits when kAttrQFlagsOffset applied");
static_assert(CheckEnumBits<AttributeQualityFlags::kTimed, kAttrQFlagsBits, kAttrQFlagsOffset>(),
              "All AttributeQualityFlags must fit inside kAttrQFlagsBits when kAttrQFlagsOffset applied");

struct AttributeEntry
{

    StartBitFieldInit // Keep constructor the same
        constexpr AttributeEntry(AttributeId attributeId, BitFlags<AttributeQualityFlags> flags,
                                 std::optional<Access::Privilege> readPrivilege  = Access::Privilege::kView,
                                 std::optional<Access::Privilege> writePrivilege = Access::Privilege::kOperate) :
        attributeId_{ attributeId }, flags_{ flags.Raw() >> kAttrQFlagsOffset },
        readPrivilege_{ !readPrivilege ? 0 : chip::to_underlying(readPrivilege.value()) },
        writePrivilege_{ !writePrivilege ? 0 : chip::to_underlying(writePrivilege.value()) }
    {}
    EndBitFieldInit;

    constexpr AttributeId GetAttributeId() const { return attributeId_; }
    constexpr BitFlags<AttributeQualityFlags> GetAttributeQualityFlags() const
    {
        return BitFlags<AttributeQualityFlags>{ std::underlying_type_t<AttributeQualityFlags>(flags_) << kAttrQFlagsOffset };
    }
    constexpr std::optional<Access::Privilege> GetReadPrivilege() const
    {
        if (!readPrivilege_)
            return std::nullopt;
        return static_cast<Access::Privilege>(readPrivilege_);
    }
    constexpr std::optional<Access::Privilege> GetWritePrivilege() const
    {
        if (!writePrivilege_)
            return std::nullopt;
        return static_cast<Access::Privilege>(writePrivilege_);
    }

private:
    AttributeId attributeId_;                                                                 // 4
    std::underlying_type_t<AttributeQualityFlags> flags_ : kAttrQFlagsBits;                   // 4
    std::underlying_type_t<Access::Privilege> readPrivilege_ : Access::kAccessPrivilegeBits;  // 1
    std::underlying_type_t<Access::Privilege> writePrivilege_ : Access::kAccessPrivilegeBits; // 1
};

// From 10 + padding 6 = 16 bytes
// to   7 + 1 = 8 bytes
static_assert(sizeof(AttributeEntry) == 8);

// Sanity checks
static_assert(AttributeEntry{ 0, AttributeQualityFlags::kTimed }.GetAttributeQualityFlags().Has(AttributeQualityFlags::kTimed));
static_assert(
    !AttributeEntry{ 0, AttributeQualityFlags::kTimed }.GetAttributeQualityFlags().Has(AttributeQualityFlags::kListAttribute));
static_assert(!AttributeEntry{ 0, AttributeQualityFlags::kTimed, std::nullopt }.GetReadPrivilege());
static_assert(AttributeEntry{ 0, AttributeQualityFlags::kTimed, Access::Privilege::kAdminister }.GetReadPrivilege().value() ==
              Access::Privilege::kAdminister);
static_assert(
    !AttributeEntry{ 0, AttributeQualityFlags::kTimed, Access::Privilege::kAdminister, std::nullopt }.GetWritePrivilege());
static_assert(AttributeEntry{ 0, AttributeQualityFlags::kTimed, Access::Privilege::kAdminister, Access::Privilege::kAdminister }
                  .GetWritePrivilege()
                  .value() == Access::Privilege::kAdminister);

// Bitmask values for different Command qualities.
// We need 3 bits for this one
enum class CommandQualityFlags : uint32_t
{
    kFabricScoped = 0x0001,
    kTimed        = 0x0002, // `T` quality on commands
    kLargeMessage = 0x0004, // `L` quality on commands
};
// We need 3 bits to handle it
// kFabricScoped = 0b001
// kTimed        = 0b010
// kLargeMessage = 0b100
constexpr size_t kCommandQFlagsBits = 3;
static_assert(CheckEnumBits<CommandQualityFlags::kFabricScoped, kCommandQFlagsBits>(),
              "All CommandQualityFlags must fir inside kCommandQFlagsBits bits");
static_assert(CheckEnumBits<CommandQualityFlags::kTimed, kCommandQFlagsBits>(),
              "All CommandQualityFlags must fir inside kCommandQFlagsBits bits");
static_assert(CheckEnumBits<CommandQualityFlags::kLargeMessage, kCommandQFlagsBits>(),
              "All CommandQualityFlags must fir inside kCommandQFlagsBits bits");

struct AcceptedCommandEntry
{
    StartBitFieldInit;
    // Lets keep the interface the same
    constexpr AcceptedCommandEntry(CommandId commandId = 0, BitFlags<CommandQualityFlags> flags = {},
                                   Access::Privilege invokePrivilege = Access::Privilege::kOperate) :
        commandId_{ commandId }, flags_{ flags.Raw() }, privilege_{ chip::to_underlying(invokePrivilege) }
    {}
    EndBitFieldInit;
    constexpr CommandId GetCommandId() const { return commandId_; };
    constexpr BitFlags<CommandQualityFlags> GetCommandQualityFlags() const { return BitFlags<CommandQualityFlags>(flags_); }
    constexpr Access::Privilege GetInvokePrivilege() const { return static_cast<Access::Privilege>(flags_); }

private:
    // AcceptedCommandEntry will be 32bit aligned
    CommandId commandId_;                                                                // 4 byte -> 4 byte
    std::underlying_type_t<CommandQualityFlags> flags_ : kCommandQFlagsBits;             // 4 byte -> 3 bit
    std::underlying_type_t<Access::Privilege> privilege_ : Access::kAccessPrivilegeBits; // 1 byte -> 5 bit
                                                                                         // -----------------
                                                                                         // 9 bytes -> 5 bytes
    // because alignment there are 3 more bytes here                32-bit/4-byte Alignment 7 bytes -> 3 bytes
};

// From 9 + padding 7 = 16 bytes
// to   5 + padding 3 = 8 bytes
static_assert(sizeof(AcceptedCommandEntry) == 8);

/// Represents a device type that resides on an endpoint
struct DeviceTypeEntry
{
    DeviceTypeId deviceTypeId;
    uint8_t deviceTypeRevision;

    bool operator==(const DeviceTypeEntry & other) const
    {
        return (deviceTypeId == other.deviceTypeId) && (deviceTypeRevision == other.deviceTypeRevision);
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip

#undef StartBitFieldInit
#undef EndBitFieldInit
