// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationLauncher (cluster code: 1292/0x50C)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ApplicationLauncher/ApplicationLauncherIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ApplicationLauncher {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kCatalogListEntry = {
    .attributeId    = Attributes::CatalogList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentAppEntry = {
    .attributeId    = Attributes::CurrentApp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kLaunchAppEntry = {
    .commandId       = Commands::LaunchApp::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStopAppEntry = {
    .commandId       = Commands::StopApp::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kHideAppEntry = {
    .commandId       = Commands::HideApp::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ApplicationLauncher
} // namespace clusters
} // namespace app
} // namespace chip
