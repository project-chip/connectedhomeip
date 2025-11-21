// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AppleStabilityDiagnostics (cluster code: 323615753/0x1349FC09)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleStabilityDiagnostics {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 22;

namespace AppleExpectedBootCount {
inline constexpr AttributeId Id = 0x00000000;
} // namespace AppleExpectedBootCount

namespace AppleSystemCrashCount {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AppleSystemCrashCount

namespace AppleSystemHangCount {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AppleSystemHangCount

namespace AppleLastRebootReason {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AppleLastRebootReason

namespace AppleActiveHardwareFaults {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AppleActiveHardwareFaults

namespace AppleUnspecifiedRebootCount {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AppleUnspecifiedRebootCount

namespace ApplePowerOnRebootCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace ApplePowerOnRebootCount

namespace AppleBrownOutResetCount {
inline constexpr AttributeId Id = 0x00000007;
} // namespace AppleBrownOutResetCount

namespace AppleSoftwareWatchdogRebootCount {
inline constexpr AttributeId Id = 0x00000008;
} // namespace AppleSoftwareWatchdogRebootCount

namespace AppleHardwareWatchdogRebootCount {
inline constexpr AttributeId Id = 0x00000009;
} // namespace AppleHardwareWatchdogRebootCount

namespace AppleSoftwareUpdateRebootCount {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace AppleSoftwareUpdateRebootCount

namespace AppleSoftwareRebootCount {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace AppleSoftwareRebootCount

namespace AppleSoftwareExceptionRebootCount {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace AppleSoftwareExceptionRebootCount

namespace AppleHardwareFaultRebootCount {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace AppleHardwareFaultRebootCount

namespace AppleStackOverflowRebootCount {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace AppleStackOverflowRebootCount

namespace AppleHeapOverflowRebootCount {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace AppleHeapOverflowRebootCount

namespace AppleTemperatureLimitRebootCount {
inline constexpr AttributeId Id = 0x00000010;
} // namespace AppleTemperatureLimitRebootCount

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
} // namespace AppleStabilityDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
