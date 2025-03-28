// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace GroupKeyManagement {

inline constexpr ClusterId kClusterId = 0x0000003F;

namespace Attributes {
namespace GroupKeyMap {
inline constexpr AttributeId Id = 0x00000000;
} // namespace GroupKeyMap
namespace GroupTable {
inline constexpr AttributeId Id = 0x00000001;
} // namespace GroupTable
namespace MaxGroupsPerFabric {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxGroupsPerFabric
namespace MaxGroupKeysPerFabric {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MaxGroupKeysPerFabric

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace KeySetWrite {
inline constexpr CommandId Id = 0x00000000;
} // namespace KeySetWrite
namespace KeySetRead {
inline constexpr CommandId Id = 0x00000001;
} // namespace KeySetRead
namespace KeySetRemove {
inline constexpr CommandId Id = 0x00000003;
} // namespace KeySetRemove
namespace KeySetReadAllIndices {
inline constexpr CommandId Id = 0x00000004;
} // namespace KeySetReadAllIndices
} // namespace Commands

namespace Events {} // namespace Events

} // namespace GroupKeyManagement
} // namespace clusters
} // namespace app
} // namespace chip
