// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SoftwareDiagnostics (cluster code: 52/0x34)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/SoftwareDiagnostics/Ids.h>
#include <clusters/SoftwareDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::SoftwareDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::SoftwareDiagnostics::Attributes;
        switch (attributeId)
        {
        case ThreadMetrics::Id:
            return ThreadMetrics::kMetadataEntry;
        case CurrentHeapFree::Id:
            return CurrentHeapFree::kMetadataEntry;
        case CurrentHeapUsed::Id:
            return CurrentHeapUsed::kMetadataEntry;
        case CurrentHeapHighWatermark::Id:
            return CurrentHeapHighWatermark::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::SoftwareDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::SoftwareDiagnostics::Commands;
        switch (commandId)
        {
        case ResetWatermarks::Id:
            return ResetWatermarks::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
