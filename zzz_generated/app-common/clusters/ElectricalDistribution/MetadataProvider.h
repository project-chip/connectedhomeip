// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalDistribution (cluster code: 162/0xA2)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalDistribution/Ids.h>
#include <clusters/ElectricalDistribution/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalDistribution::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ElectricalDistribution::Attributes;
        switch (attributeId)
        {
        case MaxContinuousCurrent::Id:
            return MaxContinuousCurrent::kMetadataEntry;
        case MaxVoltage::Id:
            return MaxVoltage::kMetadataEntry;
        case NumberOfPoles::Id:
            return NumberOfPoles::kMetadataEntry;
        case EndOfLife::Id:
            return EndOfLife::kMetadataEntry;
        case ServiceEntranceRated::Id:
            return ServiceEntranceRated::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalDistribution::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalDistribution::Commands;
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
