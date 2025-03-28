// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster KeypadInput (cluster code: 1289/0x509)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/KeypadInput/KeypadInputIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace KeypadInput {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSendKeyEntry = {
    .commandId       = KeypadInput::Commands::SendKey::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace KeypadInput
} // namespace clusters
} // namespace app
} // namespace chip
