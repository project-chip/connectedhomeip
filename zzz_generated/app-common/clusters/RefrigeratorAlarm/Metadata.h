// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RefrigeratorAlarm (cluster code: 87/0x57)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RefrigeratorAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RefrigeratorAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Mask
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::State::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace State
namespace Supported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Supported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Supported

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace RefrigeratorAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
