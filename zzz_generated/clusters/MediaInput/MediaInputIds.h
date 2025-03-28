// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaInput (cluster code: 1287/0x507)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace MediaInput {

inline constexpr ClusterId kClusterId = 0x00000507;

namespace Attributes {
namespace InputList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace InputList
namespace CurrentInput {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentInput

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SelectInput {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectInput
namespace ShowInputStatus {
inline constexpr CommandId Id = 0x00000001;
} // namespace ShowInputStatus
namespace HideInputStatus {
inline constexpr CommandId Id = 0x00000002;
} // namespace HideInputStatus
namespace RenameInput {
inline constexpr CommandId Id = 0x00000003;
} // namespace RenameInput
} // namespace Commands

namespace Events {} // namespace Events
} // namespace MediaInput
} // namespace clusters
} // namespace app
} // namespace chip
