// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Binding (cluster code: 30/0x1E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Binding/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Binding {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Binding::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Binding

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace chip
