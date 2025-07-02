// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyPreference (cluster code: 155/0x9B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EnergyPreference/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyPreference {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace EnergyBalances {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EnergyBalances::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace EnergyBalances
namespace CurrentEnergyBalance {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentEnergyBalance::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CurrentEnergyBalance
namespace EnergyPriorities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EnergyPriorities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace EnergyPriorities
namespace LowPowerModeSensitivities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(LowPowerModeSensitivities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace LowPowerModeSensitivities
namespace CurrentLowPowerModeSensitivity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentLowPowerModeSensitivity::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace CurrentLowPowerModeSensitivity

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace EnergyPreference
} // namespace Clusters
} // namespace app
} // namespace chip
