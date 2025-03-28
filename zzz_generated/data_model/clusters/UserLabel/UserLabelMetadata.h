// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UserLabel (cluster code: 65/0x41)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UserLabel/UserLabelIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace UserLabel {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLabelListEntry = {
    .attributeId    = UserLabel::Attributes::LabelList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace UserLabel
} // namespace clusters
} // namespace app
} // namespace chip
