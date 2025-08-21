// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThermostatUserInterfaceConfiguration (cluster code: 516/0x204)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Ids.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ThermostatUserInterfaceConfiguration::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ThermostatUserInterfaceConfiguration::Attributes;
        switch (attributeId)
        {
        case TemperatureDisplayMode::Id:
            return TemperatureDisplayMode::kMetadataEntry;
        case KeypadLockout::Id:
            return KeypadLockout::kMetadataEntry;
        case ScheduleProgrammingVisibility::Id:
            return ScheduleProgrammingVisibility::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThermostatUserInterfaceConfiguration::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThermostatUserInterfaceConfiguration::Commands;
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
