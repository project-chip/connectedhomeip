// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureControlledCabinetTopology (cluster code: 75/0x4B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TemperatureControlledCabinetTopology/Ids.h>
#include <clusters/TemperatureControlledCabinetTopology/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TemperatureControlledCabinetTopology::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TemperatureControlledCabinetTopology::Attributes;
        switch (attributeId)
        {
        case DisabledCabinets::Id:
            return DisabledCabinets::kMetadataEntry;
        case Topology::Id:
            return Topology::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TemperatureControlledCabinetTopology::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TemperatureControlledCabinetTopology::Commands;
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
