// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterHeaterManagement (cluster code: 148/0x94)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WaterHeaterManagement/WaterHeaterManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WaterHeaterManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kHeaterTypesEntry = {
    .attributeId    = Attributes::HeaterTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHeatDemandEntry = {
    .attributeId    = Attributes::HeatDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTankVolumeEntry = {
    .attributeId    = Attributes::TankVolume::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEstimatedHeatRequiredEntry = {
    .attributeId    = Attributes::EstimatedHeatRequired::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTankPercentageEntry = {
    .attributeId    = Attributes::TankPercentage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBoostStateEntry = {
    .attributeId    = Attributes::BoostState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kBoostEntry = {
    .commandId       = Commands::Boost::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelBoostEntry = {
    .commandId       = Commands::CancelBoost::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace WaterHeaterManagement
} // namespace clusters
} // namespace app
} // namespace chip
