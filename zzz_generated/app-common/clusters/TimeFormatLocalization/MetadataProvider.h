// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeFormatLocalization (cluster code: 44/0x2C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TimeFormatLocalization/Ids.h>
#include <clusters/TimeFormatLocalization/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TimeFormatLocalization::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TimeFormatLocalization::Attributes;
        switch (attributeId)
        {
        case HourFormat::Id:
            return HourFormat::kMetadataEntry;
        case ActiveCalendarType::Id:
            return ActiveCalendarType::kMetadataEntry;
        case SupportedCalendarTypes::Id:
            return SupportedCalendarTypes::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TimeFormatLocalization::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TimeFormatLocalization::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
