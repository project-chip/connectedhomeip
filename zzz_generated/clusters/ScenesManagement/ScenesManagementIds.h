// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ScenesManagement {

inline constexpr ClusterId kClusterId = 0x00000062;

namespace Attributes {
namespace SceneTableSize {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SceneTableSize
namespace FabricSceneInfo {
inline constexpr AttributeId Id = 0x00000002;
} // namespace FabricSceneInfo

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace AddScene {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddScene
namespace ViewScene {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewScene
namespace RemoveScene {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveScene
namespace RemoveAllScenes {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveAllScenes
namespace StoreScene {
inline constexpr CommandId Id = 0x00000004;
} // namespace StoreScene
namespace RecallScene {
inline constexpr CommandId Id = 0x00000005;
} // namespace RecallScene
namespace GetSceneMembership {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetSceneMembership
namespace CopyScene {
inline constexpr CommandId Id = 0x00000040;
} // namespace CopyScene
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ScenesManagement
} // namespace clusters
} // namespace app
} // namespace chip
