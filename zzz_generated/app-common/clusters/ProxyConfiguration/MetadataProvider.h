// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ProxyConfiguration (cluster code: 66/0x42)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ProxyConfiguration/Ids.h>
#include <clusters/ProxyConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ProxyConfiguration::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ProxyConfiguration::Attributes;
        switch (attributeId)
        {
        default:
            return AttributeEntry({}, {}, std::nullopt, std::nullopt);
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ProxyConfiguration::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ProxyConfiguration::Commands;
        switch (commandId)
        {

        default:
            return AcceptedCommandEntry();
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
