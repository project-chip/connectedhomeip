// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IcdManagement {
namespace Attributes {
namespace IdleModeDuration {
inline constexpr AttributeId Id = 0x00000000;
} // namespace IdleModeDuration

namespace ActiveModeDuration {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ActiveModeDuration

namespace ActiveModeThreshold {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ActiveModeThreshold

namespace RegisteredClients {
inline constexpr AttributeId Id = 0x00000003;
} // namespace RegisteredClients

namespace ICDCounter {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ICDCounter

namespace ClientsSupportedPerFabric {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ClientsSupportedPerFabric

namespace UserActiveModeTriggerHint {
inline constexpr AttributeId Id = 0x00000006;
} // namespace UserActiveModeTriggerHint

namespace UserActiveModeTriggerInstruction {
inline constexpr AttributeId Id = 0x00000007;
} // namespace UserActiveModeTriggerInstruction

namespace OperatingMode {
inline constexpr AttributeId Id = 0x00000008;
} // namespace OperatingMode

namespace MaximumCheckInBackOff {
inline constexpr AttributeId Id = 0x00000009;
} // namespace MaximumCheckInBackOff

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
} // namespace IcdManagement
} // namespace Clusters
} // namespace app
} // namespace chip
