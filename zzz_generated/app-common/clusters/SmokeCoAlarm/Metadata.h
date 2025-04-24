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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ExpressedState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExpressedState
namespace SmokeState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SmokeState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SmokeState
namespace COState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = COState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace COState
namespace BatteryAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatteryAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatteryAlert
namespace DeviceMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DeviceMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DeviceMuted
namespace TestInProgress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TestInProgress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TestInProgress
namespace HardwareFaultAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HardwareFaultAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HardwareFaultAlert
namespace EndOfServiceAlert {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndOfServiceAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndOfServiceAlert
namespace InterconnectSmokeAlarm {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InterconnectSmokeAlarm::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InterconnectSmokeAlarm
namespace InterconnectCOAlarm {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InterconnectCOAlarm::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InterconnectCOAlarm
namespace ContaminationState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ContaminationState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ContaminationState
namespace SmokeSensitivityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SmokeSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SmokeSensitivityLevel
namespace ExpiryDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ExpiryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExpiryDate

} // namespace Attributes

namespace Commands {
namespace SelfTestRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SmokeCoAlarm::Commands::SelfTestRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SelfTestRequest

} // namespace Commands
} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
