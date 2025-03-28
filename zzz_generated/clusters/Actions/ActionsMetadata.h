// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Actions (cluster code: 37/0x25)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Actions {

inline constexpr ClusterId kClusterId = 0x00000025;

namespace Attributes {
namespace ActionList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ActionList
namespace EndpointLists {
inline constexpr AttributeId Id = 0x00000001;
} // namespace EndpointLists
namespace SetupURL {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SetupURL

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace InstantAction {
inline constexpr CommandId Id = 0x00000000;
} // namespace InstantAction
namespace InstantActionWithTransition {
inline constexpr CommandId Id = 0x00000001;
} // namespace InstantActionWithTransition
namespace StartAction {
inline constexpr CommandId Id = 0x00000002;
} // namespace StartAction
namespace StartActionWithDuration {
inline constexpr CommandId Id = 0x00000003;
} // namespace StartActionWithDuration
namespace StopAction {
inline constexpr CommandId Id = 0x00000004;
} // namespace StopAction
namespace PauseAction {
inline constexpr CommandId Id = 0x00000005;
} // namespace PauseAction
namespace PauseActionWithDuration {
inline constexpr CommandId Id = 0x00000006;
} // namespace PauseActionWithDuration
namespace ResumeAction {
inline constexpr CommandId Id = 0x00000007;
} // namespace ResumeAction
namespace EnableAction {
inline constexpr CommandId Id = 0x00000008;
} // namespace EnableAction
namespace EnableActionWithDuration {
inline constexpr CommandId Id = 0x00000009;
} // namespace EnableActionWithDuration
namespace DisableAction {
inline constexpr CommandId Id = 0x0000000A;
} // namespace DisableAction
namespace DisableActionWithDuration {
inline constexpr CommandId Id = 0x0000000B;
} // namespace DisableActionWithDuration
} // namespace Commands

namespace Events {
namespace StateChanged {
inline constexpr EventId Id = 0x00000000;
} // namespace StateChanged
namespace ActionFailed {
inline constexpr EventId Id = 0x00000001;
} // namespace ActionFailed
} // namespace Events

} // namespace Actions
} // namespace clusters
} // namespace app
} // namespace chip
