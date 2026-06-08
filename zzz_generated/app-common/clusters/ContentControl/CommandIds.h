// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 16;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace UpdatePIN {
inline constexpr CommandId Id = 0x00000000;
} // namespace UpdatePIN

namespace ResetPIN {
inline constexpr CommandId Id = 0x00000001;
} // namespace ResetPIN

namespace Enable {
inline constexpr CommandId Id = 0x00000003;
} // namespace Enable

namespace Disable {
inline constexpr CommandId Id = 0x00000004;
} // namespace Disable

namespace AddBonusTime {
inline constexpr CommandId Id = 0x00000005;
} // namespace AddBonusTime

namespace SetScreenDailyTime {
inline constexpr CommandId Id = 0x00000006;
} // namespace SetScreenDailyTime

namespace BlockUnratedContent {
inline constexpr CommandId Id = 0x00000007;
} // namespace BlockUnratedContent

namespace UnblockUnratedContent {
inline constexpr CommandId Id = 0x00000008;
} // namespace UnblockUnratedContent

namespace SetOnDemandRatingThreshold {
inline constexpr CommandId Id = 0x00000009;
} // namespace SetOnDemandRatingThreshold

namespace SetScheduledContentRatingThreshold {
inline constexpr CommandId Id = 0x0000000A;
} // namespace SetScheduledContentRatingThreshold

namespace AddBlockChannels {
inline constexpr CommandId Id = 0x0000000B;
} // namespace AddBlockChannels

namespace RemoveBlockChannels {
inline constexpr CommandId Id = 0x0000000C;
} // namespace RemoveBlockChannels

namespace AddBlockApplications {
inline constexpr CommandId Id = 0x0000000D;
} // namespace AddBlockApplications

namespace RemoveBlockApplications {
inline constexpr CommandId Id = 0x0000000E;
} // namespace RemoveBlockApplications

namespace SetBlockContentTimeWindow {
inline constexpr CommandId Id = 0x0000000F;
} // namespace SetBlockContentTimeWindow

namespace RemoveBlockContentTimeWindow {
inline constexpr CommandId Id = 0x00000010;
} // namespace RemoveBlockContentTimeWindow

namespace ResetPINResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace ResetPINResponse

} // namespace Commands
} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
