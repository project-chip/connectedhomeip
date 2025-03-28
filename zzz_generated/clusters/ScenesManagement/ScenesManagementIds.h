// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ScenesManagement {

inline constexpr ClusterId Id = 0x00000062;

namespace Attributes {
namespace SceneTableSize {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SceneTableSize
namespace FabricSceneInfo {
inline constexpr AttributeId Id = 0x00000002;
} // namespace FabricSceneInfo
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList
namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap
namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes

namespace Commands {
namespace AddScene {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddScene
namespace ViewScene {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewScene
namespace RemoveScene {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveScene
namespace RemoveAllScenes {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveAllScenes
namespace StoreScene {
inline constexpr CommandId Id = 0x00000004;
} // namespace StoreScene
namespace RecallScene {
inline constexpr CommandId Id = 0x00000005;
} // namespace RecallScene
namespace GetSceneMembership {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetSceneMembership
namespace CopyScene {
inline constexpr CommandId Id = 0x00000040;
} // namespace CopyScene
namespace AddSceneResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddSceneResponse
namespace ViewSceneResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewSceneResponse
namespace RemoveSceneResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveSceneResponse
namespace RemoveAllScenesResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveAllScenesResponse
namespace StoreSceneResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace StoreSceneResponse
namespace GetSceneMembershipResponse {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetSceneMembershipResponse
namespace CopySceneResponse {
inline constexpr CommandId Id = 0x00000040;
} // namespace CopySceneResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip
