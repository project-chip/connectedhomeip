// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster IlluminanceMeasurement (cluster code: 1024/0x400)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IlluminanceMeasurement {

inline constexpr ClusterId Id = 0x00000400;

namespace Attributes {
namespace MeasuredValue {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxMeasuredValue
namespace Tolerance {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Tolerance
namespace LightSensorType {
inline constexpr AttributeId Id = 0x00000004;
} // namespace LightSensorType
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events

} // namespace IlluminanceMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
