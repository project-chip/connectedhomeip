// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaFileManagement (cluster code: 1297/0x511)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 5;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace AddFile {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddFile

namespace DeleteFile {
inline constexpr CommandId Id = 0x00000002;
} // namespace DeleteFile

namespace RequestSharedFiles {
inline constexpr CommandId Id = 0x00000003;
} // namespace RequestSharedFiles

namespace GetSharedFile {
inline constexpr CommandId Id = 0x00000004;
} // namespace GetSharedFile

namespace OfferFile {
inline constexpr CommandId Id = 0x00000006;
} // namespace OfferFile

namespace AddFileResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddFileResponse

namespace GetSharedFileResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace GetSharedFileResponse

} // namespace Commands
} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
