// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WaterHeaterManagement (cluster code: 148/0x94)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace WaterHeaterManagement {

inline constexpr ClusterId kClusterId = 0x00000094;

namespace Attributes {
namespace HeaterTypes {
inline constexpr AttributeId Id = 0x00000000;
} // namespace HeaterTypes
namespace HeatDemand {
inline constexpr AttributeId Id = 0x00000001;
} // namespace HeatDemand
namespace TankVolume {
inline constexpr AttributeId Id = 0x00000002;
} // namespace TankVolume
namespace EstimatedHeatRequired {
inline constexpr AttributeId Id = 0x00000003;
} // namespace EstimatedHeatRequired
namespace TankPercentage {
inline constexpr AttributeId Id = 0x00000004;
} // namespace TankPercentage
namespace BoostState {
inline constexpr AttributeId Id = 0x00000005;
} // namespace BoostState

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Boost {
inline constexpr CommandId Id = 0x00000000;
} // namespace Boost
namespace CancelBoost {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelBoost
} // namespace Commands

namespace Events {
namespace BoostStarted {
inline constexpr CommandId Id = 0x00000000;
} // namespace BoostStarted
namespace BoostEnded {
inline constexpr CommandId Id = 0x00000001;
} // namespace BoostEnded
} // namespace Events
} // namespace WaterHeaterManagement
} // namespace clusters
} // namespace app
} // namespace chip
