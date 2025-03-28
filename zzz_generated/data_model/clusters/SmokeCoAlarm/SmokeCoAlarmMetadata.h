// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SmokeCoAlarm (cluster code: 92/0x5C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/SmokeCoAlarm/SmokeCoAlarmIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace SmokeCoAlarm {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kExpressedStateEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::ExpressedState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSmokeStateEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::SmokeState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCOStateEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::COState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBatteryAlertEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::BatteryAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDeviceMutedEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::DeviceMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTestInProgressEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::TestInProgress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHardwareFaultAlertEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::HardwareFaultAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEndOfServiceAlertEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::EndOfServiceAlert::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInterconnectSmokeAlarmEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::InterconnectSmokeAlarm::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInterconnectCOAlarmEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::InterconnectCOAlarm::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kContaminationStateEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::ContaminationState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSmokeSensitivityLevelEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::SmokeSensitivityLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kExpiryDateEntry = {
    .attributeId    = SmokeCoAlarm::Attributes::ExpiryDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSelfTestRequestEntry = {
    .commandId       = SmokeCoAlarm::Commands::SelfTestRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace SmokeCoAlarm
} // namespace clusters
} // namespace app
} // namespace chip
