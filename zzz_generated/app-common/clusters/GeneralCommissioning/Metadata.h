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
namespace clusters {
namespace GeneralCommissioning {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Breadcrumb {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::Breadcrumb::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace Breadcrumb
namespace BasicCommissioningInfo {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::BasicCommissioningInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BasicCommissioningInfo
namespace RegulatoryConfig {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::RegulatoryConfig::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RegulatoryConfig
namespace LocationCapability {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::LocationCapability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LocationCapability
namespace SupportsConcurrentConnection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::SupportsConcurrentConnection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportsConcurrentConnection
namespace TCAcceptedVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcceptedVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace TCAcceptedVersion
namespace TCMinRequiredVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCMinRequiredVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace TCMinRequiredVersion
namespace TCAcknowledgements {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcknowledgements::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace TCAcknowledgements
namespace TCAcknowledgementsRequired {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcknowledgementsRequired::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace TCAcknowledgementsRequired
namespace TCUpdateDeadline {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCUpdateDeadline::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace TCUpdateDeadline

} // namespace Attributes

namespace Commands {
namespace ArmFailSafe {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = GeneralCommissioning::Commands::ArmFailSafe::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ArmFailSafe
namespace SetRegulatoryConfig {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = GeneralCommissioning::Commands::SetRegulatoryConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetRegulatoryConfig
namespace CommissioningComplete {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = GeneralCommissioning::Commands::CommissioningComplete::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace CommissioningComplete
namespace SetTCAcknowledgements {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = GeneralCommissioning::Commands::SetTCAcknowledgements::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetTCAcknowledgements

} // namespace Commands
} // namespace GeneralCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
