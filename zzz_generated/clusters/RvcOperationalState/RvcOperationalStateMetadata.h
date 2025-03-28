// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster RvcOperationalState (cluster code: 97/0x61)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace RvcOperationalState {

inline constexpr ClusterId kClusterId = 0x00000061;

namespace Attributes {
namespace PhaseList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace PhaseList
namespace CurrentPhase {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentPhase
namespace CountdownTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CountdownTime
namespace OperationalStateList {
inline constexpr AttributeId Id = 0x00000003;
} // namespace OperationalStateList
namespace OperationalState {
inline constexpr AttributeId Id = 0x00000004;
} // namespace OperationalState
namespace OperationalError {
inline constexpr AttributeId Id = 0x00000005;
} // namespace OperationalError

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Pause {
inline constexpr CommandId Id = 0x00000000;
} // namespace Pause
namespace Resume {
inline constexpr CommandId Id = 0x00000003;
} // namespace Resume
namespace GoHome {
inline constexpr CommandId Id = 0x00000080;
} // namespace GoHome
} // namespace Commands

namespace Events {
namespace OperationalError {
inline constexpr EventId Id = 0x00000000;
} // namespace OperationalError
namespace OperationCompletion {
inline constexpr EventId Id = 0x00000001;
} // namespace OperationCompletion
} // namespace Events

} // namespace RvcOperationalState
} // namespace clusters
} // namespace app
} // namespace chip
