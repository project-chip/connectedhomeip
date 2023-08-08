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
#include "chip_type_traits.hpp"
#include "zcl_global_types.hpp"

// Default translation
template <typename T>
std::optional<T> inline from_json(const nlohmann::json& value)
{
    if constexpr (is_nullable<T>::value) {
        auto v = from_json<typename T::UnderlyingType>(value);
        if (v.has_value()) {
            return T(v.value());
        } else {
            return std::nullopt;
        }
    } else {
        return std::optional<T>(value);
    }
}

template <>
std::optional<chip::Span<const char>> inline from_json(const nlohmann::json& value)
{
    std::string s = value.get<std::string>();
    chip::Span<const char> span(s.c_str(), s.length());
    return span;
}

/***************************** Bitmap Converters **************/

template <>
inline std::optional<Identify::EffectIdentifierEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::EffectIdentifierEnum> table = {
        { "Blink", Identify::EffectIdentifierEnum::kBlink },
        { "Breathe", Identify::EffectIdentifierEnum::kBreathe },
        { "Okay", Identify::EffectIdentifierEnum::kOkay },
        { "ChannelChange", Identify::EffectIdentifierEnum::kChannelChange },
        { "FinishEffect", Identify::EffectIdentifierEnum::kFinishEffect },
        { "StopEffect", Identify::EffectIdentifierEnum::kStopEffect },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::EffectVariantEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::EffectVariantEnum> table = {
        { "Default", Identify::EffectVariantEnum::kDefault },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::IdentifyTypeEnum> table = {
        { "None", Identify::IdentifyTypeEnum::kNone },
        { "LightOutput", Identify::IdentifyTypeEnum::kLightOutput },
        { "VisibleIndicator", Identify::IdentifyTypeEnum::kVisibleIndicator },
        { "AudibleBeep", Identify::IdentifyTypeEnum::kAudibleBeep },
        { "Display", Identify::IdentifyTypeEnum::kDisplay },
        { "Actuator", Identify::IdentifyTypeEnum::kActuator },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Groups::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Groups::Feature> r;
    r.SetField(Groups::Feature::kGroupNames, obj.value("GroupNames", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Groups::NameSupportBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Groups::NameSupportBitmap> r;
    r.SetField(Groups::NameSupportBitmap::kGroupNames, obj.value("GroupNames", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Scenes::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Scenes::Feature> r;
    r.SetField(Scenes::Feature::kSceneNames, obj.value("SceneNames", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Scenes::ScenesCopyMode>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Scenes::ScenesCopyMode> r;
    r.SetField(Scenes::ScenesCopyMode::kCopyAllScenes, obj.value("CopyAllScenes", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<OnOff::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::Feature> r;
    r.SetField(OnOff::Feature::kLighting, obj.value("Lighting", false));
    r.SetField(OnOff::Feature::kDeadFront, obj.value("DeadFront", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OnOff::OnOffControl>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::OnOffControl> r;
    r.SetField(OnOff::OnOffControl::kAcceptOnlyWhenOn, obj.value("AcceptOnlyWhenOn", false));
    return r;
}

template <>
inline std::optional<OnOff::OnOffDelayedAllOffEffectVariant> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::OnOffDelayedAllOffEffectVariant> table = {
        { "FadeToOffIn_0p8Seconds", OnOff::OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds },
        { "NoFade", OnOff::OnOffDelayedAllOffEffectVariant::kNoFade },
        { "50PercentDimDownIn_0p8SecondsThenFadeToOffIn_12Seconds",
            OnOff::OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffDyingLightEffectVariant> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::OnOffDyingLightEffectVariant> table = {
        { "20PercenterDimUpIn_0p5SecondsThenFadeToOffIn_1Second",
            OnOff::OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffEffectIdentifier> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::OnOffEffectIdentifier> table = {
        { "DelayedAllOff", OnOff::OnOffEffectIdentifier::kDelayedAllOff },
        { "DyingLight", OnOff::OnOffEffectIdentifier::kDyingLight },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::OnOffStartUpOnOff> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::OnOffStartUpOnOff> table = {
        { "SetOnOffTo0", OnOff::OnOffStartUpOnOff::kOff },
        { "SetOnOffTo1", OnOff::OnOffStartUpOnOff::kOn },
        { "TogglePreviousOnOff", OnOff::OnOffStartUpOnOff::kTogglePreviousOnOff },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<LevelControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<LevelControl::Feature> r;
    r.SetField(LevelControl::Feature::kOnOff, obj.value("OnOff", false));
    r.SetField(LevelControl::Feature::kLighting, obj.value("Lighting", false));
    r.SetField(LevelControl::Feature::kFrequency, obj.value("Frequency", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<LevelControl::LevelControlOptions>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<LevelControl::LevelControlOptions> r;
    r.SetField(LevelControl::LevelControlOptions::kExecuteIfOff, obj.value("ExecuteIfOff", false));
    r.SetField(LevelControl::LevelControlOptions::kCoupleColorTempToLevel, obj.value("CoupleColorTempToLevel", false));
    return r;
}

template <>
inline std::optional<LevelControl::MoveMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, LevelControl::MoveMode> table = {
        { "Up", LevelControl::MoveMode::kUp },
        { "Down", LevelControl::MoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<LevelControl::StepMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, LevelControl::StepMode> table = {
        { "Up", LevelControl::StepMode::kUp },
        { "Down", LevelControl::StepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<DoorLock::DaysMaskMap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DaysMaskMap> r;
    r.SetField(DoorLock::DaysMaskMap::kSunday, obj.value("Sun", false));
    r.SetField(DoorLock::DaysMaskMap::kMonday, obj.value("Mon", false));
    r.SetField(DoorLock::DaysMaskMap::kTuesday, obj.value("Tue", false));
    r.SetField(DoorLock::DaysMaskMap::kWednesday, obj.value("Wed", false));
    r.SetField(DoorLock::DaysMaskMap::kThursday, obj.value("Thu", false));
    r.SetField(DoorLock::DaysMaskMap::kFriday, obj.value("Fri", false));
    r.SetField(DoorLock::DaysMaskMap::kSaturday, obj.value("Sat", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlCredentialRuleMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRuleMask> r;
    r.SetField(DoorLock::DlCredentialRuleMask::kSingle, obj.value("Single", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kDual, obj.value("Dual", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kTri, obj.value("Tri", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlCredentialRulesSupport>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRulesSupport> r;
    r.SetField(DoorLock::DlCredentialRulesSupport::kSingle, obj.value("Single", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kDual, obj.value("Dual", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kTri, obj.value("Tri", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlDefaultConfigurationRegister>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlDefaultConfigurationRegister> r;
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kEnableLocalProgrammingEnabled,
        obj.value("DefaultEnableLocalProgrammingAttributeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kKeypadInterfaceDefaultAccessEnabled,
        obj.value("DefaultKeypadInterfaceIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kRemoteInterfaceDefaultAccessIsEnabled,
        obj.value("DefaultRFInterfaceIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kSoundEnabled, obj.value("DefaultSoundVolumeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kAutoRelockTimeSet, obj.value("DefaultAutoRelockTimeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kLEDSettingsSet, obj.value("DefaultLEDSettingsIsEnabled", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlKeypadOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlKeypadOperationEventMask> r;
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnknown, obj.value("KeypadOpUnknownOrMS", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLock, obj.value("KeypadOpLock", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlock, obj.value("KeypadOpUnlock", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLockInvalidPIN, obj.value("KeypadOpLockErrorInvalidPIN", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLockInvalidSchedule, obj.value("KeypadOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlockInvalidCode, obj.value("KeypadOpUnlockInvalidPIN", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlockInvalidSchedule, obj.value("KeypadOpUnlockInvalidSchedule", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kNonAccessUserOpEvent, obj.value("KeypadOpNonAccessUser", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlKeypadProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlKeypadProgrammingEventMask> r;
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kUnknown, obj.value("KeypadProgUnknownOrMS", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kProgrammingPINChanged, obj.value("KeypadProgMasterCodeChanged", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINAdded, obj.value("KeypadProgPINAdded", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINCleared, obj.value("KeypadProgPINDeleted", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINChanged, obj.value("KeypadProgPINChanged", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlLocalProgrammingFeatures>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlLocalProgrammingFeatures> r;
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kAddUsersCredentialsSchedulesLocally,
        obj.value("AddUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kModifyUsersCredentialsSchedulesLocally,
        obj.value("ModifyUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kClearUsersCredentialsSchedulesLocally,
        obj.value("ClearUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kAdjustLockSettingsLocally, obj.value("AdjustLockSettingsLocally", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlManualOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlManualOperationEventMask> r;
    r.SetField(DoorLock::DlManualOperationEventMask::kUnknown, obj.value("ManualOpUnknownOrMS", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnLock, obj.value("ManualOpThumbturnLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnUnlock, obj.value("ManualOpThumbturnUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kOneTouchLock, obj.value("ManualOpOneTouchLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyLock, obj.value("ManualOpKeyLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyUnlock, obj.value("ManualOpKeyUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kAutoLock, obj.value("ManualOpAutoLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleLock, obj.value("ManualOpScheduleLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleUnlock, obj.value("ManualOpScheduleUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualLock, obj.value("ManualOpLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualUnlock, obj.value("ManualOpUnlock", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRFIDOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRFIDOperationEventMask> r;
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnknown, obj.value("RFIDOpUnknownOrMS", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLock, obj.value("RFIDOpLock", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlock, obj.value("RFIDOpUnlock", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLockInvalidRFID, obj.value("RFIDOpLockErrorInvalidRFID", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLockInvalidSchedule, obj.value("RFIDOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlockInvalidRFID, obj.value("RFIDOpUnlockErrorInvalidRFID", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlockInvalidSchedule, obj.value("RFIDOpUnlockErrorInvalidSchedule", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRFIDProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRFIDProgrammingEventMask> r;
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kUnknown, obj.value("RFIDProgUnknownOrMS", false));
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kRFIDCodeAdded, obj.value("RFIDProgRFIDAdded", false));
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kRFIDCodeCleared, obj.value("RFIDProgRFIDDeleted", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRemoteOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRemoteOperationEventMask> r;
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnknown, obj.value("RFOpUnknownOrMS", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLock, obj.value("RFOpLock", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlock, obj.value("RFOpUnlock", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLockInvalidCode, obj.value("RFOpLockErrorInvalidCode", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLockInvalidSchedule, obj.value("RFOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlockInvalidCode, obj.value("RFOpUnlockInvalidCode", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlockInvalidSchedule, obj.value("RFOpUnlockInvalidSchedule", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRemoteProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRemoteProgrammingEventMask> r;
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kUnknown, obj.value("RFProgUnknownOrMS", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kProgrammingPINChanged, obj.value("", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINAdded, obj.value("RFProgPINAdded", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINCleared, obj.value("RFProgPINDeleted", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINChanged, obj.value("RFProgPINChanged", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kRFIDCodeAdded, obj.value("RFProgRFIDAdded", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kRFIDCodeCleared, obj.value("RFProgRFIDDeleted", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlSupportedOperatingModes>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlSupportedOperatingModes> r;
    r.SetField(DoorLock::DlSupportedOperatingModes::kNormal, obj.value("NormalModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kVacation, obj.value("VacationModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPrivacy, obj.value("PrivacyModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kNoRemoteLockUnlock, obj.value("NoRFLockOrUnlockModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPassage, obj.value("PassageModeSupported", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DoorLockDayOfWeek>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DoorLockDayOfWeek> r;
    r.SetField(DoorLock::DoorLockDayOfWeek::kSunday, obj.value("Sunday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kMonday, obj.value("Monday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kTuesday, obj.value("Tuesday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kWednesday, obj.value("Wednesday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kThursday, obj.value("Thursday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kFriday, obj.value("Friday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kSaturday, obj.value("Saturday", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::Feature> r;
    r.SetField(DoorLock::Feature::kPinCredential, obj.value("PIN Credential", false));
    r.SetField(DoorLock::Feature::kRfidCredential, obj.value("RFID Credential", false));
    r.SetField(DoorLock::Feature::kFingerCredentials, obj.value("Finger Credentials", false));
    r.SetField(DoorLock::Feature::kLogging, obj.value("Logging", false));
    r.SetField(DoorLock::Feature::kWeekDayAccessSchedules, obj.value("Week Day Access Schedules", false));
    r.SetField(DoorLock::Feature::kDoorPositionSensor, obj.value("Door Position Sensor", false));
    r.SetField(DoorLock::Feature::kFaceCredentials, obj.value("Face Credentials", false));
    r.SetField(DoorLock::Feature::kCredentialsOverTheAirAccess, obj.value("Credentials Over-the-Air Access", false));
    r.SetField(DoorLock::Feature::kUser, obj.value("User", false));
    r.SetField(DoorLock::Feature::kNotification, obj.value("Notification", false));
    r.SetField(DoorLock::Feature::kYearDayAccessSchedules, obj.value("Year Day Access Schedules", false));
    r.SetField(DoorLock::Feature::kHolidaySchedules, obj.value("Holiday Schedules", false));
    r.SetField(DoorLock::Feature::kUnbolt, obj.value("Unbolt", false));
    return r;
}

template <>
inline std::optional<DoorLock::AlarmCodeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::AlarmCodeEnum> table = {
        { "LockJammed", DoorLock::AlarmCodeEnum::kLockJammed },
        { "LockFactoryReset", DoorLock::AlarmCodeEnum::kLockFactoryReset },
        { "LockRadioPowerCycled", DoorLock::AlarmCodeEnum::kLockRadioPowerCycled },
        { "WrongCodeEntryLimit", DoorLock::AlarmCodeEnum::kWrongCodeEntryLimit },
        { "FrontEsceutcheonRemoved", DoorLock::AlarmCodeEnum::kFrontEsceutcheonRemoved },
        { "DoorForcedOpen", DoorLock::AlarmCodeEnum::kDoorForcedOpen },
        { "DoorAjar", DoorLock::AlarmCodeEnum::kDoorAjar },
        { "ForcedUser", DoorLock::AlarmCodeEnum::kForcedUser },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::CredentialRuleEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::CredentialRuleEnum> table = {
        { "Single", DoorLock::CredentialRuleEnum::kSingle },
        { "Dual", DoorLock::CredentialRuleEnum::kDual },
        { "Tri", DoorLock::CredentialRuleEnum::kTri },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::CredentialTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::CredentialTypeEnum> table = {
        { "ProgrammingPIN", DoorLock::CredentialTypeEnum::kProgrammingPIN },
        { "PIN", DoorLock::CredentialTypeEnum::kPin },
        { "RFID", DoorLock::CredentialTypeEnum::kRfid },
        { "Fingerprint", DoorLock::CredentialTypeEnum::kFingerprint },
        { "FingerVein", DoorLock::CredentialTypeEnum::kFingerVein },
        { "Face", DoorLock::CredentialTypeEnum::kFace },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DataOperationTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DataOperationTypeEnum> table = {
        { "Add", DoorLock::DataOperationTypeEnum::kAdd },
        { "Clear", DoorLock::DataOperationTypeEnum::kClear },
        { "Modify", DoorLock::DataOperationTypeEnum::kModify },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockState> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlLockState> table = {
        { "NotFullyLocked", DoorLock::DlLockState::kNotFullyLocked },
        { "Locked", DoorLock::DlLockState::kLocked },
        { "Unlocked", DoorLock::DlLockState::kUnlocked },
        { "Undefined", DoorLock::DlLockState::kUnlatched },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockType> from_json(const nlohmann::json& value)
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
        { "", DoorLock::DlLockType::kEurocylinder },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlStatus> table = {
        { "Success", DoorLock::DlStatus::kSuccess },
        { "Failure", DoorLock::DlStatus::kFailure },
        { "Duplicate", DoorLock::DlStatus::kDuplicate },
        { "Occupied", DoorLock::DlStatus::kOccupied },
        { "InvalidField", DoorLock::DlStatus::kInvalidField },
        { "ResourceExhausted", DoorLock::DlStatus::kResourceExhausted },
        { "NotFound", DoorLock::DlStatus::kNotFound },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockOperationEventCode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockOperationEventCode> table = {
        { "UnknownOrMS", DoorLock::DoorLockOperationEventCode::kUnknownOrMfgSpecific },
        { "Lock", DoorLock::DoorLockOperationEventCode::kLock },
        { "Unlock", DoorLock::DoorLockOperationEventCode::kUnlock },
        { "LockFailureInvalidPINOrID", DoorLock::DoorLockOperationEventCode::kLockInvalidPinOrId },
        { "LockFailureInvalidSchedule", DoorLock::DoorLockOperationEventCode::kLockInvalidSchedule },
        { "UnlockFailureInvalidPINOrID", DoorLock::DoorLockOperationEventCode::kUnlockInvalidPinOrId },
        { "UnlockFailureInvalidSchedule", DoorLock::DoorLockOperationEventCode::kUnlockInvalidSchedule },
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
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockProgrammingEventCode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockProgrammingEventCode> table = {
        { "UnknownOrMS", DoorLock::DoorLockProgrammingEventCode::kUnknownOrMfgSpecific },
        { "MasterCodeChanged", DoorLock::DoorLockProgrammingEventCode::kMasterCodeChanged },
        { "PINCodeAdded", DoorLock::DoorLockProgrammingEventCode::kPinAdded },
        { "PINCodeDeleted", DoorLock::DoorLockProgrammingEventCode::kPinDeleted },
        { "PINCodeChanged", DoorLock::DoorLockProgrammingEventCode::kPinChanged },
        { "RFIDCodeAdded", DoorLock::DoorLockProgrammingEventCode::kIdAdded },
        { "RFIDCodeDeleted", DoorLock::DoorLockProgrammingEventCode::kIdDeleted },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockSetPinOrIdStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockSetPinOrIdStatus> table = {
        { "Success", DoorLock::DoorLockSetPinOrIdStatus::kSuccess },
        { "GeneralFailure", DoorLock::DoorLockSetPinOrIdStatus::kGeneralFailure },
        { "MemoryFull", DoorLock::DoorLockSetPinOrIdStatus::kMemoryFull },
        { "DuplicateCode", DoorLock::DoorLockSetPinOrIdStatus::kDuplicateCodeError },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockUserStatus> table = {
        { "Available", DoorLock::DoorLockUserStatus::kAvailable },
        { "OccupiedEnabled", DoorLock::DoorLockUserStatus::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::DoorLockUserStatus::kOccupiedDisabled },
        { "NotSupported", DoorLock::DoorLockUserStatus::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockUserType> table = {
        { "UnrestrictedUser", DoorLock::DoorLockUserType::kUnrestricted },
        { "YearDayScheduleUser", DoorLock::DoorLockUserType::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::DoorLockUserType::kWeekDayScheduleUser },
        { "MasterUser", DoorLock::DoorLockUserType::kMasterUser },
        { "NonAccessUser", DoorLock::DoorLockUserType::kNonAccessUser },
        { "NotSupported", DoorLock::DoorLockUserType::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorStateEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorStateEnum> table = {
        { "Open", DoorLock::DoorStateEnum::kDoorOpen },
        { "Closed", DoorLock::DoorStateEnum::kDoorClosed },
        { "ErrorJammed", DoorLock::DoorStateEnum::kDoorJammed },
        { "ErrorForcedOpen", DoorLock::DoorStateEnum::kDoorForcedOpen },
        { "ErrorUnspecified", DoorLock::DoorStateEnum::kDoorUnspecifiedError },
        { "Undefined", DoorLock::DoorStateEnum::kDoorAjar },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::LockDataTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::LockDataTypeEnum> table = {
        { "Unspecified", DoorLock::LockDataTypeEnum::kUnspecified },
        { "ProgrammingCode", DoorLock::LockDataTypeEnum::kProgrammingCode },
        { "UserIndex", DoorLock::LockDataTypeEnum::kUserIndex },
        { "WeekDaySchedule", DoorLock::LockDataTypeEnum::kWeekDaySchedule },
        { "YearDaySchedule", DoorLock::LockDataTypeEnum::kYearDaySchedule },
        { "HolidaySchedule", DoorLock::LockDataTypeEnum::kHolidaySchedule },
        { "PIN", DoorLock::LockDataTypeEnum::kPin },
        { "RFID", DoorLock::LockDataTypeEnum::kRfid },
        { "Fingerprint", DoorLock::LockDataTypeEnum::kFingerprint },
        { "FingerVein", DoorLock::LockDataTypeEnum::kFingerVein },
        { "Face", DoorLock::LockDataTypeEnum::kFace },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::LockOperationTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::LockOperationTypeEnum> table = {
        { "Lock", DoorLock::LockOperationTypeEnum::kLock },
        { "Unlock", DoorLock::LockOperationTypeEnum::kUnlock },
        { "NonAccessUserEvent", DoorLock::LockOperationTypeEnum::kNonAccessUserEvent },
        { "ForcedUserEvent", DoorLock::LockOperationTypeEnum::kForcedUserEvent },
        { "Unlatch", DoorLock::LockOperationTypeEnum::kUnlatch },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperatingModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperatingModeEnum> table = {
        { "Normal", DoorLock::OperatingModeEnum::kNormal },
        { "Vacation", DoorLock::OperatingModeEnum::kVacation },
        { "Privacy", DoorLock::OperatingModeEnum::kPrivacy },
        { "NoRemoteLockUnlock", DoorLock::OperatingModeEnum::kNoRemoteLockUnlock },
        { "Passage", DoorLock::OperatingModeEnum::kPassage },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperationErrorEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperationErrorEnum> table = {
        { "Unspecified", DoorLock::OperationErrorEnum::kUnspecified },
        { "InvalidCredential", DoorLock::OperationErrorEnum::kInvalidCredential },
        { "DisabledUserDenied", DoorLock::OperationErrorEnum::kDisabledUserDenied },
        { "Restricted", DoorLock::OperationErrorEnum::kRestricted },
        { "InsufficientBattery", DoorLock::OperationErrorEnum::kInsufficientBattery },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperationSourceEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperationSourceEnum> table = {
        { "Unspecified", DoorLock::OperationSourceEnum::kUnspecified },
        { "Manual", DoorLock::OperationSourceEnum::kManual },
        { "ProprietaryRemote", DoorLock::OperationSourceEnum::kProprietaryRemote },
        { "Keypad", DoorLock::OperationSourceEnum::kKeypad },
        { "Auto", DoorLock::OperationSourceEnum::kAuto },
        { "Button", DoorLock::OperationSourceEnum::kButton },
        { "Schedule", DoorLock::OperationSourceEnum::kSchedule },
        { "Remote", DoorLock::OperationSourceEnum::kRemote },
        { "RFID", DoorLock::OperationSourceEnum::kRfid },
        { "Biometric", DoorLock::OperationSourceEnum::kBiometric },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::UserStatusEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::UserStatusEnum> table = {
        { "Available", DoorLock::UserStatusEnum::kAvailable },
        { "OccupiedEnabled", DoorLock::UserStatusEnum::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::UserStatusEnum::kOccupiedDisabled },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::UserTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::UserTypeEnum> table = {
        { "UnrestrictedUser", DoorLock::UserTypeEnum::kUnrestrictedUser },
        { "YearDayScheduleUser", DoorLock::UserTypeEnum::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::UserTypeEnum::kWeekDayScheduleUser },
        { "ProgrammingUser", DoorLock::UserTypeEnum::kProgrammingUser },
        { "NonAccessUser", DoorLock::UserTypeEnum::kNonAccessUser },
        { "ForcedUser", DoorLock::UserTypeEnum::kForcedUser },
        { "DisposableUser", DoorLock::UserTypeEnum::kDisposableUser },
        { "ExpiringUser", DoorLock::UserTypeEnum::kExpiringUser },
        { "ScheduleRestrictedUser", DoorLock::UserTypeEnum::kScheduleRestrictedUser },
        { "RemoteOnlyUser", DoorLock::UserTypeEnum::kRemoteOnlyUser },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Thermostat::DayOfWeek>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::DayOfWeek> r;
    r.SetField(Thermostat::DayOfWeek::kSunday, obj.value("Sunday", false));
    r.SetField(Thermostat::DayOfWeek::kMonday, obj.value("Monday", false));
    r.SetField(Thermostat::DayOfWeek::kTuesday, obj.value("Tuesday", false));
    r.SetField(Thermostat::DayOfWeek::kWednesday, obj.value("Wednesday", false));
    r.SetField(Thermostat::DayOfWeek::kThursday, obj.value("Thursday", false));
    r.SetField(Thermostat::DayOfWeek::kFriday, obj.value("Friday", false));
    r.SetField(Thermostat::DayOfWeek::kSaturday, obj.value("Saturday", false));
    r.SetField(Thermostat::DayOfWeek::kAway, obj.value("AwayOrVacation", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::Feature> r;
    r.SetField(Thermostat::Feature::kHeating, obj.value("Heating", false));
    r.SetField(Thermostat::Feature::kCooling, obj.value("Cooling", false));
    r.SetField(Thermostat::Feature::kOccupancy, obj.value("Occupancy", false));
    r.SetField(Thermostat::Feature::kScheduleConfiguration, obj.value("ScheduleConfiguration", false));
    r.SetField(Thermostat::Feature::kSetback, obj.value("Setback", false));
    r.SetField(Thermostat::Feature::kAutoMode, obj.value("AutoMode", false));
    r.SetField(Thermostat::Feature::kLocalTemperatureNotExposed, obj.value("LocalTemperatureNotExposed", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ModeForSequence>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ModeForSequence> r;
    r.SetField(Thermostat::ModeForSequence::kHeatSetpointPresent, obj.value("Heat", false));
    r.SetField(Thermostat::ModeForSequence::kCoolSetpointPresent, obj.value("Cool", false));
    return r;
}

template <>
inline std::optional<Thermostat::SetpointAdjustMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::SetpointAdjustMode> table = {
        { "Heat", Thermostat::SetpointAdjustMode::kHeat },
        { "Cool", Thermostat::SetpointAdjustMode::kCool },
        { "Both", Thermostat::SetpointAdjustMode::kBoth },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatControlSequence> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ThermostatControlSequence> table = {
        { "CoolingOnly", Thermostat::ThermostatControlSequence::kCoolingOnly },
        { "CoolingWithReheat", Thermostat::ThermostatControlSequence::kCoolingWithReheat },
        { "HeatingOnly", Thermostat::ThermostatControlSequence::kHeatingOnly },
        { "HeatingWithReheat", Thermostat::ThermostatControlSequence::kHeatingWithReheat },
        { "CoolingAndHeating4Pipes", Thermostat::ThermostatControlSequence::kCoolingAndHeating },
        { "CoolingAndHeating4PipesWithReheat", Thermostat::ThermostatControlSequence::kCoolingAndHeatingWithReheat },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatRunningMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ThermostatRunningMode> table = {
        { "Off", Thermostat::ThermostatRunningMode::kOff },
        { "Cool", Thermostat::ThermostatRunningMode::kCool },
        { "Heat", Thermostat::ThermostatRunningMode::kHeat },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatSystemMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ThermostatSystemMode> table = {
        { "Off", Thermostat::ThermostatSystemMode::kOff },
        { "Auto", Thermostat::ThermostatSystemMode::kAuto },
        { "Cool", Thermostat::ThermostatSystemMode::kCool },
        { "Heat", Thermostat::ThermostatSystemMode::kHeat },
        { "EmergencyHeating", Thermostat::ThermostatSystemMode::kEmergencyHeat },
        { "Precooling", Thermostat::ThermostatSystemMode::kPrecooling },
        { "FanOnly", Thermostat::ThermostatSystemMode::kFanOnly },
        { "Dry", Thermostat::ThermostatSystemMode::kDry },
        { "Sleep", Thermostat::ThermostatSystemMode::kSleep },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<FanControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::Feature> r;
    r.SetField(FanControl::Feature::kMultiSpeed, obj.value("MultiSpeed", false));
    r.SetField(FanControl::Feature::kAuto, obj.value("Auto", false));
    r.SetField(FanControl::Feature::kRocking, obj.value("Rocking", false));
    r.SetField(FanControl::Feature::kWind, obj.value("Wind", false));
    r.SetField(FanControl::Feature::kStep, obj.value("Step", false));
    r.SetField(FanControl::Feature::kAirflowDirection, obj.value("Airflow Direction", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::RockBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::RockBitmap> r;
    r.SetField(FanControl::RockBitmap::kRockLeftRight, obj.value("RockLeftRight", false));
    r.SetField(FanControl::RockBitmap::kRockUpDown, obj.value("RockUpDown", false));
    r.SetField(FanControl::RockBitmap::kRockRound, obj.value("RockRound", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::WindBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::WindBitmap> r;
    r.SetField(FanControl::WindBitmap::kSleepWind, obj.value("Sleep Wind", false));
    r.SetField(FanControl::WindBitmap::kNaturalWind, obj.value("Natural Wind", false));
    return r;
}

template <>
inline std::optional<FanControl::AirflowDirectionEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::AirflowDirectionEnum> table = {
        { "Forward", FanControl::AirflowDirectionEnum::kForward },
        { "Reverse", FanControl::AirflowDirectionEnum::kReverse },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeEnum> table = {
        { "Off", FanControl::FanModeEnum::kOff },
        { "Low", FanControl::FanModeEnum::kLow },
        { "Medium", FanControl::FanModeEnum::kMedium },
        { "High", FanControl::FanModeEnum::kHigh },
        { "On", FanControl::FanModeEnum::kOn },
        { "Auto", FanControl::FanModeEnum::kAuto },
        { "Smart", FanControl::FanModeEnum::kSmart },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeSequenceEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeSequenceEnum> table = {
        { "Off/Low/Med/High", FanControl::FanModeSequenceEnum::kOffLowMedHigh },
        { "Off/Low/High", FanControl::FanModeSequenceEnum::kOffLowHigh },
        { "Off/Low/Med/High/Auto", FanControl::FanModeSequenceEnum::kOffLowMedHighAuto },
        { "Off/Low/High/Auto", FanControl::FanModeSequenceEnum::kOffLowHighAuto },
        { "Off/On/Auto", FanControl::FanModeSequenceEnum::kOffOnAuto },
        { "Off/On", FanControl::FanModeSequenceEnum::kOffOn },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::StepDirectionEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::StepDirectionEnum> table = {
        { "Increase", FanControl::StepDirectionEnum::kIncrease },
        { "Decrease", FanControl::StepDirectionEnum::kDecrease },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<ColorControl::ColorCapabilities>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::ColorCapabilities> r;
    r.SetField(ColorControl::ColorCapabilities::kHueSaturationSupported, obj.value("HueSaturationSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kEnhancedHueSupported, obj.value("EnhancedHueSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kColorLoopSupported, obj.value("ColorLoopSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kXYAttributesSupported, obj.value("XYSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kColorTemperatureSupported, obj.value("ColorTemperatureSupported", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<ColorControl::ColorLoopUpdateFlags>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::ColorLoopUpdateFlags> r;
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateAction, obj.value("UpdateAction", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateDirection, obj.value("UpdateDirection", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateTime, obj.value("UpdateTime", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateStartHue, obj.value("UpdateStartHue", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<ColorControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::Feature> r;
    r.SetField(ColorControl::Feature::kHueAndSaturation, obj.value("HueAndSaturation", false));
    r.SetField(ColorControl::Feature::kEnhancedHue, obj.value("EnhancedHue", false));
    r.SetField(ColorControl::Feature::kColorLoop, obj.value("ColorLoop", false));
    r.SetField(ColorControl::Feature::kXy, obj.value("XY", false));
    r.SetField(ColorControl::Feature::kColorTemperature, obj.value("ColorTemperature", false));
    return r;
}

template <>
inline std::optional<ColorControl::ColorLoopAction> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorLoopAction> table = {
        { "DeactivateColorLoop", ColorControl::ColorLoopAction::kDeactivate },
        { "ActivateColorLoopFromColorLoopStartEnhancedHue", ColorControl::ColorLoopAction::kActivateFromColorLoopStartEnhancedHue },
        { "ActivateColorLoopFromEnhancedCurrentHue", ColorControl::ColorLoopAction::kActivateFromEnhancedCurrentHue },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorLoopDirection> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorLoopDirection> table = {
        { "DecrementEnhancedCurrentHue", ColorControl::ColorLoopDirection::kDecrementHue },
        { "IncrementEnhancedCurrentHue", ColorControl::ColorLoopDirection::kIncrementHue },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorMode> table = {
        { "CurrentHueAndCurrentSaturation", ColorControl::ColorMode::kCurrentHueAndCurrentSaturation },
        { "CurrentXAndCurrentY", ColorControl::ColorMode::kCurrentXAndCurrentY },
        { "ColorTemperatureMireds", ColorControl::ColorMode::kColorTemperature },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueDirection> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueDirection> table = {
        { "ShortestDistance", ColorControl::HueDirection::kShortestDistance },
        { "LongestDistance", ColorControl::HueDirection::kLongestDistance },
        { "Up", ColorControl::HueDirection::kUp },
        { "Down", ColorControl::HueDirection::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueMoveMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueMoveMode> table = {
        { "Stop", ColorControl::HueMoveMode::kStop },
        { "Up", ColorControl::HueMoveMode::kUp },
        { "Down", ColorControl::HueMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueStepMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueStepMode> table = {
        { "Up", ColorControl::HueStepMode::kUp },
        { "Down", ColorControl::HueStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationMoveMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::SaturationMoveMode> table = {
        { "Stop", ColorControl::SaturationMoveMode::kStop },
        { "Up", ColorControl::SaturationMoveMode::kUp },
        { "Down", ColorControl::SaturationMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationStepMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::SaturationStepMode> table = {
        { "Up", ColorControl::SaturationStepMode::kUp },
        { "Down", ColorControl::SaturationStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

template <>
inline std::optional<IlluminanceMeasurement::LightSensorTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, IlluminanceMeasurement::LightSensorTypeEnum> table = {
        { "Photodiode", IlluminanceMeasurement::LightSensorTypeEnum::kPhotodiode },
        { "CMOS", IlluminanceMeasurement::LightSensorTypeEnum::kCmos },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<PressureMeasurement::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<PressureMeasurement::Feature> r;
    r.SetField(PressureMeasurement::Feature::kExtended, obj.value("Extended", false));
    return r;
}

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<OccupancySensing::OccupancyBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OccupancySensing::OccupancyBitmap> r;
    r.SetField(OccupancySensing::OccupancyBitmap::kOccupied, obj.value("SensedOccupancy", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OccupancySensing::OccupancySensorTypeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OccupancySensing::OccupancySensorTypeBitmap> r;
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kPir, obj.value("PIR", false));
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic, obj.value("Ultrasonic", false));
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact, obj.value("PhysicalContact", false));
    return r;
}

template <>
inline std::optional<OccupancySensing::OccupancySensorTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OccupancySensing::OccupancySensorTypeEnum> table = {
        { "PIR", OccupancySensing::OccupancySensorTypeEnum::kPir },
        { "Ultrasonic", OccupancySensing::OccupancySensorTypeEnum::kUltrasonic },
        { "PIRAndUltrasonic", OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic },
        { "PhysicalContact", OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
