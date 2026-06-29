// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissioningProxy (cluster code: 1109/0x455)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CommissioningProxy/Ids.h>
#include <clusters/CommissioningProxy/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CommissioningProxy::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CommissioningProxy::Attributes;
        switch (attributeId)
        {
        case Transport::Id:
            return Transport::kMetadataEntry;
        case ScanMaxTime::Id:
            return ScanMaxTime::kMetadataEntry;
        case MaxSessions::Id:
            return MaxSessions::kMetadataEntry;
        case MaxCachedResults::Id:
            return MaxCachedResults::kMetadataEntry;
        case NumCachedResults::Id:
            return NumCachedResults::kMetadataEntry;
        case CacheTimeout::Id:
            return CacheTimeout::kMetadataEntry;
        case CachedResults::Id:
            return CachedResults::kMetadataEntry;
        case WiFiBand::Id:
            return WiFiBand::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommissioningProxy::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommissioningProxy::Commands;
        switch (commandId)
        {
        case ProxyConnectRequest::Id:
            return ProxyConnectRequest::kMetadataEntry;
        case ProxyDisconnectRequest::Id:
            return ProxyDisconnectRequest::kMetadataEntry;
        case ProxyScanRequest::Id:
            return ProxyScanRequest::kMetadataEntry;
        case ProxyBackGroundScanStartRequest::Id:
            return ProxyBackGroundScanStartRequest::kMetadataEntry;
        case ProxyBackGroundScanStopRequest::Id:
            return ProxyBackGroundScanStopRequest::kMetadataEntry;
        case ProxyMessageRequest::Id:
            return ProxyMessageRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
