// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster HepaFilterMonitoring (cluster code: 113/0x71)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/HepaFilterMonitoring/HepaFilterMonitoringIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace HepaFilterMonitoring {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kConditionEntry = {
    .attributeId    = Attributes::Condition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDegradationDirectionEntry = {
    .attributeId    = Attributes::DegradationDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kChangeIndicationEntry = {
    .attributeId    = Attributes::ChangeIndication::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInPlaceIndicatorEntry = {
    .attributeId    = Attributes::InPlaceIndicator::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLastChangedTimeEntry = {
    .attributeId    = Attributes::LastChangedTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kReplacementProductListEntry = {
    .attributeId    = Attributes::ReplacementProductList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetConditionEntry = {
    .commandId       = Commands::ResetCondition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace HepaFilterMonitoring
} // namespace clusters
} // namespace app
} // namespace chip
