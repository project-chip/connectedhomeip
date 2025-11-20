// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureControl (cluster code: 86/0x56)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TemperatureControl/Ids.h>
#include <clusters/TemperatureControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TemperatureControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TemperatureControl::Attributes;
        switch (attributeId)
        {
        case TemperatureSetpoint::Id:
            return TemperatureSetpoint::kMetadataEntry;
        case MinTemperature::Id:
            return MinTemperature::kMetadataEntry;
        case MaxTemperature::Id:
            return MaxTemperature::kMetadataEntry;
        case Step::Id:
            return Step::kMetadataEntry;
        case SelectedTemperatureLevel::Id:
            return SelectedTemperatureLevel::kMetadataEntry;
        case SupportedTemperatureLevels::Id:
            return SupportedTemperatureLevels::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TemperatureControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TemperatureControl::Commands;
        switch (commandId)
        {
        case SetTemperature::Id:
            return SetTemperature::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
