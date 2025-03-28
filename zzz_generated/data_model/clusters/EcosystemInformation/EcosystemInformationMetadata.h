// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EcosystemInformation (cluster code: 1872/0x750)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EcosystemInformation/EcosystemInformationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace EcosystemInformation {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kDeviceDirectoryEntry = {
    .attributeId    = EcosystemInformation::Attributes::DeviceDirectory::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLocationDirectoryEntry = {
    .attributeId    = EcosystemInformation::Attributes::LocationDirectory::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace EcosystemInformation
} // namespace clusters
} // namespace app
} // namespace chip
