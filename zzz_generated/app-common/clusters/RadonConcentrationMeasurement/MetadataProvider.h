// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RadonConcentrationMeasurement (cluster code: 1071/0x42F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/RadonConcentrationMeasurement/Ids.h>
#include <clusters/RadonConcentrationMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::RadonConcentrationMeasurement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::RadonConcentrationMeasurement::Attributes;
        switch (attributeId)
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
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::RadonConcentrationMeasurement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::RadonConcentrationMeasurement::Commands;
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
