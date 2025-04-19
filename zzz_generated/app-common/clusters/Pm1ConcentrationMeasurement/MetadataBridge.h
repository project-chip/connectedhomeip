// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Pm1ConcentrationMeasurement (cluster code: 1068/0x42C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Pm1ConcentrationMeasurement/Ids.h>
#include <clusters/Pm1ConcentrationMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Pm1ConcentrationMeasurement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Pm1ConcentrationMeasurement::Attributes;
        switch (commandId)
        {
        case MeasuredValue::Id:
            return MeasuredValue::kMetadataEntry;
        case MinMeasuredValue::Id:
            return MinMeasuredValue::kMetadataEntry;
        case MaxMeasuredValue::Id:
            return MaxMeasuredValue::kMetadataEntry;
        case PeakMeasuredValue::Id:
            return PeakMeasuredValue::kMetadataEntry;
        case PeakMeasuredValueWindow::Id:
            return PeakMeasuredValueWindow::kMetadataEntry;
        case AverageMeasuredValue::Id:
            return AverageMeasuredValue::kMetadataEntry;
        case AverageMeasuredValueWindow::Id:
            return AverageMeasuredValueWindow::kMetadataEntry;
        case Uncertainty::Id:
            return Uncertainty::kMetadataEntry;
        case MeasurementUnit::Id:
            return MeasurementUnit::kMetadataEntry;
        case MeasurementMedium::Id:
            return MeasurementMedium::kMetadataEntry;
        case LevelValue::Id:
            return LevelValue::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Pm1ConcentrationMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Pm1ConcentrationMeasurement::Commands;
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
