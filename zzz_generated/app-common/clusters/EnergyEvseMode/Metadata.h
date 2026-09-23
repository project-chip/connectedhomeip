// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyEvseMode (cluster code: 157/0x9D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EnergyEvseMode/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvseMode {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

namespace SupportedModes {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedModes::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentMode
namespace CoreModeTags {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CoreModeTags::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CoreModeTags
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    SupportedModes::kMetadataEntry,
    CurrentMode::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ChangeToMode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ChangeToMode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ChangeToMode
namespace ChangeToModeByCoreTag {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ChangeToModeByCoreTag::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace ChangeToModeByCoreTag

} // namespace Commands

namespace Events {} // namespace Events
} // namespace EnergyEvseMode
} // namespace Clusters
} // namespace app
} // namespace chip
