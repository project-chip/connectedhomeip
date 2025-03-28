// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace CommissionerControl {

inline constexpr ClusterId kClusterId = 0x00000751;

namespace Attributes {
namespace SupportedDeviceCategories {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedDeviceCategories

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace RequestCommissioningApproval {
inline constexpr CommandId Id = 0x00000000;
} // namespace RequestCommissioningApproval
namespace CommissionNode {
inline constexpr CommandId Id = 0x00000001;
} // namespace CommissionNode
} // namespace Commands

namespace Events {
namespace CommissioningRequestResult {
inline constexpr EventId Id = 0x00000000;
} // namespace CommissioningRequestResult
} // namespace Events

} // namespace CommissionerControl
} // namespace clusters
} // namespace app
} // namespace chip
