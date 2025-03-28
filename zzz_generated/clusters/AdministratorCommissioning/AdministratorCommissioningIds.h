// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace AdministratorCommissioning {

inline constexpr ClusterId kClusterId = 0x0000003C;

namespace Attributes {
namespace WindowStatus {
inline constexpr AttributeId Id = 0x00000000;
} // namespace WindowStatus
namespace AdminFabricIndex {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AdminFabricIndex
namespace AdminVendorId {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AdminVendorId

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace OpenCommissioningWindow {
inline constexpr CommandId Id = 0x00000000;
} // namespace OpenCommissioningWindow
namespace OpenBasicCommissioningWindow {
inline constexpr CommandId Id = 0x00000001;
} // namespace OpenBasicCommissioningWindow
namespace RevokeCommissioning {
inline constexpr CommandId Id = 0x00000002;
} // namespace RevokeCommissioning
} // namespace Commands

namespace Events {} // namespace Events
} // namespace AdministratorCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
