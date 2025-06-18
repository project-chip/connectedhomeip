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
inline constexpr DataModel::AttributeEntry kMetadataEntry(CookTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CookTime
namespace MaxCookTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxCookTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxCookTime
namespace PowerSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerSetting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerSetting
namespace MinPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinPower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinPower
namespace MaxPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxPower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxPower
namespace PowerStep {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerStep::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerStep
namespace SupportedWatts {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedWatts::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedWatts
namespace SelectedWattIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SelectedWattIndex::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SelectedWattIndex
namespace WattRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WattRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WattRating

} // namespace Attributes

namespace Commands {
namespace SetCookingParameters {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetCookingParameters::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace SetCookingParameters
namespace AddMoreTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddMoreTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddMoreTime

} // namespace Commands
} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
