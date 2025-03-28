// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace TlsCertificateManagement {

inline constexpr ClusterId kClusterId = 0x00000801;

namespace Attributes {
namespace MaxRootCertificates {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxRootCertificates
namespace ProvisionedRootCertificates {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ProvisionedRootCertificates
namespace MaxClientCertificates {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxClientCertificates
namespace ProvisionedClientCertificates {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ProvisionedClientCertificates

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
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
namespace TLSClientCSR {
inline constexpr CommandId Id = 0x00000007;
} // namespace TLSClientCSR
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
} // namespace Commands

namespace Events {} // namespace Events

} // namespace TlsCertificateManagement
} // namespace clusters
} // namespace app
} // namespace chip
