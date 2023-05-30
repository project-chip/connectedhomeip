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
nlohmann::json inline to_json(const T& value)
{
    return value;
}

nlohmann::json inline to_json(const chip::Span<const char>& value)
{
    return std::string(value.data(), value.size());
}

nlohmann::json inline to_json(const chip::Span<const unsigned char>& value)
{
    return std::string(reinterpret_cast<const char*>(value.data()), value.size());
}

template <typename T>
nlohmann::json inline to_json(const chip::app::DataModel::DecodableList<T>& value)
{
    return "{}";
}

template <typename T>
nlohmann::json inline to_json(chip::app::DataModel::Nullable<T>& value)
{
    if ((!value.IsNull()) && value.HasValidValue()) {
        return to_json(value.Value());
    }
    return nlohmann::json::value_t::null;
}

/***************************** Bitmap Convertes **************/
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::EffectIdentifierEnum& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case EffectIdentifierEnum::kBlink:
        return "Blink";
    case EffectIdentifierEnum::kBreathe:
        return "Breathe";
    case EffectIdentifierEnum::kOkay:
        return "Okay";
    case EffectIdentifierEnum::kChannelChange:
        return "ChannelChange";
    case EffectIdentifierEnum::kFinishEffect:
        return "FinishEffect";
    case EffectIdentifierEnum::kStopEffect:
        return "StopEffect";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::EffectVariantEnum& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case EffectVariantEnum::kDefault:
        return "Default";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyTypeEnum& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case IdentifyTypeEnum::kNone:
        return "None";
    case IdentifyTypeEnum::kLightOutput:
        return "LightOutput";
    case IdentifyTypeEnum::kVisibleIndicator:
        return "VisibleIndicator";
    case IdentifyTypeEnum::kAudibleBeep:
        return "AudibleBeep";
    case IdentifyTypeEnum::kDisplay:
        return "Display";
    case IdentifyTypeEnum::kActuator:
        return "Actuator";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Groups::Feature>& value)
{
    using namespace chip::app::Clusters::Groups;
    nlohmann::json obj;
    obj["GroupNames"] = static_cast<bool>(value.GetField(Feature::kGroupNames));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Groups::NameSupportBitmap>& value)
{
    using namespace chip::app::Clusters::Groups;
    nlohmann::json obj;
    obj["GroupNames"] = static_cast<bool>(value.GetField(NameSupportBitmap::kGroupNames));
    return obj;
}

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Scenes::Feature>& value)
{
    using namespace chip::app::Clusters::Scenes;
    nlohmann::json obj;
    obj["SceneNames"] = static_cast<bool>(value.GetField(Feature::kSceneNames));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Scenes::ScenesCopyMode>& value)
{
    using namespace chip::app::Clusters::Scenes;
    nlohmann::json obj;
    obj["CopyAllScenes"] = static_cast<bool>(value.GetField(ScenesCopyMode::kCopyAllScenes));
    return obj;
}

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OnOff::Feature>& value)
{
    using namespace chip::app::Clusters::OnOff;
    nlohmann::json obj;
    obj["Lighting"] = static_cast<bool>(value.GetField(Feature::kLighting));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OnOff::OnOffControl>& value)
{
    using namespace chip::app::Clusters::OnOff;
    nlohmann::json obj;
    obj["AcceptOnlyWhenOn"] = static_cast<bool>(value.GetField(OnOffControl::kAcceptOnlyWhenOn));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant& value)
{
    using namespace chip::app::Clusters::OnOff;
    return static_cast<uint8_t>(value);
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant& value)
{
    using namespace chip::app::Clusters::OnOff;
    switch (value) {
    case OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second:
        return "20PercenterDimUpIn_0p5SecondsThenFadeToOffIn_1Second";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffEffectIdentifier& value)
{
    using namespace chip::app::Clusters::OnOff;
    switch (value) {
    case OnOffEffectIdentifier::kDelayedAllOff:
        return "DelayedAllOff";
    case OnOffEffectIdentifier::kDyingLight:
        return "DyingLight";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::OnOff::OnOffStartUpOnOff& value)
{
    using namespace chip::app::Clusters::OnOff;
    switch (value) {
    case OnOffStartUpOnOff::kOff:
        return "SetOnOffTo0";
    case OnOffStartUpOnOff::kOn:
        return "SetOnOffTo1";
    case OnOffStartUpOnOff::kTogglePreviousOnOff:
        return "TogglePreviousOnOff";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::LevelControl::Feature>& value)
{
    using namespace chip::app::Clusters::LevelControl;
    nlohmann::json obj;
    obj["OnOff"] = static_cast<bool>(value.GetField(Feature::kOnOff));
    obj["Lighting"] = static_cast<bool>(value.GetField(Feature::kLighting));
    obj["Frequency"] = static_cast<bool>(value.GetField(Feature::kFrequency));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions>& value)
{
    using namespace chip::app::Clusters::LevelControl;
    nlohmann::json obj;
    obj["ExecuteIfOff"] = static_cast<bool>(value.GetField(LevelControlOptions::kExecuteIfOff));
    obj["CoupleColorTempToLevel"] = static_cast<bool>(value.GetField(LevelControlOptions::kCoupleColorTempToLevel));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::LevelControl::MoveMode& value)
{
    using namespace chip::app::Clusters::LevelControl;
    switch (value) {
    case MoveMode::kUp:
        return "Up";
    case MoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::LevelControl::StepMode& value)
{
    using namespace chip::app::Clusters::LevelControl;
    switch (value) {
    case StepMode::kUp:
        return "Up";
    case StepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DaysMaskMap>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Sun"] = static_cast<bool>(value.GetField(DaysMaskMap::kSunday));
    obj["Mon"] = static_cast<bool>(value.GetField(DaysMaskMap::kMonday));
    obj["Tue"] = static_cast<bool>(value.GetField(DaysMaskMap::kTuesday));
    obj["Wed"] = static_cast<bool>(value.GetField(DaysMaskMap::kWednesday));
    obj["Thu"] = static_cast<bool>(value.GetField(DaysMaskMap::kThursday));
    obj["Fri"] = static_cast<bool>(value.GetField(DaysMaskMap::kFriday));
    obj["Sat"] = static_cast<bool>(value.GetField(DaysMaskMap::kSaturday));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Single"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kSingle));
    obj["Dual"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kDual));
    obj["Tri"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kTri));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRulesSupport>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Single"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kSingle));
    obj["Dual"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kDual));
    obj["Tri"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kTri));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["DefaultEnableLocalProgrammingAttributeIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kEnableLocalProgrammingEnabled));
    obj["DefaultKeypadInterfaceIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kKeypadInterfaceDefaultAccessEnabled));
    obj["DefaultRFInterfaceIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kRemoteInterfaceDefaultAccessIsEnabled));
    obj["DefaultSoundVolumeIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kSoundEnabled));
    obj["DefaultAutoRelockTimeIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kAutoRelockTimeSet));
    obj["DefaultLEDSettingsIsEnabled"] = static_cast<bool>(value.GetField(DlDefaultConfigurationRegister::kLEDSettingsSet));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlKeypadOperationEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["KeypadOpUnknownOrMS"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kUnknown));
    obj["KeypadOpLock"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kLock));
    obj["KeypadOpUnlock"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kUnlock));
    obj["KeypadOpLockErrorInvalidPIN"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kLockInvalidPIN));
    obj["KeypadOpLockErrorInvalidSchedule"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kLockInvalidSchedule));
    obj["KeypadOpUnlockInvalidPIN"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kUnlockInvalidCode));
    obj["KeypadOpUnlockInvalidSchedule"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kUnlockInvalidSchedule));
    obj["KeypadOpNonAccessUser"] = static_cast<bool>(value.GetField(DlKeypadOperationEventMask::kNonAccessUserOpEvent));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlKeypadProgrammingEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["KeypadProgUnknownOrMS"] = static_cast<bool>(value.GetField(DlKeypadProgrammingEventMask::kUnknown));
    obj["KeypadProgMasterCodeChanged"] = static_cast<bool>(value.GetField(DlKeypadProgrammingEventMask::kProgrammingPINChanged));
    obj["KeypadProgPINAdded"] = static_cast<bool>(value.GetField(DlKeypadProgrammingEventMask::kPINAdded));
    obj["KeypadProgPINDeleted"] = static_cast<bool>(value.GetField(DlKeypadProgrammingEventMask::kPINCleared));
    obj["KeypadProgPINChanged"] = static_cast<bool>(value.GetField(DlKeypadProgrammingEventMask::kPINChanged));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["AddUsersCredentialsSchedulesLocally"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kAddUsersCredentialsSchedulesLocally));
    obj["ModifyUsersCredentialsSchedulesLocally"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kModifyUsersCredentialsSchedulesLocally));
    obj["ClearUsersCredentialsSchedulesLocally"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kClearUsersCredentialsSchedulesLocally));
    obj["AdjustLockSettingsLocally"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kAdjustLockSettingsLocally));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlManualOperationEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["ManualOpUnknownOrMS"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kUnknown));
    obj["ManualOpThumbturnLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kThumbturnLock));
    obj["ManualOpThumbturnUnlock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kThumbturnUnlock));
    obj["ManualOpOneTouchLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kOneTouchLock));
    obj["ManualOpKeyLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kKeyLock));
    obj["ManualOpKeyUnlock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kKeyUnlock));
    obj["ManualOpAutoLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kAutoLock));
    obj["ManualOpScheduleLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kScheduleLock));
    obj["ManualOpScheduleUnlock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kScheduleUnlock));
    obj["ManualOpLock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kManualLock));
    obj["ManualOpUnlock"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kManualUnlock));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlRFIDOperationEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["RFIDOpUnknownOrMS"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kUnknown));
    obj["RFIDOpLock"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kLock));
    obj["RFIDOpUnlock"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kUnlock));
    obj["RFIDOpLockErrorInvalidRFID"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kLockInvalidRFID));
    obj["RFIDOpLockErrorInvalidSchedule"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kLockInvalidSchedule));
    obj["RFIDOpUnlockErrorInvalidRFID"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kUnlockInvalidRFID));
    obj["RFIDOpUnlockErrorInvalidSchedule"] = static_cast<bool>(value.GetField(DlRFIDOperationEventMask::kUnlockInvalidSchedule));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlRFIDProgrammingEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["RFIDProgUnknownOrMS"] = static_cast<bool>(value.GetField(DlRFIDProgrammingEventMask::kUnknown));
    obj["RFIDProgRFIDAdded"] = static_cast<bool>(value.GetField(DlRFIDProgrammingEventMask::kRFIDCodeAdded));
    obj["RFIDProgRFIDDeleted"] = static_cast<bool>(value.GetField(DlRFIDProgrammingEventMask::kRFIDCodeCleared));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlRemoteOperationEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["RFOpUnknownOrMS"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kUnknown));
    obj["RFOpLock"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kLock));
    obj["RFOpUnlock"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kUnlock));
    obj["RFOpLockErrorInvalidCode"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kLockInvalidCode));
    obj["RFOpLockErrorInvalidSchedule"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kLockInvalidSchedule));
    obj["RFOpUnlockInvalidCode"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kUnlockInvalidCode));
    obj["RFOpUnlockInvalidSchedule"] = static_cast<bool>(value.GetField(DlRemoteOperationEventMask::kUnlockInvalidSchedule));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlRemoteProgrammingEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["RFProgUnknownOrMS"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kUnknown));
    obj[""] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kProgrammingPINChanged));
    obj["RFProgPINAdded"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kPINAdded));
    obj["RFProgPINDeleted"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kPINCleared));
    obj["RFProgPINChanged"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kPINChanged));
    obj["RFProgRFIDAdded"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kRFIDCodeAdded));
    obj["RFProgRFIDDeleted"] = static_cast<bool>(value.GetField(DlRemoteProgrammingEventMask::kRFIDCodeCleared));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["NormalModeSupported"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kNormal));
    obj["VacationModeSupported"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kVacation));
    obj["PrivacyModeSupported"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kPrivacy));
    obj["NoRFLockOrUnlockModeSupported"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kNoRemoteLockUnlock));
    obj["PassageModeSupported"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kPassage));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DoorLockDayOfWeek>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Sunday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kSunday));
    obj["Monday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kMonday));
    obj["Tuesday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kTuesday));
    obj["Wednesday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kWednesday));
    obj["Thursday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kThursday));
    obj["Friday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kFriday));
    obj["Saturday"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kSaturday));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::Feature>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["PIN Credential"] = static_cast<bool>(value.GetField(Feature::kPinCredential));
    obj["RFID Credential"] = static_cast<bool>(value.GetField(Feature::kRfidCredential));
    obj["Finger Credentials"] = static_cast<bool>(value.GetField(Feature::kFingerCredentials));
    obj["Logging"] = static_cast<bool>(value.GetField(Feature::kLogging));
    obj["Week Day Access Schedules"] = static_cast<bool>(value.GetField(Feature::kWeekDayAccessSchedules));
    obj["Door Position Sensor"] = static_cast<bool>(value.GetField(Feature::kDoorPositionSensor));
    obj["Face Credentials"] = static_cast<bool>(value.GetField(Feature::kFaceCredentials));
    obj["Credentials Over-the-Air Access"] = static_cast<bool>(value.GetField(Feature::kCredentialsOverTheAirAccess));
    obj["User"] = static_cast<bool>(value.GetField(Feature::kUser));
    obj["Notification"] = static_cast<bool>(value.GetField(Feature::kNotification));
    obj["Year Day Access Schedules"] = static_cast<bool>(value.GetField(Feature::kYearDayAccessSchedules));
    obj["Holiday Schedules"] = static_cast<bool>(value.GetField(Feature::kHolidaySchedules));
    obj["Unbolt"] = static_cast<bool>(value.GetField(Feature::kUnbolt));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::AlarmCodeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case AlarmCodeEnum::kLockJammed:
        return "LockJammed";
    case AlarmCodeEnum::kLockFactoryReset:
        return "LockFactoryReset";
    case AlarmCodeEnum::kLockRadioPowerCycled:
        return "LockRadioPowerCycled";
    case AlarmCodeEnum::kWrongCodeEntryLimit:
        return "WrongCodeEntryLimit";
    case AlarmCodeEnum::kFrontEsceutcheonRemoved:
        return "FrontEsceutcheonRemoved";
    case AlarmCodeEnum::kDoorForcedOpen:
        return "DoorForcedOpen";
    case AlarmCodeEnum::kDoorAjar:
        return "DoorAjar";
    case AlarmCodeEnum::kForcedUser:
        return "ForcedUser";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::CredentialRuleEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case CredentialRuleEnum::kSingle:
        return "Single";
    case CredentialRuleEnum::kDual:
        return "Dual";
    case CredentialRuleEnum::kTri:
        return "Tri";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::CredentialTypeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case CredentialTypeEnum::kProgrammingPIN:
        return "ProgrammingPIN";
    case CredentialTypeEnum::kPin:
        return "PIN";
    case CredentialTypeEnum::kRfid:
        return "RFID";
    case CredentialTypeEnum::kFingerprint:
        return "Fingerprint";
    case CredentialTypeEnum::kFingerVein:
        return "FingerVein";
    case CredentialTypeEnum::kFace:
        return "Face";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DataOperationTypeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DataOperationTypeEnum::kAdd:
        return "Add";
    case DataOperationTypeEnum::kClear:
        return "Clear";
    case DataOperationTypeEnum::kModify:
        return "Modify";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockState& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DlLockState::kNotFullyLocked:
        return "NotFullyLocked";
    case DlLockState::kLocked:
        return "Locked";
    case DlLockState::kUnlocked:
        return "Unlocked";
    case DlLockState::kUnlatched:
        return "Undefined";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlLockType& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DlLockType::kDeadBolt:
        return "DeadBolt";
    case DlLockType::kMagnetic:
        return "Magnetic";
    case DlLockType::kOther:
        return "Other";
    case DlLockType::kMortise:
        return "Mortise";
    case DlLockType::kRim:
        return "Rim";
    case DlLockType::kLatchBolt:
        return "LatchBolt";
    case DlLockType::kCylindricalLock:
        return "CylindricalLock";
    case DlLockType::kTubularLock:
        return "TubularLock";
    case DlLockType::kInterconnectedLock:
        return "InterconnectedLock";
    case DlLockType::kDeadLatch:
        return "DeadLatch";
    case DlLockType::kDoorFurniture:
        return "DoorFurniture";
    case DlLockType::kEurocylinder:
        return "";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DlStatus& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DlStatus::kSuccess:
        return "Success";
    case DlStatus::kFailure:
        return "Failure";
    case DlStatus::kDuplicate:
        return "Duplicate";
    case DlStatus::kOccupied:
        return "Occupied";
    case DlStatus::kInvalidField:
        return "InvalidField";
    case DlStatus::kResourceExhausted:
        return "ResourceExhausted";
    case DlStatus::kNotFound:
        return "NotFound";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockOperationEventCode& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorLockOperationEventCode::kUnknownOrMfgSpecific:
        return "UnknownOrMS";
    case DoorLockOperationEventCode::kLock:
        return "Lock";
    case DoorLockOperationEventCode::kUnlock:
        return "Unlock";
    case DoorLockOperationEventCode::kLockInvalidPinOrId:
        return "LockFailureInvalidPINOrID";
    case DoorLockOperationEventCode::kLockInvalidSchedule:
        return "LockFailureInvalidSchedule";
    case DoorLockOperationEventCode::kUnlockInvalidPinOrId:
        return "UnlockFailureInvalidPINOrID";
    case DoorLockOperationEventCode::kUnlockInvalidSchedule:
        return "UnlockFailureInvalidSchedule";
    case DoorLockOperationEventCode::kOneTouchLock:
        return "OneTouchLock";
    case DoorLockOperationEventCode::kKeyLock:
        return "KeyLock";
    case DoorLockOperationEventCode::kKeyUnlock:
        return "KeyUnlock";
    case DoorLockOperationEventCode::kAutoLock:
        return "AutoLock";
    case DoorLockOperationEventCode::kScheduleLock:
        return "ScheduleLock";
    case DoorLockOperationEventCode::kScheduleUnlock:
        return "ScheduleUnlock";
    case DoorLockOperationEventCode::kManualLock:
        return "ManualLock";
    case DoorLockOperationEventCode::kManualUnlock:
        return "ManualUnlock";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorLockProgrammingEventCode::kUnknownOrMfgSpecific:
        return "UnknownOrMS";
    case DoorLockProgrammingEventCode::kMasterCodeChanged:
        return "MasterCodeChanged";
    case DoorLockProgrammingEventCode::kPinAdded:
        return "PINCodeAdded";
    case DoorLockProgrammingEventCode::kPinDeleted:
        return "PINCodeDeleted";
    case DoorLockProgrammingEventCode::kPinChanged:
        return "PINCodeChanged";
    case DoorLockProgrammingEventCode::kIdAdded:
        return "RFIDCodeAdded";
    case DoorLockProgrammingEventCode::kIdDeleted:
        return "RFIDCodeDeleted";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorLockSetPinOrIdStatus::kSuccess:
        return "Success";
    case DoorLockSetPinOrIdStatus::kGeneralFailure:
        return "GeneralFailure";
    case DoorLockSetPinOrIdStatus::kMemoryFull:
        return "MemoryFull";
    case DoorLockSetPinOrIdStatus::kDuplicateCodeError:
        return "DuplicateCode";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockUserStatus& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorLockUserStatus::kAvailable:
        return "Available";
    case DoorLockUserStatus::kOccupiedEnabled:
        return "OccupiedEnabled";
    case DoorLockUserStatus::kOccupiedDisabled:
        return "OccupiedDisabled";
    case DoorLockUserStatus::kNotSupported:
        return "NotSupported";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorLockUserType& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorLockUserType::kUnrestricted:
        return "UnrestrictedUser";
    case DoorLockUserType::kYearDayScheduleUser:
        return "YearDayScheduleUser";
    case DoorLockUserType::kWeekDayScheduleUser:
        return "WeekDayScheduleUser";
    case DoorLockUserType::kMasterUser:
        return "MasterUser";
    case DoorLockUserType::kNonAccessUser:
        return "NonAccessUser";
    case DoorLockUserType::kNotSupported:
        return "NotSupported";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::DoorStateEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case DoorStateEnum::kDoorOpen:
        return "Open";
    case DoorStateEnum::kDoorClosed:
        return "Closed";
    case DoorStateEnum::kDoorJammed:
        return "ErrorJammed";
    case DoorStateEnum::kDoorForcedOpen:
        return "ErrorForcedOpen";
    case DoorStateEnum::kDoorUnspecifiedError:
        return "ErrorUnspecified";
    case DoorStateEnum::kDoorAjar:
        return "Undefined";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::LockDataTypeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case LockDataTypeEnum::kUnspecified:
        return "Unspecified";
    case LockDataTypeEnum::kProgrammingCode:
        return "ProgrammingCode";
    case LockDataTypeEnum::kUserIndex:
        return "UserIndex";
    case LockDataTypeEnum::kWeekDaySchedule:
        return "WeekDaySchedule";
    case LockDataTypeEnum::kYearDaySchedule:
        return "YearDaySchedule";
    case LockDataTypeEnum::kHolidaySchedule:
        return "HolidaySchedule";
    case LockDataTypeEnum::kPin:
        return "PIN";
    case LockDataTypeEnum::kRfid:
        return "RFID";
    case LockDataTypeEnum::kFingerprint:
        return "Fingerprint";
    case LockDataTypeEnum::kFingerVein:
        return "FingerVein";
    case LockDataTypeEnum::kFace:
        return "Face";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::LockOperationTypeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case LockOperationTypeEnum::kLock:
        return "Lock";
    case LockOperationTypeEnum::kUnlock:
        return "Unlock";
    case LockOperationTypeEnum::kNonAccessUserEvent:
        return "NonAccessUserEvent";
    case LockOperationTypeEnum::kForcedUserEvent:
        return "ForcedUserEvent";
    case LockOperationTypeEnum::kUnlatch:
        return "Unlatch";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::OperatingModeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case OperatingModeEnum::kNormal:
        return "Normal";
    case OperatingModeEnum::kVacation:
        return "Vacation";
    case OperatingModeEnum::kPrivacy:
        return "Privacy";
    case OperatingModeEnum::kNoRemoteLockUnlock:
        return "NoRemoteLockUnlock";
    case OperatingModeEnum::kPassage:
        return "Passage";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::OperationErrorEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case OperationErrorEnum::kUnspecified:
        return "Unspecified";
    case OperationErrorEnum::kInvalidCredential:
        return "InvalidCredential";
    case OperationErrorEnum::kDisabledUserDenied:
        return "DisabledUserDenied";
    case OperationErrorEnum::kRestricted:
        return "Restricted";
    case OperationErrorEnum::kInsufficientBattery:
        return "InsufficientBattery";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::OperationSourceEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case OperationSourceEnum::kUnspecified:
        return "Unspecified";
    case OperationSourceEnum::kManual:
        return "Manual";
    case OperationSourceEnum::kProprietaryRemote:
        return "ProprietaryRemote";
    case OperationSourceEnum::kKeypad:
        return "Keypad";
    case OperationSourceEnum::kAuto:
        return "Auto";
    case OperationSourceEnum::kButton:
        return "Button";
    case OperationSourceEnum::kSchedule:
        return "Schedule";
    case OperationSourceEnum::kRemote:
        return "Remote";
    case OperationSourceEnum::kRfid:
        return "RFID";
    case OperationSourceEnum::kBiometric:
        return "Biometric";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::UserStatusEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case UserStatusEnum::kAvailable:
        return "Available";
    case UserStatusEnum::kOccupiedEnabled:
        return "OccupiedEnabled";
    case UserStatusEnum::kOccupiedDisabled:
        return "OccupiedDisabled";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::UserTypeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case UserTypeEnum::kUnrestrictedUser:
        return "UnrestrictedUser";
    case UserTypeEnum::kYearDayScheduleUser:
        return "YearDayScheduleUser";
    case UserTypeEnum::kWeekDayScheduleUser:
        return "WeekDayScheduleUser";
    case UserTypeEnum::kProgrammingUser:
        return "ProgrammingUser";
    case UserTypeEnum::kNonAccessUser:
        return "NonAccessUser";
    case UserTypeEnum::kForcedUser:
        return "ForcedUser";
    case UserTypeEnum::kDisposableUser:
        return "DisposableUser";
    case UserTypeEnum::kExpiringUser:
        return "ExpiringUser";
    case UserTypeEnum::kScheduleRestrictedUser:
        return "ScheduleRestrictedUser";
    case UserTypeEnum::kRemoteOnlyUser:
        return "RemoteOnlyUser";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Thermostat::DayOfWeek>& value)
{
    using namespace chip::app::Clusters::Thermostat;
    nlohmann::json obj;
    obj["Sunday"] = static_cast<bool>(value.GetField(DayOfWeek::kSunday));
    obj["Monday"] = static_cast<bool>(value.GetField(DayOfWeek::kMonday));
    obj["Tuesday"] = static_cast<bool>(value.GetField(DayOfWeek::kTuesday));
    obj["Wednesday"] = static_cast<bool>(value.GetField(DayOfWeek::kWednesday));
    obj["Thursday"] = static_cast<bool>(value.GetField(DayOfWeek::kThursday));
    obj["Friday"] = static_cast<bool>(value.GetField(DayOfWeek::kFriday));
    obj["Saturday"] = static_cast<bool>(value.GetField(DayOfWeek::kSaturday));
    obj["AwayOrVacation"] = static_cast<bool>(value.GetField(DayOfWeek::kAway));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Thermostat::Feature>& value)
{
    using namespace chip::app::Clusters::Thermostat;
    nlohmann::json obj;
    obj["Heating"] = static_cast<bool>(value.GetField(Feature::kHeating));
    obj["Cooling"] = static_cast<bool>(value.GetField(Feature::kCooling));
    obj["Occupancy"] = static_cast<bool>(value.GetField(Feature::kOccupancy));
    obj["ScheduleConfiguration"] = static_cast<bool>(value.GetField(Feature::kScheduleConfiguration));
    obj["Setback"] = static_cast<bool>(value.GetField(Feature::kSetback));
    obj["AutoMode"] = static_cast<bool>(value.GetField(Feature::kAutoMode));
    obj["LocalTemperatureNotExposed"] = static_cast<bool>(value.GetField(Feature::kLocalTemperatureNotExposed));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Thermostat::ModeForSequence>& value)
{
    using namespace chip::app::Clusters::Thermostat;
    nlohmann::json obj;
    obj["Heat"] = static_cast<bool>(value.GetField(ModeForSequence::kHeatSetpointPresent));
    obj["Cool"] = static_cast<bool>(value.GetField(ModeForSequence::kCoolSetpointPresent));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::SetpointAdjustMode& value)
{
    using namespace chip::app::Clusters::Thermostat;
    switch (value) {
    case SetpointAdjustMode::kHeat:
        return "Heat";
    case SetpointAdjustMode::kCool:
        return "Cool";
    case SetpointAdjustMode::kBoth:
        return "Both";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatControlSequence& value)
{
    using namespace chip::app::Clusters::Thermostat;
    switch (value) {
    case ThermostatControlSequence::kCoolingOnly:
        return "CoolingOnly";
    case ThermostatControlSequence::kCoolingWithReheat:
        return "CoolingWithReheat";
    case ThermostatControlSequence::kHeatingOnly:
        return "HeatingOnly";
    case ThermostatControlSequence::kHeatingWithReheat:
        return "HeatingWithReheat";
    case ThermostatControlSequence::kCoolingAndHeating:
        return "CoolingAndHeating4Pipes";
    case ThermostatControlSequence::kCoolingAndHeatingWithReheat:
        return "CoolingAndHeating4PipesWithReheat";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatRunningMode& value)
{
    using namespace chip::app::Clusters::Thermostat;
    switch (value) {
    case ThermostatRunningMode::kOff:
        return "Off";
    case ThermostatRunningMode::kCool:
        return "Cool";
    case ThermostatRunningMode::kHeat:
        return "Heat";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Thermostat::ThermostatSystemMode& value)
{
    using namespace chip::app::Clusters::Thermostat;
    switch (value) {
    case ThermostatSystemMode::kOff:
        return "Off";
    case ThermostatSystemMode::kAuto:
        return "Auto";
    case ThermostatSystemMode::kCool:
        return "Cool";
    case ThermostatSystemMode::kHeat:
        return "Heat";
    case ThermostatSystemMode::kEmergencyHeat:
        return "EmergencyHeating";
    case ThermostatSystemMode::kPrecooling:
        return "Precooling";
    case ThermostatSystemMode::kFanOnly:
        return "FanOnly";
    case ThermostatSystemMode::kDry:
        return "Dry";
    case ThermostatSystemMode::kSleep:
        return "Sleep";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::Feature>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Multi-Speed"] = static_cast<bool>(value.GetField(Feature::kMultiSpeed));
    obj["Auto"] = static_cast<bool>(value.GetField(Feature::kAuto));
    obj["Rocking"] = static_cast<bool>(value.GetField(Feature::kRocking));
    obj["Wind"] = static_cast<bool>(value.GetField(Feature::kWind));
    obj["Step"] = static_cast<bool>(value.GetField(Feature::kStep));
    obj["Airflow Direction"] = static_cast<bool>(value.GetField(Feature::kAirflowDirection));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::RockSupportMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["RockLeftRight"] = static_cast<bool>(value.GetField(RockSupportMask::kRockLeftRight));
    obj["RockUpDown"] = static_cast<bool>(value.GetField(RockSupportMask::kRockUpDown));
    obj["RockRound"] = static_cast<bool>(value.GetField(RockSupportMask::kRockRound));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::WindSettingMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Sleep Wind"] = static_cast<bool>(value.GetField(WindSettingMask::kSleepWind));
    obj["Natural Wind"] = static_cast<bool>(value.GetField(WindSettingMask::kNaturalWind));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::WindSupportMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Sleep Wind"] = static_cast<bool>(value.GetField(WindSupportMask::kSleepWind));
    obj["Natural Wind"] = static_cast<bool>(value.GetField(WindSupportMask::kNaturalWind));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::AirflowDirectionEnum& value)
{
    using namespace chip::app::Clusters::FanControl;
    switch (value) {
    case AirflowDirectionEnum::kForward:
        return "Forward";
    case AirflowDirectionEnum::kReverse:
        return "Reverse";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::DirectionEnum& value)
{
    using namespace chip::app::Clusters::FanControl;
    switch (value) {
    case DirectionEnum::kIncrease:
        return "Increase";
    case DirectionEnum::kDecrease:
        return "Decrease";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::FanModeSequenceType& value)
{
    using namespace chip::app::Clusters::FanControl;
    switch (value) {
    case FanModeSequenceType::kOffLowMedHigh:
        return "Off/Low/Med/High";
    case FanModeSequenceType::kOffLowHigh:
        return "Off/Low/High";
    case FanModeSequenceType::kOffLowMedHighAuto:
        return "Off/Low/Med/High/Auto";
    case FanModeSequenceType::kOffLowHighAuto:
        return "Off/Low/High/Auto";
    case FanModeSequenceType::kOffOnAuto:
        return "Off/On/Auto";
    case FanModeSequenceType::kOffOn:
        return "Off/On";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::FanModeType& value)
{
    using namespace chip::app::Clusters::FanControl;
    switch (value) {
    case FanModeType::kOff:
        return "Off";
    case FanModeType::kLow:
        return "Low";
    case FanModeType::kMedium:
        return "Medium";
    case FanModeType::kHigh:
        return "High";
    case FanModeType::kOn:
        return "On";
    case FanModeType::kAuto:
        return "Auto";
    case FanModeType::kSmart:
        return "Smart";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::ColorControl::ColorCapabilities>& value)
{
    using namespace chip::app::Clusters::ColorControl;
    nlohmann::json obj;
    obj["HueSaturationSupported"] = static_cast<bool>(value.GetField(ColorCapabilities::kHueSaturationSupported));
    obj["EnhancedHueSupported"] = static_cast<bool>(value.GetField(ColorCapabilities::kEnhancedHueSupported));
    obj["ColorLoopSupported"] = static_cast<bool>(value.GetField(ColorCapabilities::kColorLoopSupported));
    obj["XYSupported"] = static_cast<bool>(value.GetField(ColorCapabilities::kXYAttributesSupported));
    obj["ColorTemperatureSupported"] = static_cast<bool>(value.GetField(ColorCapabilities::kColorTemperatureSupported));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::ColorControl::ColorLoopUpdateFlags>& value)
{
    using namespace chip::app::Clusters::ColorControl;
    nlohmann::json obj;
    obj["UpdateAction"] = static_cast<bool>(value.GetField(ColorLoopUpdateFlags::kUpdateAction));
    obj["UpdateDirection"] = static_cast<bool>(value.GetField(ColorLoopUpdateFlags::kUpdateDirection));
    obj["UpdateTime"] = static_cast<bool>(value.GetField(ColorLoopUpdateFlags::kUpdateTime));
    obj["UpdateStartHue"] = static_cast<bool>(value.GetField(ColorLoopUpdateFlags::kUpdateStartHue));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::ColorControl::Feature>& value)
{
    using namespace chip::app::Clusters::ColorControl;
    nlohmann::json obj;
    obj["HueAndSaturation"] = static_cast<bool>(value.GetField(Feature::kHueAndSaturation));
    obj["EnhancedHue"] = static_cast<bool>(value.GetField(Feature::kEnhancedHue));
    obj["ColorLoop"] = static_cast<bool>(value.GetField(Feature::kColorLoop));
    obj["XY"] = static_cast<bool>(value.GetField(Feature::kXy));
    obj["ColorTemperature"] = static_cast<bool>(value.GetField(Feature::kColorTemperature));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorLoopAction& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case ColorLoopAction::kDeactivate:
        return "DeactivateColorLoop";
    case ColorLoopAction::kActivateFromColorLoopStartEnhancedHue:
        return "ActivateColorLoopFromColorLoopStartEnhancedHue";
    case ColorLoopAction::kActivateFromEnhancedCurrentHue:
        return "ActivateColorLoopFromEnhancedCurrentHue";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorLoopDirection& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case ColorLoopDirection::kDecrementHue:
        return "DecrementEnhancedCurrentHue";
    case ColorLoopDirection::kIncrementHue:
        return "IncrementEnhancedCurrentHue";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::ColorMode& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case ColorMode::kCurrentHueAndCurrentSaturation:
        return "CurrentHueAndCurrentSaturation";
    case ColorMode::kCurrentXAndCurrentY:
        return "CurrentXAndCurrentY";
    case ColorMode::kColorTemperature:
        return "ColorTemperatureMireds";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueDirection& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case HueDirection::kShortestDistance:
        return "ShortestDistance";
    case HueDirection::kLongestDistance:
        return "LongestDistance";
    case HueDirection::kUp:
        return "Up";
    case HueDirection::kDown:
        return "Down";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueMoveMode& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case HueMoveMode::kStop:
        return "Stop";
    case HueMoveMode::kUp:
        return "Up";
    case HueMoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::HueStepMode& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case HueStepMode::kUp:
        return "Up";
    case HueStepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::SaturationMoveMode& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case SaturationMoveMode::kStop:
        return "Stop";
    case SaturationMoveMode::kUp:
        return "Up";
    case SaturationMoveMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::ColorControl::SaturationStepMode& value)
{
    using namespace chip::app::Clusters::ColorControl;
    switch (value) {
    case SaturationStepMode::kUp:
        return "Up";
    case SaturationStepMode::kDown:
        return "Down";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::app::Clusters::IlluminanceMeasurement::LightSensorTypeEnum& value)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;
    switch (value) {
    case LightSensorTypeEnum::kPhotodiode:
        return "Photodiode";
    case LightSensorTypeEnum::kCmos:
        return "CMOS";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::PressureMeasurement::Feature>& value)
{
    using namespace chip::app::Clusters::PressureMeasurement;
    nlohmann::json obj;
    obj["Extended"] = static_cast<bool>(value.GetField(Feature::kExtended));
    return obj;
}

/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap>& value)
{
    using namespace chip::app::Clusters::OccupancySensing;
    nlohmann::json obj;
    obj["SensedOccupancy"] = static_cast<bool>(value.GetField(OccupancyBitmap::kOccupied));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancySensorTypeBitmap>& value)
{
    using namespace chip::app::Clusters::OccupancySensing;
    nlohmann::json obj;
    obj["PIR"] = static_cast<bool>(value.GetField(OccupancySensorTypeBitmap::kPir));
    obj["Ultrasonic"] = static_cast<bool>(value.GetField(OccupancySensorTypeBitmap::kUltrasonic));
    obj["PhysicalContact"] = static_cast<bool>(value.GetField(OccupancySensorTypeBitmap::kPhysicalContact));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::OccupancySensing::OccupancySensorTypeEnum& value)
{
    using namespace chip::app::Clusters::OccupancySensing;
    switch (value) {
    case OccupancySensorTypeEnum::kPir:
        return "PIR";
    case OccupancySensorTypeEnum::kUltrasonic:
        return "Ultrasonic";
    case OccupancySensorTypeEnum::kPIRAndUltrasonic:
        return "PIRAndUltrasonic";
    case OccupancySensorTypeEnum::kPhysicalContact:
        return "PhysicalContact";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/
