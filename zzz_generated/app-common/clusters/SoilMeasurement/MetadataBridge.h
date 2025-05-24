// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoilMeasurement (cluster code: 1072/0x430)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/SoilMeasurement/Ids.h>
#include <clusters/SoilMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::SoilMeasurement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::SoilMeasurement::Attributes;
        switch (commandId)
        {
        case SoilMoistureMeasurementLimits::Id:
            return SoilMoistureMeasurementLimits::kMetadataEntry;
        case SoilMoistureMeasuredValue::Id:
            return SoilMoistureMeasuredValue::kMetadataEntry;
        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::SoilMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::SoilMeasurement::Commands;
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
