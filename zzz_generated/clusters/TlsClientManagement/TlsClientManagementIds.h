// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace TlsClientManagement {

inline constexpr ClusterId kClusterId = 0x00000802;

namespace Attributes {
namespace MaxProvisioned {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxProvisioned
namespace ProvisionedEndpoints {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ProvisionedEndpoints

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ProvisionEndpoint {
inline constexpr CommandId Id = 0x00000000;
} // namespace ProvisionEndpoint
namespace FindEndpoint {
inline constexpr CommandId Id = 0x00000002;
} // namespace FindEndpoint
namespace RemoveEndpoint {
inline constexpr CommandId Id = 0x00000004;
} // namespace RemoveEndpoint
} // namespace Commands

namespace Events {} // namespace Events

} // namespace TlsClientManagement
} // namespace clusters
} // namespace app
} // namespace chip
