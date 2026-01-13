// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/PumpConfigurationAndControl/Ids.h>
#include <clusters/PumpConfigurationAndControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::PumpConfigurationAndControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::PumpConfigurationAndControl::Attributes;
        switch (attributeId)
        {
        case MaxPressure::Id:
            return MaxPressure::kMetadataEntry;
        case MaxSpeed::Id:
            return MaxSpeed::kMetadataEntry;
        case MaxFlow::Id:
            return MaxFlow::kMetadataEntry;
        case MinConstPressure::Id:
            return MinConstPressure::kMetadataEntry;
        case MaxConstPressure::Id:
            return MaxConstPressure::kMetadataEntry;
        case MinCompPressure::Id:
            return MinCompPressure::kMetadataEntry;
        case MaxCompPressure::Id:
            return MaxCompPressure::kMetadataEntry;
        case MinConstSpeed::Id:
            return MinConstSpeed::kMetadataEntry;
        case MaxConstSpeed::Id:
            return MaxConstSpeed::kMetadataEntry;
        case MinConstFlow::Id:
            return MinConstFlow::kMetadataEntry;
        case MaxConstFlow::Id:
            return MaxConstFlow::kMetadataEntry;
        case MinConstTemp::Id:
            return MinConstTemp::kMetadataEntry;
        case MaxConstTemp::Id:
            return MaxConstTemp::kMetadataEntry;
        case PumpStatus::Id:
            return PumpStatus::kMetadataEntry;
        case EffectiveOperationMode::Id:
            return EffectiveOperationMode::kMetadataEntry;
        case EffectiveControlMode::Id:
            return EffectiveControlMode::kMetadataEntry;
        case Capacity::Id:
            return Capacity::kMetadataEntry;
        case Speed::Id:
            return Speed::kMetadataEntry;
        case LifetimeRunningHours::Id:
            return LifetimeRunningHours::kMetadataEntry;
        case Power::Id:
            return Power::kMetadataEntry;
        case LifetimeEnergyConsumed::Id:
            return LifetimeEnergyConsumed::kMetadataEntry;
        case OperationMode::Id:
            return OperationMode::kMetadataEntry;
        case ControlMode::Id:
            return ControlMode::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PumpConfigurationAndControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::PumpConfigurationAndControl::Commands;
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
