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
    .attributeId    = CommodityTariff::Attributes::TariffInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffInfo
namespace TariffUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::TariffUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffUnit
namespace StartDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::StartDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StartDate
namespace DayEntries {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::DayEntries::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DayEntries
namespace DayPatterns {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::DayPatterns::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DayPatterns
namespace CalendarPeriods {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::CalendarPeriods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CalendarPeriods
namespace IndividualDays {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::IndividualDays::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace IndividualDays
namespace CurrentDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::CurrentDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDay
namespace NextDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::NextDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDay
namespace CurrentDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::CurrentDayEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDayEntry
namespace CurrentDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::CurrentDayEntryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentDayEntryDate
namespace NextDayEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::NextDayEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDayEntry
namespace NextDayEntryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::NextDayEntryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextDayEntryDate
namespace TariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::TariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffComponents
namespace TariffPeriods {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::TariffPeriods::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffPeriods
namespace CurrentTariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::CurrentTariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentTariffComponents
namespace NextTariffComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::NextTariffComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextTariffComponents
namespace DefaultRandomizationOffset {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::DefaultRandomizationOffset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultRandomizationOffset
namespace DefaultRandomizationType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommodityTariff::Attributes::DefaultRandomizationType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultRandomizationType


} // namespace Attributes

namespace Commands {
namespace GetTariffComponent {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = CommodityTariff::Commands::GetTariffComponent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetTariffComponent
namespace GetDayEntry {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = CommodityTariff::Commands::GetDayEntry::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetDayEntry

} // namespace Commands
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
