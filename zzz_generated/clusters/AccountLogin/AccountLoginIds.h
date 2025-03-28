// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccountLogin (cluster code: 1294/0x50E)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace AccountLogin {

inline constexpr ClusterId kClusterId = 0x0000050E;

namespace Attributes {

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace GetSetupPIN {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetSetupPIN
namespace Login {
inline constexpr CommandId Id = 0x00000002;
} // namespace Login
namespace Logout {
inline constexpr CommandId Id = 0x00000003;
} // namespace Logout
} // namespace Commands

namespace Events {
namespace LoggedOut {
inline constexpr EventId Id = 0x00000000;
} // namespace LoggedOut
} // namespace Events
} // namespace AccountLogin
} // namespace clusters
} // namespace app
} // namespace chip
