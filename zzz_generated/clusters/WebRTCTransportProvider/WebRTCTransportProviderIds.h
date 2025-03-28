// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WebRTCTransportProvider (cluster code: 1363/0x553)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace WebRTCTransportProvider {

inline constexpr ClusterId kClusterId = 0x00000553;

namespace Attributes {
namespace CurrentSessions {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentSessions

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SolicitOffer {
inline constexpr CommandId Id = 0x00000001;
} // namespace SolicitOffer
namespace ProvideOffer {
inline constexpr CommandId Id = 0x00000003;
} // namespace ProvideOffer
namespace ProvideAnswer {
inline constexpr CommandId Id = 0x00000005;
} // namespace ProvideAnswer
namespace ProvideICECandidates {
inline constexpr CommandId Id = 0x00000006;
} // namespace ProvideICECandidates
namespace EndSession {
inline constexpr CommandId Id = 0x00000007;
} // namespace EndSession
} // namespace Commands

namespace Events {} // namespace Events
} // namespace WebRTCTransportProvider
} // namespace clusters
} // namespace app
} // namespace chip
