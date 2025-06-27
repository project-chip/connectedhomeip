// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster SoftwareDiagnostics (cluster code: 52/0x34)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {
namespace Attributes {
namespace ThreadMetrics {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ThreadMetrics

namespace CurrentHeapFree {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentHeapFree

namespace CurrentHeapUsed {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentHeapUsed

namespace CurrentHeapHighWatermark {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentHeapHighWatermark

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
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
