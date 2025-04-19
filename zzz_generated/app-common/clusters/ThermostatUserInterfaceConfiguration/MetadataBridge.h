// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThermostatUserInterfaceConfiguration (cluster code: 516/0x204)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

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
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ThermostatUserInterfaceConfiguration::Attributes;
        switch (commandId)
        {
        case TemperatureDisplayMode::Id:
            return TemperatureDisplayMode::kMetadataEntry;
        case KeypadLockout::Id:
            return KeypadLockout::kMetadataEntry;
        case ScheduleProgrammingVisibility::Id:
            return ScheduleProgrammingVisibility::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThermostatUserInterfaceConfiguration::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThermostatUserInterfaceConfiguration::Commands;
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
