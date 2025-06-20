// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralCommissioning (cluster code: 48/0x30)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GeneralCommissioning/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace Breadcrumb {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Breadcrumb::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace Breadcrumb
namespace BasicCommissioningInfo {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BasicCommissioningInfo::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BasicCommissioningInfo
namespace RegulatoryConfig {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RegulatoryConfig::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RegulatoryConfig
namespace LocationCapability {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocationCapability::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LocationCapability
namespace SupportsConcurrentConnection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportsConcurrentConnection::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SupportsConcurrentConnection
namespace TCAcceptedVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TCAcceptedVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace TCAcceptedVersion
namespace TCMinRequiredVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TCMinRequiredVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace TCMinRequiredVersion
namespace TCAcknowledgements {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TCAcknowledgements::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace TCAcknowledgements
namespace TCAcknowledgementsRequired {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TCAcknowledgementsRequired::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace TCAcknowledgementsRequired
namespace TCUpdateDeadline {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TCUpdateDeadline::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace TCUpdateDeadline
namespace RecoveryIdentifier {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RecoveryIdentifier::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace RecoveryIdentifier
namespace NetworkRecoveryReason {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NetworkRecoveryReason::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace NetworkRecoveryReason

} // namespace Attributes

namespace Commands {
namespace ArmFailSafe {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ArmFailSafe::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ArmFailSafe
namespace SetRegulatoryConfig {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetRegulatoryConfig::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetRegulatoryConfig
namespace CommissioningComplete {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CommissioningComplete::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace CommissioningComplete
namespace SetTCAcknowledgements {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTCAcknowledgements::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace SetTCAcknowledgements

} // namespace Commands
} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
