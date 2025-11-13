// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccountLogin (cluster code: 1294/0x50E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AccountLogin/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

constexpr std::array<DataModel::AttributeEntry, 0> kMandatoryMetadata = {

};

} // namespace Attributes

namespace Commands {

namespace GetSetupPIN {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(
    GetSetupPIN::Id,
    BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped, DataModel::CommandQualityFlags::kTimed),
    Access::Privilege::kAdminister);
} // namespace GetSetupPIN
namespace Login {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(
    Login::Id,
    BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped, DataModel::CommandQualityFlags::kTimed),
    Access::Privilege::kAdminister);
} // namespace Login
namespace Logout {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(
    Logout::Id,
    BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped, DataModel::CommandQualityFlags::kTimed),
    Access::Privilege::kOperate);
} // namespace Logout

} // namespace Commands

namespace Events {
namespace LoggedOut {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace LoggedOut

} // namespace Events
} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip
