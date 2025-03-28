// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter

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
    .attributeId    = Attributes::CurrentSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kSupportedSensitivityLevelsEntry = {
    .attributeId    = Attributes::SupportedSensitivityLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultSensitivityLevelEntry = {
    .attributeId    = Attributes::DefaultSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsActiveEntry = {
    .attributeId    = Attributes::AlarmsActive::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsSuppressedEntry = {
    .attributeId    = Attributes::AlarmsSuppressed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsEnabledEntry = {
    .attributeId    = Attributes::AlarmsEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAlarmsSupportedEntry = {
    .attributeId    = Attributes::AlarmsSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSensorFaultEntry = {
    .attributeId    = Attributes::SensorFault::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSuppressAlarmEntry = {
    .commandId       = Commands::SuppressAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEnableDisableAlarmEntry = {
    .commandId       = Commands::EnableDisableAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace BooleanStateConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
