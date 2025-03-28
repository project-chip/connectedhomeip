// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ContentControl {

inline constexpr ClusterId kClusterId = 0x0000050F;

namespace Attributes {
namespace Enabled {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Enabled
namespace OnDemandRatings {
inline constexpr AttributeId Id = 0x00000001;
} // namespace OnDemandRatings
namespace OnDemandRatingThreshold {
inline constexpr AttributeId Id = 0x00000002;
} // namespace OnDemandRatingThreshold
namespace ScheduledContentRatings {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ScheduledContentRatings
namespace ScheduledContentRatingThreshold {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ScheduledContentRatingThreshold
namespace ScreenDailyTime {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ScreenDailyTime
namespace RemainingScreenTime {
inline constexpr AttributeId Id = 0x00000006;
} // namespace RemainingScreenTime
namespace BlockUnrated {
inline constexpr AttributeId Id = 0x00000007;
} // namespace BlockUnrated

// TODO: globals & reference globals?

} // namespace Attributes

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
} // namespace Commands

namespace Events {
namespace RemainingScreenTimeExpired {
inline constexpr CommandId Id = 0x00000000;
} // namespace RemainingScreenTimeExpired
} // namespace Events
} // namespace ContentControl
} // namespace clusters
} // namespace app
} // namespace chip
