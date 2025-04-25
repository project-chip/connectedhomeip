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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SceneTableSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SceneTableSize
namespace FabricSceneInfo {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FabricSceneInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FabricSceneInfo

} // namespace Attributes

namespace Commands {
namespace AddScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::AddScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AddScene
namespace ViewScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::ViewScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ViewScene
namespace RemoveScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::RemoveScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RemoveScene
namespace RemoveAllScenes {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::RemoveAllScenes::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RemoveAllScenes
namespace StoreScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::StoreScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace StoreScene
namespace RecallScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::RecallScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace RecallScene
namespace GetSceneMembership {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::GetSceneMembership::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetSceneMembership
namespace CopyScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScenesManagement::Commands::CopyScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace CopyScene

} // namespace Commands
} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip
