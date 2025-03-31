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
namespace clusters {
namespace ServiceArea {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedAreas {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::SupportedAreas::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedAreas
namespace SupportedMaps {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::SupportedMaps::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedMaps
namespace SelectedAreas {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::SelectedAreas::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SelectedAreas
namespace CurrentArea {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::CurrentArea::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentArea
namespace EstimatedEndTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::EstimatedEndTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EstimatedEndTime
namespace Progress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ServiceArea::Attributes::Progress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Progress

} // namespace Attributes

namespace Commands {
namespace SelectAreas {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = ServiceArea::Commands::SelectAreas::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SelectAreas
namespace SkipArea {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = ServiceArea::Commands::SkipArea::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SkipArea

} // namespace Commands
} // namespace ServiceArea
} // namespace clusters
} // namespace app
} // namespace chip
