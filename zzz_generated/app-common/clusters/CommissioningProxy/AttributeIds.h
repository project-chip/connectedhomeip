// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommissioningProxy (cluster code: 1109/0x455)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 13;

namespace Transport {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Transport

namespace ScanMaxTime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ScanMaxTime

namespace MaxSessions {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxSessions

namespace MaxCachedResults {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MaxCachedResults

namespace NumCachedResults {
inline constexpr AttributeId Id = 0x00000004;
} // namespace NumCachedResults

namespace CacheTimeout {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CacheTimeout

namespace CachedResults {
inline constexpr AttributeId Id = 0x00000006;
} // namespace CachedResults

namespace WiFiBand {
inline constexpr AttributeId Id = 0x00000007;
} // namespace WiFiBand

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
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
