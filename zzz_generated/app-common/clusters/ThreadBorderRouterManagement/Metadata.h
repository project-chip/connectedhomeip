// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterManagement (cluster code: 1106/0x452)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadBorderRouterManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace BorderRouterName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BorderRouterName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BorderRouterName
namespace BorderAgentID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BorderAgentID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BorderAgentID
namespace ThreadVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadVersion
namespace InterfaceEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InterfaceEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InterfaceEnabled
namespace ActiveDatasetTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveDatasetTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveDatasetTimestamp
namespace PendingDatasetTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PendingDatasetTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PendingDatasetTimestamp

} // namespace Attributes

namespace Commands {
namespace GetActiveDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetActiveDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace GetActiveDatasetRequest
namespace GetPendingDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetPendingDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace GetPendingDatasetRequest
namespace SetActiveDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetActiveDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetActiveDatasetRequest
namespace SetPendingDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetPendingDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetPendingDatasetRequest

} // namespace Commands
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
