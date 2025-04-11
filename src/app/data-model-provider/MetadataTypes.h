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

#define StartBitFieldInit_1
_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wconversion\"") _Pragma("GCC diagnostic ignored \"-Wnarrowing\"")
#define EndBitFieldInit_1 _Pragma("GCC diagnostic pop")
#define StartBitFieldInit_2
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wconversion\"")
        _Pragma("GCC diagnostic ignored \"-Wnarrowing\"")
#define EndBitFieldInit_2 _Pragma("GCC diagnostic pop")

            namespace chip
{
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

    enum class AttributeQualityFlags : uint32_t
    {
        kListAttribute   = 0x0004, // This attribute is a list attribute
        kFabricScoped    = 0x0008, // 'F' quality on attributes
        kFabricSensitive = 0x0010, // 'S' quality on attributes
        kChangesOmitted  = 0x0020, // `C` quality on attributes
        kTimed           = 0x0040, // `T` quality on attributes (writes require timed interactions)
    };

    struct AttributeEntry
    {
        AttributeId attributeId;

        StartBitFieldInit_1 // Start disabling conversion and narrowing warnings

            // Constructor
            constexpr AttributeEntry(
                AttributeId id = 0,                                                             // attributeId initial value,
                                                                                                // this could be altered later
                AttributeQualityFlags attrQualityFlags = AttributeQualityFlags::kListAttribute, // mask.flags initial value
                Access::Privilege readPriv             = Access::Privilege::kNoPrivilege,       // mask.readPrivilege initial value
                Access::Privilege writePriv            = Access::Privilege::kNoPrivilege        // mask.writePrivilege initial value
                ) :
            attributeId{ id },
            mask{ to_underlying(attrQualityFlags) & ((1 << 7) - 1), // Narrowing expression to 7 bits
                  to_underlying(readPriv) & ((1 << 5) - 1),         // Narrowing expression to 5 bits
                  to_underlying(writePriv) & ((1 << 5) - 1) }       // Narrowing expression to 5 bits
        {}

        AttributeQualityFlags SetFlags(AttributeQualityFlags f)
        {
            return static_cast<AttributeQualityFlags>(mask.flags |= (chip::to_underlying(f) & ((1 << 7) - 1)));
        }

        AttributeQualityFlags SetFlags(AttributeQualityFlags f, bool isSet)
        {
            return isSet ? SetFlags(f)
                         : static_cast<AttributeQualityFlags>(mask.flags &= ~(chip::to_underlying(f) & ((1 << 7) - 1)));
        }

        EndBitFieldInit_1 // Start enabling conversion and narrowing warnings

            // Getter for mask.readPrivilege
            constexpr Access::Privilege
            GetReadPrivilege() const
        {
            return static_cast<Access::Privilege>(mask.readPrivilege);
        }

        // Getter for mask.writePrivilege
        constexpr Access::Privilege GetWritePrivilege() const { return static_cast<Access::Privilege>(mask.writePrivilege); }

        constexpr bool HasFlags(AttributeQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }

        constexpr bool ReadAllowed() const { return mask.readPrivilege != to_underlying(Access::Privilege::kNoPrivilege); }
        constexpr bool WriteAllowed() const { return mask.writePrivilege != to_underlying(Access::Privilege::kNoPrivilege); }

    private:
        struct attribute_entry_mask_t
        {

            // attribute quality flags
            //
            // flags is a uint32_t bitfield of size 7, in order to accomodate all
            // the different values of "enum class AttributeQualityFlags".
            //
            // Consider that any modification on the declaration of
            // "enum class AttributeQualityFlags" will affect flags.
            std::underlying_type_t<AttributeQualityFlags> flags : 7;

            // read/write access privilege variables
            //
            // readPrivilege is a uint8_t bitfield of size 5, in order to accomodate all
            // the different values of "enum class Privilege".
            // Same case for writePrivilege.
            //
            // Consider that any modification on the declaration of "enum class Privilege"
            // will affect both readPrivilege and writePrivilege.
            //
            // The use of bitfields means that each variable holds an individual
            // Access::Privilege value, as a bitwise value. This allows us to
            // handle Access::Privilege information without any bit fiddling.
            std::underlying_type_t<Access::Privilege> readPrivilege : 5;
            std::underlying_type_t<Access::Privilege> writePrivilege : 5;
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
    };

    struct AcceptedCommandEntry
    {
        CommandId commandId;

        StartBitFieldInit_2 // Start disabling conversion and narrowing warnings

            // Constructor
            constexpr AcceptedCommandEntry(
                CommandId id = 0,                                                         // commandId initial value,
                                                                                          // this could be altered later
                CommandQualityFlags cmdQualityFlags = CommandQualityFlags::kFabricScoped, // mask.flags initial value
                Access::Privilege invokePriv        = Access::Privilege::kView            // mask.invokePrivilege initial value
                ) :
            commandId(id),
            mask{ to_underlying(cmdQualityFlags) & ((1 << 3) - 1), // Narrowing expression to 3 bits
                  to_underlying(invokePriv) & ((1 << 5) - 1) }     // Narrowing expression to 5 bits
        {}

        CommandQualityFlags SetFlags(CommandQualityFlags f)
        {
            return static_cast<CommandQualityFlags>(mask.flags |= (chip::to_underlying(f) & ((1 << 3) - 1)));
        }

        CommandQualityFlags SetFlags(CommandQualityFlags f, bool isSet)
        {
            return isSet ? SetFlags(f) : static_cast<CommandQualityFlags>(mask.flags &= ~(chip::to_underlying(f) & ((1 << 3) - 1)));
        }

        EndBitFieldInit_2 // Start enabling conversion and narrowing warnings

            // Getter for mask.invokePrivilege
            constexpr Access::Privilege
            GetInvokePrivilege() const
        {
            return static_cast<Access::Privilege>(mask.invokePrivilege);
        }

        constexpr bool HasFlags(CommandQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }

    private:
        struct accepted_command_entry_mask_t
        {

            // command quality flags
            //
            std::underlying_type_t<CommandQualityFlags> flags : 3; // generally defaults to View if readable

            // invoke privilege variable
            //
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

#undef StartBitFieldInit_1
#undef EndBitFieldInit_1
#undef StartBitFieldInit_2
#undef EndBitFieldInit_2
