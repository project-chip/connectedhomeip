// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ValveConfigurationAndControl (cluster code: 129/0x81)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ValveConfigurationAndControl {

inline constexpr ClusterId kClusterId = 0x00000081;

namespace Attributes {
namespace OpenDuration {
inline constexpr AttributeId Id = 0x00000000;
} // namespace OpenDuration
namespace DefaultOpenDuration {
inline constexpr AttributeId Id = 0x00000001;
} // namespace DefaultOpenDuration
namespace AutoCloseTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AutoCloseTime
namespace RemainingDuration {
inline constexpr AttributeId Id = 0x00000003;
} // namespace RemainingDuration
namespace CurrentState {
inline constexpr AttributeId Id = 0x00000004;
} // namespace CurrentState
namespace TargetState {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TargetState
namespace CurrentLevel {
inline constexpr AttributeId Id = 0x00000006;
} // namespace CurrentLevel
namespace TargetLevel {
inline constexpr AttributeId Id = 0x00000007;
} // namespace TargetLevel
namespace DefaultOpenLevel {
inline constexpr AttributeId Id = 0x00000008;
} // namespace DefaultOpenLevel
namespace ValveFault {
inline constexpr AttributeId Id = 0x00000009;
} // namespace ValveFault
namespace LevelStep {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace LevelStep

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Open {
inline constexpr CommandId Id = 0x00000000;
} // namespace Open
namespace Close {
inline constexpr CommandId Id = 0x00000001;
} // namespace Close
} // namespace Commands

namespace Events {
namespace ValveStateChanged {
inline constexpr CommandId Id = 0x00000000;
} // namespace ValveStateChanged
namespace ValveFault {
inline constexpr CommandId Id = 0x00000001;
} // namespace ValveFault
} // namespace Events
} // namespace ValveConfigurationAndControl
} // namespace clusters
} // namespace app
} // namespace chip
