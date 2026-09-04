// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaFileManagement (cluster code: 1297/0x511)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MediaFileManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace TotalStorage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TotalStorage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace TotalStorage
namespace AvailableStorage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AvailableStorage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace AvailableStorage
namespace AvailableFiles {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AvailableFiles::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace AvailableFiles
namespace SupportedMimeTypes {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedMimeTypes::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kManage, std::nullopt);
} // namespace SupportedMimeTypes
constexpr std::array<DataModel::AttributeEntry, 4> kMandatoryMetadata = {
    TotalStorage::kMetadataEntry,
    AvailableStorage::kMetadataEntry,
    AvailableFiles::kMetadataEntry,
    SupportedMimeTypes::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace AddFile {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddFile::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace AddFile
namespace DeleteFile {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DeleteFile::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace DeleteFile
namespace RequestSharedFiles {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RequestSharedFiles::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace RequestSharedFiles
namespace GetSharedFile {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetSharedFile::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GetSharedFile
namespace OfferFile {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(OfferFile::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace OfferFile

} // namespace Commands

namespace Events {
namespace SharedFilesAdded {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace SharedFilesAdded

} // namespace Events
} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
