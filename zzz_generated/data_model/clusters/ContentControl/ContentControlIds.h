// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentControl {

inline constexpr ClusterId Id = 0x0000050F;

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
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList
namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap
namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

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
namespace ResetPINResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace ResetPINResponse
} // namespace Commands

namespace Events {
namespace RemainingScreenTimeExpired {
inline constexpr EventId Id = 0x00000000;
} // namespace RemainingScreenTimeExpired
} // namespace Events

} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
