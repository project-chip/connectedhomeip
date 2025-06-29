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
namespace MaxUserDefinedZones {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxUserDefinedZones::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxUserDefinedZones
namespace MaxZones {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxZones::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxZones
namespace Zones {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Zones::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Zones
namespace Triggers {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Triggers::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Triggers
namespace SensitivityMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SensitivityMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SensitivityMax
namespace Sensitivity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Sensitivity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Sensitivity
namespace TwoDCartesianMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TwoDCartesianMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TwoDCartesianMax

} // namespace Attributes

namespace Commands {
namespace CreateTwoDCartesianZone {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CreateTwoDCartesianZone::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace CreateTwoDCartesianZone
namespace UpdateTwoDCartesianZone {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdateTwoDCartesianZone::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace UpdateTwoDCartesianZone
namespace RemoveZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveZone::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace RemoveZone
namespace CreateOrUpdateTrigger {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CreateOrUpdateTrigger::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace CreateOrUpdateTrigger
namespace RemoveTrigger {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveTrigger::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace RemoveTrigger

} // namespace Commands
} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
