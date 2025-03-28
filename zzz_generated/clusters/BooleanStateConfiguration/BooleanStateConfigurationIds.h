// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace BooleanStateConfiguration {

inline constexpr ClusterId kClusterId = 0x00000080;

namespace Attributes {
namespace CurrentSensitivityLevel {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentSensitivityLevel
namespace SupportedSensitivityLevels {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedSensitivityLevels
namespace DefaultSensitivityLevel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace DefaultSensitivityLevel
namespace AlarmsActive {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AlarmsActive
namespace AlarmsSuppressed {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AlarmsSuppressed
namespace AlarmsEnabled {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AlarmsEnabled
namespace AlarmsSupported {
inline constexpr AttributeId Id = 0x00000006;
} // namespace AlarmsSupported
namespace SensorFault {
inline constexpr AttributeId Id = 0x00000007;
} // namespace SensorFault

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SuppressAlarm {
inline constexpr CommandId Id = 0x00000000;
} // namespace SuppressAlarm
namespace EnableDisableAlarm {
inline constexpr CommandId Id = 0x00000001;
} // namespace EnableDisableAlarm
} // namespace Commands

namespace Events {
namespace AlarmsStateChanged {
inline constexpr CommandId Id = 0x00000000;
} // namespace AlarmsStateChanged
namespace SensorFault {
inline constexpr CommandId Id = 0x00000001;
} // namespace SensorFault
} // namespace Events
} // namespace BooleanStateConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
