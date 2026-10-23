// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/BooleanStateConfiguration/Ids.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::BooleanStateConfiguration::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::BooleanStateConfiguration::Attributes;
        switch (attributeId)
        {
        case CurrentSensitivityLevel::Id:
            return CurrentSensitivityLevel::kMetadataEntry;
        case SupportedSensitivityLevels::Id:
            return SupportedSensitivityLevels::kMetadataEntry;
        case DefaultSensitivityLevel::Id:
            return DefaultSensitivityLevel::kMetadataEntry;
        case AlarmsActive::Id:
            return AlarmsActive::kMetadataEntry;
        case AlarmsSuppressed::Id:
            return AlarmsSuppressed::kMetadataEntry;
        case AlarmsEnabled::Id:
            return AlarmsEnabled::kMetadataEntry;
        case AlarmsSupported::Id:
            return AlarmsSupported::kMetadataEntry;
        case SensorFault::Id:
            return SensorFault::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::BooleanStateConfiguration::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::BooleanStateConfiguration::Commands;
        switch (commandId)
        {
        case SuppressAlarm::Id:
            return SuppressAlarm::kMetadataEntry;
        case EnableDisableAlarm::Id:
            return EnableDisableAlarm::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
