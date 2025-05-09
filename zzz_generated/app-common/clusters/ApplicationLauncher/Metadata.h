// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationLauncher (cluster code: 1292/0x50C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ApplicationLauncher/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CatalogList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CatalogList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CatalogList
namespace CurrentApp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentApp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentApp

} // namespace Attributes

namespace Commands {
namespace LaunchApp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(LaunchApp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace LaunchApp
namespace StopApp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopApp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopApp
namespace HideApp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(HideApp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace HideApp

} // namespace Commands
} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
