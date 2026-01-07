// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WebRTCTransportProvider/Ids.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WebRTCTransportProvider::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WebRTCTransportProvider::Attributes;
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
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WebRTCTransportProvider::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::WebRTCTransportProvider::Commands;
        switch (commandId)
        {
        case SolicitOffer::Id:
            return SolicitOffer::kMetadataEntry;
        case ProvideOffer::Id:
            return ProvideOffer::kMetadataEntry;
        case ProvideAnswer::Id:
            return ProvideAnswer::kMetadataEntry;
        case ProvideICECandidates::Id:
            return ProvideICECandidates::kMetadataEntry;
        case EndSession::Id:
            return EndSession::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
