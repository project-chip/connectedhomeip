// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace OperationalCredentials {

inline constexpr ClusterId kClusterId = 0x0000003E;

namespace Attributes {
namespace NOCs {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NOCs
namespace Fabrics {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Fabrics
namespace SupportedFabrics {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SupportedFabrics
namespace CommissionedFabrics {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CommissionedFabrics
namespace TrustedRootCertificates {
inline constexpr AttributeId Id = 0x00000004;
} // namespace TrustedRootCertificates
namespace CurrentFabricIndex {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CurrentFabricIndex

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
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
} // namespace Commands

namespace Events {} // namespace Events
} // namespace OperationalCredentials
} // namespace clusters
} // namespace app
} // namespace chip
