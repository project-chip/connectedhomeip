// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterDiagnostics (cluster code: 1108/0x454)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ThreadBorderRouterDiagnostics/Ids.h>
#include <clusters/ThreadBorderRouterDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ThreadBorderRouterDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ThreadBorderRouterDiagnostics::Attributes;
        switch (attributeId)
        {
        case Status::Id:
            return Status::kMetadataEntry;
        case ActiveFaultsList::Id:
            return ActiveFaultsList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThreadBorderRouterDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThreadBorderRouterDiagnostics::Commands;
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
