// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WebRTCTransportRequestor::Attributes;
        switch (attributeId)
        {
        case CurrentSessions::Id:
            return CurrentSessions::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WebRTCTransportRequestor::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
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
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
