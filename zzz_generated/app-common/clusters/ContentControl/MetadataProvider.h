// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentControl (cluster code: 1295/0x50F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ContentControl/Ids.h>
#include <clusters/ContentControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ContentControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ContentControl::Attributes;
        switch (attributeId)
        {
        case Enabled::Id:
            return Enabled::kMetadataEntry;
        case OnDemandRatings::Id:
            return OnDemandRatings::kMetadataEntry;
        case OnDemandRatingThreshold::Id:
            return OnDemandRatingThreshold::kMetadataEntry;
        case ScheduledContentRatings::Id:
            return ScheduledContentRatings::kMetadataEntry;
        case ScheduledContentRatingThreshold::Id:
            return ScheduledContentRatingThreshold::kMetadataEntry;
        case ScreenDailyTime::Id:
            return ScreenDailyTime::kMetadataEntry;
        case RemainingScreenTime::Id:
            return RemainingScreenTime::kMetadataEntry;
        case BlockUnrated::Id:
            return BlockUnrated::kMetadataEntry;
        case BlockChannelList::Id:
            return BlockChannelList::kMetadataEntry;
        case BlockApplicationList::Id:
            return BlockApplicationList::kMetadataEntry;
        case BlockContentTimeWindow::Id:
            return BlockContentTimeWindow::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ContentControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ContentControl::Commands;
        switch (commandId)
        {
        case UpdatePIN::Id:
            return UpdatePIN::kMetadataEntry;
        case ResetPIN::Id:
            return ResetPIN::kMetadataEntry;
        case Enable::Id:
            return Enable::kMetadataEntry;
        case Disable::Id:
            return Disable::kMetadataEntry;
        case AddBonusTime::Id:
            return AddBonusTime::kMetadataEntry;
        case SetScreenDailyTime::Id:
            return SetScreenDailyTime::kMetadataEntry;
        case BlockUnratedContent::Id:
            return BlockUnratedContent::kMetadataEntry;
        case UnblockUnratedContent::Id:
            return UnblockUnratedContent::kMetadataEntry;
        case SetOnDemandRatingThreshold::Id:
            return SetOnDemandRatingThreshold::kMetadataEntry;
        case SetScheduledContentRatingThreshold::Id:
            return SetScheduledContentRatingThreshold::kMetadataEntry;
        case AddBlockChannels::Id:
            return AddBlockChannels::kMetadataEntry;
        case RemoveBlockChannels::Id:
            return RemoveBlockChannels::kMetadataEntry;
        case AddBlockApplications::Id:
            return AddBlockApplications::kMetadataEntry;
        case RemoveBlockApplications::Id:
            return RemoveBlockApplications::kMetadataEntry;
        case SetBlockContentTimeWindow::Id:
            return SetBlockContentTimeWindow::kMetadataEntry;
        case RemoveBlockContentTimeWindow::Id:
            return RemoveBlockContentTimeWindow::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
