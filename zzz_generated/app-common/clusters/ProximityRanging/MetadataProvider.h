// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ProximityRanging (cluster code: 1075/0x433)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ProximityRanging/Ids.h>
#include <clusters/ProximityRanging/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ProximityRanging::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ProximityRanging::Attributes;
        switch (attributeId)
        {
        case RangingCapabilities::Id:
            return RangingCapabilities::kMetadataEntry;
        case WiFiDevIK::Id:
            return WiFiDevIK::kMetadataEntry;
        case BLEDeviceID::Id:
            return BLEDeviceID::kMetadataEntry;
        case BLTDevIK::Id:
            return BLTDevIK::kMetadataEntry;
        case BLTCSSecurityLevel::Id:
            return BLTCSSecurityLevel::kMetadataEntry;
        case BLTCSModeCapability::Id:
            return BLTCSModeCapability::kMetadataEntry;
        case SessionIDList::Id:
            return SessionIDList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ProximityRanging::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ProximityRanging::Commands;
        switch (commandId)
        {
        case StartRangingRequest::Id:
            return StartRangingRequest::kMetadataEntry;
        case StopRangingRequest::Id:
            return StopRangingRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
