// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SampleMei (cluster code: 4294048800/0xFFF1FC20)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SampleMei/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace FlipFlop {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FlipFlop::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace FlipFlop

} // namespace Attributes

namespace Commands {
namespace Ping {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Ping::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Ping
namespace AddArguments {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddArguments::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddArguments

} // namespace Commands
} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip
