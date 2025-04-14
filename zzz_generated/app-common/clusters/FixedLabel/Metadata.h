// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FixedLabel (cluster code: 64/0x40)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FixedLabel/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FixedLabel {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace LabelList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LabelList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LabelList

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace FixedLabel
} // namespace Clusters
} // namespace app
} // namespace chip
