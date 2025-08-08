// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Messages/Ids.h>
#include <clusters/Messages/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Messages::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Messages::Attributes;
        switch (attributeId)
        {
        case Messages::Id:
            return Messages::kMetadataEntry;
        case ActiveMessageIDs::Id:
            return ActiveMessageIDs::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Messages::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Messages::Commands;
        switch (commandId)
        {
        case PresentMessagesRequest::Id:
            return PresentMessagesRequest::kMetadataEntry;
        case CancelMessagesRequest::Id:
            return CancelMessagesRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
