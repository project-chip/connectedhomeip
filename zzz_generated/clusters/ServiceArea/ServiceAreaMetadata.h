// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ServiceArea (cluster code: 336/0x150)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ServiceArea {

inline constexpr ClusterId kClusterId = 0x00000150;

namespace Attributes {
namespace SupportedAreas {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedAreas
namespace SupportedMaps {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedMaps
namespace SelectedAreas {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SelectedAreas
namespace CurrentArea {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentArea
namespace EstimatedEndTime {
inline constexpr AttributeId Id = 0x00000004;
} // namespace EstimatedEndTime
namespace Progress {
inline constexpr AttributeId Id = 0x00000005;
} // namespace Progress

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SelectAreas {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectAreas
namespace SkipArea {
inline constexpr CommandId Id = 0x00000002;
} // namespace SkipArea
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ServiceArea
} // namespace clusters
} // namespace app
} // namespace chip
