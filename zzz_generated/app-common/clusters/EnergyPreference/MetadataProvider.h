// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyPreference (cluster code: 155/0x9B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/EnergyPreference/Ids.h>
#include <clusters/EnergyPreference/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::EnergyPreference::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::EnergyPreference::Attributes;
        switch (attributeId)
        {
        case EnergyBalances::Id:
            return EnergyBalances::kMetadataEntry;
        case CurrentEnergyBalance::Id:
            return CurrentEnergyBalance::kMetadataEntry;
        case EnergyPriorities::Id:
            return EnergyPriorities::kMetadataEntry;
        case LowPowerModeSensitivities::Id:
            return LowPowerModeSensitivities::kMetadataEntry;
        case CurrentLowPowerModeSensitivity::Id:
            return CurrentLowPowerModeSensitivity::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::EnergyPreference::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::EnergyPreference::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
