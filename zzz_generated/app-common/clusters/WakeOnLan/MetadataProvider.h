// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WakeOnLan (cluster code: 1283/0x503)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WakeOnLan/Ids.h>
#include <clusters/WakeOnLan/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WakeOnLan::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WakeOnLan::Attributes;
        switch (attributeId)
        {
        case MACAddress::Id:
            return MACAddress::kMetadataEntry;
        case LinkLocalAddress::Id:
            return LinkLocalAddress::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WakeOnLan::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::WakeOnLan::Commands;
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
