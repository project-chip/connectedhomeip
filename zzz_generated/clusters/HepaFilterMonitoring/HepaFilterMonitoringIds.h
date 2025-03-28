// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster HepaFilterMonitoring (cluster code: 113/0x71)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace HepaFilterMonitoring {

inline constexpr ClusterId kClusterId = 0x00000071;

namespace Attributes {
namespace Condition {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Condition
namespace DegradationDirection {
inline constexpr AttributeId Id = 0x00000001;
} // namespace DegradationDirection
namespace ChangeIndication {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ChangeIndication
namespace InPlaceIndicator {
inline constexpr AttributeId Id = 0x00000003;
} // namespace InPlaceIndicator
namespace LastChangedTime {
inline constexpr AttributeId Id = 0x00000004;
} // namespace LastChangedTime
namespace ReplacementProductList {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ReplacementProductList

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ResetCondition {
inline constexpr CommandId Id = 0x00000000;
} // namespace ResetCondition
} // namespace Commands

namespace Events {} // namespace Events
} // namespace HepaFilterMonitoring
} // namespace clusters
} // namespace app
} // namespace chip
