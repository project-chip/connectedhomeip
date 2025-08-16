// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalCredentials {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 10;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 5;

namespace AttestationRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace AttestationRequest

namespace CertificateChainRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace CertificateChainRequest

namespace CSRRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace CSRRequest

namespace AddNOC {
inline constexpr CommandId Id = 0x00000006;
} // namespace AddNOC

namespace UpdateNOC {
inline constexpr CommandId Id = 0x00000007;
} // namespace UpdateNOC

namespace UpdateFabricLabel {
inline constexpr CommandId Id = 0x00000009;
} // namespace UpdateFabricLabel

namespace RemoveFabric {
inline constexpr CommandId Id = 0x0000000A;
} // namespace RemoveFabric

namespace AddTrustedRootCertificate {
inline constexpr CommandId Id = 0x0000000B;
} // namespace AddTrustedRootCertificate

namespace SetVIDVerificationStatement {
inline constexpr CommandId Id = 0x0000000C;
} // namespace SetVIDVerificationStatement

namespace SignVIDVerificationRequest {
inline constexpr CommandId Id = 0x0000000D;
} // namespace SignVIDVerificationRequest

namespace AttestationResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AttestationResponse

namespace CertificateChainResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace CertificateChainResponse

namespace CSRResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace CSRResponse

namespace NOCResponse {
inline constexpr CommandId Id = 0x00000008;
} // namespace NOCResponse

namespace SignVIDVerificationResponse {
inline constexpr CommandId Id = 0x0000000E;
} // namespace SignVIDVerificationResponse

} // namespace Commands
} // namespace OperationalCredentials
} // namespace Clusters
} // namespace app
} // namespace chip
