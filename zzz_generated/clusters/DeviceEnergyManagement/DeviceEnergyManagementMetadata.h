// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DeviceEnergyManagement/DeviceEnergyManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DeviceEnergyManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kESATypeEntry = {
    .attributeId    = Attributes::ESAType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kESACanGenerateEntry = {
    .attributeId    = Attributes::ESACanGenerate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kESAStateEntry = {
    .attributeId    = Attributes::ESAState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMinPowerEntry = {
    .attributeId    = Attributes::AbsMinPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMaxPowerEntry = {
    .attributeId    = Attributes::AbsMaxPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPowerAdjustmentCapabilityEntry = {
    .attributeId    = Attributes::PowerAdjustmentCapability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kForecastEntry = {
    .attributeId    = Attributes::Forecast::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOptOutStateEntry = {
    .attributeId    = Attributes::OptOutState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kPowerAdjustRequestEntry = {
    .commandId       = Commands::PowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelPowerAdjustRequestEntry = {
    .commandId       = Commands::CancelPowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStartTimeAdjustRequestEntry = {
    .commandId       = Commands::StartTimeAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kPauseRequestEntry = {
    .commandId       = Commands::PauseRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kResumeRequestEntry = {
    .commandId       = Commands::ResumeRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kModifyForecastRequestEntry = {
    .commandId       = Commands::ModifyForecastRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRequestConstraintBasedForecastEntry = {
    .commandId       = Commands::RequestConstraintBasedForecast::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelRequestEntry = {
    .commandId       = Commands::CancelRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DeviceEnergyManagement
} // namespace clusters
} // namespace app
} // namespace chip
