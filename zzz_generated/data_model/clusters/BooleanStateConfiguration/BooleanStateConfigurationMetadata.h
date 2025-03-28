// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BooleanStateConfiguration/BooleanStateConfigurationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace BooleanStateConfiguration {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kCurrentSensitivityLevelEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::CurrentSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kSupportedSensitivityLevelsEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::SupportedSensitivityLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultSensitivityLevelEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::DefaultSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsActiveEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::AlarmsActive::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsSuppressedEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::AlarmsSuppressed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsEnabledEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::AlarmsEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsSupportedEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::AlarmsSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSensorFaultEntry = {
    .attributeId    = BooleanStateConfiguration::Attributes::SensorFault::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSuppressAlarmEntry = {
    .commandId       = BooleanStateConfiguration::Commands::SuppressAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEnableDisableAlarmEntry = {
    .commandId       = BooleanStateConfiguration::Commands::EnableDisableAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace BooleanStateConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
