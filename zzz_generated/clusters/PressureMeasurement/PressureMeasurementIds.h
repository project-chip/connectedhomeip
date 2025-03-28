// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PressureMeasurement (cluster code: 1027/0x403)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace PressureMeasurement {

inline constexpr ClusterId kClusterId = 0x00000403;

namespace Attributes {
namespace MeasuredValue {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MeasuredValue
namespace MinMeasuredValue {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MinMeasuredValue
namespace MaxMeasuredValue {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxMeasuredValue
namespace Tolerance {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Tolerance
namespace ScaledValue {
inline constexpr AttributeId Id = 0x00000010;
} // namespace ScaledValue
namespace MinScaledValue {
inline constexpr AttributeId Id = 0x00000011;
} // namespace MinScaledValue
namespace MaxScaledValue {
inline constexpr AttributeId Id = 0x00000012;
} // namespace MaxScaledValue
namespace ScaledTolerance {
inline constexpr AttributeId Id = 0x00000013;
} // namespace ScaledTolerance
namespace Scale {
inline constexpr AttributeId Id = 0x00000014;
} // namespace Scale

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace PressureMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
