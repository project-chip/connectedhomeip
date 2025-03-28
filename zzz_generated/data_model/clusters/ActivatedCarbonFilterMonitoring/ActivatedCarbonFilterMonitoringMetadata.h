// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ActivatedCarbonFilterMonitoring (cluster code: 114/0x72)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ActivatedCarbonFilterMonitoring/ActivatedCarbonFilterMonitoringIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ActivatedCarbonFilterMonitoring {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kConditionEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::Condition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDegradationDirectionEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::DegradationDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kChangeIndicationEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::ChangeIndication::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInPlaceIndicatorEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::InPlaceIndicator::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLastChangedTimeEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kReplacementProductListEntry = {
    .attributeId    = ActivatedCarbonFilterMonitoring::Attributes::ReplacementProductList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetConditionEntry = {
    .commandId       = ActivatedCarbonFilterMonitoring::Commands::ResetCondition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ActivatedCarbonFilterMonitoring
} // namespace clusters
} // namespace app
} // namespace chip
