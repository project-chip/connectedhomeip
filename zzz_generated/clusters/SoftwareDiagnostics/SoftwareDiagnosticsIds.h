// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster SoftwareDiagnostics (cluster code: 52/0x34)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace SoftwareDiagnostics {

inline constexpr ClusterId kClusterId = 0x00000034;

namespace Attributes {
namespace ThreadMetrics {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ThreadMetrics
namespace CurrentHeapFree {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentHeapFree
namespace CurrentHeapUsed {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentHeapUsed
namespace CurrentHeapHighWatermark {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentHeapHighWatermark

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ResetWatermarks {
inline constexpr CommandId Id = 0x00000000;
} // namespace ResetWatermarks
} // namespace Commands

namespace Events {
namespace SoftwareFault {
inline constexpr EventId Id = 0x00000000;
} // namespace SoftwareFault
} // namespace Events

} // namespace SoftwareDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
