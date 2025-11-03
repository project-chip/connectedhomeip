// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterTankLevelMonitoring (cluster code: 121/0x79)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WaterTankLevelMonitoring/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterTankLevelMonitoring {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Condition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Condition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Condition
namespace DegradationDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DegradationDirection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DegradationDirection
namespace ChangeIndication {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ChangeIndication::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ChangeIndication
namespace InPlaceIndicator {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InPlaceIndicator::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InPlaceIndicator
namespace LastChangedTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LastChangedTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace LastChangedTime
namespace ReplacementProductList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ReplacementProductList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ReplacementProductList
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    ChangeIndication::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ResetCondition {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCondition::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetCondition

} // namespace Commands

namespace Events {} // namespace Events
} // namespace WaterTankLevelMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
