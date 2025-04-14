// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ActivatedCarbonFilterMonitoring (cluster code: 114/0x72)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ActivatedCarbonFilterMonitoring/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ActivatedCarbonFilterMonitoring {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Condition {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Condition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Condition
namespace DegradationDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::DegradationDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DegradationDirection
namespace ChangeIndication {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ChangeIndication::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChangeIndication
namespace InPlaceIndicator {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::InPlaceIndicator::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InPlaceIndicator
namespace LastChangedTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LastChangedTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace LastChangedTime
namespace ReplacementProductList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ReplacementProductList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ReplacementProductList

} // namespace Attributes

namespace Commands {
namespace ResetCondition {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ResetCondition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ResetCondition

} // namespace Commands
} // namespace ActivatedCarbonFilterMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
