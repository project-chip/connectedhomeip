// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace AccessControl {

inline constexpr ClusterId kClusterId = 0x0000001F;

namespace Attributes {
namespace Acl {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Acl
namespace Extension {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Extension
namespace SubjectsPerAccessControlEntry {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SubjectsPerAccessControlEntry
namespace TargetsPerAccessControlEntry {
inline constexpr AttributeId Id = 0x00000003;
} // namespace TargetsPerAccessControlEntry
namespace AccessControlEntriesPerFabric {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AccessControlEntriesPerFabric
namespace CommissioningARL {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CommissioningARL
namespace Arl {
inline constexpr AttributeId Id = 0x00000006;
} // namespace Arl

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ReviewFabricRestrictions {
inline constexpr CommandId Id = 0x00000000;
} // namespace ReviewFabricRestrictions
} // namespace Commands

namespace Events {
namespace AccessControlEntryChanged {
inline constexpr EventId Id = 0x00000000;
} // namespace AccessControlEntryChanged
namespace AccessControlExtensionChanged {
inline constexpr EventId Id = 0x00000001;
} // namespace AccessControlExtensionChanged
namespace FabricRestrictionReviewUpdate {
inline constexpr EventId Id = 0x00000002;
} // namespace FabricRestrictionReviewUpdate
} // namespace Events
} // namespace AccessControl
} // namespace clusters
} // namespace app
} // namespace chip
