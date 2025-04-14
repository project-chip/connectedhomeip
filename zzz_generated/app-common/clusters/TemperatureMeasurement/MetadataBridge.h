// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureMeasurement (cluster code: 1026/0x402)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TemperatureMeasurement/Ids.h>
#include <clusters/TemperatureMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TemperatureMeasurement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::TemperatureMeasurement::Attributes;
        switch (commandId)
        {
        case MeasuredValue::Id:
            return MeasuredValue::kMetadataEntry;
        case MinMeasuredValue::Id:
            return MinMeasuredValue::kMetadataEntry;
        case MaxMeasuredValue::Id:
            return MaxMeasuredValue::kMetadataEntry;
        case Tolerance::Id:
            return Tolerance::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TemperatureMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::TemperatureMeasurement::Commands;
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
