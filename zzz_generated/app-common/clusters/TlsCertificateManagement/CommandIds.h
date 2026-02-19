// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsCertificateManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 9;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 6;

namespace ProvisionRootCertificate {
inline constexpr CommandId Id = 0x00000000;
} // namespace ProvisionRootCertificate

namespace FindRootCertificate {
inline constexpr CommandId Id = 0x00000002;
} // namespace FindRootCertificate

namespace LookupRootCertificate {
inline constexpr CommandId Id = 0x00000004;
} // namespace LookupRootCertificate

namespace RemoveRootCertificate {
inline constexpr CommandId Id = 0x00000006;
} // namespace RemoveRootCertificate

namespace ClientCSR {
inline constexpr CommandId Id = 0x00000007;
} // namespace ClientCSR

namespace ProvisionClientCertificate {
inline constexpr CommandId Id = 0x00000009;
} // namespace ProvisionClientCertificate

namespace FindClientCertificate {
inline constexpr CommandId Id = 0x0000000A;
} // namespace FindClientCertificate

namespace LookupClientCertificate {
inline constexpr CommandId Id = 0x0000000C;
} // namespace LookupClientCertificate

namespace RemoveClientCertificate {
inline constexpr CommandId Id = 0x0000000E;
} // namespace RemoveClientCertificate

namespace ProvisionRootCertificateResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ProvisionRootCertificateResponse

namespace FindRootCertificateResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace FindRootCertificateResponse

namespace LookupRootCertificateResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace LookupRootCertificateResponse

namespace ClientCSRResponse {
inline constexpr CommandId Id = 0x00000008;
} // namespace ClientCSRResponse

namespace FindClientCertificateResponse {
inline constexpr CommandId Id = 0x0000000B;
} // namespace FindClientCertificateResponse

namespace LookupClientCertificateResponse {
inline constexpr CommandId Id = 0x0000000D;
} // namespace LookupClientCertificateResponse

} // namespace Commands
} // namespace TlsCertificateManagement
} // namespace Clusters
} // namespace app
} // namespace chip
