// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ScenesManagement/Ids.h>
#include <clusters/ScenesManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ScenesManagement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ScenesManagement::Attributes;
        switch (commandId)
        {
        case SceneTableSize::Id:
            return SceneTableSize::kMetadataEntry;
        case FabricSceneInfo::Id:
            return FabricSceneInfo::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ScenesManagement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ScenesManagement::Commands;
        switch (commandId)
        {
        case AddScene::Id:
            return AddScene::kMetadataEntry;
        case ViewScene::Id:
            return ViewScene::kMetadataEntry;
        case RemoveScene::Id:
            return RemoveScene::kMetadataEntry;
        case RemoveAllScenes::Id:
            return RemoveAllScenes::kMetadataEntry;
        case StoreScene::Id:
            return StoreScene::kMetadataEntry;
        case RecallScene::Id:
            return RecallScene::kMetadataEntry;
        case GetSceneMembership::Id:
            return GetSceneMembership::kMetadataEntry;
        case CopyScene::Id:
            return CopyScene::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
