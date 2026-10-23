// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OtaSoftwareUpdateProvider (cluster code: 41/0x29)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OtaSoftwareUpdateProvider/Ids.h>
#include <clusters/OtaSoftwareUpdateProvider/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OtaSoftwareUpdateProvider::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::OtaSoftwareUpdateProvider::Attributes;
        switch (attributeId)
        {
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OtaSoftwareUpdateProvider::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::OtaSoftwareUpdateProvider::Commands;
        switch (commandId)
        {
        case QueryImage::Id:
            return QueryImage::kMetadataEntry;
        case ApplyUpdateRequest::Id:
            return ApplyUpdateRequest::kMetadataEntry;
        case NotifyUpdateApplied::Id:
            return NotifyUpdateApplied::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
