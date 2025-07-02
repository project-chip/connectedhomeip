// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster BallastConfiguration (cluster code: 769/0x301)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BallastConfiguration {
namespace Attributes {
namespace PhysicalMinLevel {
inline constexpr AttributeId Id = 0x00000000;
} // namespace PhysicalMinLevel

namespace PhysicalMaxLevel {
inline constexpr AttributeId Id = 0x00000001;
} // namespace PhysicalMaxLevel

namespace BallastStatus {
inline constexpr AttributeId Id = 0x00000002;
} // namespace BallastStatus

namespace MinLevel {
inline constexpr AttributeId Id = 0x00000010;
} // namespace MinLevel

namespace MaxLevel {
inline constexpr AttributeId Id = 0x00000011;
} // namespace MaxLevel

namespace IntrinsicBallastFactor {
inline constexpr AttributeId Id = 0x00000014;
} // namespace IntrinsicBallastFactor

namespace BallastFactorAdjustment {
inline constexpr AttributeId Id = 0x00000015;
} // namespace BallastFactorAdjustment

namespace LampQuantity {
inline constexpr AttributeId Id = 0x00000020;
} // namespace LampQuantity

namespace LampType {
inline constexpr AttributeId Id = 0x00000030;
} // namespace LampType

namespace LampManufacturer {
inline constexpr AttributeId Id = 0x00000031;
} // namespace LampManufacturer

namespace LampRatedHours {
inline constexpr AttributeId Id = 0x00000032;
} // namespace LampRatedHours

namespace LampBurnHours {
inline constexpr AttributeId Id = 0x00000033;
} // namespace LampBurnHours

namespace LampAlarmMode {
inline constexpr AttributeId Id = 0x00000034;
} // namespace LampAlarmMode

namespace LampBurnHoursTripPoint {
inline constexpr AttributeId Id = 0x00000035;
} // namespace LampBurnHoursTripPoint

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
} // namespace BallastConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
