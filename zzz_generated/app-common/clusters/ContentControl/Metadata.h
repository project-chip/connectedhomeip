// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Enabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Enabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Enabled
namespace OnDemandRatings {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(OnDemandRatings::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace OnDemandRatings
namespace OnDemandRatingThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnDemandRatingThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OnDemandRatingThreshold
namespace ScheduledContentRatings {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ScheduledContentRatings::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ScheduledContentRatings
namespace ScheduledContentRatingThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScheduledContentRatingThreshold::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ScheduledContentRatingThreshold
namespace ScreenDailyTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScreenDailyTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ScreenDailyTime
namespace RemainingScreenTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RemainingScreenTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RemainingScreenTime
namespace BlockUnrated {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BlockUnrated::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BlockUnrated
namespace BlockChannelList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(BlockChannelList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace BlockChannelList
namespace BlockApplicationList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(BlockApplicationList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace BlockApplicationList
namespace BlockContentTimeWindow {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(BlockContentTimeWindow::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace BlockContentTimeWindow
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    Enabled::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace UpdatePIN {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdatePIN::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace UpdatePIN
namespace ResetPIN {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ResetPIN::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace ResetPIN
namespace Enable {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(Enable::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace Enable
namespace Disable {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(Disable::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace Disable
namespace AddBonusTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddBonusTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddBonusTime
namespace SetScreenDailyTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetScreenDailyTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace SetScreenDailyTime
namespace BlockUnratedContent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(BlockUnratedContent::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace BlockUnratedContent
namespace UnblockUnratedContent {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UnblockUnratedContent::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace UnblockUnratedContent
namespace SetOnDemandRatingThreshold {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetOnDemandRatingThreshold::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SetOnDemandRatingThreshold
namespace SetScheduledContentRatingThreshold {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetScheduledContentRatingThreshold::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SetScheduledContentRatingThreshold
namespace AddBlockChannels {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddBlockChannels::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace AddBlockChannels
namespace RemoveBlockChannels {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveBlockChannels::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace RemoveBlockChannels
namespace AddBlockApplications {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddBlockApplications::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace AddBlockApplications
namespace RemoveBlockApplications {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveBlockApplications::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace RemoveBlockApplications
namespace SetBlockContentTimeWindow {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetBlockContentTimeWindow::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SetBlockContentTimeWindow
namespace RemoveBlockContentTimeWindow {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveBlockContentTimeWindow::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace RemoveBlockContentTimeWindow

} // namespace Commands

namespace Events {
namespace RemainingScreenTimeExpired {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace RemainingScreenTimeExpired
namespace EnteringBlockContentTimeWindow {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace EnteringBlockContentTimeWindow

} // namespace Events
} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
