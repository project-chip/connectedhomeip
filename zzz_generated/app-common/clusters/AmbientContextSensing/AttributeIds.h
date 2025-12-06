// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AmbientContextSensing (cluster code: 1073/0x431)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AmbientContextSensing {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 18;

namespace HumanActivityDetected {
inline constexpr AttributeId Id = 0x00000000;
} // namespace HumanActivityDetected

namespace ObjectIdentified {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ObjectIdentified

namespace AudioContextDetected {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AudioContextDetected

namespace AmbientContextType {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AmbientContextType

namespace AmbientContextTypeSupported {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AmbientContextTypeSupported

namespace SimultaneousDetectionLimit {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SimultaneousDetectionLimit

namespace CountThresholdReached {
inline constexpr AttributeId Id = 0x00000006;
} // namespace CountThresholdReached

namespace CountThreshold {
inline constexpr AttributeId Id = 0x00000007;
} // namespace CountThreshold

namespace ObjectCount {
inline constexpr AttributeId Id = 0x00000008;
} // namespace ObjectCount

namespace HoldTime {
inline constexpr AttributeId Id = 0x00000009;
} // namespace HoldTime

namespace HoldTimeLimits {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace HoldTimeLimits

namespace PredictedActivity {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace PredictedActivity

namespace PrivacyModeEnabled {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace PrivacyModeEnabled

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
} // namespace AmbientContextSensing
} // namespace Clusters
} // namespace app
} // namespace chip
