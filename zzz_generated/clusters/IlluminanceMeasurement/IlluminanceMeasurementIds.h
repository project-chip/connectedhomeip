// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster IlluminanceMeasurement (cluster code: 1024/0x400)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace IlluminanceMeasurement {

inline constexpr ClusterId kClusterId = 0x00000400;

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
namespace LightSensorType {
inline constexpr AttributeId Id = 0x00000004;
} // namespace LightSensorType

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events

} // namespace IlluminanceMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
