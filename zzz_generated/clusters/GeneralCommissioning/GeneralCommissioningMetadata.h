// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralCommissioning (cluster code: 48/0x30)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GeneralCommissioning/GeneralCommissioningIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace GeneralCommissioning {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kBreadcrumbEntry = {
    .attributeId    = GeneralCommissioning::Attributes::Breadcrumb::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kBasicCommissioningInfoEntry = {
    .attributeId    = GeneralCommissioning::Attributes::BasicCommissioningInfo::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRegulatoryConfigEntry = {
    .attributeId    = GeneralCommissioning::Attributes::RegulatoryConfig::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLocationCapabilityEntry = {
    .attributeId    = GeneralCommissioning::Attributes::LocationCapability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportsConcurrentConnectionEntry = {
    .attributeId    = GeneralCommissioning::Attributes::SupportsConcurrentConnection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTCAcceptedVersionEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcceptedVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTCMinRequiredVersionEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCMinRequiredVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTCAcknowledgementsEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcknowledgements::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTCAcknowledgementsRequiredEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCAcknowledgementsRequired::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTCUpdateDeadlineEntry = {
    .attributeId    = GeneralCommissioning::Attributes::TCUpdateDeadline::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kArmFailSafeEntry = {
    .commandId       = GeneralCommissioning::Commands::ArmFailSafe::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetRegulatoryConfigEntry = {
    .commandId       = GeneralCommissioning::Commands::SetRegulatoryConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kCommissioningCompleteEntry = {
    .commandId       = GeneralCommissioning::Commands::CommissioningComplete::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetTCAcknowledgementsEntry = {
    .commandId       = GeneralCommissioning::Commands::SetTCAcknowledgements::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace GeneralCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
