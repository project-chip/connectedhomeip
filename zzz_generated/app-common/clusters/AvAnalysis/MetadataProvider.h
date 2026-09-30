// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AvAnalysis (cluster code: 1367/0x557)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AvAnalysis/Ids.h>
#include <clusters/AvAnalysis/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AvAnalysis::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AvAnalysis::Attributes;
        switch (attributeId)
        {
        case SupportedAmbientContexts::Id:
            return SupportedAmbientContexts::kMetadataEntry;
        case ActiveAmbientContextTriggers::Id:
            return ActiveAmbientContextTriggers::kMetadataEntry;
        case MaxAnalysisStreamCount::Id:
            return MaxAnalysisStreamCount::kMetadataEntry;
        case CurrentAnalysisStreamCount::Id:
            return CurrentAnalysisStreamCount::kMetadataEntry;
        case AnalysisStreams::Id:
            return AnalysisStreams::kMetadataEntry;
        case TrackingEnabled::Id:
            return TrackingEnabled::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AvAnalysis::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AvAnalysis::Commands;
        switch (commandId)
        {
        case EnableContextTriggers::Id:
            return EnableContextTriggers::kMetadataEntry;
        case DisableContextTriggers::Id:
            return DisableContextTriggers::kMetadataEntry;
        case EstablishAnalysisStream::Id:
            return EstablishAnalysisStream::kMetadataEntry;
        case ActivateAnalysisStream::Id:
            return ActivateAnalysisStream::kMetadataEntry;
        case DeactivateAnalysisStream::Id:
            return DeactivateAnalysisStream::kMetadataEntry;
        case RemoveAnalysisStream::Id:
            return RemoveAnalysisStream::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
