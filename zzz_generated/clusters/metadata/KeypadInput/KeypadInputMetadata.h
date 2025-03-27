// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster KeypadInput (cluster code: 1289/0x509)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip {
namespace app {
namespace clusters {
namespace KeypadInput {
namespace Metadata {

inline constexpr ClusterId kClusterId = 0x0509;
inline constexpr uint32_t kRevision   = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSendKeyEntry = {
    .commandId       = 0,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands

} // namespace Metadata
} // namespace KeypadInput
} // namespace clusters
} // namespace app
} // namespace chip
