// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace PushAvStreamTransport {

inline constexpr ClusterId kClusterId = 0x00000555;

namespace Attributes {
namespace SupportedContainerFormats {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedContainerFormats
namespace SupportedIngestMethods {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedIngestMethods
namespace CurrentConnections {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentConnections

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace AllocatePushTransport {
inline constexpr CommandId Id = 0x00000000;
} // namespace AllocatePushTransport
namespace DeallocatePushTransport {
inline constexpr CommandId Id = 0x00000002;
} // namespace DeallocatePushTransport
namespace ModifyPushTransport {
inline constexpr CommandId Id = 0x00000003;
} // namespace ModifyPushTransport
namespace SetTransportStatus {
inline constexpr CommandId Id = 0x00000004;
} // namespace SetTransportStatus
namespace ManuallyTriggerTransport {
inline constexpr CommandId Id = 0x00000005;
} // namespace ManuallyTriggerTransport
namespace FindTransport {
inline constexpr CommandId Id = 0x00000006;
} // namespace FindTransport
} // namespace Commands

namespace Events {
namespace PushTransportBegin {
inline constexpr EventId Id = 0x00000000;
} // namespace PushTransportBegin
namespace PushTransportEnd {
inline constexpr EventId Id = 0x00000001;
} // namespace PushTransportEnd
} // namespace Events

} // namespace PushAvStreamTransport
} // namespace clusters
} // namespace app
} // namespace chip
