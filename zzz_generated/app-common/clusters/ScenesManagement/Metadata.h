// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ScenesManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ScenesManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SceneTableSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SceneTableSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SceneTableSize
namespace FabricSceneInfo {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(FabricSceneInfo::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace FabricSceneInfo

} // namespace Attributes

namespace Commands {
namespace AddScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace AddScene
namespace ViewScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ViewScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ViewScene
namespace RemoveScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace RemoveScene
namespace RemoveAllScenes {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveAllScenes::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace RemoveAllScenes
namespace StoreScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(StoreScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace StoreScene
namespace RecallScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RecallScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace RecallScene
namespace GetSceneMembership {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetSceneMembership::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace GetSceneMembership
namespace CopyScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CopyScene::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace CopyScene

} // namespace Commands
} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip
