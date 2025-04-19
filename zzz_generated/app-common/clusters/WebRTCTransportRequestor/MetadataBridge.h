// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WebRTCTransportRequestor/Ids.h>
#include <clusters/WebRTCTransportRequestor/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WebRTCTransportRequestor::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::WebRTCTransportRequestor::Attributes;
        switch (commandId)
        {
        case CurrentSessions::Id:
            return CurrentSessions::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WebRTCTransportRequestor::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::WebRTCTransportRequestor::Commands;
        switch (commandId)
        {
        case Offer::Id:
            return Offer::kMetadataEntry;
        case Answer::Id:
            return Answer::kMetadataEntry;
        case ICECandidates::Id:
            return ICECandidates::kMetadataEntry;
        case End::Id:
            return End::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
