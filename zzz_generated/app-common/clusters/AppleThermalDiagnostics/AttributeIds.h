// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AppleThermalDiagnostics (cluster code: 323615754/0x1349FC0A)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleThermalDiagnostics {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 11;

namespace AppleTemperatureLimitViolationCount {
inline constexpr AttributeId Id = 0x00000000;
} // namespace AppleTemperatureLimitViolationCount

namespace AppleTemperatureLimitViolationDuration {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AppleTemperatureLimitViolationDuration

namespace AppleMinimumDeviceTemperature {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AppleMinimumDeviceTemperature

namespace AppleMaximumDeviceTemperature {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AppleMaximumDeviceTemperature

namespace AppleAverageDeviceTemperature {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AppleAverageDeviceTemperature

namespace AppleThermalResetCountBootRelativeTime {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AppleThermalResetCountBootRelativeTime

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
} // namespace AppleThermalDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
