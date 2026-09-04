// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AudioControl (cluster code: 1298/0x512)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioControl {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 24;

namespace SoftMuted {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SoftMuted

namespace PhysicallyMuted {
inline constexpr AttributeId Id = 0x00000001;
} // namespace PhysicallyMuted

namespace Volume {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Volume

namespace MinDeviceVolume {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MinDeviceVolume

namespace MaxDeviceVolume {
inline constexpr AttributeId Id = 0x00000004;
} // namespace MaxDeviceVolume

namespace MaxDeviceVolumeDB {
inline constexpr AttributeId Id = 0x00000005;
} // namespace MaxDeviceVolumeDB

namespace MaxUserVolume {
inline constexpr AttributeId Id = 0x00000006;
} // namespace MaxUserVolume

namespace DefaultStepSize {
inline constexpr AttributeId Id = 0x00000007;
} // namespace DefaultStepSize

namespace SetVolumeUnmutePolicy {
inline constexpr AttributeId Id = 0x00000008;
} // namespace SetVolumeUnmutePolicy

namespace IncreaseVolumeUnmutePolicy {
inline constexpr AttributeId Id = 0x00000009;
} // namespace IncreaseVolumeUnmutePolicy

namespace IncreaseVolumeUnmuteVolume {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace IncreaseVolumeUnmuteVolume

namespace DecreaseVolumeUnmutePolicy {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace DecreaseVolumeUnmutePolicy

namespace StartUpMuted {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace StartUpMuted

namespace StartUpVolume {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace StartUpVolume

namespace Bass {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace Bass

namespace Mid {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace Mid

namespace Treble {
inline constexpr AttributeId Id = 0x00000010;
} // namespace Treble

namespace MinCorrection {
inline constexpr AttributeId Id = 0x00000011;
} // namespace MinCorrection

namespace MaxCorrection {
inline constexpr AttributeId Id = 0x00000012;
} // namespace MaxCorrection

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

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
} // namespace AudioControl
} // namespace Clusters
} // namespace app
} // namespace chip
