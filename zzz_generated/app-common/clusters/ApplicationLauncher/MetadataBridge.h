// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationLauncher (cluster code: 1292/0x50C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ApplicationLauncher/Ids.h>
#include <clusters/ApplicationLauncher/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ApplicationLauncher::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ApplicationLauncher::Attributes;
        switch (commandId)
        {
        case CatalogList::Id:
            return CatalogList::kMetadataEntry;
        case CurrentApp::Id:
            return CurrentApp::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ApplicationLauncher::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ApplicationLauncher::Commands;
        switch (commandId)
        {
        case LaunchApp::Id:
            return LaunchApp::kMetadataEntry;
        case StopApp::Id:
            return StopApp::kMetadataEntry;
        case HideApp::Id:
            return HideApp::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
