// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ServiceArea (cluster code: 336/0x150)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ServiceArea/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace SupportedAreas {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedAreas::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedAreas
namespace SupportedMaps {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedMaps::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedMaps
namespace SelectedAreas {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SelectedAreas::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SelectedAreas
namespace CurrentArea {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentArea::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentArea
namespace EstimatedEndTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EstimatedEndTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EstimatedEndTime
namespace Progress {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Progress::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Progress

} // namespace Attributes

namespace Commands {
namespace SelectAreas {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SelectAreas::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SelectAreas
namespace SkipArea {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SkipArea::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SkipArea

} // namespace Commands
} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
