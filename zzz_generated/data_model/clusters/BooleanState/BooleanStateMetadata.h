// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanState (cluster code: 69/0x45)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BooleanState/BooleanStateIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace BooleanState {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kStateValueEntry = {
    .attributeId    = BooleanState::Attributes::StateValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace BooleanState
} // namespace clusters
} // namespace app
} // namespace chip
