// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalEnergyMeasurement/Ids.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalEnergyMeasurement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ElectricalEnergyMeasurement::Attributes;
        switch (attributeId)
        {
        case Accuracy::Id:
            return Accuracy::kMetadataEntry;
        case CumulativeEnergyImported::Id:
            return CumulativeEnergyImported::kMetadataEntry;
        case CumulativeEnergyExported::Id:
            return CumulativeEnergyExported::kMetadataEntry;
        case PeriodicEnergyImported::Id:
            return PeriodicEnergyImported::kMetadataEntry;
        case PeriodicEnergyExported::Id:
            return PeriodicEnergyExported::kMetadataEntry;
        case CumulativeEnergyReset::Id:
            return CumulativeEnergyReset::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalEnergyMeasurement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalEnergyMeasurement::Commands;
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
