// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ElectricalEnergyMeasurement {

inline constexpr ClusterId kClusterId = 0x00000091;

namespace Attributes {
namespace Accuracy {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Accuracy
namespace CumulativeEnergyImported {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CumulativeEnergyImported
namespace CumulativeEnergyExported {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CumulativeEnergyExported
namespace PeriodicEnergyImported {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PeriodicEnergyImported
namespace PeriodicEnergyExported {
inline constexpr AttributeId Id = 0x00000004;
} // namespace PeriodicEnergyExported
namespace CumulativeEnergyReset {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CumulativeEnergyReset

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace CumulativeEnergyMeasured {
inline constexpr CommandId Id = 0x00000000;
} // namespace CumulativeEnergyMeasured
namespace PeriodicEnergyMeasured {
inline constexpr CommandId Id = 0x00000001;
} // namespace PeriodicEnergyMeasured
} // namespace Events
} // namespace ElectricalEnergyMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
