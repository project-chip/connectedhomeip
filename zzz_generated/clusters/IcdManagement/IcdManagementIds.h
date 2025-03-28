// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace IcdManagement {

inline constexpr ClusterId kClusterId = 0x00000046;

namespace Attributes {
namespace IdleModeDuration {
inline constexpr AttributeId Id = 0x00000000;
} // namespace IdleModeDuration
namespace ActiveModeDuration {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ActiveModeDuration
namespace ActiveModeThreshold {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ActiveModeThreshold
namespace RegisteredClients {
inline constexpr AttributeId Id = 0x00000003;
} // namespace RegisteredClients
namespace ICDCounter {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ICDCounter
namespace ClientsSupportedPerFabric {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ClientsSupportedPerFabric
namespace UserActiveModeTriggerHint {
inline constexpr AttributeId Id = 0x00000006;
} // namespace UserActiveModeTriggerHint
namespace UserActiveModeTriggerInstruction {
inline constexpr AttributeId Id = 0x00000007;
} // namespace UserActiveModeTriggerInstruction
namespace OperatingMode {
inline constexpr AttributeId Id = 0x00000008;
} // namespace OperatingMode
namespace MaximumCheckInBackOff {
inline constexpr AttributeId Id = 0x00000009;
} // namespace MaximumCheckInBackOff

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace RegisterClient {
inline constexpr CommandId Id = 0x00000000;
} // namespace RegisterClient
namespace UnregisterClient {
inline constexpr CommandId Id = 0x00000002;
} // namespace UnregisterClient
namespace StayActiveRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace StayActiveRequest
} // namespace Commands

namespace Events {} // namespace Events

} // namespace IcdManagement
} // namespace clusters
} // namespace app
} // namespace chip
