// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaFileManagement (cluster code: 1297/0x511)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/MediaFileManagement/Ids.h>
#include <clusters/MediaFileManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::MediaFileManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::MediaFileManagement::Attributes;
        switch (attributeId)
        {
        case TotalStorage::Id:
            return TotalStorage::kMetadataEntry;
        case AvailableStorage::Id:
            return AvailableStorage::kMetadataEntry;
        case AvailableFiles::Id:
            return AvailableFiles::kMetadataEntry;
        case SupportedMimeTypes::Id:
            return SupportedMimeTypes::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::MediaFileManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::MediaFileManagement::Commands;
        switch (commandId)
        {
        case AddFile::Id:
            return AddFile::kMetadataEntry;
        case DeleteFile::Id:
            return DeleteFile::kMetadataEntry;
        case RequestSharedFiles::Id:
            return RequestSharedFiles::kMetadataEntry;
        case GetSharedFile::Id:
            return GetSharedFile::kMetadataEntry;
        case OfferFile::Id:
            return OfferFile::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
