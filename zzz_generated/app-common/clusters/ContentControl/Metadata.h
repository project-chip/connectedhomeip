// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Enabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Enabled
namespace OnDemandRatings {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OnDemandRatings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OnDemandRatings
namespace OnDemandRatingThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OnDemandRatingThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OnDemandRatingThreshold
namespace ScheduledContentRatings {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ScheduledContentRatings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScheduledContentRatings
namespace ScheduledContentRatingThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ScheduledContentRatingThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScheduledContentRatingThreshold
namespace ScreenDailyTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ScreenDailyTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScreenDailyTime
namespace RemainingScreenTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RemainingScreenTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RemainingScreenTime
namespace BlockUnrated {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BlockUnrated::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BlockUnrated

} // namespace Attributes

namespace Commands {
namespace UpdatePIN {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdatePIN::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UpdatePIN
namespace ResetPIN {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ResetPIN::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ResetPIN
namespace Enable {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Enable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Enable
namespace Disable {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Disable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Disable
namespace AddBonusTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddBonusTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace AddBonusTime
namespace SetScreenDailyTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetScreenDailyTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetScreenDailyTime
namespace BlockUnratedContent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = BlockUnratedContent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace BlockUnratedContent
namespace UnblockUnratedContent {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UnblockUnratedContent::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UnblockUnratedContent
namespace SetOnDemandRatingThreshold {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetOnDemandRatingThreshold::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetOnDemandRatingThreshold
namespace SetScheduledContentRatingThreshold {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetScheduledContentRatingThreshold::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetScheduledContentRatingThreshold

} // namespace Commands
} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
