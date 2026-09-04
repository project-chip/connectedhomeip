// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DynamicLighting (cluster code: 773/0x305)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DynamicLighting/Ids.h>
#include <clusters/DynamicLighting/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DynamicLighting::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::DynamicLighting::Attributes;
        switch (attributeId)
        {
        case AvailableEffects::Id:
            return AvailableEffects::kMetadataEntry;
        case CurrentEffectID::Id:
            return CurrentEffectID::kMetadataEntry;
        case CurrentSpeed::Id:
            return CurrentSpeed::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DynamicLighting::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::DynamicLighting::Commands;
        switch (commandId)
        {
        case StartEffect::Id:
            return StartEffect::kMetadataEntry;
        case StopEffect::Id:
            return StopEffect::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
