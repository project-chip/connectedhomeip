// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CarbonDioxideConcentrationMeasurement (cluster code: 1037/0x40D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Ids.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CarbonDioxideConcentrationMeasurement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::CarbonDioxideConcentrationMeasurement::Attributes;
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
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CarbonDioxideConcentrationMeasurement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::CarbonDioxideConcentrationMeasurement::Commands;
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
