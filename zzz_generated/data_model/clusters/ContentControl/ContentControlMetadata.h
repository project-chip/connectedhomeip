// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentControl/ContentControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ContentControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kEnabledEntry = {
    .attributeId    = ContentControl::Attributes::Enabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOnDemandRatingsEntry = {
    .attributeId    = ContentControl::Attributes::OnDemandRatings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOnDemandRatingThresholdEntry = {
    .attributeId    = ContentControl::Attributes::OnDemandRatingThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kScheduledContentRatingsEntry = {
    .attributeId    = ContentControl::Attributes::ScheduledContentRatings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kScheduledContentRatingThresholdEntry = {
    .attributeId    = ContentControl::Attributes::ScheduledContentRatingThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kScreenDailyTimeEntry = {
    .attributeId    = ContentControl::Attributes::ScreenDailyTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRemainingScreenTimeEntry = {
    .attributeId    = ContentControl::Attributes::RemainingScreenTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBlockUnratedEntry = {
    .attributeId    = ContentControl::Attributes::BlockUnrated::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kUpdatePINEntry = {
    .commandId       = ContentControl::Commands::UpdatePIN::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kResetPINEntry = {
    .commandId       = ContentControl::Commands::ResetPIN::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kEnableEntry = {
    .commandId       = ContentControl::Commands::Enable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kDisableEntry = {
    .commandId       = ContentControl::Commands::Disable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAddBonusTimeEntry = {
    .commandId       = ContentControl::Commands::AddBonusTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetScreenDailyTimeEntry = {
    .commandId       = ContentControl::Commands::SetScreenDailyTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kBlockUnratedContentEntry = {
    .commandId       = ContentControl::Commands::BlockUnratedContent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kUnblockUnratedContentEntry = {
    .commandId       = ContentControl::Commands::UnblockUnratedContent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetOnDemandRatingThresholdEntry = {
    .commandId       = ContentControl::Commands::SetOnDemandRatingThreshold::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetScheduledContentRatingThresholdEntry = {
    .commandId       = ContentControl::Commands::SetScheduledContentRatingThreshold::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ContentControl
} // namespace clusters
} // namespace app
} // namespace chip
