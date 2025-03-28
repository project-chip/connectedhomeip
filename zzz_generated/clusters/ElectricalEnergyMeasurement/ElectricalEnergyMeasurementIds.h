// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

inline constexpr ClusterId Id = 0x00000091;

namespace Attributes {
namespace Accuracy {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Accuracy
namespace CumulativeEnergyImported {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CumulativeEnergyImported
namespace CumulativeEnergyExported {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CumulativeEnergyExported
namespace PeriodicEnergyImported {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PeriodicEnergyImported
namespace PeriodicEnergyExported {
inline constexpr AttributeId Id = 0x00000004;
} // namespace PeriodicEnergyExported
namespace CumulativeEnergyReset {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CumulativeEnergyReset
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

namespace Events {
namespace CumulativeEnergyMeasured {
inline constexpr EventId Id = 0x00000000;
} // namespace CumulativeEnergyMeasured
namespace PeriodicEnergyMeasured {
inline constexpr EventId Id = 0x00000001;
} // namespace PeriodicEnergyMeasured
} // namespace Events

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
