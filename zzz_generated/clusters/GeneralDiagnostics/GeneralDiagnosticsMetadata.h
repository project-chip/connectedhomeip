// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace GeneralDiagnostics {

inline constexpr ClusterId kClusterId = 0x00000033;

namespace Attributes {
namespace NetworkInterfaces {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NetworkInterfaces
namespace RebootCount {
inline constexpr AttributeId Id = 0x00000001;
} // namespace RebootCount
namespace UpTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpTime
namespace TotalOperationalHours {
inline constexpr AttributeId Id = 0x00000003;
} // namespace TotalOperationalHours
namespace BootReason {
inline constexpr AttributeId Id = 0x00000004;
} // namespace BootReason
namespace ActiveHardwareFaults {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ActiveHardwareFaults
namespace ActiveRadioFaults {
inline constexpr AttributeId Id = 0x00000006;
} // namespace ActiveRadioFaults
namespace ActiveNetworkFaults {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ActiveNetworkFaults
namespace TestEventTriggersEnabled {
inline constexpr AttributeId Id = 0x00000008;
} // namespace TestEventTriggersEnabled

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace TestEventTrigger {
inline constexpr CommandId Id = 0x00000000;
} // namespace TestEventTrigger
namespace TimeSnapshot {
inline constexpr CommandId Id = 0x00000001;
} // namespace TimeSnapshot
namespace PayloadTestRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace PayloadTestRequest
} // namespace Commands

namespace Events {
namespace HardwareFaultChange {
inline constexpr EventId Id = 0x00000000;
} // namespace HardwareFaultChange
namespace RadioFaultChange {
inline constexpr EventId Id = 0x00000001;
} // namespace RadioFaultChange
namespace NetworkFaultChange {
inline constexpr EventId Id = 0x00000002;
} // namespace NetworkFaultChange
namespace BootReason {
inline constexpr EventId Id = 0x00000003;
} // namespace BootReason
} // namespace Events

} // namespace GeneralDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
