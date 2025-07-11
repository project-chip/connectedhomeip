/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPVendorIdentifiers.hpp> // For VendorId
#include <lib/core/GroupId.h>
#include <lib/core/NodeId.h>
#include <lib/core/PasscodeId.h>

namespace chip {

typedef uint8_t ActionId;
typedef uint32_t AttributeId;
typedef uint32_t ClusterId;
typedef uint8_t ClusterStatus;
typedef uint32_t CommandId;
typedef uint16_t CommandRef;
typedef uint64_t CompressedFabricId;
typedef uint32_t DataVersion;
typedef uint32_t DeviceTypeId;
typedef uint32_t ElapsedS;
typedef uint16_t EndpointId;
typedef uint32_t EventId;
typedef uint64_t EventNumber;
typedef uint64_t FabricId;
typedef uint8_t FabricIndex;
typedef uint32_t FieldId;
typedef uint16_t ListIndex;
typedef uint16_t LocalizedStringIdentifier;
typedef uint32_t TransactionId;
typedef uint16_t KeysetId;
typedef uint8_t InteractionModelRevision;
typedef uint32_t SubscriptionId;
typedef uint8_t SceneId;

typedef uint8_t Percent;
typedef uint16_t Percent100ths;
typedef int64_t Energy_mWh;
typedef int64_t Energy_mVAh;
typedef int64_t Energy_mVARh;
typedef int64_t Amperage_mA;
typedef int64_t Power_mW;
typedef int64_t Power_mVA;
typedef int64_t Power_mVAR;
typedef int64_t Voltage_mV;
typedef int64_t Money;

inline constexpr CompressedFabricId kUndefinedCompressedFabricId = 0ULL;
inline constexpr FabricId kUndefinedFabricId                     = 0ULL;

inline constexpr FabricIndex kUndefinedFabricIndex = 0;
inline constexpr FabricIndex kMinValidFabricIndex  = 1;
inline constexpr FabricIndex kMaxValidFabricIndex  = UINT8_MAX - 1;

inline constexpr EndpointId kInvalidEndpointId = 0xFFFF;
inline constexpr EndpointId kRootEndpointId    = 0;
inline constexpr ListIndex kInvalidListIndex   = 0xFFFF; // List index is a uint16 thus 0xFFFF is a invalid list index.
inline constexpr KeysetId kInvalidKeysetId     = 0xFFFF;

// Invalid IC identifier is provisional. Value will most likely change when identifying token is defined
// https://github.com/project-chip/connectedhomeip/issues/24251
inline constexpr uint64_t kInvalidIcId = 0;

// These are MEIs, 0xFFFF is not a valid manufacturer code,
// thus 0xFFFF'FFFF is not a valid MEI.
static constexpr ClusterId kInvalidClusterId     = 0xFFFF'FFFF;
static constexpr AttributeId kInvalidAttributeId = 0xFFFF'FFFF;
static constexpr CommandId kInvalidCommandId     = 0xFFFF'FFFF;
static constexpr EventId kInvalidEventId         = 0xFFFF'FFFF;
static constexpr FieldId kInvalidFieldId         = 0xFFFF'FFFF;

static constexpr uint16_t kMaxVendorId = VendorId::TestVendor4;

static constexpr uint16_t ExtractIdFromMEI(uint32_t aMEI)
{
    constexpr uint32_t kIdMask = 0x0000'FFFF;
    return static_cast<uint16_t>(aMEI & kIdMask);
}

static constexpr uint16_t ExtractVendorFromMEI(uint32_t aMEI)
{
    constexpr uint32_t kVendorMask  = 0xFFFF'0000;
    constexpr uint32_t kVendorShift = 16;
    return static_cast<uint16_t>((aMEI & kVendorMask) >> kVendorShift);
}

static constexpr bool IsValidVendorId(uint16_t aVendorId)
{
    return aVendorId <= kMaxVendorId;
}

constexpr bool IsValidClusterId(ClusterId aClusterId)
{
    const auto id     = ExtractIdFromMEI(aClusterId);
    const auto vendor = ExtractVendorFromMEI(aClusterId);
    // Cluster id suffixes in the range 0x0000 to 0x7FFF indicate a standard
    // cluster.
    //
    // Cluster id suffixes in the range 0xFC00 to 0xFFFE indicate an MS cluster.
    return IsValidVendorId(vendor) && ((vendor == 0x0000 && id <= 0x7FFF) || (vendor >= 0x0001 && id >= 0xFC00 && id <= 0xFFFE));
}

constexpr bool IsGlobalAttribute(AttributeId aAttributeId)
{
    const auto id     = ExtractIdFromMEI(aAttributeId);
    const auto vendor = ExtractVendorFromMEI(aAttributeId);
    // Attribute id suffixes in the range 0xF000 to 0xFFFE indicate a standard
    // global attribute.
    return (vendor == 0x0000 && id >= 0xF000 && id <= 0xFFFE);
}

constexpr bool IsValidAttributeId(AttributeId aAttributeId)
{
    const auto id     = ExtractIdFromMEI(aAttributeId);
    const auto vendor = ExtractVendorFromMEI(aAttributeId);
    // Attribute id suffixes in the range 0x0000 to 0x4FFF indicate a non-global
    // attribute (standard or MS).  The vendor id must not be wildcard in this
    // case.
    return (id <= 0x4FFF && IsValidVendorId(vendor)) || IsGlobalAttribute(aAttributeId);
}

constexpr bool IsValidCommandId(CommandId aCommandId)
{
    const auto id     = ExtractIdFromMEI(aCommandId);
    const auto vendor = ExtractVendorFromMEI(aCommandId);

    // Command id suffixes must be in the range 0x00 to 0xFF, and the prefix
    // must be a valid prefix (standard or MC).
    return id <= 0xFF && IsValidVendorId(vendor);
}

constexpr bool IsValidDeviceTypeId(DeviceTypeId aDeviceTypeId)
{
    const DeviceTypeId kIdMask     = 0x0000'FFFF;
    const DeviceTypeId kVendorMask = 0xFFFF'0000;
    const auto id                  = aDeviceTypeId & kIdMask;
    const auto vendor              = aDeviceTypeId & kVendorMask;
    return vendor <= 0xFFFE'0000 && id <= 0xBFFF;
}

constexpr bool IsValidEndpointId(EndpointId aEndpointId)
{
    return aEndpointId != kInvalidEndpointId;
}

constexpr bool IsValidFabricIndex(FabricIndex fabricIndex)
{
    return (fabricIndex >= kMinValidFabricIndex) && (fabricIndex <= kMaxValidFabricIndex);
}

constexpr bool IsValidFabricId(FabricId fabricId)
{
    return fabricId != kUndefinedFabricId;
}

} // namespace chip
