// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EcosystemInformation (cluster code: 1872/0x750)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EcosystemInformation/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EcosystemInformation {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace DeviceDirectory {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DeviceDirectory::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace DeviceDirectory
namespace LocationDirectory {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(LocationDirectory::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace LocationDirectory
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    DeviceDirectory::kMetadataEntry,
    LocationDirectory::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace EcosystemInformation
} // namespace Clusters
} // namespace app
} // namespace chip
