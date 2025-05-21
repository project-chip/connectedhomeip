// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SmokeCoAlarm (cluster code: 92/0x5C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SmokeCoAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace ExpressedState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ExpressedState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ExpressedState
namespace SmokeState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SmokeState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SmokeState
namespace COState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(COState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace COState
namespace BatteryAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatteryAlert::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatteryAlert
namespace DeviceMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DeviceMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DeviceMuted
namespace TestInProgress {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TestInProgress::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TestInProgress
namespace HardwareFaultAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HardwareFaultAlert::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HardwareFaultAlert
namespace EndOfServiceAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EndOfServiceAlert::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EndOfServiceAlert
namespace InterconnectSmokeAlarm {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InterconnectSmokeAlarm::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InterconnectSmokeAlarm
namespace InterconnectCOAlarm {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InterconnectCOAlarm::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InterconnectCOAlarm
namespace ContaminationState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ContaminationState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ContaminationState
namespace SmokeSensitivityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SmokeSensitivityLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace SmokeSensitivityLevel
namespace ExpiryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ExpiryDate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ExpiryDate

} // namespace Attributes

namespace Commands {
namespace SelfTestRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SelfTestRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SelfTestRequest

} // namespace Commands
} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
