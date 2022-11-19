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
#include <app-common/zap-generated/cluster-objects.h>
#include <nlohmann/json.hpp>

// Default translation
template <typename T>
nlohmann::json inline to_json(const T & value)
{
    return "unknown value";
}

/***************************** Simple types Convertes **************/

template <>
nlohmann::json inline to_json(const bool & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const int & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const int16_t & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const std::string & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const uint8_t & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const uint16_t & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const uint32_t & value)
{
    return value;
}
template <>
nlohmann::json inline to_json(const uint64_t & value)
{
    return value;
}

template <typename T>
nlohmann::json inline to_json(chip::app::DataModel::Nullable<T> & value)
{
    if ((!value.IsNull()) && value.HasValidValue())
    {
        return to_json(value.Value());
    }
    return nlohmann::json::value_t::null;
}

/***************************** Bitmap Convertes **************/

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyEffectIdentifier & value)
{
    switch (value)
    {
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kBlink:
        return "Blink";
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kBreathe:
        return "Breathe";
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kOkay:
        return "Okay";
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kChannelChange:
        return "ChannelChange";
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kFinishEffect:
        return "FinishEffect";
    case chip::app::Clusters::Identify::IdentifyEffectIdentifier::kStopEffect:
        return "StopEffect";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyEffectVariant & value)
{
    switch (value)
    {
    case chip::app::Clusters::Identify::IdentifyEffectVariant::kDefault:
        return "Default";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyIdentifyType & value)
{
    switch (value)
    {
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kNone:
        return "None";
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kVisibleLight:
        return "VisibleLight";
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kVisibleLED:
        return "VisibleLED";
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kAudibleBeep:
        return "AudibleBeep";
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kDisplay:
        return "Display";
    case chip::app::Clusters::Identify::IdentifyIdentifyType::kActuator:
        return "Actuator";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<Scenes::ScenesCopyMode, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const Scenes::Structs::AttributeValuePair::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const Scenes::Structs::ExtensionFieldSet::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<OnOff::OnOffControl, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<OnOff::OnOffFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<OnOff::SceneFeatures, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant & value)
{
    switch (value)
    {
    case chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds:
        return "FadeToOffIn0p8Seconds";
    case chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant::kNoFade:
        return "NoFade";
    case chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds:
        return "50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant & value)
{
    switch (value)
    {
    case chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second:
        return "20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffEffectIdentifier & value)
{
    switch (value)
    {
    case chip::app::Clusters::OnOff::OnOffEffectIdentifier::kDelayedAllOff:
        return "DelayedAllOff";
    case chip::app::Clusters::OnOff::OnOffEffectIdentifier::kDyingLight:
        return "DyingLight";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffStartUpOnOff & value)
{
    switch (value)
    {
    case chip::app::Clusters::OnOff::OnOffStartUpOnOff::kOff:
        return "Off";
    case chip::app::Clusters::OnOff::OnOffStartUpOnOff::kOn:
        return "On";
    case chip::app::Clusters::OnOff::OnOffStartUpOnOff::kTogglePreviousOnOff:
        return "TogglePreviousOnOff";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<LevelControl::LevelControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::LevelControl::MoveMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::LevelControl::MoveMode::kUp:
        return "Up";
    case chip::app::Clusters::LevelControl::MoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::LevelControl::StepMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::LevelControl::StepMode::kUp:
        return "Up";
    case chip::app::Clusters::LevelControl::StepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const Basic::Structs::CapabilityMinimaStruct::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kProceed:
        return "Proceed";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kAwaitNextAction:
        return "AwaitNextAction";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction::kDiscontinue:
        return "Discontinue";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol::kBDXSynchronous:
        return "BDXSynchronous";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol::kBDXAsynchronous:
        return "BDXAsynchronous";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol::kHttps:
        return "Https";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol::kVendorSpecific:
        return "VendorSpecific";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus::kUpdateAvailable:
        return "UpdateAvailable";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus::kBusy:
        return "Busy";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus::kNotAvailable:
        return "NotAvailable";
    case chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus::kDownloadProtocolNotSupported:
        return "DownloadProtocolNotSupported";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kSimpleAnnouncement:
        return "SimpleAnnouncement";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kUpdateAvailable:
        return "UpdateAvailable";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason::kUrgentUpdateAvailable:
        return "UrgentUpdateAvailable";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kUnknown:
        return "Unknown";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kFailure:
        return "Failure";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kTimeOut:
        return "TimeOut";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum::kDelayByProvider:
        return "DelayByProvider";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kUnknown:
        return "Unknown";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle:
        return "Idle";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kQuerying:
        return "Querying";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnQuery:
        return "DelayedOnQuery";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDownloading:
        return "Downloading";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying:
        return "Applying";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnApply:
        return "DelayedOnApply";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kRollingBack:
        return "RollingBack";
    case chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnUserConsent:
        return "DelayedOnUserConsent";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::TimeFormatLocalization::CalendarType & value)
{
    switch (value)
    {
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kBuddhist:
        return "Buddhist";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kChinese:
        return "Chinese";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kCoptic:
        return "Coptic";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kEthiopian:
        return "Ethiopian";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kGregorian:
        return "Gregorian";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kHebrew:
        return "Hebrew";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kIndian:
        return "Indian";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kIslamic:
        return "Islamic";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kJapanese:
        return "Japanese";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kKorean:
        return "Korean";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kPersian:
        return "Persian";
    case chip::app::Clusters::TimeFormatLocalization::CalendarType::kTaiwanese:
        return "Taiwanese";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::TimeFormatLocalization::HourFormat & value)
{
    switch (value)
    {
    case chip::app::Clusters::TimeFormatLocalization::HourFormat::k12hr:
        return "12hr";
    case chip::app::Clusters::TimeFormatLocalization::HourFormat::k24hr:
        return "24hr";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitLocalization::UnitLocalizationFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::UnitLocalization::TempUnit & value)
{
    switch (value)
    {
    case chip::app::Clusters::UnitLocalization::TempUnit::kFahrenheit:
        return "Fahrenheit";
    case chip::app::Clusters::UnitLocalization::TempUnit::kCelsius:
        return "Celsius";
    case chip::app::Clusters::UnitLocalization::TempUnit::kKelvin:
        return "Kelvin";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<PowerSource::PowerSourceFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const PowerSource::Structs::BatChargeFaultChangeType::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const PowerSource::Structs::BatFaultChangeType::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const PowerSource::Structs::WiredFaultChangeType::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::BatChargeFault & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::BatChargeFault::kUnspecfied:
        return "Unspecfied";
    case chip::app::Clusters::PowerSource::BatChargeFault::kAmbientTooHot:
        return "AmbientTooHot";
    case chip::app::Clusters::PowerSource::BatChargeFault::kAmbientTooCold:
        return "AmbientTooCold";
    case chip::app::Clusters::PowerSource::BatChargeFault::kBatteryTooHot:
        return "BatteryTooHot";
    case chip::app::Clusters::PowerSource::BatChargeFault::kBatteryTooCold:
        return "BatteryTooCold";
    case chip::app::Clusters::PowerSource::BatChargeFault::kBatteryAbsent:
        return "BatteryAbsent";
    case chip::app::Clusters::PowerSource::BatChargeFault::kBatteryOverVoltage:
        return "BatteryOverVoltage";
    case chip::app::Clusters::PowerSource::BatChargeFault::kBatteryUnderVoltage:
        return "BatteryUnderVoltage";
    case chip::app::Clusters::PowerSource::BatChargeFault::kChargerOverVoltage:
        return "ChargerOverVoltage";
    case chip::app::Clusters::PowerSource::BatChargeFault::kChargerUnderVoltage:
        return "ChargerUnderVoltage";
    case chip::app::Clusters::PowerSource::BatChargeFault::kSafetyTimeout:
        return "SafetyTimeout";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::BatChargeLevel & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::BatChargeLevel::kOk:
        return "Ok";
    case chip::app::Clusters::PowerSource::BatChargeLevel::kWarning:
        return "Warning";
    case chip::app::Clusters::PowerSource::BatChargeLevel::kCritical:
        return "Critical";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::BatChargeState & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::BatChargeState::kUnknown:
        return "Unknown";
    case chip::app::Clusters::PowerSource::BatChargeState::kIsCharging:
        return "IsCharging";
    case chip::app::Clusters::PowerSource::BatChargeState::kIsAtFullCharge:
        return "IsAtFullCharge";
    case chip::app::Clusters::PowerSource::BatChargeState::kIsNotCharging:
        return "IsNotCharging";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::BatFault & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::BatFault::kUnspecfied:
        return "Unspecfied";
    case chip::app::Clusters::PowerSource::BatFault::kOverTemp:
        return "OverTemp";
    case chip::app::Clusters::PowerSource::BatFault::kUnderTemp:
        return "UnderTemp";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::BatReplaceability & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::BatReplaceability::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::PowerSource::BatReplaceability::kNotReplaceable:
        return "NotReplaceable";
    case chip::app::Clusters::PowerSource::BatReplaceability::kUserReplaceable:
        return "UserReplaceable";
    case chip::app::Clusters::PowerSource::BatReplaceability::kFactoryReplaceable:
        return "FactoryReplaceable";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::PowerSourceStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::PowerSourceStatus::kUnspecfied:
        return "Unspecfied";
    case chip::app::Clusters::PowerSource::PowerSourceStatus::kActive:
        return "Active";
    case chip::app::Clusters::PowerSource::PowerSourceStatus::kStandby:
        return "Standby";
    case chip::app::Clusters::PowerSource::PowerSourceStatus::kUnavailable:
        return "Unavailable";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::WiredCurrentType & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::WiredCurrentType::kAc:
        return "Ac";
    case chip::app::Clusters::PowerSource::WiredCurrentType::kDc:
        return "Dc";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::PowerSource::WiredFault & value)
{
    switch (value)
    {
    case chip::app::Clusters::PowerSource::WiredFault::kUnspecfied:
        return "Unspecfied";
    case chip::app::Clusters::PowerSource::WiredFault::kOverVoltage:
        return "OverVoltage";
    case chip::app::Clusters::PowerSource::WiredFault::kUnderVoltage:
        return "UnderVoltage";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const GeneralCommissioning::Structs::BasicCommissioningInfo::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralCommissioning::CommissioningError & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralCommissioning::CommissioningError::kOk:
        return "Ok";
    case chip::app::Clusters::GeneralCommissioning::CommissioningError::kValueOutsideRange:
        return "ValueOutsideRange";
    case chip::app::Clusters::GeneralCommissioning::CommissioningError::kInvalidAuthentication:
        return "InvalidAuthentication";
    case chip::app::Clusters::GeneralCommissioning::CommissioningError::kNoFailSafe:
        return "NoFailSafe";
    case chip::app::Clusters::GeneralCommissioning::CommissioningError::kBusyWithOtherAdmin:
        return "BusyWithOtherAdmin";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoor:
        return "Indoor";
    case chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType::kOutdoor:
        return "Outdoor";
    case chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoorOutdoor:
        return "IndoorOutdoor";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DiagnosticLogs::LogsIntent & value)
{
    switch (value)
    {
    case chip::app::Clusters::DiagnosticLogs::LogsIntent::kEndUserSupport:
        return "EndUserSupport";
    case chip::app::Clusters::DiagnosticLogs::LogsIntent::kNetworkDiag:
        return "NetworkDiag";
    case chip::app::Clusters::DiagnosticLogs::LogsIntent::kCrashLogs:
        return "CrashLogs";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DiagnosticLogs::LogsStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::DiagnosticLogs::LogsStatus::kSuccess:
        return "Success";
    case chip::app::Clusters::DiagnosticLogs::LogsStatus::kExhausted:
        return "Exhausted";
    case chip::app::Clusters::DiagnosticLogs::LogsStatus::kNoLogs:
        return "NoLogs";
    case chip::app::Clusters::DiagnosticLogs::LogsStatus::kBusy:
        return "Busy";
    case chip::app::Clusters::DiagnosticLogs::LogsStatus::kDenied:
        return "Denied";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol & value)
{
    switch (value)
    {
    case chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol::kResponsePayload:
        return "ResponsePayload";
    case chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol::kBdx:
        return "Bdx";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const GeneralDiagnostics::Structs::NetworkInterfaceType::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralDiagnostics::BootReasonType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kPowerOnReboot:
        return "PowerOnReboot";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kBrownOutReset:
        return "BrownOutReset";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kSoftwareWatchdogReset:
        return "SoftwareWatchdogReset";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kHardwareWatchdogReset:
        return "HardwareWatchdogReset";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kSoftwareUpdateCompleted:
        return "SoftwareUpdateCompleted";
    case chip::app::Clusters::GeneralDiagnostics::BootReasonType::kSoftwareReset:
        return "SoftwareReset";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralDiagnostics::HardwareFaultType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kRadio:
        return "Radio";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kSensor:
        return "Sensor";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kResettableOverTemp:
        return "ResettableOverTemp";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kNonResettableOverTemp:
        return "NonResettableOverTemp";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kPowerSource:
        return "PowerSource";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kVisualDisplayFault:
        return "VisualDisplayFault";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kAudioOutputFault:
        return "AudioOutputFault";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kUserInterfaceFault:
        return "UserInterfaceFault";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kNonVolatileMemoryError:
        return "NonVolatileMemoryError";
    case chip::app::Clusters::GeneralDiagnostics::HardwareFaultType::kTamperDetected:
        return "TamperDetected";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralDiagnostics::InterfaceType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralDiagnostics::InterfaceType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::GeneralDiagnostics::InterfaceType::kWiFi:
        return "WiFi";
    case chip::app::Clusters::GeneralDiagnostics::InterfaceType::kEthernet:
        return "Ethernet";
    case chip::app::Clusters::GeneralDiagnostics::InterfaceType::kCellular:
        return "Cellular";
    case chip::app::Clusters::GeneralDiagnostics::InterfaceType::kThread:
        return "Thread";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralDiagnostics::NetworkFaultType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralDiagnostics::NetworkFaultType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::GeneralDiagnostics::NetworkFaultType::kHardwareFailure:
        return "HardwareFailure";
    case chip::app::Clusters::GeneralDiagnostics::NetworkFaultType::kNetworkJammed:
        return "NetworkJammed";
    case chip::app::Clusters::GeneralDiagnostics::NetworkFaultType::kConnectionFailed:
        return "ConnectionFailed";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GeneralDiagnostics::RadioFaultType & value)
{
    switch (value)
    {
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kWiFiFault:
        return "WiFiFault";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kCellularFault:
        return "CellularFault";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kThreadFault:
        return "ThreadFault";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kNFCFault:
        return "NFCFault";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kBLEFault:
        return "BLEFault";
    case chip::app::Clusters::GeneralDiagnostics::RadioFaultType::kEthernetFault:
        return "EthernetFault";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<SoftwareDiagnostics::SoftwareDiagnosticsFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const SoftwareDiagnostics::Structs::ThreadMetrics::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsFeature, uint8_t>&
// value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const ThreadNetworkDiagnostics::Structs::NeighborTable::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ThreadNetworkDiagnostics::Structs::OperationalDatasetComponents::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ThreadNetworkDiagnostics::Structs::RouteTable::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ThreadNetworkDiagnostics::Structs::SecurityPolicy::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault & value)
{
    switch (value)
    {
    case chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault::kLinkDown:
        return "LinkDown";
    case chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault::kHardwareFailure:
        return "HardwareFailure";
    case chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault::kNetworkJammed:
        return "NetworkJammed";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole & value)
{
    switch (value)
    {
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kUnassigned:
        return "Unassigned";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kSleepyEndDevice:
        return "SleepyEndDevice";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kEndDevice:
        return "EndDevice";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kReed:
        return "Reed";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kRouter:
        return "Router";
    case chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole::kLeader:
        return "Leader";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ThreadNetworkDiagnostics::ThreadConnectionStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::ThreadNetworkDiagnostics::ThreadConnectionStatus::kConnected:
        return "Connected";
    case chip::app::Clusters::ThreadNetworkDiagnostics::ThreadConnectionStatus::kNotConnected:
        return "NotConnected";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause & value)
{
    switch (value)
    {
    case chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kUnknown:
        return "Unknown";
    case chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAssociationFailed:
        return "AssociationFailed";
    case chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAuthenticationFailed:
        return "AuthenticationFailed";
    case chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kSsidNotFound:
        return "SsidNotFound";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType & value)
{
    switch (value)
    {
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kNone:
        return "None";
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kWep:
        return "Wep";
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kWpa:
        return "Wpa";
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kWpa2:
        return "Wpa2";
    case chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType::kWpa3:
        return "Wpa3";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus::kConnected:
        return "Connected";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus::kNotConnected:
        return "NotConnected";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType & value)
{
    switch (value)
    {
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211a:
        return "80211a";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211b:
        return "80211b";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211g:
        return "80211g";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211n:
        return "80211n";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211ac:
        return "80211ac";
    case chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType::k80211ax:
        return "80211ax";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const TimeSynchronization::Structs::DstOffsetType::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const TimeSynchronization::Structs::TimeZoneType::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::TimeSynchronization::GranularityEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::TimeSynchronization::GranularityEnum::kNoTimeGranularity:
        return "NoTimeGranularity";
    case chip::app::Clusters::TimeSynchronization::GranularityEnum::kMinutesGranularity:
        return "MinutesGranularity";
    case chip::app::Clusters::TimeSynchronization::GranularityEnum::kSecondsGranularity:
        return "SecondsGranularity";
    case chip::app::Clusters::TimeSynchronization::GranularityEnum::kMillisecondsGranularity:
        return "MillisecondsGranularity";
    case chip::app::Clusters::TimeSynchronization::GranularityEnum::kMicrosecondsGranularity:
        return "MicrosecondsGranularity";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::TimeSynchronization::TimeSourceEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNone:
        return "None";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kUnknown:
        return "Unknown";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kAdmin:
        return "Admin";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNodeTimeCluster:
        return "NodeTimeCluster";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNonFabricSntp:
        return "NonFabricSntp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNonFabricNtp:
        return "NonFabricNtp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kFabricSntp:
        return "FabricSntp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kFabricNtp:
        return "FabricNtp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kMixedNtp:
        return "MixedNtp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNonFabricSntpNts:
        return "NonFabricSntpNts";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kNonFabricNtpNts:
        return "NonFabricNtpNts";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kFabricSntpNts:
        return "FabricSntpNts";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kFabricNtpNts:
        return "FabricNtpNts";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kMixedNtpNts:
        return "MixedNtpNts";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kCloudSource:
        return "CloudSource";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kPtp:
        return "Ptp";
    case chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kGnss:
        return "Gnss";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<Switch::SwitchFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const OperationalCredentials::Structs::FabricDescriptor::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const OperationalCredentials::Structs::NOCStruct::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OperationalCredentials::OperationalCertStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kSuccess:
        return "Success";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kInvalidPublicKey:
        return "InvalidPublicKey";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kInvalidNodeOpId:
        return "InvalidNodeOpId";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kInvalidNOC:
        return "InvalidNOC";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kMissingCsr:
        return "MissingCsr";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kTableFull:
        return "TableFull";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kInvalidAdminSubject:
        return "InvalidAdminSubject";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kFabricConflict:
        return "FabricConflict";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kLabelConflict:
        return "LabelConflict";
    case chip::app::Clusters::OperationalCredentials::OperationalCertStatus::kInvalidFabricIndex:
        return "InvalidFabricIndex";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const GroupKeyManagement::Structs::GroupInfoMapStruct::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const GroupKeyManagement::Structs::GroupKeyMapStruct::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const GroupKeyManagement::Structs::GroupKeySetStruct::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy & value)
{
    switch (value)
    {
    case chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy::kTrustFirst:
        return "TrustFirst";
    case chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy::kCacheAndSync:
        return "CacheAndSync";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const FixedLabel::Structs::LabelStruct::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const UserLabel::Structs::LabelStruct::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<ModeSelect::ModeSelectFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const ModeSelect::Structs::SemanticTag::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ModeSelect::Structs::ModeOptionStruct::Type& value) {
//    return "no struct support";
//}
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlCredentialRuleMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlCredentialRulesSupport, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlDaysMaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlDefaultConfigurationRegister, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlKeypadOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlKeypadProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlLocalProgrammingFeatures, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlManualOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRFIDOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRFIDProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRemoteOperationEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlRemoteProgrammingEventMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DlSupportedOperatingModes, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DoorLockDayOfWeek, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<DoorLock::DoorLockFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const DoorLock::Structs::DlCredential::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlAlarmCode & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlAlarmCode::kLockJammed:
        return "LockJammed";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kLockFactoryReset:
        return "LockFactoryReset";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kLockRadioPowerCycled:
        return "LockRadioPowerCycled";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kWrongCodeEntryLimit:
        return "WrongCodeEntryLimit";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kFrontEsceutcheonRemoved:
        return "FrontEsceutcheonRemoved";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kDoorForcedOpen:
        return "DoorForcedOpen";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kDoorAjar:
        return "DoorAjar";
    case chip::app::Clusters::DoorLock::DlAlarmCode::kForcedUser:
        return "ForcedUser";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlCredentialRule & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlCredentialRule::kSingle:
        return "Single";
    case chip::app::Clusters::DoorLock::DlCredentialRule::kDouble:
        return "Double";
    case chip::app::Clusters::DoorLock::DlCredentialRule::kTri:
        return "Tri";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlCredentialType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlCredentialType::kProgrammingPIN:
        return "ProgrammingPIN";
    case chip::app::Clusters::DoorLock::DlCredentialType::kPin:
        return "Pin";
    case chip::app::Clusters::DoorLock::DlCredentialType::kRfid:
        return "Rfid";
    case chip::app::Clusters::DoorLock::DlCredentialType::kFingerprint:
        return "Fingerprint";
    case chip::app::Clusters::DoorLock::DlCredentialType::kFingerVein:
        return "FingerVein";
    case chip::app::Clusters::DoorLock::DlCredentialType::kFace:
        return "Face";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlDataOperationType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlDataOperationType::kAdd:
        return "Add";
    case chip::app::Clusters::DoorLock::DlDataOperationType::kClear:
        return "Clear";
    case chip::app::Clusters::DoorLock::DlDataOperationType::kModify:
        return "Modify";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlDoorState & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorOpen:
        return "DoorOpen";
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorClosed:
        return "DoorClosed";
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorJammed:
        return "DoorJammed";
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorForcedOpen:
        return "DoorForcedOpen";
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorUnspecifiedError:
        return "DoorUnspecifiedError";
    case chip::app::Clusters::DoorLock::DlDoorState::kDoorAjar:
        return "DoorAjar";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockDataType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlLockDataType::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::DoorLock::DlLockDataType::kProgrammingCode:
        return "ProgrammingCode";
    case chip::app::Clusters::DoorLock::DlLockDataType::kUserIndex:
        return "UserIndex";
    case chip::app::Clusters::DoorLock::DlLockDataType::kWeekDaySchedule:
        return "WeekDaySchedule";
    case chip::app::Clusters::DoorLock::DlLockDataType::kYearDaySchedule:
        return "YearDaySchedule";
    case chip::app::Clusters::DoorLock::DlLockDataType::kHolidaySchedule:
        return "HolidaySchedule";
    case chip::app::Clusters::DoorLock::DlLockDataType::kPin:
        return "Pin";
    case chip::app::Clusters::DoorLock::DlLockDataType::kRfid:
        return "Rfid";
    case chip::app::Clusters::DoorLock::DlLockDataType::kFingerprint:
        return "Fingerprint";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockOperationType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlLockOperationType::kLock:
        return "Lock";
    case chip::app::Clusters::DoorLock::DlLockOperationType::kUnlock:
        return "Unlock";
    case chip::app::Clusters::DoorLock::DlLockOperationType::kNonAccessUserEvent:
        return "NonAccessUserEvent";
    case chip::app::Clusters::DoorLock::DlLockOperationType::kForcedUserEvent:
        return "ForcedUserEvent";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockState & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlLockState::kNotFullyLocked:
        return "NotFullyLocked";
    case chip::app::Clusters::DoorLock::DlLockState::kLocked:
        return "Locked";
    case chip::app::Clusters::DoorLock::DlLockState::kUnlocked:
        return "Unlocked";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlLockType::kDeadBolt:
        return "DeadBolt";
    case chip::app::Clusters::DoorLock::DlLockType::kMagnetic:
        return "Magnetic";
    case chip::app::Clusters::DoorLock::DlLockType::kOther:
        return "Other";
    case chip::app::Clusters::DoorLock::DlLockType::kMortise:
        return "Mortise";
    case chip::app::Clusters::DoorLock::DlLockType::kRim:
        return "Rim";
    case chip::app::Clusters::DoorLock::DlLockType::kLatchBolt:
        return "LatchBolt";
    case chip::app::Clusters::DoorLock::DlLockType::kCylindricalLock:
        return "CylindricalLock";
    case chip::app::Clusters::DoorLock::DlLockType::kTubularLock:
        return "TubularLock";
    case chip::app::Clusters::DoorLock::DlLockType::kInterconnectedLock:
        return "InterconnectedLock";
    case chip::app::Clusters::DoorLock::DlLockType::kDeadLatch:
        return "DeadLatch";
    case chip::app::Clusters::DoorLock::DlLockType::kDoorFurniture:
        return "DoorFurniture";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlOperatingMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlOperatingMode::kNormal:
        return "Normal";
    case chip::app::Clusters::DoorLock::DlOperatingMode::kVacation:
        return "Vacation";
    case chip::app::Clusters::DoorLock::DlOperatingMode::kPrivacy:
        return "Privacy";
    case chip::app::Clusters::DoorLock::DlOperatingMode::kNoRemoteLockUnlock:
        return "NoRemoteLockUnlock";
    case chip::app::Clusters::DoorLock::DlOperatingMode::kPassage:
        return "Passage";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlOperationError & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlOperationError::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::DoorLock::DlOperationError::kInvalidCredential:
        return "InvalidCredential";
    case chip::app::Clusters::DoorLock::DlOperationError::kDisabledUserDenied:
        return "DisabledUserDenied";
    case chip::app::Clusters::DoorLock::DlOperationError::kRestricted:
        return "Restricted";
    case chip::app::Clusters::DoorLock::DlOperationError::kInsufficientBattery:
        return "InsufficientBattery";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlOperationSource & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlOperationSource::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::DoorLock::DlOperationSource::kManual:
        return "Manual";
    case chip::app::Clusters::DoorLock::DlOperationSource::kProprietaryRemote:
        return "ProprietaryRemote";
    case chip::app::Clusters::DoorLock::DlOperationSource::kKeypad:
        return "Keypad";
    case chip::app::Clusters::DoorLock::DlOperationSource::kAuto:
        return "Auto";
    case chip::app::Clusters::DoorLock::DlOperationSource::kButton:
        return "Button";
    case chip::app::Clusters::DoorLock::DlOperationSource::kSchedule:
        return "Schedule";
    case chip::app::Clusters::DoorLock::DlOperationSource::kRemote:
        return "Remote";
    case chip::app::Clusters::DoorLock::DlOperationSource::kRfid:
        return "Rfid";
    case chip::app::Clusters::DoorLock::DlOperationSource::kBiometric:
        return "Biometric";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlStatus::kSuccess:
        return "Success";
    case chip::app::Clusters::DoorLock::DlStatus::kFailure:
        return "Failure";
    case chip::app::Clusters::DoorLock::DlStatus::kDuplicate:
        return "Duplicate";
    case chip::app::Clusters::DoorLock::DlStatus::kOccupied:
        return "SensedOccupancy";
    case chip::app::Clusters::DoorLock::DlStatus::kInvalidField:
        return "InvalidField";
    case chip::app::Clusters::DoorLock::DlStatus::kResourceExhausted:
        return "ResourceExhausted";
    case chip::app::Clusters::DoorLock::DlStatus::kNotFound:
        return "NotFound";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlUserStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlUserStatus::kAvailable:
        return "Available";
    case chip::app::Clusters::DoorLock::DlUserStatus::kOccupiedEnabled:
        return "OccupiedEnabled";
    case chip::app::Clusters::DoorLock::DlUserStatus::kOccupiedDisabled:
        return "OccupiedDisabled";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlUserType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DlUserType::kUnrestrictedUser:
        return "UnrestrictedUser";
    case chip::app::Clusters::DoorLock::DlUserType::kYearDayScheduleUser:
        return "YearDayScheduleUser";
    case chip::app::Clusters::DoorLock::DlUserType::kWeekDayScheduleUser:
        return "WeekDayScheduleUser";
    case chip::app::Clusters::DoorLock::DlUserType::kProgrammingUser:
        return "ProgrammingUser";
    case chip::app::Clusters::DoorLock::DlUserType::kNonAccessUser:
        return "NonAccessUser";
    case chip::app::Clusters::DoorLock::DlUserType::kForcedUser:
        return "ForcedUser";
    case chip::app::Clusters::DoorLock::DlUserType::kDisposableUser:
        return "DisposableUser";
    case chip::app::Clusters::DoorLock::DlUserType::kExpiringUser:
        return "ExpiringUser";
    case chip::app::Clusters::DoorLock::DlUserType::kScheduleRestrictedUser:
        return "ScheduleRestrictedUser";
    case chip::app::Clusters::DoorLock::DlUserType::kRemoteOnlyUser:
        return "RemoteOnlyUser";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockOperationEventCode & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kUnknownOrMfgSpecific:
        return "UnknownOrMfgSpecific";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kLock:
        return "Lock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kUnlock:
        return "Unlock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kLockInvalidPinOrId:
        return "LockInvalidPinOrId";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kLockInvalidSchedule:
        return "LockInvalidSchedule";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kUnlockInvalidPinOrId:
        return "UnlockInvalidPinOrId";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kUnlockInvalidSchedule:
        return "UnlockInvalidSchedule";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kOneTouchLock:
        return "OneTouchLock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kKeyLock:
        return "KeyLock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kKeyUnlock:
        return "KeyUnlock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kAutoLock:
        return "AutoLock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kScheduleLock:
        return "ScheduleLock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kScheduleUnlock:
        return "ScheduleUnlock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kManualLock:
        return "ManualLock";
    case chip::app::Clusters::DoorLock::DoorLockOperationEventCode::kManualUnlock:
        return "ManualUnlock";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kUnknownOrMfgSpecific:
        return "UnknownOrMfgSpecific";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kMasterCodeChanged:
        return "MasterCodeChanged";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kPinAdded:
        return "PinAdded";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kPinDeleted:
        return "PinDeleted";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kPinChanged:
        return "PinChanged";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kIdAdded:
        return "IdAdded";
    case chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode::kIdDeleted:
        return "IdDeleted";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus::kSuccess:
        return "Success";
    case chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus::kGeneralFailure:
        return "GeneralFailure";
    case chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus::kMemoryFull:
        return "MemoryFull";
    case chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus::kDuplicateCodeError:
        return "DuplicateCodeError";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockUserStatus & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DoorLockUserStatus::kAvailable:
        return "Available";
    case chip::app::Clusters::DoorLock::DoorLockUserStatus::kOccupiedEnabled:
        return "OccupiedEnabled";
    case chip::app::Clusters::DoorLock::DoorLockUserStatus::kOccupiedDisabled:
        return "OccupiedDisabled";
    case chip::app::Clusters::DoorLock::DoorLockUserStatus::kNotSupported:
        return "NotSupported";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockUserType & value)
{
    switch (value)
    {
    case chip::app::Clusters::DoorLock::DoorLockUserType::kUnrestricted:
        return "Unrestricted";
    case chip::app::Clusters::DoorLock::DoorLockUserType::kYearDayScheduleUser:
        return "YearDayScheduleUser";
    case chip::app::Clusters::DoorLock::DoorLockUserType::kWeekDayScheduleUser:
        return "WeekDayScheduleUser";
    case chip::app::Clusters::DoorLock::DoorLockUserType::kMasterUser:
        return "MasterUser";
    case chip::app::Clusters::DoorLock::DoorLockUserType::kNonAccessUser:
        return "NonAccessUser";
    case chip::app::Clusters::DoorLock::DoorLockUserType::kNotSupported:
        return "NotSupported";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<WindowCovering::ConfigStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<WindowCovering::Feature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<WindowCovering::Mode, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<WindowCovering::OperationalStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<WindowCovering::SafetyStatus, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WindowCovering::EndProductType & value)
{
    switch (value)
    {
    case chip::app::Clusters::WindowCovering::EndProductType::kRollerShade:
        return "RollerShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kRomanShade:
        return "RomanShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kBalloonShade:
        return "BalloonShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kWovenWood:
        return "WovenWood";
    case chip::app::Clusters::WindowCovering::EndProductType::kPleatedShade:
        return "PleatedShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kCellularShade:
        return "CellularShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kLayeredShade:
        return "LayeredShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kLayeredShade2D:
        return "LayeredShade2D";
    case chip::app::Clusters::WindowCovering::EndProductType::kSheerShade:
        return "SheerShade";
    case chip::app::Clusters::WindowCovering::EndProductType::kTiltOnlyInteriorBlind:
        return "TiltOnlyInteriorBlind";
    case chip::app::Clusters::WindowCovering::EndProductType::kInteriorBlind:
        return "InteriorBlind";
    case chip::app::Clusters::WindowCovering::EndProductType::kVerticalBlindStripCurtain:
        return "VerticalBlindStripCurtain";
    case chip::app::Clusters::WindowCovering::EndProductType::kInteriorVenetianBlind:
        return "InteriorVenetianBlind";
    case chip::app::Clusters::WindowCovering::EndProductType::kExteriorVenetianBlind:
        return "ExteriorVenetianBlind";
    case chip::app::Clusters::WindowCovering::EndProductType::kLateralLeftCurtain:
        return "LateralLeftCurtain";
    case chip::app::Clusters::WindowCovering::EndProductType::kLateralRightCurtain:
        return "LateralRightCurtain";
    case chip::app::Clusters::WindowCovering::EndProductType::kCentralCurtain:
        return "CentralCurtain";
    case chip::app::Clusters::WindowCovering::EndProductType::kRollerShutter:
        return "RollerShutter";
    case chip::app::Clusters::WindowCovering::EndProductType::kExteriorVerticalScreen:
        return "ExteriorVerticalScreen";
    case chip::app::Clusters::WindowCovering::EndProductType::kAwningTerracePatio:
        return "AwningTerracePatio";
    case chip::app::Clusters::WindowCovering::EndProductType::kAwningVerticalScreen:
        return "AwningVerticalScreen";
    case chip::app::Clusters::WindowCovering::EndProductType::kTiltOnlyPergola:
        return "TiltOnlyPergola";
    case chip::app::Clusters::WindowCovering::EndProductType::kSwingingShutter:
        return "SwingingShutter";
    case chip::app::Clusters::WindowCovering::EndProductType::kSlidingShutter:
        return "SlidingShutter";
    case chip::app::Clusters::WindowCovering::EndProductType::kUnknown:
        return "Unknown";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::WindowCovering::Type & value)
{
    switch (value)
    {
    case chip::app::Clusters::WindowCovering::Type::kRollerShade:
        return "RollerShade";
    case chip::app::Clusters::WindowCovering::Type::kRollerShade2Motor:
        return "RollerShade2Motor";
    case chip::app::Clusters::WindowCovering::Type::kRollerShadeExterior:
        return "RollerShadeExterior";
    case chip::app::Clusters::WindowCovering::Type::kRollerShadeExterior2Motor:
        return "RollerShadeExterior2Motor";
    case chip::app::Clusters::WindowCovering::Type::kDrapery:
        return "Drapery";
    case chip::app::Clusters::WindowCovering::Type::kAwning:
        return "Awning";
    case chip::app::Clusters::WindowCovering::Type::kShutter:
        return "Shutter";
    case chip::app::Clusters::WindowCovering::Type::kTiltBlindTiltOnly:
        return "TiltBlindTiltOnly";
    case chip::app::Clusters::WindowCovering::Type::kTiltBlindLiftAndTilt:
        return "TiltBlindLiftAndTilt";
    case chip::app::Clusters::WindowCovering::Type::kProjectorScreen:
        return "ProjectorScreen";
    case chip::app::Clusters::WindowCovering::Type::kUnknown:
        return "Unknown";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<Thermostat::DayOfWeek, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<Thermostat::ModeForSequence, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<Thermostat::ThermostatFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const Thermostat::Structs::ThermostatScheduleTransition::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::SetpointAdjustMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::Thermostat::SetpointAdjustMode::kHeatSetpoint:
        return "HeatSetpoint";
    case chip::app::Clusters::Thermostat::SetpointAdjustMode::kCoolSetpoint:
        return "CoolSetpoint";
    case chip::app::Clusters::Thermostat::SetpointAdjustMode::kHeatAndCoolSetpoints:
        return "HeatAndCoolSetpoints";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatControlSequence & value)
{
    switch (value)
    {
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kCoolingOnly:
        return "CoolingOnly";
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kCoolingWithReheat:
        return "CoolingWithReheat";
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kHeatingOnly:
        return "HeatingOnly";
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kHeatingWithReheat:
        return "HeatingWithReheat";
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kCoolingAndHeating:
        return "CoolingAndHeating";
    case chip::app::Clusters::Thermostat::ThermostatControlSequence::kCoolingAndHeatingWithReheat:
        return "CoolingAndHeatingWithReheat";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatRunningMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::Thermostat::ThermostatRunningMode::kOff:
        return "Off";
    case chip::app::Clusters::Thermostat::ThermostatRunningMode::kCool:
        return "Cool";
    case chip::app::Clusters::Thermostat::ThermostatRunningMode::kHeat:
        return "Heat";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatSystemMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kOff:
        return "Off";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kAuto:
        return "Auto";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kCool:
        return "Cool";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kHeat:
        return "Heat";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kEmergencyHeating:
        return "EmergencyHeating";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kPrecooling:
        return "Precooling";
    case chip::app::Clusters::Thermostat::ThermostatSystemMode::kFanOnly:
        return "FanOnly";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<FanControl::FanControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<FanControl::RockSupportMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<FanControl::WindSettingMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<FanControl::WindSupportMask, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::FanModeSequenceType & value)
{
    switch (value)
    {
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffLowMedHigh:
        return "OffLowMedHigh";
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffLowHigh:
        return "OffLowHigh";
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffLowMedHighAuto:
        return "OffLowMedHighAuto";
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffLowHighAuto:
        return "OffLowHighAuto";
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffOnAuto:
        return "OffOnAuto";
    case chip::app::Clusters::FanControl::FanModeSequenceType::kOffOn:
        return "OffOn";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::FanModeType & value)
{
    switch (value)
    {
    case chip::app::Clusters::FanControl::FanModeType::kOff:
        return "Off";
    case chip::app::Clusters::FanControl::FanModeType::kLow:
        return "Low";
    case chip::app::Clusters::FanControl::FanModeType::kMedium:
        return "Medium";
    case chip::app::Clusters::FanControl::FanModeType::kHigh:
        return "High";
    case chip::app::Clusters::FanControl::FanModeType::kOn:
        return "On";
    case chip::app::Clusters::FanControl::FanModeType::kAuto:
        return "Auto";
    case chip::app::Clusters::FanControl::FanModeType::kSmart:
        return "Smart";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorCapabilities, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorControlFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<ColorControl::ColorLoopUpdateFlags, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorLoopAction & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::ColorLoopAction::kDeactivate:
        return "Deactivate";
    case chip::app::Clusters::ColorControl::ColorLoopAction::kActivateFromColorLoopStartEnhancedHue:
        return "ActivateFromColorLoopStartEnhancedHue";
    case chip::app::Clusters::ColorControl::ColorLoopAction::kActivateFromEnhancedCurrentHue:
        return "ActivateFromEnhancedCurrentHue";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorLoopDirection & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::ColorLoopDirection::kDecrementHue:
        return "DecrementHue";
    case chip::app::Clusters::ColorControl::ColorLoopDirection::kIncrementHue:
        return "IncrementHue";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::ColorMode::kCurrentHueAndCurrentSaturation:
        return "CurrentHueAndCurrentSaturation";
    case chip::app::Clusters::ColorControl::ColorMode::kCurrentXAndCurrentY:
        return "CurrentXAndCurrentY";
    case chip::app::Clusters::ColorControl::ColorMode::kColorTemperature:
        return "ColorTemperatureMireds";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueDirection & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::HueDirection::kShortestDistance:
        return "ShortestDistance";
    case chip::app::Clusters::ColorControl::HueDirection::kLongestDistance:
        return "LongestDistance";
    case chip::app::Clusters::ColorControl::HueDirection::kUp:
        return "Up";
    case chip::app::Clusters::ColorControl::HueDirection::kDown:
        return "Down";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueMoveMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::HueMoveMode::kStop:
        return "Stop";
    case chip::app::Clusters::ColorControl::HueMoveMode::kUp:
        return "Up";
    case chip::app::Clusters::ColorControl::HueMoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueStepMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::HueStepMode::kUp:
        return "Up";
    case chip::app::Clusters::ColorControl::HueStepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::SaturationMoveMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::SaturationMoveMode::kStop:
        return "Stop";
    case chip::app::Clusters::ColorControl::SaturationMoveMode::kUp:
        return "Up";
    case chip::app::Clusters::ColorControl::SaturationMoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::SaturationStepMode & value)
{
    switch (value)
    {
    case chip::app::Clusters::ColorControl::SaturationStepMode::kUp:
        return "Up";
    case chip::app::Clusters::ColorControl::SaturationStepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::IlluminanceMeasurement::LightSensorType & value)
{
    switch (value)
    {
    case chip::app::Clusters::IlluminanceMeasurement::LightSensorType::kPhotodiode:
        return "Photodiode";
    case chip::app::Clusters::IlluminanceMeasurement::LightSensorType::kCmos:
        return "Cmos";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<PressureMeasurement::PressureFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<Channel::ChannelFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const Channel::Structs::ChannelInfo::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const Channel::Structs::LineupInfo::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Channel::ChannelStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::Channel::ChannelStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::Channel::ChannelStatusEnum::kMultipleMatches:
        return "MultipleMatches";
    case chip::app::Clusters::Channel::ChannelStatusEnum::kNoMatches:
        return "NoMatches";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Channel::LineupInfoTypeEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::Channel::LineupInfoTypeEnum::kMso:
        return "Mso";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const TargetNavigator::Structs::TargetInfo::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum::kTargetNotFound:
        return "TargetNotFound";
    case chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum::kNotAllowed:
        return "NotAllowed";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<MediaPlayback::MediaPlaybackFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const MediaPlayback::Structs::PlaybackPosition::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kInvalidStateForCommand:
        return "InvalidStateForCommand";
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kNotAllowed:
        return "NotAllowed";
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kNotActive:
        return "NotActive";
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kSpeedOutOfRange:
        return "SpeedOutOfRange";
    case chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum::kSeekOutOfRange:
        return "SeekOutOfRange";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::MediaPlayback::PlaybackStateEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPlaying:
        return "Playing";
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPaused:
        return "Paused";
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kNotPlaying:
        return "NotPlaying";
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kBuffering:
        return "Buffering";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<MediaInput::MediaInputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const MediaInput::Structs::InputInfo::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::MediaInput::InputTypeEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::MediaInput::InputTypeEnum::kInternal:
        return "Internal";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kAux:
        return "Aux";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kCoax:
        return "Coax";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kComposite:
        return "Composite";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi:
        return "Hdmi";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kInput:
        return "Input";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kLine:
        return "Line";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kOptical:
        return "Optical";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kVideo:
        return "Video";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kScart:
        return "Scart";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kUsb:
        return "Usb";
    case chip::app::Clusters::MediaInput::InputTypeEnum::kOther:
        return "Other";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<KeypadInput::KeypadInputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::KeypadInput::CecKeyCode & value)
{
    switch (value)
    {
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelect:
        return "Select";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kUp:
        return "Up";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kDown:
        return "Down";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kLeft:
        return "Left";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRight:
        return "Right";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRightUp:
        return "RightUp";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRightDown:
        return "RightDown";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kLeftUp:
        return "LeftUp";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kLeftDown:
        return "LeftDown";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRootMenu:
        return "RootMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSetupMenu:
        return "SetupMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kContentsMenu:
        return "ContentsMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kFavoriteMenu:
        return "FavoriteMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kExit:
        return "Exit";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kMediaTopMenu:
        return "MediaTopMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kMediaContextSensitiveMenu:
        return "MediaContextSensitiveMenu";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumberEntryMode:
        return "NumberEntryMode";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumber11:
        return "Number11";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumber12:
        return "Number12";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumber0OrNumber10:
        return "Number0OrNumber10";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers1:
        return "Numbers1";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers2:
        return "Numbers2";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers3:
        return "Numbers3";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers4:
        return "Numbers4";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers5:
        return "Numbers5";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers6:
        return "Numbers6";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers7:
        return "Numbers7";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers8:
        return "Numbers8";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNumbers9:
        return "Numbers9";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kDot:
        return "Dot";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kEnter:
        return "Enter";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kClear:
        return "Clear";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kNextFavorite:
        return "NextFavorite";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kChannelUp:
        return "ChannelUp";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kChannelDown:
        return "ChannelDown";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPreviousChannel:
        return "PreviousChannel";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSoundSelect:
        return "SoundSelect";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kInputSelect:
        return "InputSelect";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kDisplayInformation:
        return "DisplayInformation";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kHelp:
        return "Help";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPageUp:
        return "PageUp";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPageDown:
        return "PageDown";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPower:
        return "Power";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kVolumeUp:
        return "VolumeUp";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kVolumeDown:
        return "VolumeDown";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kMute:
        return "Mute";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPlay:
        return "Play";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kStop:
        return "Stop";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPause:
        return "Pause";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRecord:
        return "Record";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRewind:
        return "Rewind";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kFastForward:
        return "FastForward";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kEject:
        return "Eject";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kForward:
        return "Forward";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kBackward:
        return "Backward";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kStopRecord:
        return "StopRecord";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPauseRecord:
        return "PauseRecord";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kReserved:
        return "Reserved";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kAngle:
        return "Angle";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSubPicture:
        return "SubPicture";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kVideoOnDemand:
        return "VideoOnDemand";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kElectronicProgramGuide:
        return "ElectronicProgramGuide";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kTimerProgramming:
        return "TimerProgramming";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kInitialConfiguration:
        return "InitialConfiguration";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelectBroadcastType:
        return "SelectBroadcastType";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelectSoundPresentation:
        return "SelectSoundPresentation";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPlayFunction:
        return "PlayFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPausePlayFunction:
        return "PausePlayFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRecordFunction:
        return "RecordFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPauseRecordFunction:
        return "PauseRecordFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kStopFunction:
        return "StopFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kMuteFunction:
        return "MuteFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kRestoreVolumeFunction:
        return "RestoreVolumeFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kTuneFunction:
        return "TuneFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelectMediaFunction:
        return "SelectMediaFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelectAvInputFunction:
        return "SelectAvInputFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kSelectAudioInputFunction:
        return "SelectAudioInputFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPowerToggleFunction:
        return "PowerToggleFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPowerOffFunction:
        return "PowerOffFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kPowerOnFunction:
        return "PowerOnFunction";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kF1Blue:
        return "F1Blue";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kF2Red:
        return "F2Red";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kF3Green:
        return "F3Green";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kF4Yellow:
        return "F4Yellow";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kF5:
        return "F5";
    case chip::app::Clusters::KeypadInput::CecKeyCode::kData:
        return "Data";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::KeypadInput::KeypadInputStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kUnsupportedKey:
        return "UnsupportedKey";
    case chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kInvalidKeyInCurrentState:
        return "InvalidKeyInCurrentState";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<ContentLauncher::ContentLauncherFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<ContentLauncher::SupportedStreamingProtocol, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::Dimension::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::AdditionalInfo::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::Parameter::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::ContentSearch::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::StyleInformation::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ContentLauncher::Structs::BrandingInformation::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum::kUrlNotAvailable:
        return "UrlNotAvailable";
    case chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum::kAuthFailed:
        return "AuthFailed";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ContentLauncher::MetricTypeEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::ContentLauncher::MetricTypeEnum::kPixels:
        return "Pixels";
    case chip::app::Clusters::ContentLauncher::MetricTypeEnum::kPercentage:
        return "Percentage";
    default:
        return "{}";
    }
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ContentLauncher::ParameterEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kActor:
        return "Actor";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kChannel:
        return "Channel";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kCharacter:
        return "Character";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kDirector:
        return "Director";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kEvent:
        return "Event";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kFranchise:
        return "Franchise";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kGenre:
        return "Genre";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kLeague:
        return "League";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kPopularity:
        return "Popularity";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kProvider:
        return "Provider";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kSport:
        return "Sport";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kSportsTeam:
        return "SportsTeam";
    case chip::app::Clusters::ContentLauncher::ParameterEnum::kType:
        return "Type";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<AudioOutput::AudioOutputFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const AudioOutput::Structs::OutputInfo::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::AudioOutput::OutputTypeEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi:
        return "Hdmi";
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kBt:
        return "Bt";
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kOptical:
        return "Optical";
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kHeadphone:
        return "Headphone";
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kInternal:
        return "Internal";
    case chip::app::Clusters::AudioOutput::OutputTypeEnum::kOther:
        return "Other";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<ApplicationLauncher::ApplicationLauncherFeature, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const ApplicationLauncher::Structs::Application::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const ApplicationLauncher::Structs::ApplicationEP::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum::kSuccess:
        return "Success";
    case chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum::kAppNotAvailable:
        return "AppNotAvailable";
    case chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum::kSystemBusy:
        return "SystemBusy";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const ApplicationBasic::Structs::ApplicationBasicApplication::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum::kStopped:
        return "Stopped";
    case chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum::kActiveVisibleFocus:
        return "ActiveVisibleFocus";
    case chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum::kActiveHidden:
        return "ActiveHidden";
    case chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum::kActiveVisibleNotFocus:
        return "ActiveVisibleNotFocus";
    default:
        return "{}";
    }
}

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//

/***************************** Struct Converted FIXME**************/
//

/***************************** Bitmap Converter FIXME**************/
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap16MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap32MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap64MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitTesting::Bitmap8MaskMap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//
// template<> nlohmann::json to_json(const chip::BitFlags<UnitTesting::SimpleBitmap, uint8_t>& value) {
//    return "{\"no bitmap support\"}";
//}
//

/***************************** Struct Converted FIXME**************/
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::SimpleStruct::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::TestFabricScoped::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::NullablesAndOptionalsStruct::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::NestedStruct::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::NestedStructList::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::DoubleNestedStructList::Type& value) {
//    return "no struct support";
//}
//
// template<> nlohmann::json inline to_json(const UnitTesting::Structs::TestListStructOctet::Type& value) {
//    return "no struct support";
//}
//

template <>
nlohmann::json inline to_json(const chip::app::Clusters::UnitTesting::SimpleEnum & value)
{
    switch (value)
    {
    case chip::app::Clusters::UnitTesting::SimpleEnum::kUnspecified:
        return "Unspecified";
    case chip::app::Clusters::UnitTesting::SimpleEnum::kValueA:
        return "ValueA";
    case chip::app::Clusters::UnitTesting::SimpleEnum::kValueB:
        return "ValueB";
    case chip::app::Clusters::UnitTesting::SimpleEnum::kValueC:
        return "ValueC";
    default:
        return "{}";
    }
}
