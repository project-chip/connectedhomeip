// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ZoneManagement (cluster code: 1360/0x550)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ZoneManagement {

inline constexpr ClusterId kClusterId = 0x00000550;

namespace Attributes {
namespace SupportedZoneSources {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedZoneSources
namespace Zones {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Zones
namespace Triggers {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Triggers
namespace Sensitivity {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Sensitivity

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace CreateTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000000;
} // namespace CreateTwoDCartesianZone
namespace UpdateTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000002;
} // namespace UpdateTwoDCartesianZone
namespace GetTwoDCartesianZone {
inline constexpr CommandId Id = 0x00000003;
} // namespace GetTwoDCartesianZone
namespace RemoveZone {
inline constexpr CommandId Id = 0x00000005;
} // namespace RemoveZone
} // namespace Commands

namespace Events {
namespace ZoneTriggered {
inline constexpr EventId Id = 0x00000000;
} // namespace ZoneTriggered
namespace ZoneStopped {
inline constexpr EventId Id = 0x00000001;
} // namespace ZoneStopped
} // namespace Events
} // namespace ZoneManagement
} // namespace clusters
} // namespace app
} // namespace chip
