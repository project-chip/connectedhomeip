// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SampleMei (cluster code: 4294048800/0xFFF1FC20)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SampleMei/SampleMeiIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace SampleMei {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kFlipFlopEntry = {
    .attributeId    = SampleMei::Attributes::FlipFlop::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kPingEntry = {
    .commandId       = SampleMei::Commands::Ping::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAddArgumentsEntry = {
    .commandId       = SampleMei::Commands::AddArguments::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace SampleMei
} // namespace clusters
} // namespace app
} // namespace chip
