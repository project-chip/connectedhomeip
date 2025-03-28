// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FixedLabel (cluster code: 64/0x40)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FixedLabel/FixedLabelIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace FixedLabel {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLabelListEntry = {
    .attributeId    = FixedLabel::Attributes::LabelList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace FixedLabel
} // namespace clusters
} // namespace app
} // namespace chip
