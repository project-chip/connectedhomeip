// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {
namespace Events {
namespace SupplyVoltageLow {
inline constexpr EventId Id = 0x00000000;
} // namespace SupplyVoltageLow

namespace SupplyVoltageHigh {
inline constexpr EventId Id = 0x00000001;
} // namespace SupplyVoltageHigh

namespace PowerMissingPhase {
inline constexpr EventId Id = 0x00000002;
} // namespace PowerMissingPhase

namespace SystemPressureLow {
inline constexpr EventId Id = 0x00000003;
} // namespace SystemPressureLow

namespace SystemPressureHigh {
inline constexpr EventId Id = 0x00000004;
} // namespace SystemPressureHigh

namespace DryRunning {
inline constexpr EventId Id = 0x00000005;
} // namespace DryRunning

namespace MotorTemperatureHigh {
inline constexpr EventId Id = 0x00000006;
} // namespace MotorTemperatureHigh

namespace PumpMotorFatalFailure {
inline constexpr EventId Id = 0x00000007;
} // namespace PumpMotorFatalFailure

namespace ElectronicTemperatureHigh {
inline constexpr EventId Id = 0x00000008;
} // namespace ElectronicTemperatureHigh

namespace PumpBlocked {
inline constexpr EventId Id = 0x00000009;
} // namespace PumpBlocked

namespace SensorFailure {
inline constexpr EventId Id = 0x0000000A;
} // namespace SensorFailure

namespace ElectronicNonFatalFailure {
inline constexpr EventId Id = 0x0000000B;
} // namespace ElectronicNonFatalFailure

namespace ElectronicFatalFailure {
inline constexpr EventId Id = 0x0000000C;
} // namespace ElectronicFatalFailure

namespace GeneralFault {
inline constexpr EventId Id = 0x0000000D;
} // namespace GeneralFault

namespace Leakage {
inline constexpr EventId Id = 0x0000000E;
} // namespace Leakage

namespace AirDetection {
inline constexpr EventId Id = 0x0000000F;
} // namespace AirDetection

namespace TurbineOperation {
inline constexpr EventId Id = 0x00000010;
} // namespace TurbineOperation

} // namespace Events
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
