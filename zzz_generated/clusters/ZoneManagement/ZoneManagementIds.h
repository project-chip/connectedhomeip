// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ZoneManagement (cluster code: 1360/0x550)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

inline constexpr ClusterId Id = 0x00000550;

namespace Attributes {
namespace SupportedZoneSources {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedZoneSources
namespace Zones {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Zones
namespace Triggers {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Triggers
namespace Sensitivity {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Sensitivity
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

namespace Commands {
namespace CreateTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000000;
} // namespace CreateTwoDCartesianZone
namespace UpdateTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000002;
} // namespace UpdateTwoDCartesianZone
namespace GetTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000003;
} // namespace GetTwoDCartesianZone
namespace RemoveZone {
inline constexpr CommandId Id = 0x00000005;
} // namespace RemoveZone
namespace CreateTwoDCartesianZoneResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace CreateTwoDCartesianZoneResponse
namespace GetTwoDCartesianZoneResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace GetTwoDCartesianZoneResponse
} // namespace Commands

namespace Events {
namespace ZoneTriggered {
inline constexpr EventId Id = 0x00000000;
} // namespace ZoneTriggered
namespace ZoneStopped {
inline constexpr EventId Id = 0x00000001;
} // namespace ZoneStopped
} // namespace Events

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
