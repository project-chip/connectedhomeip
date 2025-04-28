// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MicrowaveOvenControl (cluster code: 95/0x5F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MicrowaveOvenControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CookTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CookTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CookTime
namespace MaxCookTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxCookTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxCookTime
namespace PowerSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PowerSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerSetting
namespace MinPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinPower
namespace MaxPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPower
namespace PowerStep {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PowerStep::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerStep
namespace SupportedWatts {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedWatts::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedWatts
namespace SelectedWattIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SelectedWattIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SelectedWattIndex
namespace WattRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WattRating::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WattRating

} // namespace Attributes

namespace Commands {
namespace SetCookingParameters {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetCookingParameters::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetCookingParameters
namespace AddMoreTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddMoreTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace AddMoreTime

} // namespace Commands
} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
