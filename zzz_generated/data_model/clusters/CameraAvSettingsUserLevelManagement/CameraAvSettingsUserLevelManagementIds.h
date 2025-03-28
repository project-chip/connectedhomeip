// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

inline constexpr ClusterId Id = 0x00000552;

namespace Attributes {
namespace MPTZPosition {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MPTZPosition
namespace MaxPresets {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxPresets
namespace MPTZPresets {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MPTZPresets
namespace DPTZRelativeMove {
inline constexpr AttributeId Id = 0x00000003;
} // namespace DPTZRelativeMove
namespace ZoomMax {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ZoomMax
namespace TiltMin {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TiltMin
namespace TiltMax {
inline constexpr AttributeId Id = 0x00000006;
} // namespace TiltMax
namespace PanMin {
inline constexpr AttributeId Id = 0x00000007;
} // namespace PanMin
namespace PanMax {
inline constexpr AttributeId Id = 0x00000008;
} // namespace PanMax
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
namespace MPTZSetPosition {
inline constexpr CommandId Id = 0x00000000;
} // namespace MPTZSetPosition
namespace MPTZRelativeMove {
inline constexpr CommandId Id = 0x00000001;
} // namespace MPTZRelativeMove
namespace MPTZMoveToPreset {
inline constexpr CommandId Id = 0x00000002;
} // namespace MPTZMoveToPreset
namespace MPTZSavePreset {
inline constexpr CommandId Id = 0x00000003;
} // namespace MPTZSavePreset
namespace MPTZRemovePreset {
inline constexpr CommandId Id = 0x00000004;
} // namespace MPTZRemovePreset
namespace DPTZSetViewport {
inline constexpr CommandId Id = 0x00000005;
} // namespace DPTZSetViewport
namespace DPTZRelativeMove {
inline constexpr CommandId Id = 0x00000006;
} // namespace DPTZRelativeMove
} // namespace Commands

namespace Events {} // namespace Events

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
