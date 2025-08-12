// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ContentAppObserver/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentAppObserver {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

constexpr std::array<DataModel::AttributeEntry, 0> kMandatoryMetadata = {

};

} // namespace Attributes

namespace Commands {

namespace ContentAppMessage {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ContentAppMessage::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ContentAppMessage

} // namespace Commands

namespace Events {} // namespace Events
} // namespace ContentAppObserver
} // namespace Clusters
} // namespace app
} // namespace chip
