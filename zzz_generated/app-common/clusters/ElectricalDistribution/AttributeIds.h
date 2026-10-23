// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalDistribution (cluster code: 162/0xA2)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalDistribution {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 10;

namespace MaxContinuousCurrent {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxContinuousCurrent

namespace MaxVoltage {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxVoltage

namespace NumberOfPoles {
inline constexpr AttributeId Id = 0x00000002;
} // namespace NumberOfPoles

namespace EndOfLife {
inline constexpr AttributeId Id = 0x00000003;
} // namespace EndOfLife

namespace ServiceEntranceRated {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ServiceEntranceRated

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
} // namespace ElectricalDistribution
} // namespace Clusters
} // namespace app
} // namespace chip
