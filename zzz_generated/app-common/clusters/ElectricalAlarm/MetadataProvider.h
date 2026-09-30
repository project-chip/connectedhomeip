// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalAlarm (cluster code: 161/0xA1)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalAlarm/Ids.h>
#include <clusters/ElectricalAlarm/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalAlarm::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ElectricalAlarm::Attributes;
        switch (attributeId)
        {
        case Mask::Id:
            return Mask::kMetadataEntry;
        case Latch::Id:
            return Latch::kMetadataEntry;
        case State::Id:
            return State::kMetadataEntry;
        case Supported::Id:
            return Supported::kMetadataEntry;
        case OverVoltageThreshold::Id:
            return OverVoltageThreshold::kMetadataEntry;
        case UnderVoltageThreshold::Id:
            return UnderVoltageThreshold::kMetadataEntry;
        case OverFrequencyThreshold::Id:
            return OverFrequencyThreshold::kMetadataEntry;
        case UnderFrequencyThreshold::Id:
            return UnderFrequencyThreshold::kMetadataEntry;
        case OverPowerThreshold::Id:
            return OverPowerThreshold::kMetadataEntry;
        case UnderPowerThreshold::Id:
            return UnderPowerThreshold::kMetadataEntry;
        case OverCurrentThreshold::Id:
            return OverCurrentThreshold::kMetadataEntry;
        case UnderCurrentThreshold::Id:
            return UnderCurrentThreshold::kMetadataEntry;
        case PowerImportThreshold::Id:
            return PowerImportThreshold::kMetadataEntry;
        case PowerExportThreshold::Id:
            return PowerExportThreshold::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalAlarm::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalAlarm::Commands;
        switch (commandId)
        {
        case Reset::Id:
            return Reset::kMetadataEntry;
        case ModifyEnabledAlarms::Id:
            return ModifyEnabledAlarms::kMetadataEntry;
        case SetElectricalAlarmThresholds::Id:
            return SetElectricalAlarmThresholds::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
