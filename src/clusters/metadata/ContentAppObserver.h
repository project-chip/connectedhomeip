// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace ContentAppObserver {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0510;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kContentAppMessageEntry = {
    .commandId       = 0,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands

} // namespace Metadata
} // namespace ContentAppObserver
} // namespace clusters
} // namespace app
} // namespace chip

// TODO:
//   - help out with mandatory attributes (maybe that array is useful)
//
//   - would probably also want the BUILD.gn file generated to contain all data
//
