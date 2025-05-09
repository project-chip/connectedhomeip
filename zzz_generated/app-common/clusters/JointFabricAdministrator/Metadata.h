// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster JointFabricAdministrator (cluster code: 1875/0x753)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/JointFabricAdministrator/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace JointFabricAdministrator {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace AdministratorFabricIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AdministratorFabricIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AdministratorFabricIndex

} // namespace Attributes

namespace Commands {
namespace ICACCSRRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ICACCSRRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ICACCSRRequest
namespace AddICAC {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddICAC::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddICAC
namespace OpenJointCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = OpenJointCommissioningWindow::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace OpenJointCommissioningWindow
namespace TransferAnchorRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = TransferAnchorRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace TransferAnchorRequest
namespace TransferAnchorComplete {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = TransferAnchorComplete::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace TransferAnchorComplete
namespace AnnounceJointFabricAdministrator {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AnnounceJointFabricAdministrator::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AnnounceJointFabricAdministrator

} // namespace Commands
} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
