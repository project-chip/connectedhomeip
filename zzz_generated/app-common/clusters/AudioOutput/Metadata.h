// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioOutput (cluster code: 1291/0x50B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AudioOutput/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace OutputList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(OutputList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace OutputList
namespace CurrentOutput {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentOutput::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentOutput

} // namespace Attributes

namespace Commands {
namespace SelectOutput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SelectOutput::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SelectOutput
namespace RenameOutput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RenameOutput::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace RenameOutput

} // namespace Commands
} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
