// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace DeviceEnergyManagement {

inline constexpr ClusterId kClusterId = 0x00000098;

namespace Attributes {
namespace ESAType {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ESAType
namespace ESACanGenerate {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ESACanGenerate
namespace ESAState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ESAState
namespace AbsMinPower {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AbsMinPower
namespace AbsMaxPower {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AbsMaxPower
namespace PowerAdjustmentCapability {
inline constexpr AttributeId Id = 0x00000005;
} // namespace PowerAdjustmentCapability
namespace Forecast {
inline constexpr AttributeId Id = 0x00000006;
} // namespace Forecast
namespace OptOutState {
inline constexpr AttributeId Id = 0x00000007;
} // namespace OptOutState

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace PowerAdjustRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace PowerAdjustRequest
namespace CancelPowerAdjustRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelPowerAdjustRequest
namespace StartTimeAdjustRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace StartTimeAdjustRequest
namespace PauseRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace PauseRequest
namespace ResumeRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace ResumeRequest
namespace ModifyForecastRequest {
inline constexpr CommandId Id = 0x00000005;
} // namespace ModifyForecastRequest
namespace RequestConstraintBasedForecast {
inline constexpr CommandId Id = 0x00000006;
} // namespace RequestConstraintBasedForecast
namespace CancelRequest {
inline constexpr CommandId Id = 0x00000007;
} // namespace CancelRequest
} // namespace Commands

namespace Events {
namespace PowerAdjustStart {
inline constexpr EventId Id = 0x00000000;
} // namespace PowerAdjustStart
namespace PowerAdjustEnd {
inline constexpr EventId Id = 0x00000001;
} // namespace PowerAdjustEnd
namespace Paused {
inline constexpr EventId Id = 0x00000002;
} // namespace Paused
namespace Resumed {
inline constexpr EventId Id = 0x00000003;
} // namespace Resumed
} // namespace Events

} // namespace DeviceEnergyManagement
} // namespace clusters
} // namespace app
} // namespace chip
