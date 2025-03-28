// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyPreference (cluster code: 155/0x9B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EnergyPreference/EnergyPreferenceIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace EnergyPreference {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kEnergyBalancesEntry = {
    .attributeId    = EnergyPreference::Attributes::EnergyBalances::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentEnergyBalanceEntry = {
    .attributeId    = EnergyPreference::Attributes::CurrentEnergyBalance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEnergyPrioritiesEntry = {
    .attributeId    = EnergyPreference::Attributes::EnergyPriorities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLowPowerModeSensitivitiesEntry = {
    .attributeId    = EnergyPreference::Attributes::LowPowerModeSensitivities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentLowPowerModeSensitivityEntry = {
    .attributeId    = EnergyPreference::Attributes::CurrentLowPowerModeSensitivity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace EnergyPreference
} // namespace clusters
} // namespace app
} // namespace chip
