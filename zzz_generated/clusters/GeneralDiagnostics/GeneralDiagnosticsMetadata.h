// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GeneralDiagnostics/GeneralDiagnosticsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace GeneralDiagnostics {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kNetworkInterfacesEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::NetworkInterfaces::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRebootCountEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::RebootCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUpTimeEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::UpTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTotalOperationalHoursEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::TotalOperationalHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBootReasonEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::BootReason::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveHardwareFaultsEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveRadioFaultsEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::ActiveRadioFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveNetworkFaultsEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTestEventTriggersEnabledEntry = {
    .attributeId    = GeneralDiagnostics::Attributes::TestEventTriggersEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kTestEventTriggerEntry = {
    .commandId       = GeneralDiagnostics::Commands::TestEventTrigger::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kTimeSnapshotEntry = {
    .commandId       = GeneralDiagnostics::Commands::TimeSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kPayloadTestRequestEntry = {
    .commandId       = GeneralDiagnostics::Commands::PayloadTestRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace GeneralDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
