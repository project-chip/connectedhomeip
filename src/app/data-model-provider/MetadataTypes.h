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
#include <lib/support/BitMask.h>
#include <lib/support/TypeTraits.h>

// Pragma macro to disable the "conversion" and "narrowing" warnings.
// This is done in some sections of the code in order to allow
// the use of narrowing masks when assigning values to bitfields variables.
// Without the use of these macros, the compiler would not allow
// the narrowing and conversion of input values during the settings
// of the variables inside of both 'AttributeEntry.mask' and 'AcceptedCommandEntry.mask'.
#define _StartBitFieldInit                                                                                                         \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wconversion\"")                                              \
        _Pragma("GCC diagnostic ignored \"-Wnarrowing\"")
#define _EndBitFieldInit _Pragma("GCC diagnostic pop")

namespace chip {
namespace app {
namespace DataModel {

/// Represents various endpoint composition patterns as defined in the spec
/// as `9.2.1. Endpoint Composition patterns`
enum class EndpointCompositionPattern : uint8_t
{
    // Tree pattern supports a general tree of endpoints. Commonly used for
    // device types that support physical device composition (e.g. Refrigerator)
    kTree = 0x1,

    // A full-family pattern is a list of all descendant endpoints, with no
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
    DataVersion dataVersion; // current cluster data version,
    BitFlags<ClusterQualityFlags> flags;
};

struct ClusterInfo
{
    DataVersion dataVersion; // current cluster data version,
    BitFlags<ClusterQualityFlags> flags;

    /// Constructor that marks data version as mandatory
    /// for this structure.
    ClusterInfo(DataVersion version) : dataVersion(version) {}
};

// Constants used to narrow binary expressions
constexpr uint8_t kPrivilegeBits   = 5;
constexpr uint8_t kAttrQualityBits = 7;
constexpr uint8_t kCmdQualityBits  = 3;

// Masks used in the narrowing of binary expressions
constexpr uint8_t kPrivilegeMask   = ((1 << kPrivilegeBits) - 1);
constexpr uint8_t kAttrQualityMask = ((1 << kAttrQualityBits) - 1);
constexpr uint8_t kCmdQualityMask  = ((1 << kCmdQualityBits) - 1);

enum class AttributeQualityFlags : uint32_t
{
    kListAttribute   = 0x0004, // This attribute is a list attribute
    kFabricScoped    = 0x0008, // 'F' quality on attributes
    kFabricSensitive = 0x0010, // 'S' quality on attributes
    kChangesOmitted  = 0x0020, // `C` quality on attributes
    kTimed           = 0x0040, // `T` quality on attributes (writes require timed interactions)

    // If you add new items here, remember to change kAttrQualityBits
};

struct AttributeEntry
{
    AttributeId attributeId;

    // Constructor

    _StartBitFieldInit; // Disabling '-Wconversion' & '-Wconversion'

    constexpr AttributeEntry(AttributeId id = 0, BitMask<AttributeQualityFlags> attrQualityFlags = BitMask<AttributeQualityFlags>(),
                             std::optional<Access::Privilege> readPriv  = std::nullopt,
                             std::optional<Access::Privilege> writePriv = std::nullopt) :
        attributeId{ id }, mask{
            .flags          = attrQualityFlags.Raw() & kAttrQualityMask,
            .readPrivilege  = readPriv.has_value() ? (to_underlying(*readPriv) & kPrivilegeMask) : 0,
            .writePrivilege = writePriv.has_value() ? (to_underlying(*writePriv) & kPrivilegeMask) : 0,
        }
    {}

    _EndBitFieldInit; // Enabling '-Wconversion' & '-Wconversion'

    // Getter for mask.readPrivilege
    [[nodiscard]] constexpr std::optional<Access::Privilege> GetReadPrivilege() const
    {
        if (ReadAllowed())
        {
            return static_cast<Access::Privilege>(mask.readPrivilege);
        }
        return std::nullopt;
    }

