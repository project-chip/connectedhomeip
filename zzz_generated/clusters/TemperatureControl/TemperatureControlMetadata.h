// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TemperatureControl (cluster code: 86/0x56)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace TemperatureControl {

inline constexpr ClusterId kClusterId = 0x00000056;

namespace Attributes {
namespace TemperatureSetpoint {
inline constexpr AttributeId Id = 0x00000000;
} // namespace TemperatureSetpoint
namespace MinTemperature {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MinTemperature
namespace MaxTemperature {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxTemperature
namespace Step {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Step
namespace SelectedTemperatureLevel {
inline constexpr AttributeId Id = 0x00000004;
} // namespace SelectedTemperatureLevel
namespace SupportedTemperatureLevels {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SupportedTemperatureLevels

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SetTemperature {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTemperature
} // namespace Commands

namespace Events {} // namespace Events

} // namespace TemperatureControl
} // namespace clusters
} // namespace app
} // namespace chip
