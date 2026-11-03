// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalProtectionAlarm (cluster code: 163/0xA3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalProtectionAlarm {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 15;

namespace Mask {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Mask

namespace State {
inline constexpr AttributeId Id = 0x00000002;
} // namespace State

namespace Supported {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Supported

namespace ArcCause {
inline constexpr AttributeId Id = 0x00000080;
} // namespace ArcCause

namespace OverLoadRating {
inline constexpr AttributeId Id = 0x00000081;
} // namespace OverLoadRating

namespace OverVoltageRating {
inline constexpr AttributeId Id = 0x00000082;
} // namespace OverVoltageRating

namespace SurgeProtectionRating {
inline constexpr AttributeId Id = 0x00000083;
} // namespace SurgeProtectionRating

namespace ShortCircuitRating {
inline constexpr AttributeId Id = 0x00000084;
} // namespace ShortCircuitRating

namespace ResidualCurrentRating {
inline constexpr AttributeId Id = 0x00000085;
} // namespace ResidualCurrentRating

namespace ArcFaultRating {
inline constexpr AttributeId Id = 0x00000086;
} // namespace ArcFaultRating

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
} // namespace ElectricalProtectionAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
