// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DeviceEnergyManagement/Ids.h>
#include <clusters/DeviceEnergyManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DeviceEnergyManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::DeviceEnergyManagement::Attributes;
        switch (attributeId)
        {
        case ESAType::Id:
            return ESAType::kMetadataEntry;
        case ESACanGenerate::Id:
            return ESACanGenerate::kMetadataEntry;
        case ESAState::Id:
            return ESAState::kMetadataEntry;
        case AbsMinPower::Id:
            return AbsMinPower::kMetadataEntry;
        case AbsMaxPower::Id:
            return AbsMaxPower::kMetadataEntry;
        case PowerAdjustmentCapability::Id:
            return PowerAdjustmentCapability::kMetadataEntry;
        case Forecast::Id:
            return Forecast::kMetadataEntry;
        case OptOutState::Id:
            return OptOutState::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DeviceEnergyManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::DeviceEnergyManagement::Commands;
        switch (commandId)
        {
        case PowerAdjustRequest::Id:
            return PowerAdjustRequest::kMetadataEntry;
        case CancelPowerAdjustRequest::Id:
            return CancelPowerAdjustRequest::kMetadataEntry;
        case StartTimeAdjustRequest::Id:
            return StartTimeAdjustRequest::kMetadataEntry;
        case PauseRequest::Id:
            return PauseRequest::kMetadataEntry;
        case ResumeRequest::Id:
            return ResumeRequest::kMetadataEntry;
        case ModifyForecastRequest::Id:
            return ModifyForecastRequest::kMetadataEntry;
        case RequestConstraintBasedForecast::Id:
            return RequestConstraintBasedForecast::kMetadataEntry;
        case CancelRequest::Id:
            return CancelRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
