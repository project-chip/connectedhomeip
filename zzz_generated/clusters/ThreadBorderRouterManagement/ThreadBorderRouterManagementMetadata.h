// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterManagement (cluster code: 1106/0x452)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadBorderRouterManagement/ThreadBorderRouterManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ThreadBorderRouterManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kBorderRouterNameEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::BorderRouterName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBorderAgentIDEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::BorderAgentID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kThreadVersionEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::ThreadVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInterfaceEnabledEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveDatasetTimestampEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPendingDatasetTimestampEntry = {
    .attributeId    = ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kGetActiveDatasetRequestEntry = {
    .commandId       = ThreadBorderRouterManagement::Commands::GetActiveDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kGetPendingDatasetRequestEntry = {
    .commandId       = ThreadBorderRouterManagement::Commands::GetPendingDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetActiveDatasetRequestEntry = {
    .commandId       = ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetPendingDatasetRequestEntry = {
    .commandId       = ThreadBorderRouterManagement::Commands::SetPendingDatasetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace ThreadBorderRouterManagement
} // namespace clusters
} // namespace app
} // namespace chip
