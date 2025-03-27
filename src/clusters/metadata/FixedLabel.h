// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FixedLabel (cluster code: 64/0x40)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace FixedLabel {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0040;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLabelListEntry = {
    .attributeId    = 0,
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

// TODO:
//   - help out with mandatory attributes (maybe that array is useful)
//
//   - would probably also want the BUILD.gn file generated to contain all data
//
