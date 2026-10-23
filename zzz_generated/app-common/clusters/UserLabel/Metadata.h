// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UserLabel (cluster code: 65/0x41)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    LabelList::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace UserLabel
} // namespace Clusters
} // namespace app
} // namespace chip
