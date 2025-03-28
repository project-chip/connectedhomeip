// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkManagement (cluster code: 1105/0x451)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WiFiNetworkManagement/WiFiNetworkManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WiFiNetworkManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSsidEntry = {
    .attributeId    = WiFiNetworkManagement::Attributes::Ssid::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPassphraseSurrogateEntry = {
    .attributeId    = WiFiNetworkManagement::Attributes::PassphraseSurrogate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kNetworkPassphraseRequestEntry = {
    .commandId       = WiFiNetworkManagement::Commands::NetworkPassphraseRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace WiFiNetworkManagement
} // namespace clusters
} // namespace app
} // namespace chip
