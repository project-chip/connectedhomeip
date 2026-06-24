// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalAlarm (cluster code: 161/0xA1)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mask::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Mask
namespace Latch {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Latch::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Latch
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry(State::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace State
namespace Supported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Supported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Supported
namespace OverVoltageThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverVoltageThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverVoltageThreshold
namespace UnderVoltageThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnderVoltageThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnderVoltageThreshold
namespace OverFrequencyThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverFrequencyThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverFrequencyThreshold
namespace UnderFrequencyThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnderFrequencyThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnderFrequencyThreshold
namespace OverPowerThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverPowerThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverPowerThreshold
namespace UnderPowerThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnderPowerThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnderPowerThreshold
namespace OverCurrentThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverCurrentThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverCurrentThreshold
namespace UnderCurrentThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnderCurrentThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnderCurrentThreshold
namespace PowerImportThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerImportThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerImportThreshold
namespace PowerExportThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerExportThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerExportThreshold
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    Mask::kMetadataEntry,
    State::kMetadataEntry,
    Supported::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace Reset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Reset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Reset
namespace ModifyEnabledAlarms {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ModifyEnabledAlarms::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ModifyEnabledAlarms
namespace SetElectricalAlarmThresholds {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetElectricalAlarmThresholds::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace SetElectricalAlarmThresholds

} // namespace Commands

namespace Events {
namespace Notify {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace Notify

} // namespace Events
} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
