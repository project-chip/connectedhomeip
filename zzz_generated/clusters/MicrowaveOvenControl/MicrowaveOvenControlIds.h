// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MicrowaveOvenControl (cluster code: 95/0x5F)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace MicrowaveOvenControl {

inline constexpr ClusterId kClusterId = 0x0000005F;

namespace Attributes {
namespace CookTime {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CookTime
namespace MaxCookTime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxCookTime
namespace PowerSetting {
inline constexpr AttributeId Id = 0x00000002;
} // namespace PowerSetting
namespace MinPower {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MinPower
namespace MaxPower {
inline constexpr AttributeId Id = 0x00000004;
} // namespace MaxPower
namespace PowerStep {
inline constexpr AttributeId Id = 0x00000005;
} // namespace PowerStep
namespace SupportedWatts {
inline constexpr AttributeId Id = 0x00000006;
} // namespace SupportedWatts
namespace SelectedWattIndex {
inline constexpr AttributeId Id = 0x00000007;
} // namespace SelectedWattIndex
namespace WattRating {
inline constexpr AttributeId Id = 0x00000008;
} // namespace WattRating

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SetCookingParameters {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetCookingParameters
namespace AddMoreTime {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddMoreTime
} // namespace Commands

namespace Events {} // namespace Events

} // namespace MicrowaveOvenControl
} // namespace clusters
} // namespace app
} // namespace chip
