// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaFileManagement (cluster code: 1297/0x511)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 9;

namespace TotalStorage {
inline constexpr AttributeId Id = 0x00000000;
} // namespace TotalStorage

namespace AvailableStorage {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AvailableStorage

namespace AvailableFiles {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AvailableFiles

namespace SupportedMimeTypes {
inline constexpr AttributeId Id = 0x00000003;
} // namespace SupportedMimeTypes

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
} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
