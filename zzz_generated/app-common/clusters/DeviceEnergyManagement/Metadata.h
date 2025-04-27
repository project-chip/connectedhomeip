// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DeviceEnergyManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace ESAType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ESAType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ESAType
namespace ESACanGenerate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ESACanGenerate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ESACanGenerate
namespace ESAState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ESAState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ESAState
namespace AbsMinPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMinPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMinPower
namespace AbsMaxPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMaxPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMaxPower
namespace PowerAdjustmentCapability {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PowerAdjustmentCapability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerAdjustmentCapability
namespace Forecast {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Forecast::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Forecast
namespace OptOutState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OptOutState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OptOutState

} // namespace Attributes

namespace Commands {
namespace PowerAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = PowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace PowerAdjustRequest
namespace CancelPowerAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CancelPowerAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace CancelPowerAdjustRequest
namespace StartTimeAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StartTimeAdjustRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartTimeAdjustRequest
namespace PauseRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = PauseRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace PauseRequest
namespace ResumeRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ResumeRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ResumeRequest
namespace ModifyForecastRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ModifyForecastRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ModifyForecastRequest
namespace RequestConstraintBasedForecast {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RequestConstraintBasedForecast::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace RequestConstraintBasedForecast
namespace CancelRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CancelRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace CancelRequest

} // namespace Commands
} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
