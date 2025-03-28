// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ScenesManagement/ScenesManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ScenesManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSceneTableSizeEntry = {
    .attributeId    = ScenesManagement::Attributes::SceneTableSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kFabricSceneInfoEntry = {
    .attributeId    = ScenesManagement::Attributes::FabricSceneInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAddSceneEntry = {
    .commandId       = ScenesManagement::Commands::AddScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kViewSceneEntry = {
    .commandId       = ScenesManagement::Commands::ViewScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveSceneEntry = {
    .commandId       = ScenesManagement::Commands::RemoveScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveAllScenesEntry = {
    .commandId       = ScenesManagement::Commands::RemoveAllScenes::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kStoreSceneEntry = {
    .commandId       = ScenesManagement::Commands::StoreScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kRecallSceneEntry = {
    .commandId       = ScenesManagement::Commands::RecallScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGetSceneMembershipEntry = {
    .commandId       = ScenesManagement::Commands::GetSceneMembership::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCopySceneEntry = {
    .commandId       = ScenesManagement::Commands::CopyScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace ScenesManagement
} // namespace clusters
} // namespace app
} // namespace chip
