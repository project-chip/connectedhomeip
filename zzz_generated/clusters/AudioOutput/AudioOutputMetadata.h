// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioOutput (cluster code: 1291/0x50B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AudioOutput/AudioOutputIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace AudioOutput {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kOutputListEntry = {
    .attributeId    = AudioOutput::Attributes::OutputList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentOutputEntry = {
    .attributeId    = AudioOutput::Attributes::CurrentOutput::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSelectOutputEntry = {
    .commandId       = AudioOutput::Commands::SelectOutput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRenameOutputEntry = {
    .commandId       = AudioOutput::Commands::RenameOutput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace AudioOutput
} // namespace clusters
} // namespace app
} // namespace chip
