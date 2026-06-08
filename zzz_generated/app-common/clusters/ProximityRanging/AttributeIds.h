// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ProximityRanging (cluster code: 1075/0x433)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 12;

namespace RangingCapabilities {
inline constexpr AttributeId Id = 0x00000000;
} // namespace RangingCapabilities

namespace WiFiDevIK {
inline constexpr AttributeId Id = 0x00000001;
} // namespace WiFiDevIK

namespace BLEDeviceID {
inline constexpr AttributeId Id = 0x00000002;
} // namespace BLEDeviceID

namespace BLTDevIK {
inline constexpr AttributeId Id = 0x00000003;
} // namespace BLTDevIK

namespace BLTCSSecurityLevel {
inline constexpr AttributeId Id = 0x00000004;
} // namespace BLTCSSecurityLevel

namespace BLTCSModeCapability {
inline constexpr AttributeId Id = 0x00000005;
} // namespace BLTCSModeCapability

namespace SessionIDList {
inline constexpr AttributeId Id = 0x00000006;
} // namespace SessionIDList

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
} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
