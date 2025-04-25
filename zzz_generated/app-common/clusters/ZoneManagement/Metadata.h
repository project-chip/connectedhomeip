// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ZoneManagement (cluster code: 1360/0x550)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ZoneManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedZoneSources {
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    SupportedZoneSources::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, std::nullopt
};
} // namespace SupportedZoneSources
namespace Zones {
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    Zones::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, std::nullopt
};
} // namespace Zones
namespace Triggers {
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    Triggers::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, Access::Privilege::kOperate
};
} // namespace Triggers
namespace Sensitivity {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ Sensitivity::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, Access::Privilege::kOperate };
} // namespace Sensitivity

} // namespace Attributes

namespace Commands {
namespace CreateTwoDCartesianZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ ZoneManagement::Commands::CreateTwoDCartesianZone::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kManage };
} // namespace CreateTwoDCartesianZone
namespace UpdateTwoDCartesianZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ ZoneManagement::Commands::UpdateTwoDCartesianZone::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kManage };
} // namespace UpdateTwoDCartesianZone
namespace GetTwoDCartesianZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ ZoneManagement::Commands::GetTwoDCartesianZone::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kManage };
} // namespace GetTwoDCartesianZone
namespace RemoveZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ ZoneManagement::Commands::RemoveZone::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kManage };
} // namespace RemoveZone

} // namespace Commands
} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
