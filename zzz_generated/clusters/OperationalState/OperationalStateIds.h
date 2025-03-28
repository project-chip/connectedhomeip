// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OperationalState (cluster code: 96/0x60)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace OperationalState {

inline constexpr ClusterId kClusterId = 0x00000060;

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
namespace Stop {
inline constexpr CommandId Id = 0x00000001;
} // namespace Stop
namespace Start {
inline constexpr CommandId Id = 0x00000002;
} // namespace Start
namespace Resume {
inline constexpr CommandId Id = 0x00000003;
} // namespace Resume
} // namespace Commands

namespace Events {
namespace OperationalError {
inline constexpr EventId Id = 0x00000000;
} // namespace OperationalError
namespace OperationCompletion {
inline constexpr EventId Id = 0x00000001;
} // namespace OperationCompletion
} // namespace Events
} // namespace OperationalState
} // namespace clusters
} // namespace app
} // namespace chip
