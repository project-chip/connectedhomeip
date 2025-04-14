// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkManagement (cluster code: 1105/0x451)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WiFiNetworkManagement/Ids.h>
#include <clusters/WiFiNetworkManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WiFiNetworkManagement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::WiFiNetworkManagement::Attributes;
        switch (commandId)
        {
        case Ssid::Id:
            return Ssid::kMetadataEntry;
        case PassphraseSurrogate::Id:
            return PassphraseSurrogate::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WiFiNetworkManagement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::WiFiNetworkManagement::Commands;
        switch (commandId)
        {
        case NetworkPassphraseRequest::Id:
            return NetworkPassphraseRequest::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
