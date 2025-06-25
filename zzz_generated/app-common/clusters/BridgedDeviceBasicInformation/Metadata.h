// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BridgedDeviceBasicInformation (cluster code: 57/0x39)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BridgedDeviceBasicInformation/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BridgedDeviceBasicInformation {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace VendorName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VendorName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VendorName
namespace VendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VendorID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VendorID
namespace ProductName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductName
namespace ProductID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductID
namespace NodeLabel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NodeLabel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace NodeLabel
namespace HardwareVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HardwareVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HardwareVersion
namespace HardwareVersionString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HardwareVersionString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HardwareVersionString
namespace SoftwareVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoftwareVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SoftwareVersion
namespace SoftwareVersionString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoftwareVersionString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SoftwareVersionString
namespace ManufacturingDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ManufacturingDate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ManufacturingDate
namespace PartNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PartNumber::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PartNumber
namespace ProductURL {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductURL::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductURL
namespace ProductLabel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductLabel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductLabel
namespace SerialNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SerialNumber::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SerialNumber
namespace Reachable {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Reachable::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Reachable
namespace UniqueID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UniqueID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UniqueID
namespace ProductAppearance {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductAppearance::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductAppearance
namespace ConfigurationVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ConfigurationVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ConfigurationVersion

} // namespace Attributes

namespace Commands {
namespace KeepActive {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(KeepActive::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace KeepActive

} // namespace Commands
} // namespace BridgedDeviceBasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
