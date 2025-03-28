// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThermostatUserInterfaceConfiguration (cluster code: 516/0x204)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThermostatUserInterfaceConfiguration/ThermostatUserInterfaceConfigurationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ThermostatUserInterfaceConfiguration {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kTemperatureDisplayModeEntry = {
    .attributeId    = ThermostatUserInterfaceConfiguration::Attributes::TemperatureDisplayMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kKeypadLockoutEntry = {
    .attributeId    = ThermostatUserInterfaceConfiguration::Attributes::KeypadLockout::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kScheduleProgrammingVisibilityEntry = {
    .attributeId    = ThermostatUserInterfaceConfiguration::Attributes::ScheduleProgrammingVisibility::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace ThermostatUserInterfaceConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
