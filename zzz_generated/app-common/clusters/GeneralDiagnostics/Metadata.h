// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GeneralDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace NetworkInterfaces {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NetworkInterfaces::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NetworkInterfaces
namespace RebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RebootCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RebootCount
namespace UpTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UpTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UpTime
namespace TotalOperationalHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TotalOperationalHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TotalOperationalHours
namespace BootReason {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BootReason::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BootReason
namespace ActiveHardwareFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveHardwareFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveHardwareFaults
namespace ActiveRadioFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveRadioFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveRadioFaults
namespace ActiveNetworkFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveNetworkFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveNetworkFaults
namespace TestEventTriggersEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TestEventTriggersEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TestEventTriggersEnabled

} // namespace Attributes

namespace Commands {
namespace TestEventTrigger {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = TestEventTrigger::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace TestEventTrigger
namespace TimeSnapshot {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = TimeSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace TimeSnapshot
namespace PayloadTestRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = PayloadTestRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace PayloadTestRequest

} // namespace Commands
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
