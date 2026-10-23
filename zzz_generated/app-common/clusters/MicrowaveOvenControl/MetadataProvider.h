// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MicrowaveOvenControl (cluster code: 95/0x5F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/MicrowaveOvenControl/Ids.h>
#include <clusters/MicrowaveOvenControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::MicrowaveOvenControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::MicrowaveOvenControl::Attributes;
        switch (attributeId)
        {
        case CookTime::Id:
            return CookTime::kMetadataEntry;
        case MaxCookTime::Id:
            return MaxCookTime::kMetadataEntry;
        case PowerSetting::Id:
            return PowerSetting::kMetadataEntry;
        case MinPower::Id:
            return MinPower::kMetadataEntry;
        case MaxPower::Id:
            return MaxPower::kMetadataEntry;
        case PowerStep::Id:
            return PowerStep::kMetadataEntry;
        case SupportedWatts::Id:
            return SupportedWatts::kMetadataEntry;
        case SelectedWattIndex::Id:
            return SelectedWattIndex::kMetadataEntry;
        case WattRating::Id:
            return WattRating::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::MicrowaveOvenControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::MicrowaveOvenControl::Commands;
        switch (commandId)
        {
        case SetCookingParameters::Id:
            return SetCookingParameters::kMetadataEntry;
        case AddMoreTime::Id:
            return AddMoreTime::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
