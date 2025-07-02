// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkManagement (cluster code: 1105/0x451)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WiFiNetworkManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Ssid {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Ssid::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Ssid
namespace PassphraseSurrogate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PassphraseSurrogate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace PassphraseSurrogate

} // namespace Attributes

namespace Commands {
namespace NetworkPassphraseRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(NetworkPassphraseRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace NetworkPassphraseRequest

} // namespace Commands
} // namespace WiFiNetworkManagement
} // namespace Clusters
} // namespace app
} // namespace chip
