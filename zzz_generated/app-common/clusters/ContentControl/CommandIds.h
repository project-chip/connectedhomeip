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

namespace ResetPINResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace ResetPINResponse

} // namespace Commands
} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
