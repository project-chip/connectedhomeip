// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BooleanStateConfiguration/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CurrentSensitivityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace CurrentSensitivityLevel
namespace SupportedSensitivityLevels {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedSensitivityLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedSensitivityLevels
namespace DefaultSensitivityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultSensitivityLevel
namespace AlarmsActive {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AlarmsActive::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AlarmsActive
namespace AlarmsSuppressed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AlarmsSuppressed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AlarmsSuppressed
namespace AlarmsEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AlarmsEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AlarmsEnabled
namespace AlarmsSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AlarmsSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AlarmsSupported
namespace SensorFault {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SensorFault::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SensorFault

} // namespace Attributes

namespace Commands {
namespace SuppressAlarm {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SuppressAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SuppressAlarm
namespace EnableDisableAlarm {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = EnableDisableAlarm::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableDisableAlarm

} // namespace Commands
} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
