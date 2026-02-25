// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 19;

namespace Type {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Type

namespace NumberOfActuationsLift {
inline constexpr AttributeId Id = 0x00000005;
} // namespace NumberOfActuationsLift

namespace NumberOfActuationsTilt {
inline constexpr AttributeId Id = 0x00000006;
} // namespace NumberOfActuationsTilt

namespace ConfigStatus {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ConfigStatus

namespace CurrentPositionLiftPercentage {
inline constexpr AttributeId Id = 0x00000008;
} // namespace CurrentPositionLiftPercentage

namespace CurrentPositionTiltPercentage {
inline constexpr AttributeId Id = 0x00000009;
} // namespace CurrentPositionTiltPercentage

namespace OperationalStatus {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace OperationalStatus

namespace TargetPositionLiftPercent100ths {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace TargetPositionLiftPercent100ths

namespace TargetPositionTiltPercent100ths {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace TargetPositionTiltPercent100ths

namespace EndProductType {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace EndProductType

namespace CurrentPositionLiftPercent100ths {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace CurrentPositionLiftPercent100ths

namespace CurrentPositionTiltPercent100ths {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace CurrentPositionTiltPercent100ths

namespace Mode {
inline constexpr AttributeId Id = 0x00000017;
} // namespace Mode

namespace SafetyStatus {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace SafetyStatus

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
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
