// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LocalizationConfiguration (cluster code: 43/0x2B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LocalizationConfiguration/Ids.h>
#include <clusters/LocalizationConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LocalizationConfiguration::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::LocalizationConfiguration::Attributes;
        switch (commandId)
        {
        case ActiveLocale::Id:
            return ActiveLocale::kMetadataEntry;
        case SupportedLocales::Id:
            return SupportedLocales::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LocalizationConfiguration::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::LocalizationConfiguration::Commands;
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
