// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationBasic (cluster code: 1293/0x50D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ApplicationBasic/Ids.h>
#include <clusters/ApplicationBasic/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ApplicationBasic::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ApplicationBasic::Attributes;
        switch (commandId)
        {
        case VendorName::Id:
            return VendorName::kMetadataEntry;
        case VendorID::Id:
            return VendorID::kMetadataEntry;
        case ApplicationName::Id:
            return ApplicationName::kMetadataEntry;
        case ProductID::Id:
            return ProductID::kMetadataEntry;
        case Application::Id:
            return Application::kMetadataEntry;
        case Status::Id:
            return Status::kMetadataEntry;
        case ApplicationVersion::Id:
            return ApplicationVersion::kMetadataEntry;
        case AllowedVendorList::Id:
            return AllowedVendorList::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ApplicationBasic::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ApplicationBasic::Commands;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
