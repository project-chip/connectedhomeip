// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OtaSoftwareUpdateRequestor/Ids.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OtaSoftwareUpdateRequestor::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::OtaSoftwareUpdateRequestor::Attributes;
        switch (commandId)
        {
        case DefaultOTAProviders::Id:
            return DefaultOTAProviders::kMetadataEntry;
        case UpdatePossible::Id:
            return UpdatePossible::kMetadataEntry;
        case UpdateState::Id:
            return UpdateState::kMetadataEntry;
        case UpdateStateProgress::Id:
            return UpdateStateProgress::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OtaSoftwareUpdateRequestor::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::OtaSoftwareUpdateRequestor::Commands;
        switch (commandId)
        {
        case AnnounceOTAProvider::Id:
            return AnnounceOTAProvider::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
