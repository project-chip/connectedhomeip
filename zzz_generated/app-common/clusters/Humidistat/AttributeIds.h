// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Humidistat (cluster code: 517/0x205)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Humidistat {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 19;

namespace SupportedModes {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedModes

namespace Mode {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Mode

namespace SystemState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SystemState

namespace UserSetpoint {
inline constexpr AttributeId Id = 0x00000003;
} // namespace UserSetpoint

namespace MinSetpoint {
inline constexpr AttributeId Id = 0x00000004;
} // namespace MinSetpoint

namespace MaxSetpoint {
inline constexpr AttributeId Id = 0x00000005;
} // namespace MaxSetpoint

namespace Step {
inline constexpr AttributeId Id = 0x00000006;
} // namespace Step

namespace TargetSetpoint {
inline constexpr AttributeId Id = 0x00000007;
} // namespace TargetSetpoint

namespace MistType {
inline constexpr AttributeId Id = 0x00000008;
} // namespace MistType

namespace Continuous {
inline constexpr AttributeId Id = 0x00000009;
} // namespace Continuous

namespace Sleep {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace Sleep

namespace Optimal {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace Optimal

namespace CondPumpEnabled {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace CondPumpEnabled

namespace CondRunCount {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace CondRunCount

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
} // namespace Humidistat
} // namespace Clusters
} // namespace app
} // namespace chip
