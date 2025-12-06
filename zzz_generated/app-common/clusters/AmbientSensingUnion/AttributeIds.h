// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AmbientSensingUnion (cluster code: 1074/0x432)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AmbientSensingUnion {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 12;

namespace UnionID {
inline constexpr AttributeId Id = 0x00000000;
} // namespace UnionID

namespace UnionName {
inline constexpr AttributeId Id = 0x00000001;
} // namespace UnionName

namespace UnionHealth {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UnionHealth

namespace UnionSensorList {
inline constexpr AttributeId Id = 0x00000003;
} // namespace UnionSensorList

namespace UnionSensorHealth {
inline constexpr AttributeId Id = 0x00000004;
} // namespace UnionSensorHealth

namespace SensorID {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SensorID

namespace ZoneSensorList {
inline constexpr AttributeId Id = 0x00000006;
} // namespace ZoneSensorList

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
} // namespace AmbientSensingUnion
} // namespace Clusters
} // namespace app
} // namespace chip
