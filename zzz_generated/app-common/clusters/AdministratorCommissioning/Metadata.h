// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AdministratorCommissioning/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AdministratorCommissioning {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace WindowStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WindowStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WindowStatus
namespace AdminFabricIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AdminFabricIndex::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AdminFabricIndex
namespace AdminVendorId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AdminVendorId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AdminVendorId

} // namespace Attributes

namespace Commands {
namespace OpenCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(OpenCommissioningWindow::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace OpenCommissioningWindow
namespace OpenBasicCommissioningWindow {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(OpenBasicCommissioningWindow::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace OpenBasicCommissioningWindow
namespace RevokeCommissioning {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RevokeCommissioning::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace RevokeCommissioning

} // namespace Commands
} // namespace AdministratorCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
