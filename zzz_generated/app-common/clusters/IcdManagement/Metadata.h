// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/IcdManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IcdManagement {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace IdleModeDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IdleModeDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace IdleModeDuration
namespace ActiveModeDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveModeDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveModeDuration
namespace ActiveModeThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveModeThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveModeThreshold
namespace RegisteredClients {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(RegisteredClients::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace RegisteredClients
namespace ICDCounter {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ICDCounter::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace ICDCounter
namespace ClientsSupportedPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ClientsSupportedPerFabric::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ClientsSupportedPerFabric
namespace UserActiveModeTriggerHint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UserActiveModeTriggerHint::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace UserActiveModeTriggerHint
namespace UserActiveModeTriggerInstruction {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UserActiveModeTriggerInstruction::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace UserActiveModeTriggerInstruction
namespace OperatingMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperatingMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OperatingMode
namespace MaximumCheckInBackOff {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaximumCheckInBackOff::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaximumCheckInBackOff

} // namespace Attributes

namespace Commands {
namespace RegisterClient {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RegisterClient::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace RegisterClient
namespace UnregisterClient {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UnregisterClient::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace UnregisterClient
namespace StayActiveRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StayActiveRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace StayActiveRequest

} // namespace Commands
} // namespace IcdManagement
} // namespace Clusters
} // namespace app
} // namespace chip
