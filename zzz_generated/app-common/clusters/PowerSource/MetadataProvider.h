// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerSource (cluster code: 47/0x2F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/PowerSource/Ids.h>
#include <clusters/PowerSource/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::PowerSource::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::PowerSource::Attributes;
        switch (attributeId)
        {
        case Status::Id:
            return Status::kMetadataEntry;
        case Order::Id:
            return Order::kMetadataEntry;
        case Description::Id:
            return Description::kMetadataEntry;
        case WiredAssessedInputVoltage::Id:
            return WiredAssessedInputVoltage::kMetadataEntry;
        case WiredAssessedInputFrequency::Id:
            return WiredAssessedInputFrequency::kMetadataEntry;
        case WiredCurrentType::Id:
            return WiredCurrentType::kMetadataEntry;
        case WiredAssessedCurrent::Id:
            return WiredAssessedCurrent::kMetadataEntry;
        case WiredNominalVoltage::Id:
            return WiredNominalVoltage::kMetadataEntry;
        case WiredMaximumCurrent::Id:
            return WiredMaximumCurrent::kMetadataEntry;
        case WiredPresent::Id:
            return WiredPresent::kMetadataEntry;
        case ActiveWiredFaults::Id:
            return ActiveWiredFaults::kMetadataEntry;
        case BatVoltage::Id:
            return BatVoltage::kMetadataEntry;
        case BatPercentRemaining::Id:
            return BatPercentRemaining::kMetadataEntry;
        case BatTimeRemaining::Id:
            return BatTimeRemaining::kMetadataEntry;
        case BatChargeLevel::Id:
            return BatChargeLevel::kMetadataEntry;
        case BatReplacementNeeded::Id:
            return BatReplacementNeeded::kMetadataEntry;
        case BatReplaceability::Id:
            return BatReplaceability::kMetadataEntry;
        case BatPresent::Id:
            return BatPresent::kMetadataEntry;
        case ActiveBatFaults::Id:
            return ActiveBatFaults::kMetadataEntry;
        case BatReplacementDescription::Id:
            return BatReplacementDescription::kMetadataEntry;
        case BatCommonDesignation::Id:
            return BatCommonDesignation::kMetadataEntry;
        case BatANSIDesignation::Id:
            return BatANSIDesignation::kMetadataEntry;
        case BatIECDesignation::Id:
            return BatIECDesignation::kMetadataEntry;
        case BatApprovedChemistry::Id:
            return BatApprovedChemistry::kMetadataEntry;
        case BatCapacity::Id:
            return BatCapacity::kMetadataEntry;
        case BatQuantity::Id:
            return BatQuantity::kMetadataEntry;
        case BatChargeState::Id:
            return BatChargeState::kMetadataEntry;
        case BatTimeToFullCharge::Id:
            return BatTimeToFullCharge::kMetadataEntry;
        case BatFunctionalWhileCharging::Id:
            return BatFunctionalWhileCharging::kMetadataEntry;
        case BatChargingCurrent::Id:
            return BatChargingCurrent::kMetadataEntry;
        case ActiveBatChargeFaults::Id:
            return ActiveBatChargeFaults::kMetadataEntry;
        case EndpointList::Id:
            return EndpointList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PowerSource::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::PowerSource::Commands;
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
