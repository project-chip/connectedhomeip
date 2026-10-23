// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AvAnalysis (cluster code: 1367/0x557)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 11;

namespace SupportedAmbientContexts {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedAmbientContexts

namespace ActiveAmbientContextTriggers {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ActiveAmbientContextTriggers

namespace MaxAnalysisStreamCount {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxAnalysisStreamCount

namespace CurrentAnalysisStreamCount {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentAnalysisStreamCount

namespace AnalysisStreams {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AnalysisStreams

namespace TrackingEnabled {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TrackingEnabled

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
} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
