// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace OtaSoftwareUpdateRequestor {

inline constexpr ClusterId kClusterId = 0x0000002A;

namespace Attributes {
namespace DefaultOTAProviders {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DefaultOTAProviders
namespace UpdatePossible {
inline constexpr AttributeId Id = 0x00000001;
} // namespace UpdatePossible
namespace UpdateState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpdateState
namespace UpdateStateProgress {
inline constexpr AttributeId Id = 0x00000003;
} // namespace UpdateStateProgress

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace AnnounceOTAProvider {
inline constexpr CommandId Id = 0x00000000;
} // namespace AnnounceOTAProvider
} // namespace Commands

namespace Events {
namespace StateTransition {
inline constexpr EventId Id = 0x00000000;
} // namespace StateTransition
namespace VersionApplied {
inline constexpr EventId Id = 0x00000001;
} // namespace VersionApplied
namespace DownloadError {
inline constexpr EventId Id = 0x00000002;
} // namespace DownloadError
} // namespace Events

} // namespace OtaSoftwareUpdateRequestor
} // namespace clusters
} // namespace app
} // namespace chip
