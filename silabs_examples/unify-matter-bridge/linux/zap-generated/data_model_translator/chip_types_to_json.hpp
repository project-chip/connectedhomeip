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
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyEffectIdentifier& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case IdentifyEffectIdentifier::kBlink:
        return "Blink";
    case IdentifyEffectIdentifier::kBreathe:
        return "Breathe";
    case IdentifyEffectIdentifier::kOkay:
        return "Okay";
    case IdentifyEffectIdentifier::kChannelChange:
        return "ChannelChange";
    case IdentifyEffectIdentifier::kFinishEffect:
        return "FinishEffect";
    case IdentifyEffectIdentifier::kStopEffect:
        return "StopEffect";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyEffectVariant& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case IdentifyEffectVariant::kDefault:
        return "Default";
    default:
        return "{}";
    }
}
template <>
nlohmann::json inline to_json(const chip::app::Clusters::Identify::IdentifyIdentifyType& value)
{
    using namespace chip::app::Clusters::Identify;
    switch (value) {
    case IdentifyIdentifyType::kNone:
        return "None FIXME";
    case IdentifyIdentifyType::kVisibleLight:
        return "VisibleLight FIXME";
    case IdentifyIdentifyType::kVisibleLED:
        return "VisibleLED FIXME";
    case IdentifyIdentifyType::kAudibleBeep:
        return "AudibleBeep FIXME";
    case IdentifyIdentifyType::kDisplay:
        return "Display FIXME";
    case IdentifyIdentifyType::kActuator:
        return "Actuator FIXME";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Groups::GroupClusterFeature>& value)
{
    using namespace chip::app::Clusters::Groups;
    nlohmann::json obj;
    obj["GroupNames FIXME"] = static_cast<bool>(value.GetField(GroupClusterFeature::kGroupNames));
    return obj;
}

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Scenes::SceneFeatures>& value)
{
    using namespace chip::app::Clusters::Scenes;
    nlohmann::json obj;
    obj["SceneNames FIXME"] = static_cast<bool>(value.GetField(SceneFeatures::kSceneNames));
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
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OnOff::OnOffControl>& value)
{
    using namespace chip::app::Clusters::OnOff;
    nlohmann::json obj;
    obj["AcceptOnlyWhenOn"] = static_cast<bool>(value.GetField(OnOffControl::kAcceptOnlyWhenOn));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::OnOff::OnOffFeature>& value)
{
    using namespace chip::app::Clusters::OnOff;
    nlohmann::json obj;
    obj["Lighting FIXME"] = static_cast<bool>(value.GetField(OnOffFeature::kLighting));
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
        return "20PercenterDimUpIn_0p5SecondsThenFadeToOffIn_1Second FIXME";
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
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::LevelControl::LevelControlFeature>& value)
{
    using namespace chip::app::Clusters::LevelControl;
    nlohmann::json obj;
    obj["OnOff FIXME"] = static_cast<bool>(value.GetField(LevelControlFeature::kOnOff));
    obj["Lighting FIXME"] = static_cast<bool>(value.GetField(LevelControlFeature::kLighting));
    obj["Frequency FIXME"] = static_cast<bool>(value.GetField(LevelControlFeature::kFrequency));
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
    obj["Sunday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kSunday));
    obj["Monday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kMonday));
    obj["Tuesday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kTuesday));
    obj["Wednesday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kWednesday));
    obj["Thursday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kThursday));
    obj["Friday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kFriday));
    obj["Saturday FIXME"] = static_cast<bool>(value.GetField(DaysMaskMap::kSaturday));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Single FIXME"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kSingle));
    obj["Dual FIXME"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kDual));
    obj["Tri FIXME"] = static_cast<bool>(value.GetField(DlCredentialRuleMask::kTri));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRulesSupport>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Single FIXME"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kSingle));
    obj["Dual FIXME"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kDual));
    obj["Tri FIXME"] = static_cast<bool>(value.GetField(DlCredentialRulesSupport::kTri));
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
    obj["AddUsersCredentialsSchedulesLocally FIXME"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kAddUsersCredentialsSchedulesLocally));
    obj["ModifyUsersCredentialsSchedulesLocally FIXME"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kModifyUsersCredentialsSchedulesLocally));
    obj["ClearUsersCredentialsSchedulesLocally FIXME"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kClearUsersCredentialsSchedulesLocally));
    obj["AdjustLockSettingsLocally FIXME"] = static_cast<bool>(value.GetField(DlLocalProgrammingFeatures::kAdjustLockSettingsLocally));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DlManualOperationEventMask>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Unknown FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kUnknown));
    obj["ThumbturnLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kThumbturnLock));
    obj["ThumbturnUnlock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kThumbturnUnlock));
    obj["OneTouchLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kOneTouchLock));
    obj["KeyLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kKeyLock));
    obj["KeyUnlock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kKeyUnlock));
    obj["AutoLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kAutoLock));
    obj["ScheduleLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kScheduleLock));
    obj["ScheduleUnlock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kScheduleUnlock));
    obj["ManualLock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kManualLock));
    obj["ManualUnlock FIXME"] = static_cast<bool>(value.GetField(DlManualOperationEventMask::kManualUnlock));
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
    obj["Normal FIXME"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kNormal));
    obj["Vacation FIXME"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kVacation));
    obj["Privacy FIXME"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kPrivacy));
    obj["NoRemoteLockUnlock FIXME"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kNoRemoteLockUnlock));
    obj["Passage FIXME"] = static_cast<bool>(value.GetField(DlSupportedOperatingModes::kPassage));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DoorLockDayOfWeek>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["Sunday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kSunday));
    obj["Monday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kMonday));
    obj["Tuesday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kTuesday));
    obj["Wednesday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kWednesday));
    obj["Thursday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kThursday));
    obj["Friday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kFriday));
    obj["Saturday FIXME"] = static_cast<bool>(value.GetField(DoorLockDayOfWeek::kSaturday));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::DoorLock::DoorLockFeature>& value)
{
    using namespace chip::app::Clusters::DoorLock;
    nlohmann::json obj;
    obj["PIN Credential FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kPinCredential));
    obj["RFID Credential FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kRfidCredential));
    obj["Finger Credentials FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kFingerCredentials));
    obj["Logging FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kLogging));
    obj["Week Day Access Schedules FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kWeekDayAccessSchedules));
    obj["Door Position Sensor FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kDoorPositionSensor));
    obj["Face Credentials FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kFaceCredentials));
    obj["Credentials Over-the-Air Access FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kCredentialsOverTheAirAccess));
    obj["User FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kUser));
    obj["Notification FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kNotification));
    obj["Year Day Access Schedules FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kYearDayAccessSchedules));
    obj["Holiday Schedules FIXME"] = static_cast<bool>(value.GetField(DoorLockFeature::kHolidaySchedules));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::DoorLock::AlarmCodeEnum& value)
{
    using namespace chip::app::Clusters::DoorLock;
    switch (value) {
    case AlarmCodeEnum::kLockJammed:
        return "LockJammed FIXME";
    case AlarmCodeEnum::kLockFactoryReset:
        return "LockFactoryReset FIXME";
    case AlarmCodeEnum::kLockRadioPowerCycled:
        return "LockRadioPowerCycled FIXME";
    case AlarmCodeEnum::kWrongCodeEntryLimit:
        return "WrongCodeEntryLimit FIXME";
    case AlarmCodeEnum::kFrontEsceutcheonRemoved:
        return "FrontEsceutcheonRemoved FIXME";
    case AlarmCodeEnum::kDoorForcedOpen:
        return "DoorForcedOpen FIXME";
    case AlarmCodeEnum::kDoorAjar:
        return "DoorAjar FIXME";
    case AlarmCodeEnum::kForcedUser:
        return "ForcedUser FIXME";
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
        return "Single FIXME";
    case CredentialRuleEnum::kDual:
        return "Dual FIXME";
    case CredentialRuleEnum::kTri:
        return "Tri FIXME";
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
        return "ProgrammingPIN FIXME";
    case CredentialTypeEnum::kPin:
        return "PIN FIXME";
    case CredentialTypeEnum::kRfid:
        return "RFID FIXME";
    case CredentialTypeEnum::kFingerprint:
        return "Fingerprint FIXME";
    case CredentialTypeEnum::kFingerVein:
        return "FingerVein FIXME";
    case CredentialTypeEnum::kFace:
        return "Face FIXME";
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
        return "Add FIXME";
    case DataOperationTypeEnum::kClear:
        return "Clear FIXME";
    case DataOperationTypeEnum::kModify:
        return "Modify FIXME";
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
        return "Success FIXME";
    case DlStatus::kFailure:
        return "Failure FIXME";
    case DlStatus::kDuplicate:
        return "Duplicate FIXME";
    case DlStatus::kOccupied:
        return "Occupied FIXME";
    case DlStatus::kInvalidField:
        return "InvalidField FIXME";
    case DlStatus::kResourceExhausted:
        return "ResourceExhausted FIXME";
    case DlStatus::kNotFound:
        return "NotFound FIXME";
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
        return "DoorOpen FIXME";
    case DoorStateEnum::kDoorClosed:
        return "DoorClosed FIXME";
    case DoorStateEnum::kDoorJammed:
        return "DoorJammed FIXME";
    case DoorStateEnum::kDoorForcedOpen:
        return "DoorForcedOpen FIXME";
    case DoorStateEnum::kDoorUnspecifiedError:
        return "DoorUnspecifiedError FIXME";
    case DoorStateEnum::kDoorAjar:
        return "DoorAjar FIXME";
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
        return "Unspecified FIXME";
    case LockDataTypeEnum::kProgrammingCode:
        return "ProgrammingCode FIXME";
    case LockDataTypeEnum::kUserIndex:
        return "UserIndex FIXME";
    case LockDataTypeEnum::kWeekDaySchedule:
        return "WeekDaySchedule FIXME";
    case LockDataTypeEnum::kYearDaySchedule:
        return "YearDaySchedule FIXME";
    case LockDataTypeEnum::kHolidaySchedule:
        return "HolidaySchedule FIXME";
    case LockDataTypeEnum::kPin:
        return "PIN FIXME";
    case LockDataTypeEnum::kRfid:
        return "RFID FIXME";
    case LockDataTypeEnum::kFingerprint:
        return "Fingerprint FIXME";
    case LockDataTypeEnum::kFingerVein:
        return "FingerVein FIXME";
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
        return "Lock FIXME";
    case LockOperationTypeEnum::kUnlock:
        return "Unlock FIXME";
    case LockOperationTypeEnum::kNonAccessUserEvent:
        return "NonAccessUserEvent FIXME";
    case LockOperationTypeEnum::kForcedUserEvent:
        return "ForcedUserEvent FIXME";
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
        return "Normal FIXME";
    case OperatingModeEnum::kVacation:
        return "Vacation FIXME";
    case OperatingModeEnum::kPrivacy:
        return "Privacy FIXME";
    case OperatingModeEnum::kNoRemoteLockUnlock:
        return "NoRemoteLockUnlock FIXME";
    case OperatingModeEnum::kPassage:
        return "Passage FIXME";
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
        return "Unspecified FIXME";
    case OperationErrorEnum::kInvalidCredential:
        return "InvalidCredential FIXME";
    case OperationErrorEnum::kDisabledUserDenied:
        return "DisabledUserDenied FIXME";
    case OperationErrorEnum::kRestricted:
        return "Restricted FIXME";
    case OperationErrorEnum::kInsufficientBattery:
        return "InsufficientBattery FIXME";
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
        return "Unspecified FIXME";
    case OperationSourceEnum::kManual:
        return "Manual FIXME";
    case OperationSourceEnum::kProprietaryRemote:
        return "ProprietaryRemote FIXME";
    case OperationSourceEnum::kKeypad:
        return "Keypad FIXME";
    case OperationSourceEnum::kAuto:
        return "Auto FIXME";
    case OperationSourceEnum::kButton:
        return "Button FIXME";
    case OperationSourceEnum::kSchedule:
        return "Schedule FIXME";
    case OperationSourceEnum::kRemote:
        return "Remote FIXME";
    case OperationSourceEnum::kRfid:
        return "RFID FIXME";
    case OperationSourceEnum::kBiometric:
        return "Biometric FIXME";
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
        return "Available FIXME";
    case UserStatusEnum::kOccupiedEnabled:
        return "OccupiedEnabled FIXME";
    case UserStatusEnum::kOccupiedDisabled:
        return "OccupiedDisabled FIXME";
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
        return "UnrestrictedUser FIXME";
    case UserTypeEnum::kYearDayScheduleUser:
        return "YearDayScheduleUser FIXME";
    case UserTypeEnum::kWeekDayScheduleUser:
        return "WeekDayScheduleUser FIXME";
    case UserTypeEnum::kProgrammingUser:
        return "ProgrammingUser FIXME";
    case UserTypeEnum::kNonAccessUser:
        return "NonAccessUser FIXME";
    case UserTypeEnum::kForcedUser:
        return "ForcedUser FIXME";
    case UserTypeEnum::kDisposableUser:
        return "DisposableUser FIXME";
    case UserTypeEnum::kExpiringUser:
        return "ExpiringUser FIXME";
    case UserTypeEnum::kScheduleRestrictedUser:
        return "ScheduleRestrictedUser FIXME";
    case UserTypeEnum::kRemoteOnlyUser:
        return "RemoteOnlyUser FIXME";
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
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Thermostat::ModeForSequence>& value)
{
    using namespace chip::app::Clusters::Thermostat;
    nlohmann::json obj;
    obj["Heat"] = static_cast<bool>(value.GetField(ModeForSequence::kHeatSetpointPresent));
    obj["Cool"] = static_cast<bool>(value.GetField(ModeForSequence::kCoolSetpointPresent));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::Thermostat::ThermostatFeature>& value)
{
    using namespace chip::app::Clusters::Thermostat;
    nlohmann::json obj;
    obj["Heating FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kHeating));
    obj["Cooling FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kCooling));
    obj["Occupancy FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kOccupancy));
    obj["ScheduleConfiguration FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kScheduleConfiguration));
    obj["Setback FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kSetback));
    obj["AutoMode FIXME"] = static_cast<bool>(value.GetField(ThermostatFeature::kAutoMode));
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
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::FanControlFeature>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Multi-Speed FIXME"] = static_cast<bool>(value.GetField(FanControlFeature::kMultiSpeed));
    obj["Auto FIXME"] = static_cast<bool>(value.GetField(FanControlFeature::kAuto));
    obj["Rocking FIXME"] = static_cast<bool>(value.GetField(FanControlFeature::kRocking));
    obj["Wind FIXME"] = static_cast<bool>(value.GetField(FanControlFeature::kWind));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::RockSupportMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["RockLeftRight FIXME"] = static_cast<bool>(value.GetField(RockSupportMask::kRockLeftRight));
    obj["RockUpDown FIXME"] = static_cast<bool>(value.GetField(RockSupportMask::kRockUpDown));
    obj["RockRound FIXME"] = static_cast<bool>(value.GetField(RockSupportMask::kRockRound));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::WindSettingMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Sleep Wind FIXME"] = static_cast<bool>(value.GetField(WindSettingMask::kSleepWind));
    obj["Natural Wind FIXME"] = static_cast<bool>(value.GetField(WindSettingMask::kNaturalWind));
    return obj;
}
template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::FanControl::WindSupportMask>& value)
{
    using namespace chip::app::Clusters::FanControl;
    nlohmann::json obj;
    obj["Sleep Wind FIXME"] = static_cast<bool>(value.GetField(WindSupportMask::kSleepWind));
    obj["Natural Wind FIXME"] = static_cast<bool>(value.GetField(WindSupportMask::kNaturalWind));
    return obj;
}

template <>
nlohmann::json inline to_json(const chip::app::Clusters::FanControl::FanModeSequenceType& value)
{
    using namespace chip::app::Clusters::FanControl;
    switch (value) {
    case FanModeSequenceType::kOffLowMedHigh:
        return "Off/Low/Med/High FIXME";
    case FanModeSequenceType::kOffLowHigh:
        return "Off/Low/High FIXME";
    case FanModeSequenceType::kOffLowMedHighAuto:
        return "Off/Low/Med/High/Auto FIXME";
    case FanModeSequenceType::kOffLowHighAuto:
        return "Off/Low/High/Auto FIXME";
    case FanModeSequenceType::kOffOnAuto:
        return "Off/On/Auto FIXME";
    case FanModeSequenceType::kOffOn:
        return "Off/On FIXME";
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
        return "Off FIXME";
    case FanModeType::kLow:
        return "Low FIXME";
    case FanModeType::kMedium:
        return "Medium FIXME";
    case FanModeType::kHigh:
        return "High FIXME";
    case FanModeType::kOn:
        return "On FIXME";
    case FanModeType::kAuto:
        return "Auto FIXME";
    case FanModeType::kSmart:
        return "Smart FIXME";
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
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::ColorControl::ColorControlFeature>& value)
{
    using namespace chip::app::Clusters::ColorControl;
    nlohmann::json obj;
    obj["HueAndSaturation FIXME"] = static_cast<bool>(value.GetField(ColorControlFeature::kHueAndSaturation));
    obj["EnhancedHue FIXME"] = static_cast<bool>(value.GetField(ColorControlFeature::kEnhancedHue));
    obj["ColorLoop FIXME"] = static_cast<bool>(value.GetField(ColorControlFeature::kColorLoop));
    obj["XY FIXME"] = static_cast<bool>(value.GetField(ColorControlFeature::kXy));
    obj["ColorTemperature FIXME"] = static_cast<bool>(value.GetField(ColorControlFeature::kColorTemperature));
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
        return "Stop FIXME";
    case SaturationMoveMode::kUp:
        return "Up FIXME";
    case SaturationMoveMode::kDown:
        return "Down FIXME";
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
        return "Up FIXME";
    case SaturationStepMode::kDown:
        return "Down FIXME";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::app::Clusters::IlluminanceMeasurement::LightSensorType& value)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;
    switch (value) {
    case LightSensorType::kPhotodiode:
        return "Photodiode";
    case LightSensorType::kCmos:
        return "CMOS";
    default:
        return "{}";
    }
}
/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

template <>
nlohmann::json inline to_json(const chip::BitMask<chip::app::Clusters::PressureMeasurement::PressureFeature>& value)
{
    using namespace chip::app::Clusters::PressureMeasurement;
    nlohmann::json obj;
    obj["Extended FIXME"] = static_cast<bool>(value.GetField(PressureFeature::kExtended));
    return obj;
}

/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/

/***************************** Bitmap Converter FIXME**************/
