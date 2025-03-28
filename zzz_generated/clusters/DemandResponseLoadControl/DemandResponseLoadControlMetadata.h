// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace DemandResponseLoadControl {

inline constexpr ClusterId kClusterId = 0x00000096;

namespace Attributes {
namespace LoadControlPrograms {
inline constexpr AttributeId Id = 0x00000000;
} // namespace LoadControlPrograms
namespace NumberOfLoadControlPrograms {
inline constexpr AttributeId Id = 0x00000001;
} // namespace NumberOfLoadControlPrograms
namespace Events {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Events
namespace ActiveEvents {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ActiveEvents
namespace NumberOfEventsPerProgram {
inline constexpr AttributeId Id = 0x00000004;
} // namespace NumberOfEventsPerProgram
namespace NumberOfTransitions {
inline constexpr AttributeId Id = 0x00000005;
} // namespace NumberOfTransitions
namespace DefaultRandomStart {
inline constexpr AttributeId Id = 0x00000006;
} // namespace DefaultRandomStart
namespace DefaultRandomDuration {
inline constexpr AttributeId Id = 0x00000007;
} // namespace DefaultRandomDuration

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace RegisterLoadControlProgramRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace RegisterLoadControlProgramRequest
namespace UnregisterLoadControlProgramRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace UnregisterLoadControlProgramRequest
namespace AddLoadControlEventRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddLoadControlEventRequest
namespace RemoveLoadControlEventRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveLoadControlEventRequest
namespace ClearLoadControlEventsRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace ClearLoadControlEventsRequest
} // namespace Commands

namespace Events {
namespace LoadControlEventStatusChange {
inline constexpr EventId Id = 0x00000000;
} // namespace LoadControlEventStatusChange
} // namespace Events

} // namespace DemandResponseLoadControl
} // namespace clusters
} // namespace app
} // namespace chip
