// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BasicInformation (cluster code: 40/0x28)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/BasicInformation/Ids.h>
#include <clusters/BasicInformation/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::BasicInformation::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::BasicInformation::Attributes;
        switch (attributeId)
        {
        case DataModelRevision::Id:
            return DataModelRevision::kMetadataEntry;
        case VendorName::Id:
            return VendorName::kMetadataEntry;
        case VendorID::Id:
            return VendorID::kMetadataEntry;
        case ProductName::Id:
            return ProductName::kMetadataEntry;
        case ProductID::Id:
            return ProductID::kMetadataEntry;
        case NodeLabel::Id:
            return NodeLabel::kMetadataEntry;
        case Location::Id:
            return Location::kMetadataEntry;
        case HardwareVersion::Id:
            return HardwareVersion::kMetadataEntry;
        case HardwareVersionString::Id:
            return HardwareVersionString::kMetadataEntry;
        case SoftwareVersion::Id:
            return SoftwareVersion::kMetadataEntry;
        case SoftwareVersionString::Id:
            return SoftwareVersionString::kMetadataEntry;
        case ManufacturingDate::Id:
            return ManufacturingDate::kMetadataEntry;
        case PartNumber::Id:
            return PartNumber::kMetadataEntry;
        case ProductURL::Id:
            return ProductURL::kMetadataEntry;
        case ProductLabel::Id:
            return ProductLabel::kMetadataEntry;
        case SerialNumber::Id:
            return SerialNumber::kMetadataEntry;
        case LocalConfigDisabled::Id:
            return LocalConfigDisabled::kMetadataEntry;
        case Reachable::Id:
            return Reachable::kMetadataEntry;
        case UniqueID::Id:
            return UniqueID::kMetadataEntry;
        case CapabilityMinima::Id:
            return CapabilityMinima::kMetadataEntry;
        case ProductAppearance::Id:
            return ProductAppearance::kMetadataEntry;
        case SpecificationVersion::Id:
            return SpecificationVersion::kMetadataEntry;
        case MaxPathsPerInvoke::Id:
            return MaxPathsPerInvoke::kMetadataEntry;
        case ConfigurationVersion::Id:
            return ConfigurationVersion::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::BasicInformation::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::BasicInformation::Commands;
        switch (commandId)
        {
        case MfgSpecificPing::Id:
            return MfgSpecificPing::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
