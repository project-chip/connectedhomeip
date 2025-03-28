// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace WebRTCTransportRequestor {

inline constexpr ClusterId kClusterId = 0x00000554;

namespace Attributes {
namespace CurrentSessions {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentSessions

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Offer {
inline constexpr CommandId Id = 0x00000001;
} // namespace Offer
namespace Answer {
inline constexpr CommandId Id = 0x00000002;
} // namespace Answer
namespace ICECandidates {
inline constexpr CommandId Id = 0x00000003;
} // namespace ICECandidates
namespace End {
inline constexpr CommandId Id = 0x00000004;
} // namespace End
} // namespace Commands

namespace Events {} // namespace Events

} // namespace WebRTCTransportRequestor
} // namespace clusters
} // namespace app
} // namespace chip
