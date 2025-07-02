// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UserLabel (cluster code: 65/0x41)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UserLabel/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace UserLabel {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace LabelList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(LabelList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LabelList

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace UserLabel
} // namespace Clusters
} // namespace app
} // namespace chip