    // Getter for mask.writePrivilege
    [[nodiscard]] constexpr std::optional<Access::Privilege> GetWritePrivilege() const
    {
        if (WriteAllowed())
        {
            return static_cast<Access::Privilege>(mask.writePrivilege);
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr bool HasFlags(AttributeQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }

    [[nodiscard]] constexpr bool ReadAllowed() const { return mask.readPrivilege != 0; }
    [[nodiscard]] constexpr bool WriteAllowed() const { return mask.writePrivilege != 0; }

private:
    struct attribute_entry_mask_t
    {

        // attribute quality flags
        //
        // flags is a uint32_t bitfield of size kAttrQualityBits (aka 7),
        // in order to accomodate all the different values of
        // "enum class AttributeQualityFlags".
        //
        // Consider that any modification on the declaration of
        // "enum class AttributeQualityFlags" will affect flags.
        std::underlying_type_t<AttributeQualityFlags> flags : kAttrQualityBits;

        // read/write access privilege variables
        //
        // readPrivilege is a uint8_t bitfield of size kPrivilegeBits (aka 5),
        // in order to accomodate all the different values of "enum class Privilege".
        // Same case for writePrivilege.
        //
        // Consider that any modification on the declaration of "enum class Privilege"
        // will affect both readPrivilege and writePrivilege.
        //
        // The use of bitfields means that each variable holds an individual
        // Access::Privilege value, as a bitwise value. This allows us to
        // handle Access::Privilege information without any bit fiddling.
        std::underlying_type_t<Access::Privilege> readPrivilege : kPrivilegeBits;
        std::underlying_type_t<Access::Privilege> writePrivilege : kPrivilegeBits;
    };

    // Static ASSERT to check size of mask type "attribute_entry_mask_t"
    static_assert(sizeof(attribute_entry_mask_t) <= 4, "Size of attribute_entry_mask_t is not as expected.");

    attribute_entry_mask_t mask;
};

// Static ASSERT to check size of AttributeEntry
static_assert(sizeof(AttributeEntry) <= 8, "Size of AttributeEntry is not as expected.");

// Bitmask values for different Command qualities.
enum class CommandQualityFlags : uint32_t
{
    kFabricScoped = 0x0001,
    kTimed        = 0x0002, // `T` quality on commands
    kLargeMessage = 0x0004, // `L` quality on commands

    // If you add new items here, remember to change kCmdQualityBits
};

struct AcceptedCommandEntry
{
    CommandId commandId;

    // Constructor

    _StartBitFieldInit; // Disabling '-Wconversion' & '-Wconversion'

    constexpr AcceptedCommandEntry(CommandId id = 0, BitMask<CommandQualityFlags> cmdQualityFlags = BitMask<CommandQualityFlags>(),
                                   Access::Privilege invokePriv = Access::Privilege::kOperate) :
        commandId(id), mask{
            .flags           = cmdQualityFlags.Raw() & kCmdQualityMask,
            .invokePrivilege = to_underlying(invokePriv) & kPrivilegeMask,
        }
    {}

    _EndBitFieldInit; // Enabling '-Wconversion' & '-Wconversion'

    // Getter for mask.invokePrivilege
    [[nodiscard]] constexpr Access::Privilege GetInvokePrivilege() const
    {
        return static_cast<Access::Privilege>(mask.invokePrivilege);
    }

    [[nodiscard]] constexpr bool HasFlags(CommandQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }

private:
    struct accepted_command_entry_mask_t
    {
        // command quality flags
        //
        std::underlying_type_t<CommandQualityFlags> flags : 3;

        std::underlying_type_t<Access::Privilege> invokePrivilege : 5;
    };

    // Static ASSERT to check size of mask type "accepted_command_entry_mask_t"
    static_assert(sizeof(accepted_command_entry_mask_t) <= 4, "Size of accepted_command_entry_mask_t is not as expected.");

    accepted_command_entry_mask_t mask;
};

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

#undef _StartBitFieldInit
#undef _EndBitFieldInit
