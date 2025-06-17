// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Chime/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace InstalledChimeSounds {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(InstalledChimeSounds::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace InstalledChimeSounds
namespace SelectedChime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SelectedChime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace SelectedChime
namespace Enabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Enabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Enabled

} // namespace Attributes

namespace Commands {
namespace PlayChimeSound {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(PlayChimeSound::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace PlayChimeSound

} // namespace Commands
} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
