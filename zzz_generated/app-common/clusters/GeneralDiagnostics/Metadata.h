// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/GeneralDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

namespace NetworkInterfaces {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NetworkInterfaces::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace NetworkInterfaces
namespace RebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RebootCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RebootCount
namespace UpTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UpTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UpTime
namespace TotalOperationalHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TotalOperationalHours::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TotalOperationalHours
namespace BootReason {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BootReason::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BootReason
namespace ActiveHardwareFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveHardwareFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveHardwareFaults
namespace ActiveRadioFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveRadioFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveRadioFaults
namespace ActiveNetworkFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveNetworkFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveNetworkFaults
namespace TestEventTriggersEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TestEventTriggersEnabled::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TestEventTriggersEnabled
namespace DeviceLoadStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DeviceLoadStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DeviceLoadStatus
constexpr std::array<DataModel::AttributeEntry, 4> kMandatoryMetadata = {
    NetworkInterfaces::kMetadataEntry,
    RebootCount::kMetadataEntry,
    UpTime::kMetadataEntry,
    TestEventTriggersEnabled::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace TestEventTrigger {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestEventTrigger::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace TestEventTrigger
namespace TimeSnapshot {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TimeSnapshot::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TimeSnapshot
namespace PayloadTestRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(PayloadTestRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace PayloadTestRequest

} // namespace Commands

namespace Events {
namespace HardwareFaultChange {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace HardwareFaultChange
namespace RadioFaultChange {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace RadioFaultChange
namespace NetworkFaultChange {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace NetworkFaultChange
namespace BootReason {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace BootReason

} // namespace Events
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
