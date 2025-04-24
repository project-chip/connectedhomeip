// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

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
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ElectricalEnergyMeasurement::Attributes;
        switch (commandId)
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
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalEnergyMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalEnergyMeasurement::Commands;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
