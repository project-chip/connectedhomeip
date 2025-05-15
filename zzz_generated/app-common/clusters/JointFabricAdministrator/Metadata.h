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
inline constexpr DataModel::AttributeEntry kMetadataEntry(AdministratorFabricIndex::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AdministratorFabricIndex

} // namespace Attributes

namespace Commands {
namespace ICACCSRRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ICACCSRRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ICACCSRRequest
namespace AddICAC {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddICAC::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddICAC
namespace OpenJointCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(OpenJointCommissioningWindow::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace OpenJointCommissioningWindow
namespace TransferAnchorRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TransferAnchorRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace TransferAnchorRequest
namespace TransferAnchorComplete {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TransferAnchorComplete::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace TransferAnchorComplete
namespace AnnounceJointFabricAdministrator {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AnnounceJointFabricAdministrator::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AnnounceJointFabricAdministrator

} // namespace Commands
} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
