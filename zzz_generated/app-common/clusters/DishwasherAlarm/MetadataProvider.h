// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DishwasherAlarm (cluster code: 93/0x5D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DishwasherAlarm/Ids.h>
#include <clusters/DishwasherAlarm/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DishwasherAlarm::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::DishwasherAlarm::Attributes;
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
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DishwasherAlarm::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::DishwasherAlarm::Commands;
        switch (commandId)
        {
        case Reset::Id:
            return Reset::kMetadataEntry;
        case ModifyEnabledAlarms::Id:
            return ModifyEnabledAlarms::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
