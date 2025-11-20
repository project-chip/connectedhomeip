// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccountLogin (cluster code: 1294/0x50E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AccountLogin/Ids.h>
#include <clusters/AccountLogin/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AccountLogin::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AccountLogin::Attributes;
        switch (attributeId)
        {
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AccountLogin::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AccountLogin::Commands;
        switch (commandId)
        {
        case GetSetupPIN::Id:
            return GetSetupPIN::kMetadataEntry;
        case Login::Id:
            return Login::kMetadataEntry;
        case Logout::Id:
            return Logout::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
