// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalPowerMeasurement (cluster code: 144/0x90)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalPowerMeasurement/Ids.h>
#include <clusters/ElectricalPowerMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalPowerMeasurement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ElectricalPowerMeasurement::Attributes;
        switch (attributeId)
        {
        case PowerMode::Id:
            return PowerMode::kMetadataEntry;
        case NumberOfMeasurementTypes::Id:
            return NumberOfMeasurementTypes::kMetadataEntry;
        case Accuracy::Id:
            return Accuracy::kMetadataEntry;
        case Ranges::Id:
            return Ranges::kMetadataEntry;
        case Voltage::Id:
            return Voltage::kMetadataEntry;
        case ActiveCurrent::Id:
            return ActiveCurrent::kMetadataEntry;
        case ReactiveCurrent::Id:
            return ReactiveCurrent::kMetadataEntry;
        case ApparentCurrent::Id:
            return ApparentCurrent::kMetadataEntry;
        case ActivePower::Id:
            return ActivePower::kMetadataEntry;
        case ReactivePower::Id:
            return ReactivePower::kMetadataEntry;
        case ApparentPower::Id:
            return ApparentPower::kMetadataEntry;
        case RMSVoltage::Id:
            return RMSVoltage::kMetadataEntry;
        case RMSCurrent::Id:
            return RMSCurrent::kMetadataEntry;
        case RMSPower::Id:
            return RMSPower::kMetadataEntry;
        case Frequency::Id:
            return Frequency::kMetadataEntry;
        case HarmonicCurrents::Id:
            return HarmonicCurrents::kMetadataEntry;
        case HarmonicPhases::Id:
            return HarmonicPhases::kMetadataEntry;
        case PowerFactor::Id:
            return PowerFactor::kMetadataEntry;
        case NeutralCurrent::Id:
            return NeutralCurrent::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalPowerMeasurement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalPowerMeasurement::Commands;
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
