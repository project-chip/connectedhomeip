// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OccupancySensing (cluster code: 1030/0x406)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {
namespace Attributes {
namespace Occupancy {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Occupancy

namespace OccupancySensorType {
inline constexpr AttributeId Id = 0x00000001;
} // namespace OccupancySensorType

namespace OccupancySensorTypeBitmap {
inline constexpr AttributeId Id = 0x00000002;
} // namespace OccupancySensorTypeBitmap

namespace HoldTime {
inline constexpr AttributeId Id = 0x00000003;
} // namespace HoldTime

namespace HoldTimeLimits {
inline constexpr AttributeId Id = 0x00000004;
} // namespace HoldTimeLimits

namespace PIROccupiedToUnoccupiedDelay {
inline constexpr AttributeId Id = 0x00000010;
} // namespace PIROccupiedToUnoccupiedDelay

namespace PIRUnoccupiedToOccupiedDelay {
inline constexpr AttributeId Id = 0x00000011;
} // namespace PIRUnoccupiedToOccupiedDelay

namespace PIRUnoccupiedToOccupiedThreshold {
inline constexpr AttributeId Id = 0x00000012;
} // namespace PIRUnoccupiedToOccupiedThreshold

namespace UltrasonicOccupiedToUnoccupiedDelay {
inline constexpr AttributeId Id = 0x00000020;
} // namespace UltrasonicOccupiedToUnoccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedDelay {
inline constexpr AttributeId Id = 0x00000021;
} // namespace UltrasonicUnoccupiedToOccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedThreshold {
inline constexpr AttributeId Id = 0x00000022;
} // namespace UltrasonicUnoccupiedToOccupiedThreshold

namespace PhysicalContactOccupiedToUnoccupiedDelay {
inline constexpr AttributeId Id = 0x00000030;
} // namespace PhysicalContactOccupiedToUnoccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedDelay {
inline constexpr AttributeId Id = 0x00000031;
} // namespace PhysicalContactUnoccupiedToOccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedThreshold {
inline constexpr AttributeId Id = 0x00000032;
} // namespace PhysicalContactUnoccupiedToOccupiedThreshold

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
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip
