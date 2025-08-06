// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 5;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace SolicitOffer {
inline constexpr CommandId Id = 0x00000000;
} // namespace SolicitOffer

namespace ProvideOffer {
inline constexpr CommandId Id = 0x00000002;
} // namespace ProvideOffer

namespace ProvideAnswer {
inline constexpr CommandId Id = 0x00000004;
} // namespace ProvideAnswer

namespace ProvideICECandidates {
inline constexpr CommandId Id = 0x00000005;
} // namespace ProvideICECandidates

namespace EndSession {
inline constexpr CommandId Id = 0x00000006;
} // namespace EndSession

namespace SolicitOfferResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace SolicitOfferResponse

namespace ProvideOfferResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace ProvideOfferResponse

} // namespace Commands
} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
