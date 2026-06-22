/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
#include <app/util/util.h>
#include <clusters/DeviceEnergyManagementMode/AttributeIds.h>
#include <clusters/DeviceEnergyManagementMode/Attributes.h>
#include <clusters/DeviceEnergyManagementMode/CommandIds.h>
#include <clusters/DeviceEnergyManagementMode/Commands.h>
#include <clusters/DeviceEnergyManagementMode/Enums.h>
#include <clusters/DeviceEnergyManagementMode/Metadata.h>
#include <clusters/DishwasherMode/Metadata.h>
#include <clusters/EnergyEvseMode/Metadata.h>
#include <clusters/LaundryWasherMode/Metadata.h>
#include <clusters/MicrowaveOvenMode/Metadata.h>
#include <clusters/OvenMode/Metadata.h>
#include <clusters/RefrigeratorAndTemperatureControlledCabinetMode/Metadata.h>
#include <clusters/RvcCleanMode/Metadata.h>
#include <clusters/RvcRunMode/Metadata.h>
#include <clusters/WaterHeaterMode/Metadata.h>
#include <lib/core/DataModelTypes.h>

#include <utility>

namespace chip::app::Clusters::ModeBase {

// A pair of cluster ID and revision.
struct ClusterRevisionEntry
{
    ClusterId id;
    uint32_t revision;
};

// The 10 clusters that share this attribute structure.
static constexpr ClusterRevisionEntry kAliasedClusters[] = {
    { DeviceEnergyManagementMode::Id, DeviceEnergyManagementMode::kRevision },
    { DishwasherMode::Id, DishwasherMode::kRevision },
    { EnergyEvseMode::Id, EnergyEvseMode::kRevision },
    { LaundryWasherMode::Id, LaundryWasherMode::kRevision },
    { MicrowaveOvenMode::Id, MicrowaveOvenMode::kRevision },
    { OvenMode::Id, OvenMode::kRevision },
    { RefrigeratorAndTemperatureControlledCabinetMode::Id, RefrigeratorAndTemperatureControlledCabinetMode::kRevision },
    { RvcCleanMode::Id, RvcCleanMode::kRevision },
    { RvcRunMode::Id, RvcRunMode::kRevision },
    { WaterHeaterMode::Id, WaterHeaterMode::kRevision },
};

// All aliased clusters share features, mandatory attributes, and commands (except MicrowaveOvenMode).
using Feature      = DeviceEnergyManagementMode::Feature;
namespace Commands = DeviceEnergyManagementMode::Commands;

namespace Attributes {

namespace SupportedModes = DeviceEnergyManagementMode::Attributes::SupportedModes;
namespace CurrentMode    = DeviceEnergyManagementMode::Attributes::CurrentMode;

namespace StartUpMode {
static constexpr AttributeId Id = 0x00000002;
struct TypeInfo
{
    using Type             = DataModel::Nullable<uint8_t>;
    using DecodableType    = DataModel::Nullable<uint8_t>;
    using DecodableArgType = const DecodableType &;

    static constexpr AttributeId GetAttributeId() { return StartUpMode::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StartUpMode

namespace OnMode {
static constexpr AttributeId Id = 0x00000003;
struct TypeInfo
{
    using Type             = DataModel::Nullable<uint8_t>;
    using DecodableType    = DataModel::Nullable<uint8_t>;
    using DecodableArgType = const DecodableType &;

    static constexpr AttributeId GetAttributeId() { return OnMode::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OnMode

constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    SupportedModes::kMetadataEntry,
    CurrentMode::kMetadataEntry,
};

namespace GeneratedCommandList = DeviceEnergyManagementMode::Attributes::GeneratedCommandList;
namespace AcceptedCommandList  = DeviceEnergyManagementMode::Attributes::AcceptedCommandList;
namespace AttributeList        = DeviceEnergyManagementMode::Attributes::AttributeList;
namespace FeatureMap           = DeviceEnergyManagementMode::Attributes::FeatureMap;
namespace ClusterRevision      = DeviceEnergyManagementMode::Attributes::ClusterRevision;

} // namespace Attributes

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

} // namespace chip::app::Clusters::ModeBase
