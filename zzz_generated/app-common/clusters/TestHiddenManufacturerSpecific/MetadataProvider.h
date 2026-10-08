// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TestHiddenManufacturerSpecific (cluster code: 4294048801/0xFFF1FC21)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TestHiddenManufacturerSpecific/Ids.h>
#include <clusters/TestHiddenManufacturerSpecific/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TestHiddenManufacturerSpecific::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TestHiddenManufacturerSpecific::Attributes;
        switch (attributeId)
        {
        case TestAttribute::Id:
            return TestAttribute::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TestHiddenManufacturerSpecific::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TestHiddenManufacturerSpecific::Commands;
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
