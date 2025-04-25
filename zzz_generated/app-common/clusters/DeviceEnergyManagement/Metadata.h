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
inline constexpr DataModel::AttributeEntry kMetadataEntry{ ESAType::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace ESAType
namespace ESACanGenerate {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ ESACanGenerate::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace ESACanGenerate
namespace ESAState {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ ESAState::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace ESAState
namespace AbsMinPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ AbsMinPower::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace AbsMinPower
namespace AbsMaxPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ AbsMaxPower::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace AbsMaxPower
namespace PowerAdjustmentCapability {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ PowerAdjustmentCapability::Id,
                                                           BitFlags<DataModel::AttributeQualityFlags>{}, Access::Privilege::kView,
                                                           std::nullopt };
} // namespace PowerAdjustmentCapability
namespace Forecast {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ Forecast::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace Forecast
namespace OptOutState {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ OptOutState::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace OptOutState

} // namespace Attributes

namespace Commands {
namespace PowerAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::PowerAdjustRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace PowerAdjustRequest
namespace CancelPowerAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::CancelPowerAdjustRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace CancelPowerAdjustRequest
namespace StartTimeAdjustRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::StartTimeAdjustRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace StartTimeAdjustRequest
namespace PauseRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::PauseRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace PauseRequest
namespace ResumeRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::ResumeRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace ResumeRequest
namespace ModifyForecastRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::ModifyForecastRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace ModifyForecastRequest
namespace RequestConstraintBasedForecast {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DeviceEnergyManagement::Commands::RequestConstraintBasedForecast::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace RequestConstraintBasedForecast
namespace CancelRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{ DeviceEnergyManagement::Commands::CancelRequest::Id,
                                                                 BitFlags<DataModel::CommandQualityFlags>{},
                                                                 Access::Privilege::kOperate };
} // namespace CancelRequest

} // namespace Commands
} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
