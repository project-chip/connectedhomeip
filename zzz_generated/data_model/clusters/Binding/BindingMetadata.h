// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Binding (cluster code: 30/0x1E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Binding/BindingIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace Binding {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kBindingEntry = {
    .attributeId    = Binding::Attributes::Binding::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace Binding
} // namespace clusters
} // namespace app
} // namespace chip
