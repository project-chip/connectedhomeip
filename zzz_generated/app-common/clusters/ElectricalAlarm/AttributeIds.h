// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalAlarm (cluster code: 161/0xA1)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 19;

namespace Mask {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Mask

namespace Latch {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Latch

namespace State {
inline constexpr AttributeId Id = 0x00000002;
} // namespace State

namespace Supported {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Supported

namespace OverVoltageThreshold {
inline constexpr AttributeId Id = 0x00000080;
} // namespace OverVoltageThreshold

namespace UnderVoltageThreshold {
inline constexpr AttributeId Id = 0x00000081;
} // namespace UnderVoltageThreshold

namespace OverFrequencyThreshold {
inline constexpr AttributeId Id = 0x00000082;
} // namespace OverFrequencyThreshold

namespace UnderFrequencyThreshold {
inline constexpr AttributeId Id = 0x00000083;
} // namespace UnderFrequencyThreshold

namespace OverPowerThreshold {
inline constexpr AttributeId Id = 0x00000084;
} // namespace OverPowerThreshold

namespace UnderPowerThreshold {
inline constexpr AttributeId Id = 0x00000085;
} // namespace UnderPowerThreshold

namespace OverCurrentThreshold {
inline constexpr AttributeId Id = 0x00000086;
} // namespace OverCurrentThreshold

namespace UnderCurrentThreshold {
inline constexpr AttributeId Id = 0x00000087;
} // namespace UnderCurrentThreshold

namespace PowerImportThreshold {
inline constexpr AttributeId Id = 0x00000088;
} // namespace PowerImportThreshold

namespace PowerExportThreshold {
inline constexpr AttributeId Id = 0x00000089;
} // namespace PowerExportThreshold

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
} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
