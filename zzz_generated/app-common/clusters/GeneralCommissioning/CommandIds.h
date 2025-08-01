// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralCommissioning (cluster code: 48/0x30)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 4;

namespace ArmFailSafe {
inline constexpr CommandId Id = 0x00000000;
} // namespace ArmFailSafe

namespace SetRegulatoryConfig {
inline constexpr CommandId Id = 0x00000002;
} // namespace SetRegulatoryConfig

namespace CommissioningComplete {
inline constexpr CommandId Id = 0x00000004;
} // namespace CommissioningComplete

namespace SetTCAcknowledgements {
inline constexpr CommandId Id = 0x00000006;
} // namespace SetTCAcknowledgements

namespace ArmFailSafeResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ArmFailSafeResponse

namespace SetRegulatoryConfigResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetRegulatoryConfigResponse

namespace CommissioningCompleteResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace CommissioningCompleteResponse

namespace SetTCAcknowledgementsResponse {
inline constexpr CommandId Id = 0x00000007;
} // namespace SetTCAcknowledgementsResponse

} // namespace Commands
} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
