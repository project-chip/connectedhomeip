// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IlluminanceMeasurement (cluster code: 1024/0x400)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/IlluminanceMeasurement/Ids.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::IlluminanceMeasurement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::IlluminanceMeasurement::Attributes;
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
        case LightSensorType::Id:
            return LightSensorType::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::IlluminanceMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::IlluminanceMeasurement::Commands;
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
