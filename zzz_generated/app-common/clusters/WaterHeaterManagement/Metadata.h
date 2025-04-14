// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterHeaterManagement (cluster code: 148/0x94)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WaterHeaterManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace HeaterTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::HeaterTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HeaterTypes
namespace HeatDemand {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::HeatDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HeatDemand
namespace TankVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::TankVolume::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TankVolume
namespace EstimatedHeatRequired {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::EstimatedHeatRequired::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EstimatedHeatRequired
namespace TankPercentage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::TankPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TankPercentage
namespace BoostState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BoostState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BoostState

} // namespace Attributes

namespace Commands {
namespace Boost {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::Boost::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace Boost
namespace CancelBoost {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::CancelBoost::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace CancelBoost

} // namespace Commands
} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
