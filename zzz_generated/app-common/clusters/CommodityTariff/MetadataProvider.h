// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityTariff (cluster code: 1792/0x700)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CommodityTariff/Ids.h>
#include <clusters/CommodityTariff/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CommodityTariff::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CommodityTariff::Attributes;
        switch (attributeId)
        {
        case TariffInfo::Id:
            return TariffInfo::kMetadataEntry;
        case TariffUnit::Id:
            return TariffUnit::kMetadataEntry;
        case StartDate::Id:
            return StartDate::kMetadataEntry;
        case DayEntries::Id:
            return DayEntries::kMetadataEntry;
        case DayPatterns::Id:
            return DayPatterns::kMetadataEntry;
        case CalendarPeriods::Id:
            return CalendarPeriods::kMetadataEntry;
        case IndividualDays::Id:
            return IndividualDays::kMetadataEntry;
        case CurrentDay::Id:
            return CurrentDay::kMetadataEntry;
        case NextDay::Id:
            return NextDay::kMetadataEntry;
        case CurrentDayEntry::Id:
            return CurrentDayEntry::kMetadataEntry;
        case CurrentDayEntryDate::Id:
            return CurrentDayEntryDate::kMetadataEntry;
        case NextDayEntry::Id:
            return NextDayEntry::kMetadataEntry;
        case NextDayEntryDate::Id:
            return NextDayEntryDate::kMetadataEntry;
        case TariffComponents::Id:
            return TariffComponents::kMetadataEntry;
        case TariffPeriods::Id:
            return TariffPeriods::kMetadataEntry;
        case CurrentTariffComponents::Id:
            return CurrentTariffComponents::kMetadataEntry;
        case NextTariffComponents::Id:
            return NextTariffComponents::kMetadataEntry;
        case DefaultRandomizationOffset::Id:
            return DefaultRandomizationOffset::kMetadataEntry;
        case DefaultRandomizationType::Id:
            return DefaultRandomizationType::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommodityTariff::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommodityTariff::Commands;
        switch (commandId)
        {
        case GetTariffComponent::Id:
            return GetTariffComponent::kMetadataEntry;
        case GetDayEntry::Id:
            return GetDayEntry::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
