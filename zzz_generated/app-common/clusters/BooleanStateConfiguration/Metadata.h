// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentSensitivityLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CurrentSensitivityLevel
namespace SupportedSensitivityLevels {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedSensitivityLevels::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SupportedSensitivityLevels
namespace DefaultSensitivityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultSensitivityLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DefaultSensitivityLevel
namespace AlarmsActive {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AlarmsActive::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AlarmsActive
namespace AlarmsSuppressed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AlarmsSuppressed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AlarmsSuppressed
namespace AlarmsEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AlarmsEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AlarmsEnabled
namespace AlarmsSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AlarmsSupported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AlarmsSupported
namespace SensorFault {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SensorFault::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SensorFault
constexpr std::array<DataModel::AttributeEntry, 0> kMandatoryMetadata = {

};

} // namespace Attributes

namespace Commands {

namespace SuppressAlarm {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SuppressAlarm::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SuppressAlarm
namespace EnableDisableAlarm {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(EnableDisableAlarm::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace EnableDisableAlarm

} // namespace Commands

namespace Events {
namespace AlarmsStateChanged {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace AlarmsStateChanged
namespace SensorFault {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace SensorFault

} // namespace Events
} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
