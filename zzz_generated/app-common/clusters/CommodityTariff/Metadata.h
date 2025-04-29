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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TariffInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffInfo
namespace TariffUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TariffUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffUnit
namespace StartDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StartDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StartDate
namespace DayEntries {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DayEntries::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DayEntries
namespace DayPatterns {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DayPatterns::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DayPatterns
namespace CalendarPeriods {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CalendarPeriods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CalendarPeriods
namespace IndividualDays {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = IndividualDays::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace IndividualDays
namespace CurrentDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDay
namespace NextDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDay
namespace CurrentDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentDayEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDayEntry
namespace CurrentDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentDayEntryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDayEntryDate
namespace NextDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextDayEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDayEntry
namespace NextDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextDayEntryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDayEntryDate
namespace TariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffComponents
namespace TariffPeriods {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TariffPeriods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffPeriods
namespace CurrentTariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentTariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentTariffComponents
namespace NextTariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextTariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextTariffComponents
namespace DefaultRandomizationOffset {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultRandomizationOffset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultRandomizationOffset
namespace DefaultRandomizationType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultRandomizationType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultRandomizationType

} // namespace Attributes

namespace Commands {
namespace GetTariffComponent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetTariffComponent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetTariffComponent
namespace GetDayEntry {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetDayEntry::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetDayEntry

} // namespace Commands
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
