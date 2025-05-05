// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityTariff (cluster code: 1792/0x700)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommodityTariff/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace TariffInfo {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TariffInfo::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TariffInfo
namespace TariffUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TariffUnit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TariffUnit
namespace StartDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartDate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StartDate
namespace DayEntries {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DayEntries::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DayEntries
namespace DayPatterns {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DayPatterns::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DayPatterns
namespace CalendarPeriods {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CalendarPeriods::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CalendarPeriods
namespace IndividualDays {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(IndividualDays::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace IndividualDays
namespace CurrentDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentDay::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentDay
namespace NextDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextDay::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextDay
namespace CurrentDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentDayEntry::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentDayEntry
namespace CurrentDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentDayEntryDate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentDayEntryDate
namespace NextDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextDayEntry::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextDayEntry
namespace NextDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextDayEntryDate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextDayEntryDate
namespace TariffComponents {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TariffComponents::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TariffComponents
namespace TariffPeriods {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TariffPeriods::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TariffPeriods
namespace CurrentTariffComponents {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CurrentTariffComponents::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CurrentTariffComponents
namespace NextTariffComponents {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NextTariffComponents::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace NextTariffComponents
namespace DefaultRandomizationOffset {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultRandomizationOffset::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace DefaultRandomizationOffset
namespace DefaultRandomizationType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultRandomizationType::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace DefaultRandomizationType

} // namespace Attributes

namespace Commands {
namespace GetTariffComponent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetTariffComponent::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GetTariffComponent
namespace GetDayEntry {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetDayEntry::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GetDayEntry

} // namespace Commands
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
