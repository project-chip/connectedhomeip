// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Humidistat (cluster code: 517/0x205)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Humidistat/Ids.h>
#include <clusters/Humidistat/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Humidistat::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Humidistat::Attributes;
        switch (attributeId)
        {
        case SupportedModes::Id:
            return SupportedModes::kMetadataEntry;
        case Mode::Id:
            return Mode::kMetadataEntry;
        case SystemState::Id:
            return SystemState::kMetadataEntry;
        case UserSetpoint::Id:
            return UserSetpoint::kMetadataEntry;
        case MinSetpoint::Id:
            return MinSetpoint::kMetadataEntry;
        case MaxSetpoint::Id:
            return MaxSetpoint::kMetadataEntry;
        case Step::Id:
            return Step::kMetadataEntry;
        case TargetSetpoint::Id:
            return TargetSetpoint::kMetadataEntry;
        case MistType::Id:
            return MistType::kMetadataEntry;
        case Continuous::Id:
            return Continuous::kMetadataEntry;
        case Sleep::Id:
            return Sleep::kMetadataEntry;
        case Optimal::Id:
            return Optimal::kMetadataEntry;
        case CondPumpEnabled::Id:
            return CondPumpEnabled::kMetadataEntry;
        case CondRunCount::Id:
            return CondRunCount::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Humidistat::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Humidistat::Commands;
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
