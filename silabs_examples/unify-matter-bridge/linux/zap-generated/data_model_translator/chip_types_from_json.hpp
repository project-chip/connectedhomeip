/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "app/data-model/NullObject.h"
#include "zcl_global_types.hpp"

// Default translation
template <typename T>
std::optional<T> from_json(const nlohmann::json & value)
{
    return std::nullopt;
}
/***************************** Simple type Convertes **************/

template <>
inline std::optional<bool> from_json(const nlohmann::json & value)
{
    return value;
}

template <>
inline std::optional<int> from_json(const nlohmann::json & value)
{
    return value;
}

template <>
inline std::optional<std::string> from_json(const nlohmann::json & value)
{
    return value;
}

template <>
inline std::optional<uint8_t> from_json(const nlohmann::json & value)
{
    return value;
}
template <>
inline std::optional<uint16_t> from_json(const nlohmann::json & value)
{
    return value;
}
template <>
inline std::optional<uint32_t> from_json(const nlohmann::json & value)
{
    return value;
}
template <>
inline std::optional<uint64_t> from_json(const nlohmann::json & value)
{
    return value;
}
template <>
inline std::optional<chip::app::DataModel::Nullable<int8_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<int8_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<int16_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<int16_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<int32_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<int32_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<int64_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<int64_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<uint8_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<uint8_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<uint16_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<uint16_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<uint32_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<uint32_t>(value);
    }
}
template <>
inline std::optional<chip::app::DataModel::Nullable<uint64_t>> from_json(const nlohmann::json & value)
{
    if (value.is_null())
    {
        return std::nullopt;
    }
    else
    {
        return chip::app::DataModel::Nullable<uint64_t>(value);
    }
}
/***************************** Bitmap Converters **************/
//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<Identify::IdentifyEffectIdentifier> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Identify::IdentifyEffectIdentifier> table = {
        { "Blink", Identify::IdentifyEffectIdentifier::kBlink },
        { "Breathe", Identify::IdentifyEffectIdentifier::kBreathe },
        { "Okay", Identify::IdentifyEffectIdentifier::kOkay },
        { "ChannelChange", Identify::IdentifyEffectIdentifier::kChannelChange },
        { "FinishEffect", Identify::IdentifyEffectIdentifier::kFinishEffect },
        { "StopEffect", Identify::IdentifyEffectIdentifier::kStopEffect },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyEffectVariant> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Identify::IdentifyEffectVariant> table = {
        { "Default", Identify::IdentifyEffectVariant::kDefault },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyIdentifyType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Identify::IdentifyIdentifyType> table = {
        { "None", Identify::IdentifyIdentifyType::kNone },
        { "VisibleLight", Identify::IdentifyIdentifyType::kVisibleLight },
        { "VisibleLED", Identify::IdentifyIdentifyType::kVisibleLED },
        { "AudibleBeep", Identify::IdentifyIdentifyType::kAudibleBeep },
        { "Display", Identify::IdentifyIdentifyType::kDisplay },
        { "Actuator", Identify::IdentifyIdentifyType::kActuator },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Groups::GroupClusterFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Scenes::ScenesCopyMode, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Scenes::Structs::AttributeValuePair::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const Scenes::Structs::ExtensionFieldSet::Type& value) {
//    return "no struct support";
//}
//
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<OnOff::OnOffControl, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<OnOff::OnOffFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<OnOff::SceneFeatures, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<OnOff::OnOffDelayedAllOffEffectVariant> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OnOff::OnOffDelayedAllOffEffectVariant> table = {
        { "FadeToOffIn0p8Seconds", OnOff::OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds },
        { "NoFade", OnOff::OnOffDelayedAllOffEffectVariant::kNoFade },
        { "50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds",
          OnOff::OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffDyingLightEffectVariant> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OnOff::OnOffDyingLightEffectVariant> table = {
        { "20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second",
          OnOff::OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffEffectIdentifier> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OnOff::OnOffEffectIdentifier> table = {
        { "DelayedAllOff", OnOff::OnOffEffectIdentifier::kDelayedAllOff },
        { "DyingLight", OnOff::OnOffEffectIdentifier::kDyingLight },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffStartUpOnOff> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OnOff::OnOffStartUpOnOff> table = {
        { "Off", OnOff::OnOffStartUpOnOff::kOff },
        { "On", OnOff::OnOffStartUpOnOff::kOn },
        { "TogglePreviousOnOff", OnOff::OnOffStartUpOnOff::kTogglePreviousOnOff },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<LevelControl::LevelControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<LevelControl::MoveMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, LevelControl::MoveMode> table = {
        { "Up", LevelControl::MoveMode::kUp },
        { "Down", LevelControl::MoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<LevelControl::StepMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, LevelControl::StepMode> table = {
        { "Up", LevelControl::StepMode::kUp },
        { "Down", LevelControl::StepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Descriptor::Structs::DeviceTypeStruct::Type& value) {
//    return "no struct support";
//}
//
//

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Binding::Structs::TargetStruct::Type& value) {
//    return "no struct support";
//}
//
//

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const AccessControl::Structs::Target::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const AccessControl::Structs::AccessControlEntry::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const AccessControl::Structs::ExtensionEntry::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<AccessControl::AuthMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AccessControl::AuthMode> table = {
        { "Pase", AccessControl::AuthMode::kPase },
        { "Case", AccessControl::AuthMode::kCase },
        { "Group", AccessControl::AuthMode::kGroup },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<AccessControl::ChangeTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AccessControl::ChangeTypeEnum> table = {
        { "Changed", AccessControl::ChangeTypeEnum::kChanged },
        { "Added", AccessControl::ChangeTypeEnum::kAdded },
        { "Removed", AccessControl::ChangeTypeEnum::kRemoved },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<AccessControl::Privilege> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AccessControl::Privilege> table = {
        { "View", AccessControl::Privilege::kView },
        { "ProxyView", AccessControl::Privilege::kProxyView },
        { "Operate", AccessControl::Privilege::kOperate },
        { "Manage", AccessControl::Privilege::kManage },
        { "Administer", AccessControl::Privilege::kAdminister },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Actions::CommandBits, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Actions::Structs::ActionStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const Actions::Structs::EndpointListStruct::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<Actions::ActionErrorEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Actions::ActionErrorEnum> table = {
        { "Unknown", Actions::ActionErrorEnum::kUnknown },
        { "Interrupted", Actions::ActionErrorEnum::kInterrupted },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Actions::ActionStateEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Actions::ActionStateEnum> table = {
        { "Inactive", Actions::ActionStateEnum::kInactive },
        { "Active", Actions::ActionStateEnum::kActive },
        { "Paused", Actions::ActionStateEnum::kPaused },
        { "Disabled", Actions::ActionStateEnum::kDisabled },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Actions::ActionTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Actions::ActionTypeEnum> table = {
        { "Other", Actions::ActionTypeEnum::kOther },         { "Scene", Actions::ActionTypeEnum::kScene },
        { "Sequence", Actions::ActionTypeEnum::kSequence },   { "Automation", Actions::ActionTypeEnum::kAutomation },
        { "Exception", Actions::ActionTypeEnum::kException }, { "Notification", Actions::ActionTypeEnum::kNotification },
        { "Alarm", Actions::ActionTypeEnum::kAlarm },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Actions::EndpointListTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Actions::EndpointListTypeEnum> table = {
        { "Other", Actions::EndpointListTypeEnum::kOther },
        { "Room", Actions::EndpointListTypeEnum::kRoom },
        { "Zone", Actions::EndpointListTypeEnum::kZone },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Basic::Structs::CapabilityMinimaStruct::Type& value) {
//    return "no struct support";
//}
//
//

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<OtaSoftwareUpdateProvider::OTAApplyUpdateAction> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateProvider::OTAApplyUpdateAction> table = {
        { "Proceed", OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kProceed },
        { "AwaitNextAction", OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kAwaitNextAction },
        { "Discontinue", OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kDiscontinue },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OtaSoftwareUpdateProvider::OTADownloadProtocol> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateProvider::OTADownloadProtocol> table = {
        { "BDXSynchronous", OtaSoftwareUpdateProvider::OTADownloadProtocol::kBDXSynchronous },
        { "BDXAsynchronous", OtaSoftwareUpdateProvider::OTADownloadProtocol::kBDXAsynchronous },
        { "Https", OtaSoftwareUpdateProvider::OTADownloadProtocol::kHttps },
        { "VendorSpecific", OtaSoftwareUpdateProvider::OTADownloadProtocol::kVendorSpecific },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OtaSoftwareUpdateProvider::OTAQueryStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateProvider::OTAQueryStatus> table = {
        { "UpdateAvailable", OtaSoftwareUpdateProvider::OTAQueryStatus::kUpdateAvailable },
        { "Busy", OtaSoftwareUpdateProvider::OTAQueryStatus::kBusy },
        { "NotAvailable", OtaSoftwareUpdateProvider::OTAQueryStatus::kNotAvailable },
        { "DownloadProtocolNotSupported", OtaSoftwareUpdateProvider::OTAQueryStatus::kDownloadProtocolNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<OtaSoftwareUpdateRequestor::OTAAnnouncementReason> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateRequestor::OTAAnnouncementReason> table = {
        { "SimpleAnnouncement", OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kSimpleAnnouncement },
        { "UpdateAvailable", OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kUpdateAvailable },
        { "UrgentUpdateAvailable", OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kUrgentUpdateAvailable },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OtaSoftwareUpdateRequestor::OTAChangeReasonEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateRequestor::OTAChangeReasonEnum> table = {
        { "Unknown", OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kUnknown },
        { "Success", OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kSuccess },
        { "Failure", OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kFailure },
        { "TimeOut", OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kTimeOut },
        { "DelayByProvider", OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kDelayByProvider },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<OtaSoftwareUpdateRequestor::OTAUpdateStateEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OtaSoftwareUpdateRequestor::OTAUpdateStateEnum> table = {
        { "Unknown", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kUnknown },
        { "Idle", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle },
        { "Querying", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kQuerying },
        { "DelayedOnQuery", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnQuery },
        { "Downloading", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDownloading },
        { "Applying", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying },
        { "DelayedOnApply", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnApply },
        { "RollingBack", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kRollingBack },
        { "DelayedOnUserConsent", OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnUserConsent },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<TimeFormatLocalization::CalendarType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, TimeFormatLocalization::CalendarType> table = {
        { "Buddhist", TimeFormatLocalization::CalendarType::kBuddhist },
        { "Chinese", TimeFormatLocalization::CalendarType::kChinese },
        { "Coptic", TimeFormatLocalization::CalendarType::kCoptic },
        { "Ethiopian", TimeFormatLocalization::CalendarType::kEthiopian },
        { "Gregorian", TimeFormatLocalization::CalendarType::kGregorian },
        { "Hebrew", TimeFormatLocalization::CalendarType::kHebrew },
        { "Indian", TimeFormatLocalization::CalendarType::kIndian },
        { "Islamic", TimeFormatLocalization::CalendarType::kIslamic },
        { "Japanese", TimeFormatLocalization::CalendarType::kJapanese },
        { "Korean", TimeFormatLocalization::CalendarType::kKorean },
        { "Persian", TimeFormatLocalization::CalendarType::kPersian },
        { "Taiwanese", TimeFormatLocalization::CalendarType::kTaiwanese },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<TimeFormatLocalization::HourFormat> from_json(const nlohmann::json & value)
{
    const std::map<std::string, TimeFormatLocalization::HourFormat> table = {
        { "12hr", TimeFormatLocalization::HourFormat::k12hr },
        { "24hr", TimeFormatLocalization::HourFormat::k24hr },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitLocalization::UnitLocalizationFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<UnitLocalization::TempUnit> from_json(const nlohmann::json & value)
{
    const std::map<std::string, UnitLocalization::TempUnit> table = {
        { "Fahrenheit", UnitLocalization::TempUnit::kFahrenheit },
        { "Celsius", UnitLocalization::TempUnit::kCelsius },
        { "Kelvin", UnitLocalization::TempUnit::kKelvin },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<PowerSource::PowerSourceFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const PowerSource::Structs::BatChargeFaultChangeType::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const PowerSource::Structs::BatFaultChangeType::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const PowerSource::Structs::WiredFaultChangeType::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<PowerSource::BatChargeFault> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::BatChargeFault> table = {
        { "Unspecfied", PowerSource::BatChargeFault::kUnspecfied },
        { "AmbientTooHot", PowerSource::BatChargeFault::kAmbientTooHot },
        { "AmbientTooCold", PowerSource::BatChargeFault::kAmbientTooCold },
        { "BatteryTooHot", PowerSource::BatChargeFault::kBatteryTooHot },
        { "BatteryTooCold", PowerSource::BatChargeFault::kBatteryTooCold },
        { "BatteryAbsent", PowerSource::BatChargeFault::kBatteryAbsent },
        { "BatteryOverVoltage", PowerSource::BatChargeFault::kBatteryOverVoltage },
        { "BatteryUnderVoltage", PowerSource::BatChargeFault::kBatteryUnderVoltage },
        { "ChargerOverVoltage", PowerSource::BatChargeFault::kChargerOverVoltage },
        { "ChargerUnderVoltage", PowerSource::BatChargeFault::kChargerUnderVoltage },
        { "SafetyTimeout", PowerSource::BatChargeFault::kSafetyTimeout },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::BatChargeLevel> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::BatChargeLevel> table = {
        { "Ok", PowerSource::BatChargeLevel::kOk },
        { "Warning", PowerSource::BatChargeLevel::kWarning },
        { "Critical", PowerSource::BatChargeLevel::kCritical },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::BatChargeState> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::BatChargeState> table = {
        { "Unknown", PowerSource::BatChargeState::kUnknown },
        { "IsCharging", PowerSource::BatChargeState::kIsCharging },
        { "IsAtFullCharge", PowerSource::BatChargeState::kIsAtFullCharge },
        { "IsNotCharging", PowerSource::BatChargeState::kIsNotCharging },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::BatFault> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::BatFault> table = {
        { "Unspecfied", PowerSource::BatFault::kUnspecfied },
        { "OverTemp", PowerSource::BatFault::kOverTemp },
        { "UnderTemp", PowerSource::BatFault::kUnderTemp },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::BatReplaceability> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::BatReplaceability> table = {
        { "Unspecified", PowerSource::BatReplaceability::kUnspecified },
        { "NotReplaceable", PowerSource::BatReplaceability::kNotReplaceable },
        { "UserReplaceable", PowerSource::BatReplaceability::kUserReplaceable },
        { "FactoryReplaceable", PowerSource::BatReplaceability::kFactoryReplaceable },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::PowerSourceStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::PowerSourceStatus> table = {
        { "Unspecfied", PowerSource::PowerSourceStatus::kUnspecfied },
        { "Active", PowerSource::PowerSourceStatus::kActive },
        { "Standby", PowerSource::PowerSourceStatus::kStandby },
        { "Unavailable", PowerSource::PowerSourceStatus::kUnavailable },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::WiredCurrentType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::WiredCurrentType> table = {
        { "Ac", PowerSource::WiredCurrentType::kAc },
        { "Dc", PowerSource::WiredCurrentType::kDc },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PowerSource::WiredFault> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PowerSource::WiredFault> table = {
        { "Unspecfied", PowerSource::WiredFault::kUnspecfied },
        { "OverVoltage", PowerSource::WiredFault::kOverVoltage },
        { "UnderVoltage", PowerSource::WiredFault::kUnderVoltage },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const GeneralCommissioning::Structs::BasicCommissioningInfo::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<GeneralCommissioning::CommissioningError> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralCommissioning::CommissioningError> table = {
        { "Ok", GeneralCommissioning::CommissioningError::kOk },
        { "ValueOutsideRange", GeneralCommissioning::CommissioningError::kValueOutsideRange },
        { "InvalidAuthentication", GeneralCommissioning::CommissioningError::kInvalidAuthentication },
        { "NoFailSafe", GeneralCommissioning::CommissioningError::kNoFailSafe },
        { "BusyWithOtherAdmin", GeneralCommissioning::CommissioningError::kBusyWithOtherAdmin },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<GeneralCommissioning::RegulatoryLocationType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralCommissioning::RegulatoryLocationType> table = {
        { "Indoor", GeneralCommissioning::RegulatoryLocationType::kIndoor },
        { "Outdoor", GeneralCommissioning::RegulatoryLocationType::kOutdoor },
        { "IndoorOutdoor", GeneralCommissioning::RegulatoryLocationType::kIndoorOutdoor },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<NetworkCommissioning::NetworkCommissioningFeature, uint8_t>& value)
// {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<NetworkCommissioning::WiFiSecurity, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const NetworkCommissioning::Structs::NetworkInfo::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const NetworkCommissioning::Structs::ThreadInterfaceScanResult::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const NetworkCommissioning::Structs::WiFiInterfaceScanResult::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<NetworkCommissioning::NetworkCommissioningStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, NetworkCommissioning::NetworkCommissioningStatus> table = {
        { "Success", NetworkCommissioning::NetworkCommissioningStatus::kSuccess },
        { "OutOfRange", NetworkCommissioning::NetworkCommissioningStatus::kOutOfRange },
        { "BoundsExceeded", NetworkCommissioning::NetworkCommissioningStatus::kBoundsExceeded },
        { "NetworkIDNotFound", NetworkCommissioning::NetworkCommissioningStatus::kNetworkIDNotFound },
        { "DuplicateNetworkID", NetworkCommissioning::NetworkCommissioningStatus::kDuplicateNetworkID },
        { "NetworkNotFound", NetworkCommissioning::NetworkCommissioningStatus::kNetworkNotFound },
        { "RegulatoryError", NetworkCommissioning::NetworkCommissioningStatus::kRegulatoryError },
        { "AuthFailure", NetworkCommissioning::NetworkCommissioningStatus::kAuthFailure },
        { "UnsupportedSecurity", NetworkCommissioning::NetworkCommissioningStatus::kUnsupportedSecurity },
        { "OtherConnectionFailure", NetworkCommissioning::NetworkCommissioningStatus::kOtherConnectionFailure },
        { "IPV6Failed", NetworkCommissioning::NetworkCommissioningStatus::kIPV6Failed },
        { "IPBindFailed", NetworkCommissioning::NetworkCommissioningStatus::kIPBindFailed },
        { "UnknownError", NetworkCommissioning::NetworkCommissioningStatus::kUnknownError },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<NetworkCommissioning::WiFiBand> from_json(const nlohmann::json & value)
{
    const std::map<std::string, NetworkCommissioning::WiFiBand> table = {
        { "2g4", NetworkCommissioning::WiFiBand::k2g4 }, { "3g65", NetworkCommissioning::WiFiBand::k3g65 },
        { "5g", NetworkCommissioning::WiFiBand::k5g },   { "6g", NetworkCommissioning::WiFiBand::k6g },
        { "60g", NetworkCommissioning::WiFiBand::k60g },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<DiagnosticLogs::LogsIntent> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DiagnosticLogs::LogsIntent> table = {
        { "EndUserSupport", DiagnosticLogs::LogsIntent::kEndUserSupport },
        { "NetworkDiag", DiagnosticLogs::LogsIntent::kNetworkDiag },
        { "CrashLogs", DiagnosticLogs::LogsIntent::kCrashLogs },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DiagnosticLogs::LogsStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DiagnosticLogs::LogsStatus> table = {
        { "Success", DiagnosticLogs::LogsStatus::kSuccess }, { "Exhausted", DiagnosticLogs::LogsStatus::kExhausted },
        { "NoLogs", DiagnosticLogs::LogsStatus::kNoLogs },   { "Busy", DiagnosticLogs::LogsStatus::kBusy },
        { "Denied", DiagnosticLogs::LogsStatus::kDenied },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DiagnosticLogs::LogsTransferProtocol> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DiagnosticLogs::LogsTransferProtocol> table = {
        { "ResponsePayload", DiagnosticLogs::LogsTransferProtocol::kResponsePayload },
        { "Bdx", DiagnosticLogs::LogsTransferProtocol::kBdx },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const GeneralDiagnostics::Structs::NetworkInterfaceType::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<GeneralDiagnostics::BootReasonType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralDiagnostics::BootReasonType> table = {
        { "Unspecified", GeneralDiagnostics::BootReasonType::kUnspecified },
        { "PowerOnReboot", GeneralDiagnostics::BootReasonType::kPowerOnReboot },
        { "BrownOutReset", GeneralDiagnostics::BootReasonType::kBrownOutReset },
        { "SoftwareWatchdogReset", GeneralDiagnostics::BootReasonType::kSoftwareWatchdogReset },
        { "HardwareWatchdogReset", GeneralDiagnostics::BootReasonType::kHardwareWatchdogReset },
        { "SoftwareUpdateCompleted", GeneralDiagnostics::BootReasonType::kSoftwareUpdateCompleted },
        { "SoftwareReset", GeneralDiagnostics::BootReasonType::kSoftwareReset },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<GeneralDiagnostics::HardwareFaultType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralDiagnostics::HardwareFaultType> table = {
        { "Unspecified", GeneralDiagnostics::HardwareFaultType::kUnspecified },
        { "Radio", GeneralDiagnostics::HardwareFaultType::kRadio },
        { "Sensor", GeneralDiagnostics::HardwareFaultType::kSensor },
        { "ResettableOverTemp", GeneralDiagnostics::HardwareFaultType::kResettableOverTemp },
        { "NonResettableOverTemp", GeneralDiagnostics::HardwareFaultType::kNonResettableOverTemp },
        { "PowerSource", GeneralDiagnostics::HardwareFaultType::kPowerSource },
        { "VisualDisplayFault", GeneralDiagnostics::HardwareFaultType::kVisualDisplayFault },
        { "AudioOutputFault", GeneralDiagnostics::HardwareFaultType::kAudioOutputFault },
        { "UserInterfaceFault", GeneralDiagnostics::HardwareFaultType::kUserInterfaceFault },
        { "NonVolatileMemoryError", GeneralDiagnostics::HardwareFaultType::kNonVolatileMemoryError },
        { "TamperDetected", GeneralDiagnostics::HardwareFaultType::kTamperDetected },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<GeneralDiagnostics::InterfaceType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralDiagnostics::InterfaceType> table = {
        { "Unspecified", GeneralDiagnostics::InterfaceType::kUnspecified },
        { "WiFi", GeneralDiagnostics::InterfaceType::kWiFi },
        { "Ethernet", GeneralDiagnostics::InterfaceType::kEthernet },
        { "Cellular", GeneralDiagnostics::InterfaceType::kCellular },
        { "Thread", GeneralDiagnostics::InterfaceType::kThread },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<GeneralDiagnostics::NetworkFaultType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralDiagnostics::NetworkFaultType> table = {
        { "Unspecified", GeneralDiagnostics::NetworkFaultType::kUnspecified },
        { "HardwareFailure", GeneralDiagnostics::NetworkFaultType::kHardwareFailure },
        { "NetworkJammed", GeneralDiagnostics::NetworkFaultType::kNetworkJammed },
        { "ConnectionFailed", GeneralDiagnostics::NetworkFaultType::kConnectionFailed },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<GeneralDiagnostics::RadioFaultType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GeneralDiagnostics::RadioFaultType> table = {
        { "Unspecified", GeneralDiagnostics::RadioFaultType::kUnspecified },
        { "WiFiFault", GeneralDiagnostics::RadioFaultType::kWiFiFault },
        { "CellularFault", GeneralDiagnostics::RadioFaultType::kCellularFault },
        { "ThreadFault", GeneralDiagnostics::RadioFaultType::kThreadFault },
        { "NFCFault", GeneralDiagnostics::RadioFaultType::kNFCFault },
        { "BLEFault", GeneralDiagnostics::RadioFaultType::kBLEFault },
        { "EthernetFault", GeneralDiagnostics::RadioFaultType::kEthernetFault },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<SoftwareDiagnostics::SoftwareDiagnosticsFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const SoftwareDiagnostics::Structs::ThreadMetrics::Type& value) {
//    return "no struct support";
//}
//
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsFeature,
// uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const ThreadNetworkDiagnostics::Structs::NeighborTable::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ThreadNetworkDiagnostics::Structs::OperationalDatasetComponents::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ThreadNetworkDiagnostics::Structs::RouteTable::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ThreadNetworkDiagnostics::Structs::SecurityPolicy::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<ThreadNetworkDiagnostics::NetworkFault> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ThreadNetworkDiagnostics::NetworkFault> table = {
        { "Unspecified", ThreadNetworkDiagnostics::NetworkFault::kUnspecified },
        { "LinkDown", ThreadNetworkDiagnostics::NetworkFault::kLinkDown },
        { "HardwareFailure", ThreadNetworkDiagnostics::NetworkFault::kHardwareFailure },
        { "NetworkJammed", ThreadNetworkDiagnostics::NetworkFault::kNetworkJammed },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ThreadNetworkDiagnostics::RoutingRole> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ThreadNetworkDiagnostics::RoutingRole> table = {
        { "Unspecified", ThreadNetworkDiagnostics::RoutingRole::kUnspecified },
        { "Unassigned", ThreadNetworkDiagnostics::RoutingRole::kUnassigned },
        { "SleepyEndDevice", ThreadNetworkDiagnostics::RoutingRole::kSleepyEndDevice },
        { "EndDevice", ThreadNetworkDiagnostics::RoutingRole::kEndDevice },
        { "Reed", ThreadNetworkDiagnostics::RoutingRole::kReed },
        { "Router", ThreadNetworkDiagnostics::RoutingRole::kRouter },
        { "Leader", ThreadNetworkDiagnostics::RoutingRole::kLeader },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ThreadNetworkDiagnostics::ThreadConnectionStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ThreadNetworkDiagnostics::ThreadConnectionStatus> table = {
        { "Connected", ThreadNetworkDiagnostics::ThreadConnectionStatus::kConnected },
        { "NotConnected", ThreadNetworkDiagnostics::ThreadConnectionStatus::kNotConnected },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<WiFiNetworkDiagnostics::AssociationFailureCause> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WiFiNetworkDiagnostics::AssociationFailureCause> table = {
        { "Unknown", WiFiNetworkDiagnostics::AssociationFailureCause::kUnknown },
        { "AssociationFailed", WiFiNetworkDiagnostics::AssociationFailureCause::kAssociationFailed },
        { "AuthenticationFailed", WiFiNetworkDiagnostics::AssociationFailureCause::kAuthenticationFailed },
        { "SsidNotFound", WiFiNetworkDiagnostics::AssociationFailureCause::kSsidNotFound },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<WiFiNetworkDiagnostics::SecurityType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WiFiNetworkDiagnostics::SecurityType> table = {
        { "Unspecified", WiFiNetworkDiagnostics::SecurityType::kUnspecified },
        { "None", WiFiNetworkDiagnostics::SecurityType::kNone },
        { "Wep", WiFiNetworkDiagnostics::SecurityType::kWep },
        { "Wpa", WiFiNetworkDiagnostics::SecurityType::kWpa },
        { "Wpa2", WiFiNetworkDiagnostics::SecurityType::kWpa2 },
        { "Wpa3", WiFiNetworkDiagnostics::SecurityType::kWpa3 },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<WiFiNetworkDiagnostics::WiFiConnectionStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WiFiNetworkDiagnostics::WiFiConnectionStatus> table = {
        { "Connected", WiFiNetworkDiagnostics::WiFiConnectionStatus::kConnected },
        { "NotConnected", WiFiNetworkDiagnostics::WiFiConnectionStatus::kNotConnected },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<WiFiNetworkDiagnostics::WiFiVersionType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WiFiNetworkDiagnostics::WiFiVersionType> table = {
        { "80211a", WiFiNetworkDiagnostics::WiFiVersionType::k80211a },
        { "80211b", WiFiNetworkDiagnostics::WiFiVersionType::k80211b },
        { "80211g", WiFiNetworkDiagnostics::WiFiVersionType::k80211g },
        { "80211n", WiFiNetworkDiagnostics::WiFiVersionType::k80211n },
        { "80211ac", WiFiNetworkDiagnostics::WiFiVersionType::k80211ac },
        { "80211ax", WiFiNetworkDiagnostics::WiFiVersionType::k80211ax },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<EthernetNetworkDiagnostics::PHYRateType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, EthernetNetworkDiagnostics::PHYRateType> table = {
        { "10m", EthernetNetworkDiagnostics::PHYRateType::k10m },     { "100m", EthernetNetworkDiagnostics::PHYRateType::k100m },
        { "1000m", EthernetNetworkDiagnostics::PHYRateType::k1000m }, { "25g", EthernetNetworkDiagnostics::PHYRateType::k25g },
        { "5g", EthernetNetworkDiagnostics::PHYRateType::k5g },       { "10g", EthernetNetworkDiagnostics::PHYRateType::k10g },
        { "40g", EthernetNetworkDiagnostics::PHYRateType::k40g },     { "100g", EthernetNetworkDiagnostics::PHYRateType::k100g },
        { "200g", EthernetNetworkDiagnostics::PHYRateType::k200g },   { "400g", EthernetNetworkDiagnostics::PHYRateType::k400g },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const TimeSynchronization::Structs::DstOffsetType::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const TimeSynchronization::Structs::TimeZoneType::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<TimeSynchronization::GranularityEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, TimeSynchronization::GranularityEnum> table = {
        { "NoTimeGranularity", TimeSynchronization::GranularityEnum::kNoTimeGranularity },
        { "MinutesGranularity", TimeSynchronization::GranularityEnum::kMinutesGranularity },
        { "SecondsGranularity", TimeSynchronization::GranularityEnum::kSecondsGranularity },
        { "MillisecondsGranularity", TimeSynchronization::GranularityEnum::kMillisecondsGranularity },
        { "MicrosecondsGranularity", TimeSynchronization::GranularityEnum::kMicrosecondsGranularity },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<TimeSynchronization::TimeSourceEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, TimeSynchronization::TimeSourceEnum> table = {
        { "None", TimeSynchronization::TimeSourceEnum::kNone },
        { "Unknown", TimeSynchronization::TimeSourceEnum::kUnknown },
        { "Admin", TimeSynchronization::TimeSourceEnum::kAdmin },
        { "NodeTimeCluster", TimeSynchronization::TimeSourceEnum::kNodeTimeCluster },
        { "NonFabricSntp", TimeSynchronization::TimeSourceEnum::kNonFabricSntp },
        { "NonFabricNtp", TimeSynchronization::TimeSourceEnum::kNonFabricNtp },
        { "FabricSntp", TimeSynchronization::TimeSourceEnum::kFabricSntp },
        { "FabricNtp", TimeSynchronization::TimeSourceEnum::kFabricNtp },
        { "MixedNtp", TimeSynchronization::TimeSourceEnum::kMixedNtp },
        { "NonFabricSntpNts", TimeSynchronization::TimeSourceEnum::kNonFabricSntpNts },
        { "NonFabricNtpNts", TimeSynchronization::TimeSourceEnum::kNonFabricNtpNts },
        { "FabricSntpNts", TimeSynchronization::TimeSourceEnum::kFabricSntpNts },
        { "FabricNtpNts", TimeSynchronization::TimeSourceEnum::kFabricNtpNts },
        { "MixedNtpNts", TimeSynchronization::TimeSourceEnum::kMixedNtpNts },
        { "CloudSource", TimeSynchronization::TimeSourceEnum::kCloudSource },
        { "Ptp", TimeSynchronization::TimeSourceEnum::kPtp },
        { "Gnss", TimeSynchronization::TimeSourceEnum::kGnss },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Switch::SwitchFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<AdministratorCommissioning::CommissioningWindowStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AdministratorCommissioning::CommissioningWindowStatus> table = {
        { "WindowNotOpen", AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen },
        { "EnhancedWindowOpen", AdministratorCommissioning::CommissioningWindowStatus::kEnhancedWindowOpen },
        { "BasicWindowOpen", AdministratorCommissioning::CommissioningWindowStatus::kBasicWindowOpen },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<AdministratorCommissioning::StatusCode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AdministratorCommissioning::StatusCode> table = {
        { "Busy", AdministratorCommissioning::StatusCode::kBusy },
        { "PAKEParameterError", AdministratorCommissioning::StatusCode::kPAKEParameterError },
        { "WindowNotOpen", AdministratorCommissioning::StatusCode::kWindowNotOpen },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const OperationalCredentials::Structs::FabricDescriptor::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const OperationalCredentials::Structs::NOCStruct::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<OperationalCredentials::OperationalCertStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, OperationalCredentials::OperationalCertStatus> table = {
        { "Success", OperationalCredentials::OperationalCertStatus::kSuccess },
        { "InvalidPublicKey", OperationalCredentials::OperationalCertStatus::kInvalidPublicKey },
        { "InvalidNodeOpId", OperationalCredentials::OperationalCertStatus::kInvalidNodeOpId },
        { "InvalidNOC", OperationalCredentials::OperationalCertStatus::kInvalidNOC },
        { "MissingCsr", OperationalCredentials::OperationalCertStatus::kMissingCsr },
        { "TableFull", OperationalCredentials::OperationalCertStatus::kTableFull },
        { "InvalidAdminSubject", OperationalCredentials::OperationalCertStatus::kInvalidAdminSubject },
        { "FabricConflict", OperationalCredentials::OperationalCertStatus::kFabricConflict },
        { "LabelConflict", OperationalCredentials::OperationalCertStatus::kLabelConflict },
        { "InvalidFabricIndex", OperationalCredentials::OperationalCertStatus::kInvalidFabricIndex },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const GroupKeyManagement::Structs::GroupInfoMapStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const GroupKeyManagement::Structs::GroupKeyMapStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const GroupKeyManagement::Structs::GroupKeySetStruct::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<GroupKeyManagement::GroupKeySecurityPolicy> from_json(const nlohmann::json & value)
{
    const std::map<std::string, GroupKeyManagement::GroupKeySecurityPolicy> table = {
        { "TrustFirst", GroupKeyManagement::GroupKeySecurityPolicy::kTrustFirst },
        { "CacheAndSync", GroupKeyManagement::GroupKeySecurityPolicy::kCacheAndSync },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const FixedLabel::Structs::LabelStruct::Type& value) {
//    return "no struct support";
//}
//
//

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const UserLabel::Structs::LabelStruct::Type& value) {
//    return "no struct support";
//}
//
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ModeSelect::ModeSelectFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const ModeSelect::Structs::SemanticTag::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ModeSelect::Structs::ModeOptionStruct::Type& value) {
//    return "no struct support";
//}
//
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlCredentialRuleMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlCredentialRulesSupport, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlDaysMaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlDefaultConfigurationRegister, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlKeypadOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlKeypadProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlLocalProgrammingFeatures, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlManualOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRFIDOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRFIDProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRemoteOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRemoteProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DlSupportedOperatingModes, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DoorLockDayOfWeek, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<DoorLock::DoorLockFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const DoorLock::Structs::DlCredential::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<DoorLock::DlAlarmCode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlAlarmCode> table = {
        { "LockJammed", DoorLock::DlAlarmCode::kLockJammed },
        { "LockFactoryReset", DoorLock::DlAlarmCode::kLockFactoryReset },
        { "LockRadioPowerCycled", DoorLock::DlAlarmCode::kLockRadioPowerCycled },
        { "WrongCodeEntryLimit", DoorLock::DlAlarmCode::kWrongCodeEntryLimit },
        { "FrontEsceutcheonRemoved", DoorLock::DlAlarmCode::kFrontEsceutcheonRemoved },
        { "DoorForcedOpen", DoorLock::DlAlarmCode::kDoorForcedOpen },
        { "DoorAjar", DoorLock::DlAlarmCode::kDoorAjar },
        { "ForcedUser", DoorLock::DlAlarmCode::kForcedUser },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlCredentialRule> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlCredentialRule> table = {
        { "Single", DoorLock::DlCredentialRule::kSingle },
        { "Double", DoorLock::DlCredentialRule::kDouble },
        { "Tri", DoorLock::DlCredentialRule::kTri },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlCredentialType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlCredentialType> table = {
        { "ProgrammingPIN", DoorLock::DlCredentialType::kProgrammingPIN },
        { "Pin", DoorLock::DlCredentialType::kPin },
        { "Rfid", DoorLock::DlCredentialType::kRfid },
        { "Fingerprint", DoorLock::DlCredentialType::kFingerprint },
        { "FingerVein", DoorLock::DlCredentialType::kFingerVein },
        { "Face", DoorLock::DlCredentialType::kFace },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlDataOperationType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlDataOperationType> table = {
        { "Add", DoorLock::DlDataOperationType::kAdd },
        { "Clear", DoorLock::DlDataOperationType::kClear },
        { "Modify", DoorLock::DlDataOperationType::kModify },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlDoorState> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlDoorState> table = {
        { "DoorOpen", DoorLock::DlDoorState::kDoorOpen },
        { "DoorClosed", DoorLock::DlDoorState::kDoorClosed },
        { "DoorJammed", DoorLock::DlDoorState::kDoorJammed },
        { "DoorForcedOpen", DoorLock::DlDoorState::kDoorForcedOpen },
        { "DoorUnspecifiedError", DoorLock::DlDoorState::kDoorUnspecifiedError },
        { "DoorAjar", DoorLock::DlDoorState::kDoorAjar },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockDataType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlLockDataType> table = {
        { "Unspecified", DoorLock::DlLockDataType::kUnspecified },
        { "ProgrammingCode", DoorLock::DlLockDataType::kProgrammingCode },
        { "UserIndex", DoorLock::DlLockDataType::kUserIndex },
        { "WeekDaySchedule", DoorLock::DlLockDataType::kWeekDaySchedule },
        { "YearDaySchedule", DoorLock::DlLockDataType::kYearDaySchedule },
        { "HolidaySchedule", DoorLock::DlLockDataType::kHolidaySchedule },
        { "Pin", DoorLock::DlLockDataType::kPin },
        { "Rfid", DoorLock::DlLockDataType::kRfid },
        { "Fingerprint", DoorLock::DlLockDataType::kFingerprint },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockOperationType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlLockOperationType> table = {
        { "Lock", DoorLock::DlLockOperationType::kLock },
        { "Unlock", DoorLock::DlLockOperationType::kUnlock },
        { "NonAccessUserEvent", DoorLock::DlLockOperationType::kNonAccessUserEvent },
        { "ForcedUserEvent", DoorLock::DlLockOperationType::kForcedUserEvent },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockState> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlLockState> table = {
        { "NotFullyLocked", DoorLock::DlLockState::kNotFullyLocked },
        { "Locked", DoorLock::DlLockState::kLocked },
        { "Unlocked", DoorLock::DlLockState::kUnlocked },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlLockType> table = {
        { "DeadBolt", DoorLock::DlLockType::kDeadBolt },
        { "Magnetic", DoorLock::DlLockType::kMagnetic },
        { "Other", DoorLock::DlLockType::kOther },
        { "Mortise", DoorLock::DlLockType::kMortise },
        { "Rim", DoorLock::DlLockType::kRim },
        { "LatchBolt", DoorLock::DlLockType::kLatchBolt },
        { "CylindricalLock", DoorLock::DlLockType::kCylindricalLock },
        { "TubularLock", DoorLock::DlLockType::kTubularLock },
        { "InterconnectedLock", DoorLock::DlLockType::kInterconnectedLock },
        { "DeadLatch", DoorLock::DlLockType::kDeadLatch },
        { "DoorFurniture", DoorLock::DlLockType::kDoorFurniture },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperatingMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlOperatingMode> table = {
        { "Normal", DoorLock::DlOperatingMode::kNormal },
        { "Vacation", DoorLock::DlOperatingMode::kVacation },
        { "Privacy", DoorLock::DlOperatingMode::kPrivacy },
        { "NoRemoteLockUnlock", DoorLock::DlOperatingMode::kNoRemoteLockUnlock },
        { "Passage", DoorLock::DlOperatingMode::kPassage },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperationError> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlOperationError> table = {
        { "Unspecified", DoorLock::DlOperationError::kUnspecified },
        { "InvalidCredential", DoorLock::DlOperationError::kInvalidCredential },
        { "DisabledUserDenied", DoorLock::DlOperationError::kDisabledUserDenied },
        { "Restricted", DoorLock::DlOperationError::kRestricted },
        { "InsufficientBattery", DoorLock::DlOperationError::kInsufficientBattery },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperationSource> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlOperationSource> table = {
        { "Unspecified", DoorLock::DlOperationSource::kUnspecified },
        { "Manual", DoorLock::DlOperationSource::kManual },
        { "ProprietaryRemote", DoorLock::DlOperationSource::kProprietaryRemote },
        { "Keypad", DoorLock::DlOperationSource::kKeypad },
        { "Auto", DoorLock::DlOperationSource::kAuto },
        { "Button", DoorLock::DlOperationSource::kButton },
        { "Schedule", DoorLock::DlOperationSource::kSchedule },
        { "Remote", DoorLock::DlOperationSource::kRemote },
        { "Rfid", DoorLock::DlOperationSource::kRfid },
        { "Biometric", DoorLock::DlOperationSource::kBiometric },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlStatus> table = {
        { "Success", DoorLock::DlStatus::kSuccess },           { "Failure", DoorLock::DlStatus::kFailure },
        { "Duplicate", DoorLock::DlStatus::kDuplicate },       { "Occupied", DoorLock::DlStatus::kOccupied },
        { "InvalidField", DoorLock::DlStatus::kInvalidField }, { "ResourceExhausted", DoorLock::DlStatus::kResourceExhausted },
        { "NotFound", DoorLock::DlStatus::kNotFound },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlUserStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlUserStatus> table = {
        { "Available", DoorLock::DlUserStatus::kAvailable },
        { "OccupiedEnabled", DoorLock::DlUserStatus::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::DlUserStatus::kOccupiedDisabled },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlUserType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DlUserType> table = {
        { "UnrestrictedUser", DoorLock::DlUserType::kUnrestrictedUser },
        { "YearDayScheduleUser", DoorLock::DlUserType::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::DlUserType::kWeekDayScheduleUser },
        { "ProgrammingUser", DoorLock::DlUserType::kProgrammingUser },
        { "NonAccessUser", DoorLock::DlUserType::kNonAccessUser },
        { "ForcedUser", DoorLock::DlUserType::kForcedUser },
        { "DisposableUser", DoorLock::DlUserType::kDisposableUser },
        { "ExpiringUser", DoorLock::DlUserType::kExpiringUser },
        { "ScheduleRestrictedUser", DoorLock::DlUserType::kScheduleRestrictedUser },
        { "RemoteOnlyUser", DoorLock::DlUserType::kRemoteOnlyUser },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockOperationEventCode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DoorLockOperationEventCode> table = {
        { "UnknownOrMfgSpecific", DoorLock::DoorLockOperationEventCode::kUnknownOrMfgSpecific },
        { "Lock", DoorLock::DoorLockOperationEventCode::kLock },
        { "Unlock", DoorLock::DoorLockOperationEventCode::kUnlock },
        { "LockInvalidPinOrId", DoorLock::DoorLockOperationEventCode::kLockInvalidPinOrId },
        { "LockInvalidSchedule", DoorLock::DoorLockOperationEventCode::kLockInvalidSchedule },
        { "UnlockInvalidPinOrId", DoorLock::DoorLockOperationEventCode::kUnlockInvalidPinOrId },
        { "UnlockInvalidSchedule", DoorLock::DoorLockOperationEventCode::kUnlockInvalidSchedule },
        { "OneTouchLock", DoorLock::DoorLockOperationEventCode::kOneTouchLock },
        { "KeyLock", DoorLock::DoorLockOperationEventCode::kKeyLock },
        { "KeyUnlock", DoorLock::DoorLockOperationEventCode::kKeyUnlock },
        { "AutoLock", DoorLock::DoorLockOperationEventCode::kAutoLock },
        { "ScheduleLock", DoorLock::DoorLockOperationEventCode::kScheduleLock },
        { "ScheduleUnlock", DoorLock::DoorLockOperationEventCode::kScheduleUnlock },
        { "ManualLock", DoorLock::DoorLockOperationEventCode::kManualLock },
        { "ManualUnlock", DoorLock::DoorLockOperationEventCode::kManualUnlock },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockProgrammingEventCode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DoorLockProgrammingEventCode> table = {
        { "UnknownOrMfgSpecific", DoorLock::DoorLockProgrammingEventCode::kUnknownOrMfgSpecific },
        { "MasterCodeChanged", DoorLock::DoorLockProgrammingEventCode::kMasterCodeChanged },
        { "PinAdded", DoorLock::DoorLockProgrammingEventCode::kPinAdded },
        { "PinDeleted", DoorLock::DoorLockProgrammingEventCode::kPinDeleted },
        { "PinChanged", DoorLock::DoorLockProgrammingEventCode::kPinChanged },
        { "IdAdded", DoorLock::DoorLockProgrammingEventCode::kIdAdded },
        { "IdDeleted", DoorLock::DoorLockProgrammingEventCode::kIdDeleted },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockSetPinOrIdStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DoorLockSetPinOrIdStatus> table = {
        { "Success", DoorLock::DoorLockSetPinOrIdStatus::kSuccess },
        { "GeneralFailure", DoorLock::DoorLockSetPinOrIdStatus::kGeneralFailure },
        { "MemoryFull", DoorLock::DoorLockSetPinOrIdStatus::kMemoryFull },
        { "DuplicateCodeError", DoorLock::DoorLockSetPinOrIdStatus::kDuplicateCodeError },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserStatus> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DoorLockUserStatus> table = {
        { "Available", DoorLock::DoorLockUserStatus::kAvailable },
        { "OccupiedEnabled", DoorLock::DoorLockUserStatus::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::DoorLockUserStatus::kOccupiedDisabled },
        { "NotSupported", DoorLock::DoorLockUserStatus::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, DoorLock::DoorLockUserType> table = {
        { "Unrestricted", DoorLock::DoorLockUserType::kUnrestricted },
        { "YearDayScheduleUser", DoorLock::DoorLockUserType::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::DoorLockUserType::kWeekDayScheduleUser },
        { "MasterUser", DoorLock::DoorLockUserType::kMasterUser },
        { "NonAccessUser", DoorLock::DoorLockUserType::kNonAccessUser },
        { "NotSupported", DoorLock::DoorLockUserType::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<WindowCovering::ConfigStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<WindowCovering::Feature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<WindowCovering::Mode, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<WindowCovering::OperationalStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<WindowCovering::SafetyStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<WindowCovering::EndProductType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WindowCovering::EndProductType> table = {
        { "RollerShade", WindowCovering::EndProductType::kRollerShade },
        { "RomanShade", WindowCovering::EndProductType::kRomanShade },
        { "BalloonShade", WindowCovering::EndProductType::kBalloonShade },
        { "WovenWood", WindowCovering::EndProductType::kWovenWood },
        { "PleatedShade", WindowCovering::EndProductType::kPleatedShade },
        { "CellularShade", WindowCovering::EndProductType::kCellularShade },
        { "LayeredShade", WindowCovering::EndProductType::kLayeredShade },
        { "LayeredShade2D", WindowCovering::EndProductType::kLayeredShade2D },
        { "SheerShade", WindowCovering::EndProductType::kSheerShade },
        { "TiltOnlyInteriorBlind", WindowCovering::EndProductType::kTiltOnlyInteriorBlind },
        { "InteriorBlind", WindowCovering::EndProductType::kInteriorBlind },
        { "VerticalBlindStripCurtain", WindowCovering::EndProductType::kVerticalBlindStripCurtain },
        { "InteriorVenetianBlind", WindowCovering::EndProductType::kInteriorVenetianBlind },
        { "ExteriorVenetianBlind", WindowCovering::EndProductType::kExteriorVenetianBlind },
        { "LateralLeftCurtain", WindowCovering::EndProductType::kLateralLeftCurtain },
        { "LateralRightCurtain", WindowCovering::EndProductType::kLateralRightCurtain },
        { "CentralCurtain", WindowCovering::EndProductType::kCentralCurtain },
        { "RollerShutter", WindowCovering::EndProductType::kRollerShutter },
        { "ExteriorVerticalScreen", WindowCovering::EndProductType::kExteriorVerticalScreen },
        { "AwningTerracePatio", WindowCovering::EndProductType::kAwningTerracePatio },
        { "AwningVerticalScreen", WindowCovering::EndProductType::kAwningVerticalScreen },
        { "TiltOnlyPergola", WindowCovering::EndProductType::kTiltOnlyPergola },
        { "SwingingShutter", WindowCovering::EndProductType::kSwingingShutter },
        { "SlidingShutter", WindowCovering::EndProductType::kSlidingShutter },
        { "Unknown", WindowCovering::EndProductType::kUnknown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<WindowCovering::Type> from_json(const nlohmann::json & value)
{
    const std::map<std::string, WindowCovering::Type> table = {
        { "RollerShade", WindowCovering::Type::kRollerShade },
        { "RollerShade2Motor", WindowCovering::Type::kRollerShade2Motor },
        { "RollerShadeExterior", WindowCovering::Type::kRollerShadeExterior },
        { "RollerShadeExterior2Motor", WindowCovering::Type::kRollerShadeExterior2Motor },
        { "Drapery", WindowCovering::Type::kDrapery },
        { "Awning", WindowCovering::Type::kAwning },
        { "Shutter", WindowCovering::Type::kShutter },
        { "TiltBlindTiltOnly", WindowCovering::Type::kTiltBlindTiltOnly },
        { "TiltBlindLiftAndTilt", WindowCovering::Type::kTiltBlindLiftAndTilt },
        { "ProjectorScreen", WindowCovering::Type::kProjectorScreen },
        { "Unknown", WindowCovering::Type::kUnknown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<PumpConfigurationAndControl::PumpStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<PumpConfigurationAndControl::PumpControlMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PumpConfigurationAndControl::PumpControlMode> table = {
        { "ConstantSpeed", PumpConfigurationAndControl::PumpControlMode::kConstantSpeed },
        { "ConstantPressure", PumpConfigurationAndControl::PumpControlMode::kConstantPressure },
        { "ProportionalPressure", PumpConfigurationAndControl::PumpControlMode::kProportionalPressure },
        { "ConstantFlow", PumpConfigurationAndControl::PumpControlMode::kConstantFlow },
        { "ConstantTemperature", PumpConfigurationAndControl::PumpControlMode::kConstantTemperature },
        { "Automatic", PumpConfigurationAndControl::PumpControlMode::kAutomatic },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<PumpConfigurationAndControl::PumpOperationMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, PumpConfigurationAndControl::PumpOperationMode> table = {
        { "Normal", PumpConfigurationAndControl::PumpOperationMode::kNormal },
        { "Minimum", PumpConfigurationAndControl::PumpOperationMode::kMinimum },
        { "Maximum", PumpConfigurationAndControl::PumpOperationMode::kMaximum },
        { "Local", PumpConfigurationAndControl::PumpOperationMode::kLocal },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Thermostat::DayOfWeek, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Thermostat::ModeForSequence, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Thermostat::ThermostatFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Thermostat::Structs::ThermostatScheduleTransition::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<Thermostat::SetpointAdjustMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Thermostat::SetpointAdjustMode> table = {
        { "HeatSetpoint", Thermostat::SetpointAdjustMode::kHeatSetpoint },
        { "CoolSetpoint", Thermostat::SetpointAdjustMode::kCoolSetpoint },
        { "HeatAndCoolSetpoints", Thermostat::SetpointAdjustMode::kHeatAndCoolSetpoints },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatControlSequence> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Thermostat::ThermostatControlSequence> table = {
        { "CoolingOnly", Thermostat::ThermostatControlSequence::kCoolingOnly },
        { "CoolingWithReheat", Thermostat::ThermostatControlSequence::kCoolingWithReheat },
        { "HeatingOnly", Thermostat::ThermostatControlSequence::kHeatingOnly },
        { "HeatingWithReheat", Thermostat::ThermostatControlSequence::kHeatingWithReheat },
        { "CoolingAndHeating", Thermostat::ThermostatControlSequence::kCoolingAndHeating },
        { "CoolingAndHeatingWithReheat", Thermostat::ThermostatControlSequence::kCoolingAndHeatingWithReheat },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatRunningMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Thermostat::ThermostatRunningMode> table = {
        { "Off", Thermostat::ThermostatRunningMode::kOff },
        { "Cool", Thermostat::ThermostatRunningMode::kCool },
        { "Heat", Thermostat::ThermostatRunningMode::kHeat },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatSystemMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Thermostat::ThermostatSystemMode> table = {
        { "Off", Thermostat::ThermostatSystemMode::kOff },
        { "Auto", Thermostat::ThermostatSystemMode::kAuto },
        { "Cool", Thermostat::ThermostatSystemMode::kCool },
        { "Heat", Thermostat::ThermostatSystemMode::kHeat },
        { "EmergencyHeating", Thermostat::ThermostatSystemMode::kEmergencyHeating },
        { "Precooling", Thermostat::ThermostatSystemMode::kPrecooling },
        { "FanOnly", Thermostat::ThermostatSystemMode::kFanOnly },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<FanControl::FanControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<FanControl::RockSupportMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<FanControl::WindSettingMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<FanControl::WindSupportMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<FanControl::FanModeSequenceType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, FanControl::FanModeSequenceType> table = {
        { "OffLowMedHigh", FanControl::FanModeSequenceType::kOffLowMedHigh },
        { "OffLowHigh", FanControl::FanModeSequenceType::kOffLowHigh },
        { "OffLowMedHighAuto", FanControl::FanModeSequenceType::kOffLowMedHighAuto },
        { "OffLowHighAuto", FanControl::FanModeSequenceType::kOffLowHighAuto },
        { "OffOnAuto", FanControl::FanModeSequenceType::kOffOnAuto },
        { "OffOn", FanControl::FanModeSequenceType::kOffOn },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, FanControl::FanModeType> table = {
        { "Off", FanControl::FanModeType::kOff },       { "Low", FanControl::FanModeType::kLow },
        { "Medium", FanControl::FanModeType::kMedium }, { "High", FanControl::FanModeType::kHigh },
        { "On", FanControl::FanModeType::kOn },         { "Auto", FanControl::FanModeType::kAuto },
        { "Smart", FanControl::FanModeType::kSmart },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorCapabilities, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorLoopUpdateFlags, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<ColorControl::ColorLoopAction> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::ColorLoopAction> table = {
        { "Deactivate", ColorControl::ColorLoopAction::kDeactivate },
        { "ActivateFromColorLoopStartEnhancedHue", ColorControl::ColorLoopAction::kActivateFromColorLoopStartEnhancedHue },
        { "ActivateFromEnhancedCurrentHue", ColorControl::ColorLoopAction::kActivateFromEnhancedCurrentHue },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorLoopDirection> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::ColorLoopDirection> table = {
        { "DecrementHue", ColorControl::ColorLoopDirection::kDecrementHue },
        { "IncrementHue", ColorControl::ColorLoopDirection::kIncrementHue },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::ColorMode> table = {
        { "CurrentHueAndCurrentSaturation", ColorControl::ColorMode::kCurrentHueAndCurrentSaturation },
        { "CurrentXAndCurrentY", ColorControl::ColorMode::kCurrentXAndCurrentY },
        { "ColorTemperature", ColorControl::ColorMode::kColorTemperature },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueDirection> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::HueDirection> table = {
        { "ShortestDistance", ColorControl::HueDirection::kShortestDistance },
        { "LongestDistance", ColorControl::HueDirection::kLongestDistance },
        { "Up", ColorControl::HueDirection::kUp },
        { "Down", ColorControl::HueDirection::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueMoveMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::HueMoveMode> table = {
        { "Stop", ColorControl::HueMoveMode::kStop },
        { "Up", ColorControl::HueMoveMode::kUp },
        { "Down", ColorControl::HueMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueStepMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::HueStepMode> table = {
        { "Up", ColorControl::HueStepMode::kUp },
        { "Down", ColorControl::HueStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationMoveMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::SaturationMoveMode> table = {
        { "Stop", ColorControl::SaturationMoveMode::kStop },
        { "Up", ColorControl::SaturationMoveMode::kUp },
        { "Down", ColorControl::SaturationMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationStepMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ColorControl::SaturationStepMode> table = {
        { "Up", ColorControl::SaturationStepMode::kUp },
        { "Down", ColorControl::SaturationStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<IlluminanceMeasurement::LightSensorType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, IlluminanceMeasurement::LightSensorType> table = {
        { "Photodiode", IlluminanceMeasurement::LightSensorType::kPhotodiode },
        { "Cmos", IlluminanceMeasurement::LightSensorType::kCmos },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<PressureMeasurement::PressureFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<Channel::ChannelFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const Channel::Structs::ChannelInfo::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const Channel::Structs::LineupInfo::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<Channel::ChannelStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Channel::ChannelStatusEnum> table = {
        { "Success", Channel::ChannelStatusEnum::kSuccess },
        { "MultipleMatches", Channel::ChannelStatusEnum::kMultipleMatches },
        { "NoMatches", Channel::ChannelStatusEnum::kNoMatches },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<Channel::LineupInfoTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, Channel::LineupInfoTypeEnum> table = {
        { "Mso", Channel::LineupInfoTypeEnum::kMso },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const TargetNavigator::Structs::TargetInfo::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<TargetNavigator::TargetNavigatorStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, TargetNavigator::TargetNavigatorStatusEnum> table = {
        { "Success", TargetNavigator::TargetNavigatorStatusEnum::kSuccess },
        { "TargetNotFound", TargetNavigator::TargetNavigatorStatusEnum::kTargetNotFound },
        { "NotAllowed", TargetNavigator::TargetNavigatorStatusEnum::kNotAllowed },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<MediaPlayback::MediaPlaybackFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const MediaPlayback::Structs::PlaybackPosition::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<MediaPlayback::MediaPlaybackStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, MediaPlayback::MediaPlaybackStatusEnum> table = {
        { "Success", MediaPlayback::MediaPlaybackStatusEnum::kSuccess },
        { "InvalidStateForCommand", MediaPlayback::MediaPlaybackStatusEnum::kInvalidStateForCommand },
        { "NotAllowed", MediaPlayback::MediaPlaybackStatusEnum::kNotAllowed },
        { "NotActive", MediaPlayback::MediaPlaybackStatusEnum::kNotActive },
        { "SpeedOutOfRange", MediaPlayback::MediaPlaybackStatusEnum::kSpeedOutOfRange },
        { "SeekOutOfRange", MediaPlayback::MediaPlaybackStatusEnum::kSeekOutOfRange },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<MediaPlayback::PlaybackStateEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, MediaPlayback::PlaybackStateEnum> table = {
        { "Playing", MediaPlayback::PlaybackStateEnum::kPlaying },
        { "Paused", MediaPlayback::PlaybackStateEnum::kPaused },
        { "NotPlaying", MediaPlayback::PlaybackStateEnum::kNotPlaying },
        { "Buffering", MediaPlayback::PlaybackStateEnum::kBuffering },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<MediaInput::MediaInputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const MediaInput::Structs::InputInfo::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<MediaInput::InputTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, MediaInput::InputTypeEnum> table = {
        { "Internal", MediaInput::InputTypeEnum::kInternal }, { "Aux", MediaInput::InputTypeEnum::kAux },
        { "Coax", MediaInput::InputTypeEnum::kCoax },         { "Composite", MediaInput::InputTypeEnum::kComposite },
        { "Hdmi", MediaInput::InputTypeEnum::kHdmi },         { "Input", MediaInput::InputTypeEnum::kInput },
        { "Line", MediaInput::InputTypeEnum::kLine },         { "Optical", MediaInput::InputTypeEnum::kOptical },
        { "Video", MediaInput::InputTypeEnum::kVideo },       { "Scart", MediaInput::InputTypeEnum::kScart },
        { "Usb", MediaInput::InputTypeEnum::kUsb },           { "Other", MediaInput::InputTypeEnum::kOther },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<KeypadInput::KeypadInputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<KeypadInput::CecKeyCode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, KeypadInput::CecKeyCode> table = {
        { "Select", KeypadInput::CecKeyCode::kSelect },
        { "Up", KeypadInput::CecKeyCode::kUp },
        { "Down", KeypadInput::CecKeyCode::kDown },
        { "Left", KeypadInput::CecKeyCode::kLeft },
        { "Right", KeypadInput::CecKeyCode::kRight },
        { "RightUp", KeypadInput::CecKeyCode::kRightUp },
        { "RightDown", KeypadInput::CecKeyCode::kRightDown },
        { "LeftUp", KeypadInput::CecKeyCode::kLeftUp },
        { "LeftDown", KeypadInput::CecKeyCode::kLeftDown },
        { "RootMenu", KeypadInput::CecKeyCode::kRootMenu },
        { "SetupMenu", KeypadInput::CecKeyCode::kSetupMenu },
        { "ContentsMenu", KeypadInput::CecKeyCode::kContentsMenu },
        { "FavoriteMenu", KeypadInput::CecKeyCode::kFavoriteMenu },
        { "Exit", KeypadInput::CecKeyCode::kExit },
        { "MediaTopMenu", KeypadInput::CecKeyCode::kMediaTopMenu },
        { "MediaContextSensitiveMenu", KeypadInput::CecKeyCode::kMediaContextSensitiveMenu },
        { "NumberEntryMode", KeypadInput::CecKeyCode::kNumberEntryMode },
        { "Number11", KeypadInput::CecKeyCode::kNumber11 },
        { "Number12", KeypadInput::CecKeyCode::kNumber12 },
        { "Number0OrNumber10", KeypadInput::CecKeyCode::kNumber0OrNumber10 },
        { "Numbers1", KeypadInput::CecKeyCode::kNumbers1 },
        { "Numbers2", KeypadInput::CecKeyCode::kNumbers2 },
        { "Numbers3", KeypadInput::CecKeyCode::kNumbers3 },
        { "Numbers4", KeypadInput::CecKeyCode::kNumbers4 },
        { "Numbers5", KeypadInput::CecKeyCode::kNumbers5 },
        { "Numbers6", KeypadInput::CecKeyCode::kNumbers6 },
        { "Numbers7", KeypadInput::CecKeyCode::kNumbers7 },
        { "Numbers8", KeypadInput::CecKeyCode::kNumbers8 },
        { "Numbers9", KeypadInput::CecKeyCode::kNumbers9 },
        { "Dot", KeypadInput::CecKeyCode::kDot },
        { "Enter", KeypadInput::CecKeyCode::kEnter },
        { "Clear", KeypadInput::CecKeyCode::kClear },
        { "NextFavorite", KeypadInput::CecKeyCode::kNextFavorite },
        { "ChannelUp", KeypadInput::CecKeyCode::kChannelUp },
        { "ChannelDown", KeypadInput::CecKeyCode::kChannelDown },
        { "PreviousChannel", KeypadInput::CecKeyCode::kPreviousChannel },
        { "SoundSelect", KeypadInput::CecKeyCode::kSoundSelect },
        { "InputSelect", KeypadInput::CecKeyCode::kInputSelect },
        { "DisplayInformation", KeypadInput::CecKeyCode::kDisplayInformation },
        { "Help", KeypadInput::CecKeyCode::kHelp },
        { "PageUp", KeypadInput::CecKeyCode::kPageUp },
        { "PageDown", KeypadInput::CecKeyCode::kPageDown },
        { "Power", KeypadInput::CecKeyCode::kPower },
        { "VolumeUp", KeypadInput::CecKeyCode::kVolumeUp },
        { "VolumeDown", KeypadInput::CecKeyCode::kVolumeDown },
        { "Mute", KeypadInput::CecKeyCode::kMute },
        { "Play", KeypadInput::CecKeyCode::kPlay },
        { "Stop", KeypadInput::CecKeyCode::kStop },
        { "Pause", KeypadInput::CecKeyCode::kPause },
        { "Record", KeypadInput::CecKeyCode::kRecord },
        { "Rewind", KeypadInput::CecKeyCode::kRewind },
        { "FastForward", KeypadInput::CecKeyCode::kFastForward },
        { "Eject", KeypadInput::CecKeyCode::kEject },
        { "Forward", KeypadInput::CecKeyCode::kForward },
        { "Backward", KeypadInput::CecKeyCode::kBackward },
        { "StopRecord", KeypadInput::CecKeyCode::kStopRecord },
        { "PauseRecord", KeypadInput::CecKeyCode::kPauseRecord },
        { "Reserved", KeypadInput::CecKeyCode::kReserved },
        { "Angle", KeypadInput::CecKeyCode::kAngle },
        { "SubPicture", KeypadInput::CecKeyCode::kSubPicture },
        { "VideoOnDemand", KeypadInput::CecKeyCode::kVideoOnDemand },
        { "ElectronicProgramGuide", KeypadInput::CecKeyCode::kElectronicProgramGuide },
        { "TimerProgramming", KeypadInput::CecKeyCode::kTimerProgramming },
        { "InitialConfiguration", KeypadInput::CecKeyCode::kInitialConfiguration },
        { "SelectBroadcastType", KeypadInput::CecKeyCode::kSelectBroadcastType },
        { "SelectSoundPresentation", KeypadInput::CecKeyCode::kSelectSoundPresentation },
        { "PlayFunction", KeypadInput::CecKeyCode::kPlayFunction },
        { "PausePlayFunction", KeypadInput::CecKeyCode::kPausePlayFunction },
        { "RecordFunction", KeypadInput::CecKeyCode::kRecordFunction },
        { "PauseRecordFunction", KeypadInput::CecKeyCode::kPauseRecordFunction },
        { "StopFunction", KeypadInput::CecKeyCode::kStopFunction },
        { "MuteFunction", KeypadInput::CecKeyCode::kMuteFunction },
        { "RestoreVolumeFunction", KeypadInput::CecKeyCode::kRestoreVolumeFunction },
        { "TuneFunction", KeypadInput::CecKeyCode::kTuneFunction },
        { "SelectMediaFunction", KeypadInput::CecKeyCode::kSelectMediaFunction },
        { "SelectAvInputFunction", KeypadInput::CecKeyCode::kSelectAvInputFunction },
        { "SelectAudioInputFunction", KeypadInput::CecKeyCode::kSelectAudioInputFunction },
        { "PowerToggleFunction", KeypadInput::CecKeyCode::kPowerToggleFunction },
        { "PowerOffFunction", KeypadInput::CecKeyCode::kPowerOffFunction },
        { "PowerOnFunction", KeypadInput::CecKeyCode::kPowerOnFunction },
        { "F1Blue", KeypadInput::CecKeyCode::kF1Blue },
        { "F2Red", KeypadInput::CecKeyCode::kF2Red },
        { "F3Green", KeypadInput::CecKeyCode::kF3Green },
        { "F4Yellow", KeypadInput::CecKeyCode::kF4Yellow },
        { "F5", KeypadInput::CecKeyCode::kF5 },
        { "Data", KeypadInput::CecKeyCode::kData },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<KeypadInput::KeypadInputStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, KeypadInput::KeypadInputStatusEnum> table = {
        { "Success", KeypadInput::KeypadInputStatusEnum::kSuccess },
        { "UnsupportedKey", KeypadInput::KeypadInputStatusEnum::kUnsupportedKey },
        { "InvalidKeyInCurrentState", KeypadInput::KeypadInputStatusEnum::kInvalidKeyInCurrentState },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ContentLauncher::ContentLauncherFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ContentLauncher::SupportedStreamingProtocol, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::Dimension::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::AdditionalInfo::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::Parameter::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::ContentSearch::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::StyleInformation::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ContentLauncher::Structs::BrandingInformation::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<ContentLauncher::ContentLaunchStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ContentLauncher::ContentLaunchStatusEnum> table = {
        { "Success", ContentLauncher::ContentLaunchStatusEnum::kSuccess },
        { "UrlNotAvailable", ContentLauncher::ContentLaunchStatusEnum::kUrlNotAvailable },
        { "AuthFailed", ContentLauncher::ContentLaunchStatusEnum::kAuthFailed },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ContentLauncher::MetricTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ContentLauncher::MetricTypeEnum> table = {
        { "Pixels", ContentLauncher::MetricTypeEnum::kPixels },
        { "Percentage", ContentLauncher::MetricTypeEnum::kPercentage },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ContentLauncher::ParameterEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ContentLauncher::ParameterEnum> table = {
        { "Actor", ContentLauncher::ParameterEnum::kActor },
        { "Channel", ContentLauncher::ParameterEnum::kChannel },
        { "Character", ContentLauncher::ParameterEnum::kCharacter },
        { "Director", ContentLauncher::ParameterEnum::kDirector },
        { "Event", ContentLauncher::ParameterEnum::kEvent },
        { "Franchise", ContentLauncher::ParameterEnum::kFranchise },
        { "Genre", ContentLauncher::ParameterEnum::kGenre },
        { "League", ContentLauncher::ParameterEnum::kLeague },
        { "Popularity", ContentLauncher::ParameterEnum::kPopularity },
        { "Provider", ContentLauncher::ParameterEnum::kProvider },
        { "Sport", ContentLauncher::ParameterEnum::kSport },
        { "SportsTeam", ContentLauncher::ParameterEnum::kSportsTeam },
        { "Type", ContentLauncher::ParameterEnum::kType },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<AudioOutput::AudioOutputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const AudioOutput::Structs::OutputInfo::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<AudioOutput::OutputTypeEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, AudioOutput::OutputTypeEnum> table = {
        { "Hdmi", AudioOutput::OutputTypeEnum::kHdmi },         { "Bt", AudioOutput::OutputTypeEnum::kBt },
        { "Optical", AudioOutput::OutputTypeEnum::kOptical },   { "Headphone", AudioOutput::OutputTypeEnum::kHeadphone },
        { "Internal", AudioOutput::OutputTypeEnum::kInternal }, { "Other", AudioOutput::OutputTypeEnum::kOther },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//
// template<> inline nlohmann::json to_json(const chip::BitFlags<ApplicationLauncher::ApplicationLauncherFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const ApplicationLauncher::Structs::Application::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const ApplicationLauncher::Structs::ApplicationEP::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<ApplicationLauncher::ApplicationLauncherStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ApplicationLauncher::ApplicationLauncherStatusEnum> table = {
        { "Success", ApplicationLauncher::ApplicationLauncherStatusEnum::kSuccess },
        { "AppNotAvailable", ApplicationLauncher::ApplicationLauncherStatusEnum::kAppNotAvailable },
        { "SystemBusy", ApplicationLauncher::ApplicationLauncherStatusEnum::kSystemBusy },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const ApplicationBasic::Structs::ApplicationBasicApplication::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<ApplicationBasic::ApplicationStatusEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ApplicationBasic::ApplicationStatusEnum> table = {
        { "Stopped", ApplicationBasic::ApplicationStatusEnum::kStopped },
        { "ActiveVisibleFocus", ApplicationBasic::ApplicationStatusEnum::kActiveVisibleFocus },
        { "ActiveHidden", ApplicationBasic::ApplicationStatusEnum::kActiveHidden },
        { "ActiveVisibleNotFocus", ApplicationBasic::ApplicationStatusEnum::kActiveVisibleNotFocus },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

//

/***************************** Struct Converters **************/
//

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap16MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap32MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap64MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap8MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//
// template<> inline nlohmann::json to_json(const chip::BitFlags<UnitTesting::SimpleBitmap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}

//

/***************************** Struct Converters **************/
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::SimpleStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::TestFabricScoped::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::NullablesAndOptionalsStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::NestedStruct::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::NestedStructList::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::DoubleNestedStructList::Type& value) {
//    return "no struct support";
//}
//
//
// template<> inline nlohmann::json to_json(const UnitTesting::Structs::TestListStructOctet::Type& value) {
//    return "no struct support";
//}
//
//

template <>
inline std::optional<UnitTesting::SimpleEnum> from_json(const nlohmann::json & value)
{
    const std::map<std::string, UnitTesting::SimpleEnum> table = {
        { "Unspecified", UnitTesting::SimpleEnum::kUnspecified },
        { "ValueA", UnitTesting::SimpleEnum::kValueA },
        { "ValueB", UnitTesting::SimpleEnum::kValueB },
        { "ValueC", UnitTesting::SimpleEnum::kValueC },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
//

/***************************** Struct Converters **************/
//

template <>
inline std::optional<FaultInjection::FaultType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, FaultInjection::FaultType> table = {
        { "Unspecified", FaultInjection::FaultType::kUnspecified }, { "SystemFault", FaultInjection::FaultType::kSystemFault },
        { "InetFault", FaultInjection::FaultType::kInetFault },     { "ChipFault", FaultInjection::FaultType::kChipFault },
        { "CertFault", FaultInjection::FaultType::kCertFault },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}

// ZCL global enums
template <>
inline std::optional<ZclEnumColorMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ZclEnumColorMode> table = {
        { "CurrentHueAndCurrentSaturation", ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION },
        { "CurrentXAndCurrentY", ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y },
        { "ColorTemperature", ZCL_COLOR_MODE_COLOR_TEMPERATURE },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
template <>
inline std::optional<ZclEnumEnhancedColorMode> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ZclEnumEnhancedColorMode> table = {
        { "CurrentHueAndCurrentSaturation", ZCL_ENHANCED_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION },
        { "CurrentXAndCurrentY", ZCL_ENHANCED_COLOR_MODE_CURRENT_X_AND_CURRENT_Y },
        { "ColorTemperature", ZCL_ENHANCED_COLOR_MODE_COLOR_TEMPERATURE },
        { "EnhancedCurrentHueAndCurrentSaturation", ZCL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}
// Because some bitmaps are defined as <enum> in Matter xml files, this handles translating bitmaps for them.
// TODO: Check if is enum instead of LevelControlOptions
std::optional<uint8_t> from_json_LevelControlOptions(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["ExecuteIfOff"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF;
        }
        if (value["CoupleColorTempToLevel"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LEVEL_CONTROL_OPTIONS_COUPLE_COLOR_TEMP_TO_LEVEL;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}
template <>
inline std::optional<ZclEnumOccupancySensorType> from_json(const nlohmann::json & value)
{
    const std::map<std::string, ZclEnumOccupancySensorType> table = {
        { "PIR", ZCL_OCCUPANCY_SENSOR_TYPE_PIR },
        { "Ultrasonic", ZCL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC },
        { "pirAndUltrasonic", ZCL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC },
        { "physicalContact", ZCL_OCCUPANCY_SENSOR_TYPE_PHYSICAL_CONTACT },
    };

    auto i = table.find(value);
    if (i != table.end())
    {
        return i->second;
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_AlarmMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["GeneralHwFault"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ALARM_MASK_GENERAL_HW_FAULT;
        }
        if (value["GeneralSwFault"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ALARM_MASK_GENERAL_SW_FAULT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ApplicationLauncherFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["ApplicationPlatform"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_APPLICATION_LAUNCHER_FEATURE_APPLICATION_PLATFORM;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_AudioOutputFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["NameUpdates"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_AUDIO_OUTPUT_FEATURE_NAME_UPDATES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_BallastStatus(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["NonOperational"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BALLAST_STATUS_NON_OPERATIONAL;
        }
        if (value["LampNotInSocket"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BALLAST_STATUS_LAMP_NOT_IN_SOCKET;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_BarrierControlCapabilities(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["PartialBarrier"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BARRIER_CONTROL_CAPABILITIES_PARTIAL_BARRIER;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_BarrierControlSafetyStatus(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["RemoteLockout"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BARRIER_CONTROL_SAFETY_STATUS_REMOTE_LOCKOUT;
        }
        if (value["TemperDetected"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BARRIER_CONTROL_SAFETY_STATUS_TEMPER_DETECTED;
        }
        if (value["FailedCommunication"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BARRIER_CONTROL_SAFETY_STATUS_FAILED_COMMUNICATION;
        }
        if (value["PositionFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BARRIER_CONTROL_SAFETY_STATUS_POSITION_FAILURE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_BatteryAlarmMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["VoltageTooLow"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BATTERY_ALARM_MASK_VOLTAGE_TOO_LOW;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_Bitmap16MaskMap(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["MaskVal1"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP16_MASK_MAP_MASK_VAL1;
        }
        if (value["MaskVal2"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP16_MASK_MAP_MASK_VAL2;
        }
        if (value["MaskVal3"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP16_MASK_MAP_MASK_VAL3;
        }
        if (value["MaskVal4"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP16_MASK_MAP_MASK_VAL4;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_Bitmap32MaskMap(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["MaskVal1"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP32_MASK_MAP_MASK_VAL1;
        }
        if (value["MaskVal2"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP32_MASK_MAP_MASK_VAL2;
        }
        if (value["MaskVal3"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP32_MASK_MAP_MASK_VAL3;
        }
        if (value["MaskVal4"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP32_MASK_MAP_MASK_VAL4;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint64_t> from_json_Bitmap64MaskMap(const nlohmann::json & value)
{
    uint64_t bitmap_value = 0x00;
    try
    {
        if (value["MaskVal1"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP64_MASK_MAP_MASK_VAL1;
        }
        if (value["MaskVal2"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP64_MASK_MAP_MASK_VAL2;
        }
        if (value["MaskVal3"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP64_MASK_MAP_MASK_VAL3;
        }
        if (value["MaskVal4"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP64_MASK_MAP_MASK_VAL4;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_Bitmap8MaskMap(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["MaskVal1"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP8_MASK_MAP_MASK_VAL1;
        }
        if (value["MaskVal2"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP8_MASK_MAP_MASK_VAL2;
        }
        if (value["MaskVal3"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP8_MASK_MAP_MASK_VAL3;
        }
        if (value["MaskVal4"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_BITMAP8_MASK_MAP_MASK_VAL4;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ChannelFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["ChannelList"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CHANNEL_FEATURE_CHANNEL_LIST;
        }
        if (value["LineupInfo"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CHANNEL_FEATURE_LINEUP_INFO;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_ColorCapabilities(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["HueSaturationSupported"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CAPABILITIES_HUE_SATURATION_SUPPORTED;
        }
        if (value["EnhancedHueSupported"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CAPABILITIES_ENHANCED_HUE_SUPPORTED;
        }
        if (value["ColorLoopSupported"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CAPABILITIES_COLOR_LOOP_SUPPORTED;
        }
        if (value["XYSupported"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CAPABILITIES_XY_ATTRIBUTES_SUPPORTED;
        }
        if (value["ColorTemperatureSupported"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CAPABILITIES_COLOR_TEMPERATURE_SUPPORTED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ColorControlFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["HueAndSaturation"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CONTROL_FEATURE_HUE_AND_SATURATION;
        }
        if (value["EnhancedHue"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CONTROL_FEATURE_ENHANCED_HUE;
        }
        if (value["ColorLoop"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CONTROL_FEATURE_COLOR_LOOP;
        }
        if (value["Xy"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CONTROL_FEATURE_XY;
        }
        if (value["ColorTemperatureMireds"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_CONTROL_FEATURE_COLOR_TEMPERATURE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ColorLoopUpdateFlags(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["UpdateAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_LOOP_UPDATE_FLAGS_UPDATE_ACTION;
        }
        if (value["UpdateDirection"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_LOOP_UPDATE_FLAGS_UPDATE_DIRECTION;
        }
        if (value["UpdateTime"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_LOOP_UPDATE_FLAGS_UPDATE_TIME;
        }
        if (value["UpdateStartHue"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COLOR_LOOP_UPDATE_FLAGS_UPDATE_START_HUE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_CommandBits(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["InstantAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_INSTANT_ACTION;
        }
        if (value["InstantActionWithTransition"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_INSTANT_ACTION_WITH_TRANSITION;
        }
        if (value["StartAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_START_ACTION;
        }
        if (value["StartActionWithDuration"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_START_ACTION_WITH_DURATION;
        }
        if (value["StopAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_STOP_ACTION;
        }
        if (value["PauseAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_PAUSE_ACTION;
        }
        if (value["PauseActionWithDuration"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_PAUSE_ACTION_WITH_DURATION;
        }
        if (value["ResumeAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_RESUME_ACTION;
        }
        if (value["EnableAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_ENABLE_ACTION;
        }
        if (value["EnableActionWithDuration"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_ENABLE_ACTION_WITH_DURATION;
        }
        if (value["DisableAction"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_DISABLE_ACTION;
        }
        if (value["DisableActionWithDuration"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_COMMAND_BITS_DISABLE_ACTION_WITH_DURATION;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ConfigStatus(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Operational"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_OPERATIONAL;
        }
        if (value["OnlineReserved"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_ONLINE_RESERVED;
        }
        if (value["LiftMovementReversed"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_LIFT_MOVEMENT_REVERSED;
        }
        if (value["LiftPositionAware"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_LIFT_POSITION_AWARE;
        }
        if (value["TiltPositionAware"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_TILT_POSITION_AWARE;
        }
        if (value["LiftEncoderControlled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_LIFT_ENCODER_CONTROLLED;
        }
        if (value["TiltEncoderControlled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONFIG_STATUS_TILT_ENCODER_CONTROLLED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ContentLauncherFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["ContentSearch"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONTENT_LAUNCHER_FEATURE_CONTENT_SEARCH;
        }
        if (value["URLPlayback"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_CONTENT_LAUNCHER_FEATURE_URL_PLAYBACK;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DayOfWeek(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Sunday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_SUNDAY;
        }
        if (value["Monday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_MONDAY;
        }
        if (value["Tuesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_TUESDAY;
        }
        if (value["Wednesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_WEDNESDAY;
        }
        if (value["Thursday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_THURSDAY;
        }
        if (value["Friday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_FRIDAY;
        }
        if (value["Saturday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_SATURDAY;
        }
        if (value["AwayOrVacation"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DAY_OF_WEEK_AWAY_OR_VACATION;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DeviceTempAlarmMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["TooLow"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DEVICE_TEMP_ALARM_MASK_TOO_LOW;
        }
        if (value["TooHigh"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DEVICE_TEMP_ALARM_MASK_TOO_HIGH;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DlCredentialRuleMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Single"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULE_MASK_SINGLE;
        }
        if (value["Dual"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULE_MASK_DUAL;
        }
        if (value["Tri"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULE_MASK_TRI;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DlCredentialRulesSupport(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Single"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULES_SUPPORT_SINGLE;
        }
        if (value["Dual"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULES_SUPPORT_DUAL;
        }
        if (value["Tri"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_CREDENTIAL_RULES_SUPPORT_TRI;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DlDaysMaskMap(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Sunday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_SUNDAY;
        }
        if (value["Monday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_MONDAY;
        }
        if (value["Tuesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_TUESDAY;
        }
        if (value["Wednesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_WEDNESDAY;
        }
        if (value["Thursday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_THURSDAY;
        }
        if (value["Friday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_FRIDAY;
        }
        if (value["Saturday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DAYS_MASK_MAP_SATURDAY;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlDefaultConfigurationRegister(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["EnableLocalProgrammingEnabled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_ENABLE_LOCAL_PROGRAMMING_ENABLED;
        }
        if (value["KeypadInterfaceDefaultAccessEnabled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_KEYPAD_INTERFACE_DEFAULT_ACCESS_ENABLED;
        }
        if (value["RemoteInterfaceDefaultAccessIsEnabled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_REMOTE_INTERFACE_DEFAULT_ACCESS_IS_ENABLED;
        }
        if (value["SoundEnabled"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_SOUND_ENABLED;
        }
        if (value["AutoRelockTimeSet"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_AUTO_RELOCK_TIME_SET;
        }
        if (value["LEDSettingsSet"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_DEFAULT_CONFIGURATION_REGISTER_LED_SETTINGS_SET;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlKeypadOperationEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_UNKNOWN;
        }
        if (value["Lock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_LOCK;
        }
        if (value["Unlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_UNLOCK;
        }
        if (value["LockInvalidPIN"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_LOCK_INVALID_PIN;
        }
        if (value["LockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_LOCK_INVALID_SCHEDULE;
        }
        if (value["UnlockInvalidCode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_UNLOCK_INVALID_CODE;
        }
        if (value["UnlockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_UNLOCK_INVALID_SCHEDULE;
        }
        if (value["NonAccessUserOpEvent"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_OPERATION_EVENT_MASK_NON_ACCESS_USER_OP_EVENT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlKeypadProgrammingEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_PROGRAMMING_EVENT_MASK_UNKNOWN;
        }
        if (value["ProgrammingPINChanged"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_PROGRAMMING_EVENT_MASK_PROGRAMMING_PIN_CHANGED;
        }
        if (value["PINAdded"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_PROGRAMMING_EVENT_MASK_PIN_ADDED;
        }
        if (value["PINCleared"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_PROGRAMMING_EVENT_MASK_PIN_CLEARED;
        }
        if (value["PINChanged"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_KEYPAD_PROGRAMMING_EVENT_MASK_PIN_CHANGED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DlLocalProgrammingFeatures(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["AddUsersCredentialsSchedulesLocally"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_LOCAL_PROGRAMMING_FEATURES_ADD_USERS_CREDENTIALS_SCHEDULES_LOCALLY;
        }
        if (value["ModifyUsersCredentialsSchedulesLocally"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_LOCAL_PROGRAMMING_FEATURES_MODIFY_USERS_CREDENTIALS_SCHEDULES_LOCALLY;
        }
        if (value["ClearUsersCredentialsSchedulesLocally"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_LOCAL_PROGRAMMING_FEATURES_CLEAR_USERS_CREDENTIALS_SCHEDULES_LOCALLY;
        }
        if (value["AdjustLockSettingsLocally"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_LOCAL_PROGRAMMING_FEATURES_ADJUST_LOCK_SETTINGS_LOCALLY;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlManualOperationEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_UNKNOWN;
        }
        if (value["ThumbturnLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_THUMBTURN_LOCK;
        }
        if (value["ThumbturnUnlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_THUMBTURN_UNLOCK;
        }
        if (value["OneTouchLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_ONE_TOUCH_LOCK;
        }
        if (value["KeyLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_KEY_LOCK;
        }
        if (value["KeyUnlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_KEY_UNLOCK;
        }
        if (value["AutoLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_AUTO_LOCK;
        }
        if (value["ScheduleLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_SCHEDULE_LOCK;
        }
        if (value["ScheduleUnlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_SCHEDULE_UNLOCK;
        }
        if (value["ManualLock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_MANUAL_LOCK;
        }
        if (value["ManualUnlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_MANUAL_OPERATION_EVENT_MASK_MANUAL_UNLOCK;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlRFIDOperationEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_UNKNOWN;
        }
        if (value["Lock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_LOCK;
        }
        if (value["Unlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_UNLOCK;
        }
        if (value["LockInvalidRFID"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_LOCK_INVALID_RFID;
        }
        if (value["LockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_LOCK_INVALID_SCHEDULE;
        }
        if (value["UnlockInvalidRFID"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_UNLOCK_INVALID_RFID;
        }
        if (value["UnlockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_OPERATION_EVENT_MASK_UNLOCK_INVALID_SCHEDULE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlRFIDProgrammingEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_PROGRAMMING_EVENT_MASK_UNKNOWN;
        }
        if (value["RFIDCodeAdded"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_PROGRAMMING_EVENT_MASK_RFID_CODE_ADDED;
        }
        if (value["RFIDCodeCleared"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_RFID_PROGRAMMING_EVENT_MASK_RFID_CODE_CLEARED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlRemoteOperationEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_UNKNOWN;
        }
        if (value["Lock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_LOCK;
        }
        if (value["Unlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_UNLOCK;
        }
        if (value["LockInvalidCode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_LOCK_INVALID_CODE;
        }
        if (value["LockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_LOCK_INVALID_SCHEDULE;
        }
        if (value["UnlockInvalidCode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_UNLOCK_INVALID_CODE;
        }
        if (value["UnlockInvalidSchedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_OPERATION_EVENT_MASK_UNLOCK_INVALID_SCHEDULE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlRemoteProgrammingEventMask(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Unknown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_UNKNOWN;
        }
        if (value["ProgrammingPINChanged"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_PROGRAMMING_PIN_CHANGED;
        }
        if (value["PINAdded"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_PIN_ADDED;
        }
        if (value["PINCleared"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_PIN_CLEARED;
        }
        if (value["PINChanged"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_PIN_CHANGED;
        }
        if (value["RFIDCodeAdded"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_RFID_CODE_ADDED;
        }
        if (value["RFIDCodeCleared"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_REMOTE_PROGRAMMING_EVENT_MASK_RFID_CODE_CLEARED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_DlSupportedOperatingModes(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Normal"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_SUPPORTED_OPERATING_MODES_NORMAL;
        }
        if (value["Vacation"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_SUPPORTED_OPERATING_MODES_VACATION;
        }
        if (value["Privacy"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_SUPPORTED_OPERATING_MODES_PRIVACY;
        }
        if (value["NoRemoteLockUnlock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_SUPPORTED_OPERATING_MODES_NO_REMOTE_LOCK_UNLOCK;
        }
        if (value["Passage"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DL_SUPPORTED_OPERATING_MODES_PASSAGE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_DoorLockDayOfWeek(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Sunday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_SUNDAY;
        }
        if (value["Monday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_MONDAY;
        }
        if (value["Tuesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_TUESDAY;
        }
        if (value["Wednesday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_WEDNESDAY;
        }
        if (value["Thursday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_THURSDAY;
        }
        if (value["Friday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_FRIDAY;
        }
        if (value["Saturday"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_DAY_OF_WEEK_SATURDAY;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_DoorLockFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["PINCredentials"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_PIN_CREDENTIALS;
        }
        if (value["RFIDCredentials"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_RFID_CREDENTIALS;
        }
        if (value["FingerCredentials"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_FINGER_CREDENTIALS;
        }
        if (value["Logging"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_LOGGING;
        }
        if (value["WeekDaySchedules"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_WEEK_DAY_SCHEDULES;
        }
        if (value["DoorPositionSensor"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_DOOR_POSITION_SENSOR;
        }
        if (value["FaceCredentials"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_FACE_CREDENTIALS;
        }
        if (value["CredentialsOTA"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_CREDENTIALS_OTA;
        }
        if (value["UsersManagement"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_USERS_MANAGEMENT;
        }
        if (value["Notifications"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_NOTIFICATIONS;
        }
        if (value["YearDaySchedules"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_YEAR_DAY_SCHEDULES;
        }
        if (value["HolidaySchedules"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_DOOR_LOCK_FEATURE_HOLIDAY_SCHEDULES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_EnergyFormatting(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["NumberOfDigitsToTheRightOfTheDecimalPoint"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ENERGY_FORMATTING_NUMBER_OF_DIGITS_TO_THE_RIGHT_OF_THE_DECIMAL_POINT;
        }
        if (value["NumberOfDigitsToTheLeftOfTheDecimalPoint"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ENERGY_FORMATTING_NUMBER_OF_DIGITS_TO_THE_LEFT_OF_THE_DECIMAL_POINT;
        }
        if (value["SuppressLeadingZeros"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ENERGY_FORMATTING_SUPPRESS_LEADING_ZEROS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_EthernetNetworkDiagnosticsFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["PacketCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ETHERNET_NETWORK_DIAGNOSTICS_FEATURE_PACKET_COUNTS;
        }
        if (value["ErrorCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ETHERNET_NETWORK_DIAGNOSTICS_FEATURE_ERROR_COUNTS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_FanControlFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["MultiSpeed"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FAN_CONTROL_FEATURE_MULTI_SPEED;
        }
        if (value["Auto"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FAN_CONTROL_FEATURE_AUTO;
        }
        if (value["Rocking"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FAN_CONTROL_FEATURE_ROCKING;
        }
        if (value["Wind"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FAN_CONTROL_FEATURE_WIND;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_Feature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Lift"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FEATURE_LIFT;
        }
        if (value["Tilt"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FEATURE_TILT;
        }
        if (value["PositionAwareLift"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FEATURE_POSITION_AWARE_LIFT;
        }
        if (value["AbsolutePosition"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FEATURE_ABSOLUTE_POSITION;
        }
        if (value["PositionAwareTilt"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_FEATURE_POSITION_AWARE_TILT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_GroupClusterFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["GroupNames"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_GROUP_CLUSTER_FEATURE_GROUP_NAMES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_IasZoneStatus(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Alarm1"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_ALARM1;
        }
        if (value["Alarm2"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_ALARM2;
        }
        if (value["Tamper"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_TAMPER;
        }
        if (value["Battery"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_BATTERY;
        }
        if (value["SupervisionReports"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_SUPERVISION_REPORTS;
        }
        if (value["RestoreReports"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_RESTORE_REPORTS;
        }
        if (value["Trouble"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_TROUBLE;
        }
        if (value["Ac"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_AC;
        }
        if (value["Test"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_TEST;
        }
        if (value["BatteryDefect"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_IAS_ZONE_STATUS_BATTERY_DEFECT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_KeypadInputFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["NavigationKeyCodes"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_KEYPAD_INPUT_FEATURE_NAVIGATION_KEY_CODES;
        }
        if (value["LocationKeys"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_KEYPAD_INPUT_FEATURE_LOCATION_KEYS;
        }
        if (value["NumberKeys"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_KEYPAD_INPUT_FEATURE_NUMBER_KEYS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_LampAlarmMode(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["LampBurnHours"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LAMP_ALARM_MODE_LAMP_BURN_HOURS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_LevelControlFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["OnOff"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LEVEL_CONTROL_FEATURE_ON_OFF;
        }
        if (value["Lighting"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LEVEL_CONTROL_FEATURE_LIGHTING;
        }
        if (value["Frequency"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_LEVEL_CONTROL_FEATURE_FREQUENCY;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_MainsAlarmMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["VoltageTooLow"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MAINS_ALARM_MASK_VOLTAGE_TOO_LOW;
        }
        if (value["VoltageTooHigh"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MAINS_ALARM_MASK_VOLTAGE_TOO_HIGH;
        }
        if (value["MainsPowerSupplyLost"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MAINS_ALARM_MASK_MAINS_POWER_SUPPLY_LOST;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_MediaInputFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["NameUpdates"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MEDIA_INPUT_FEATURE_NAME_UPDATES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_MediaPlaybackFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["AdvancedSeek"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MEDIA_PLAYBACK_FEATURE_ADVANCED_SEEK;
        }
        if (value["VariableSpeed"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MEDIA_PLAYBACK_FEATURE_VARIABLE_SPEED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_Mode(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["MotorDirectionReversed"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_MOTOR_DIRECTION_REVERSED;
        }
        if (value["CalibrationMode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_CALIBRATION_MODE;
        }
        if (value["MaintenanceMode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_MAINTENANCE_MODE;
        }
        if (value["LedFeedback"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_LED_FEEDBACK;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ModeForSequence(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["HeatSetpointFieldPresent"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_FOR_SEQUENCE_HEAT_SETPOINT_FIELD_PRESENT;
        }
        if (value["CoolSetpointFieldPresent"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_FOR_SEQUENCE_COOL_SETPOINT_FIELD_PRESENT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ModeSelectFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Deponoff"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_MODE_SELECT_FEATURE_DEPONOFF;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_NetworkCommissioningFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["WiFiNetworkInterface"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_NETWORK_COMMISSIONING_FEATURE_WI_FI_NETWORK_INTERFACE;
        }
        if (value["ThreadNetworkInterface"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_NETWORK_COMMISSIONING_FEATURE_THREAD_NETWORK_INTERFACE;
        }
        if (value["EthernetNetworkInterface"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_NETWORK_COMMISSIONING_FEATURE_ETHERNET_NETWORK_INTERFACE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_Occupancy(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["SensedOccupancy"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OCCUPANCY_OCCUPIED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_OccupancySensorTypeBitmap(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["PIR"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR;
        }
        if (value["Ultrasonic"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC;
        }
        if (value["PhysicalContact"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OCCUPANCY_SENSOR_TYPE_BITMAP_PHYSICAL_CONTACT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_OnOffControl(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["AcceptOnlyWhenOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ON_OFF_CONTROL_ACCEPT_ONLY_WHEN_ON;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_OnOffFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Lighting"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ON_OFF_FEATURE_LIGHTING;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_OperationalStatus(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Global"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OPERATIONAL_STATUS_GLOBAL;
        }
        if (value["Lift"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OPERATIONAL_STATUS_LIFT;
        }
        if (value["Tilt"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_OPERATIONAL_STATUS_TILT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_PowerSourceFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Wired"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_POWER_SOURCE_FEATURE_WIRED;
        }
        if (value["Battery"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_POWER_SOURCE_FEATURE_BATTERY;
        }
        if (value["Rechargeable"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_POWER_SOURCE_FEATURE_RECHARGEABLE;
        }
        if (value["Replaceable"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_POWER_SOURCE_FEATURE_REPLACEABLE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_PressureFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Ext"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PRESSURE_FEATURE_EXT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_PumpStatus(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["DeviceFault"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_DEVICE_FAULT;
        }
        if (value["Supplyfault"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_SUPPLYFAULT;
        }
        if (value["SpeedLow"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_SPEED_LOW;
        }
        if (value["SpeedHigh"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_SPEED_HIGH;
        }
        if (value["LocalOverride"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_LOCAL_OVERRIDE;
        }
        if (value["Running"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_RUNNING;
        }
        if (value["RemotePressure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_REMOTE_PRESSURE;
        }
        if (value["RemoteFlow"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_REMOTE_FLOW;
        }
        if (value["RemoteTemperature"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_PUMP_STATUS_REMOTE_TEMPERATURE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_RockSupportMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["RockLeftRight"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ROCK_SUPPORT_MASK_ROCK_LEFT_RIGHT;
        }
        if (value["RockUpDown"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ROCK_SUPPORT_MASK_ROCK_UP_DOWN;
        }
        if (value["RockRound"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_ROCK_SUPPORT_MASK_ROCK_ROUND;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_SafetyStatus(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["RemoteLockout"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_REMOTE_LOCKOUT;
        }
        if (value["TamperDetection"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_TAMPER_DETECTION;
        }
        if (value["FailedCommunication"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_FAILED_COMMUNICATION;
        }
        if (value["PositionFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_POSITION_FAILURE;
        }
        if (value["ThermalProtection"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_THERMAL_PROTECTION;
        }
        if (value["ObstacleDetected"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_OBSTACLE_DETECTED;
        }
        if (value["Power"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_POWER;
        }
        if (value["StopInput"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_STOP_INPUT;
        }
        if (value["MotorJammed"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_MOTOR_JAMMED;
        }
        if (value["HardwareFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_HARDWARE_FAILURE;
        }
        if (value["ManualOperation"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_MANUAL_OPERATION;
        }
        if (value["Protection"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SAFETY_STATUS_PROTECTION;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_SceneFeatures(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["SceneNames"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SCENE_FEATURES_SCENE_NAMES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ScenesCopyMode(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["CopyAllScenes"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SCENES_COPY_MODE_COPY_ALL_SCENES;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ShadeClosureStatus(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Operational"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SHADE_CLOSURE_STATUS_OPERATIONAL;
        }
        if (value["Adjusting"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SHADE_CLOSURE_STATUS_ADJUSTING;
        }
        if (value["Opening"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SHADE_CLOSURE_STATUS_OPENING;
        }
        if (value["MotorOpening"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SHADE_CLOSURE_STATUS_MOTOR_OPENING;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_SimpleBitmap(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["ValueA"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SIMPLE_BITMAP_VALUE_A;
        }
        if (value["ValueB"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SIMPLE_BITMAP_VALUE_B;
        }
        if (value["ValueC"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SIMPLE_BITMAP_VALUE_C;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_SoftwareDiagnosticsFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["WaterMarks"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SOFTWARE_DIAGNOSTICS_FEATURE_WATER_MARKS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_SquawkInfo(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Mode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SQUAWK_INFO_MODE;
        }
        if (value["Strobe"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SQUAWK_INFO_STROBE;
        }
        if (value["Level"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SQUAWK_INFO_LEVEL;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_StartTime(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["Minutes"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_START_TIME_MINUTES;
        }
        if (value["TimeEncoding"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_START_TIME_TIME_ENCODING;
        }
        if (value["Hours"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_START_TIME_HOURS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_SupportedStreamingProtocol(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Dash"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SUPPORTED_STREAMING_PROTOCOL_DASH;
        }
        if (value["Hls"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SUPPORTED_STREAMING_PROTOCOL_HLS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_SwitchFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["LatchingSwitch"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SWITCH_FEATURE_LATCHING_SWITCH;
        }
        if (value["MomentarySwitch"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SWITCH_FEATURE_MOMENTARY_SWITCH;
        }
        if (value["MomentarySwitchRelease"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SWITCH_FEATURE_MOMENTARY_SWITCH_RELEASE;
        }
        if (value["MomentarySwitchLongPress"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SWITCH_FEATURE_MOMENTARY_SWITCH_LONG_PRESS;
        }
        if (value["MomentarySwitchMultiPress"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_SWITCH_FEATURE_MOMENTARY_SWITCH_MULTI_PRESS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ThermostatAlarmMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["InitializationFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_ALARM_MASK_INITIALIZATION_FAILURE;
        }
        if (value["HardwareFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_ALARM_MASK_HARDWARE_FAILURE;
        }
        if (value["SelfcalibrationFailure"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_ALARM_MASK_SELFCALIBRATION_FAILURE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ThermostatFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["Heating"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_HEATING;
        }
        if (value["Cooling"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_COOLING;
        }
        if (value["Occupancy"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_OCCUPANCY;
        }
        if (value["Schedule"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_SCHEDULE;
        }
        if (value["Setback"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_SETBACK;
        }
        if (value["Automode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_FEATURE_AUTOMODE;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ThermostatOccupancy(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["SensedOccupancy"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_OCCUPANCY_OCCUPIED;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_ThermostatRunningState(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        if (value["HeatStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON;
        }
        if (value["CoolStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_COOL_STATE_ON;
        }
        if (value["FanStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_FAN_STATE_ON;
        }
        if (value["HeatSecondStageStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_HEAT_SECOND_STAGE_STATE_ON;
        }
        if (value["CoolSecondStageStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_COOL_SECOND_STAGE_STATE_ON;
        }
        if (value["FanSecondStageStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_FAN_SECOND_STAGE_STATE_ON;
        }
        if (value["FanThirdStageStateOn"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_RUNNING_STATE_FAN_THIRD_STAGE_STATE_ON;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_ThermostatSensing(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["LocalTempSensedRemotely"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_SENSING_LOCAL_TEMP_SENSED_REMOTELY;
        }
        if (value["OutdoorTempSensedRemotely"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_SENSING_OUTDOOR_TEMP_SENSED_REMOTELY;
        }
        if (value["OccupancySensedRemotely"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THERMOSTAT_SENSING_OCCUPANCY_SENSED_REMOTELY;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_ThreadNetworkDiagnosticsFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["PacketCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THREAD_NETWORK_DIAGNOSTICS_FEATURE_PACKET_COUNTS;
        }
        if (value["ErrorCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THREAD_NETWORK_DIAGNOSTICS_FEATURE_ERROR_COUNTS;
        }
        if (value["MLECounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THREAD_NETWORK_DIAGNOSTICS_FEATURE_MLE_COUNTS;
        }
        if (value["MACCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_THREAD_NETWORK_DIAGNOSTICS_FEATURE_MAC_COUNTS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_TimeStatusMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["MasterClock"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_TIME_STATUS_MASK_MASTER_CLOCK;
        }
        if (value["Synchronized"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_TIME_STATUS_MASK_SYNCHRONIZED;
        }
        if (value["MasterZoneDst"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_TIME_STATUS_MASK_MASTER_ZONE_DST;
        }
        if (value["Superseding"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_TIME_STATUS_MASK_SUPERSEDING;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_UnitLocalizationFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["TemperatureUnit"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_UNIT_LOCALIZATION_FEATURE_TEMPERATURE_UNIT;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_WarningInfo(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Mode"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WARNING_INFO_MODE;
        }
        if (value["Strobe"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WARNING_INFO_STROBE;
        }
        if (value["SirenLevel"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WARNING_INFO_SIREN_LEVEL;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_WiFiNetworkDiagnosticsFeature(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        if (value["PacketCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_NETWORK_DIAGNOSTICS_FEATURE_PACKET_COUNTS;
        }
        if (value["ErrorCounts"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_NETWORK_DIAGNOSTICS_FEATURE_ERROR_COUNTS;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_WiFiSecurity(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["Unencrypted"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_SECURITY_UNENCRYPTED;
        }
        if (value["WepPersonal"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_SECURITY_WEP_PERSONAL;
        }
        if (value["WpaPersonal"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_SECURITY_WPA_PERSONAL;
        }
        if (value["Wpa2Personal"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_SECURITY_WPA2_PERSONAL;
        }
        if (value["Wpa3Personal"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WI_FI_SECURITY_WPA3_PERSONAL;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_WindSettingMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["SleepWind"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WIND_SETTING_MASK_SLEEP_WIND;
        }
        if (value["NaturalWind"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WIND_SETTING_MASK_NATURAL_WIND;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_WindSupportMask(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        if (value["SleepWind"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WIND_SUPPORT_MASK_SLEEP_WIND;
        }
        if (value["NaturalWind"].get<bool>())
        {
            bitmap_value |= ZCL_BITMAP_WIND_SUPPORT_MASK_NATURAL_WIND;
        }
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint16_t> from_json_bitmap16(const nlohmann::json & value)
{
    uint16_t bitmap_value = 0x00;
    try
    {
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_bitmap24(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint32_t> from_json_bitmap32(const nlohmann::json & value)
{
    uint32_t bitmap_value = 0x00;
    try
    {
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint64_t> from_json_bitmap64(const nlohmann::json & value)
{
    uint64_t bitmap_value = 0x00;
    try
    {
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}

std::optional<uint8_t> from_json_bitmap8(const nlohmann::json & value)
{
    uint8_t bitmap_value = 0x00;
    try
    {
        return bitmap_value;
    } catch (const nlohmann::json::exception & e)
    {
        sl_log_warning(LOG_TAG, "Failed to parse bitmap %s, error %s", value.dump().c_str(), e.what());
        return std::nullopt;
    }
}
