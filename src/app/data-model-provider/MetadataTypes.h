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


// zero is not a valid privilege, just a default initial value for privileges
constexpr std::underlying_type_t<Access::Privilege> kNoPrivilege(0) ;


struct AttributeEntry
{

    AttributeId attributeId;

    // Constructor
    constexpr AttributeEntry(AttributeId id = 0,
                             std::underlying_type_t<AttributeQualityFlags> attrQualityFlags = 0,
                             std::underlying_type_t<Access::Privilege> readPriv = kNoPrivilege,
                             std::underlying_type_t<Access::Privilege> writePriv = kNoPrivilege
                            ) : attributeId(id), mask{ attrQualityFlags, readPriv, writePriv } {}



    // Overload assignment operator for mask.readPrivilege
    AttributeEntry& operator=(Access::Privilege value) 
    {
        // Static ASSERT to check size of readPrivilege type vs entry parameter.
        static_assert(sizeof(std::underlying_type_t<Access::Privilege>) >= 
                      sizeof(chip::to_underlying(value)),
                      "Size of readPrivilege is not able to accomodate parameter (value).");

        this->mask.readPrivilege = chip::to_underlying(value);
        return *this;
    }


    // Overload assignment operator for mask.writePrivilege
    AttributeEntry& operator=(std::underlying_type_t<Access::Privilege> value)
    {
        // Static ASSERT to check size of writePrivilege type vs entry parameter.
        static_assert(sizeof(std::underlying_type_t<Access::Privilege>) >= 
                      sizeof(value),
                      "Size of writePrivilege is not able to accomodate parameter (value).");

        this->mask.writePrivilege = value;
        return *this;
    }


    // Getter for mask.readPrivilege
    Access::Privilege GetReadPrivilege() const
    {
        return static_cast< Access::Privilege >(mask.readPrivilege);
    }

    
    // Getter for mask.writePrivilege
    Access::Privilege GetWritePrivilege() const
    {
        return static_cast< Access::Privilege >(mask.writePrivilege);
    }


    AttributeQualityFlags SetFlags(AttributeQualityFlags f)
    {
        return static_cast< AttributeQualityFlags >( mask.flags |= chip::to_underlying(f) ) ;
    }


    AttributeQualityFlags SetFlags(AttributeQualityFlags f, bool isSet)
    {
        return isSet ? 
          SetFlags(f) : 
          static_cast< AttributeQualityFlags >( mask.flags &= ~chip::to_underlying(f) );
    }


    constexpr bool FlagsHas(AttributeQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }   

    bool ReadAllowed() {return mask.readPrivilege != kNoPrivilege;}
    bool WriteAllowed() {return mask.writePrivilege != kNoPrivilege;}


    private:

        struct attribute_entry_mask_t {

            // attribute quality flags
            //
            std::underlying_type_t<AttributeQualityFlags> flags : 7 ;

            // read/write access privilege variables
            //
            std::underlying_type_t<Access::Privilege> readPrivilege : 5 ;
            std::underlying_type_t<Access::Privilege> writePrivilege : 5 ;

        };

        
        // Static ASSERT to check size of mask type "attribute_entry_mask_t".
        static_assert(sizeof(attribute_entry_mask_t) <= 4, "Size of attribute_entry_mask_t is not as expected.");

        attribute_entry_mask_t mask;

};



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

    // Constructor
    constexpr AcceptedCommandEntry(CommandId id = 0,
                                   std::underlying_type_t<CommandQualityFlags> cmdQualityFlags = 0,
                                   std::underlying_type_t<Access::Privilege> invokePriv = kNoPrivilege
                                ) : commandId(id), mask{ cmdQualityFlags, invokePriv } {}
 


    // Overload assignment operator for mask.invokePrivilege
    AcceptedCommandEntry& operator=(Access::Privilege value)
    {
        // Static ASSERT to check size of invokePrivilege type vs entry parameter.
        static_assert(sizeof(std::underlying_type_t<Access::Privilege>) >= 
                             sizeof(chip::to_underlying(value)),
                             "Size of invokePrivilege is not able to accomodate parameter (value).");

        this->mask.invokePrivilege = chip::to_underlying(value);
        return *this;
    }
                            

    // Getter for mask.invokePrivilege
    Access::Privilege GetInvokePrivilege() const
    {
        return static_cast< Access::Privilege >(mask.invokePrivilege);
    }
    
 
    CommandQualityFlags SetFlags(CommandQualityFlags f)
    {
        return static_cast< CommandQualityFlags >( mask.flags |= chip::to_underlying(f) ) ;
    }


    CommandQualityFlags SetFlags(CommandQualityFlags f, bool isSet)
    {
        return isSet ? 
          SetFlags(f) : 
          static_cast< CommandQualityFlags >( mask.flags &= ~chip::to_underlying(f) );
    }
   

    constexpr bool FlagsHas(CommandQualityFlags f) const { return (mask.flags & chip::to_underlying(f)) != 0; }   

    
    private:

        struct accepted_command_entry_mask_t {

            // command quality flags
            //
            std::underlying_type_t<CommandQualityFlags> flags : 3 ;  // generally defaults to View if readable

            // invoke privilege variable
            //
            std::underlying_type_t<Access::Privilege> invokePrivilege : 5 ;

        };


        // Static ASSERT to check size of mask type "accepted_command_entry_mask_t".
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
