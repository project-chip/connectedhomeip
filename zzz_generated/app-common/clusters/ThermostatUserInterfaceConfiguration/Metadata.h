// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThermostatUserInterfaceConfiguration (cluster code: 516/0x204)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThermostatUserInterfaceConfiguration/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThermostatUserInterfaceConfiguration {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace TemperatureDisplayMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::TemperatureDisplayMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace TemperatureDisplayMode
namespace KeypadLockout {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::KeypadLockout::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace KeypadLockout
namespace ScheduleProgrammingVisibility {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ScheduleProgrammingVisibility::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ScheduleProgrammingVisibility

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ThermostatUserInterfaceConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
