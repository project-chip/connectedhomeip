// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Identify (cluster code: 3/0x3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Identify/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Identify {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace IdentifyTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IdentifyTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace IdentifyTime
namespace IdentifyType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IdentifyType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace IdentifyType

} // namespace Attributes

namespace Commands {
namespace Identify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Identify::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace Identify
namespace TriggerEffect {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TriggerEffect::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace TriggerEffect

} // namespace Commands
} // namespace Identify
} // namespace Clusters
} // namespace app
} // namespace chip
