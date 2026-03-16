// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OnOff/Ids.h>
#include <clusters/OnOff/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OnOff::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::OnOff::Attributes;
        switch (attributeId)
        {
        case OnOff::Id:
            return OnOff::kMetadataEntry;
        case GlobalSceneControl::Id:
            return GlobalSceneControl::kMetadataEntry;
        case OnTime::Id:
            return OnTime::kMetadataEntry;
        case OffWaitTime::Id:
            return OffWaitTime::kMetadataEntry;
        case StartUpOnOff::Id:
            return StartUpOnOff::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OnOff::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::OnOff::Commands;
        switch (commandId)
        {
        case Off::Id:
            return Off::kMetadataEntry;
        case On::Id:
            return On::kMetadataEntry;
        case Toggle::Id:
            return Toggle::kMetadataEntry;
        case OffWithEffect::Id:
            return OffWithEffect::kMetadataEntry;
        case OnWithRecallGlobalScene::Id:
            return OnWithRecallGlobalScene::kMetadataEntry;
        case OnWithTimedOff::Id:
            return OnWithTimedOff::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
