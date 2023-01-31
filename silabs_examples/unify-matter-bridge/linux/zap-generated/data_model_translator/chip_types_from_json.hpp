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

template <typename T>
struct is_nullable : std::false_type {
};

template <typename T>
struct is_nullable<chip::app::DataModel::Nullable<T>> : std::true_type {
};

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
inline std::optional<Identify::IdentifyEffectIdentifier> from_json(const nlohmann::json& value)
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
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyEffectVariant> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::IdentifyEffectVariant> table = {
        { "Default", Identify::IdentifyEffectVariant::kDefault },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyIdentifyType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::IdentifyIdentifyType> table = {
        { "None FIXME", Identify::IdentifyIdentifyType::kNone },
        { "VisibleLight FIXME", Identify::IdentifyIdentifyType::kVisibleLight },
        { "VisibleLED FIXME", Identify::IdentifyIdentifyType::kVisibleLED },
        { "AudibleBeep FIXME", Identify::IdentifyIdentifyType::kAudibleBeep },
        { "Display FIXME", Identify::IdentifyIdentifyType::kDisplay },
        { "Actuator FIXME", Identify::IdentifyIdentifyType::kActuator },
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
inline std::optional<chip::BitMask<Groups::GroupClusterFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Groups::GroupClusterFeature> r;
    r.SetField(Groups::GroupClusterFeature::kGroupNames, obj.value("GroupNames FIXME", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Scenes::ScenesCopyMode>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Scenes::ScenesCopyMode> r;
    r.SetField(Scenes::ScenesCopyMode::kCopyAllScenes, obj.value("CopyAllScenes", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<OnOff::OnOffControl>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::OnOffControl> r;
    r.SetField(OnOff::OnOffControl::kAcceptOnlyWhenOn, obj.value("AcceptOnlyWhenOn", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OnOff::OnOffFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::OnOffFeature> r;
    r.SetField(OnOff::OnOffFeature::kLighting, obj.value("Lighting FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OnOff::SceneFeatures>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::SceneFeatures> r;
    r.SetField(OnOff::SceneFeatures::kSceneNames, obj.value("SceneNames FIXME", false));
    return r;
}

template <>
inline std::optional<OnOff::OnOffDelayedAllOffEffectVariant> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::OnOffDelayedAllOffEffectVariant> table = {
        { "FadeToOffIn_0p8Seconds FIXME", OnOff::OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds },
        { "NoFade FIXME", OnOff::OnOffDelayedAllOffEffectVariant::kNoFade },
        { "50PercentDimDownIn_0p8SecondsThenFadeToOffIn_12Seconds FIXME",
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
        { "20PercenterDimUpIn_0p5SecondsThenFadeToOffIn_1Second FIXME",
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
inline std::optional<chip::BitMask<LevelControl::LevelControlFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<LevelControl::LevelControlFeature> r;
    r.SetField(LevelControl::LevelControlFeature::kOnOff, obj.value("OnOff FIXME", false));
    r.SetField(LevelControl::LevelControlFeature::kLighting, obj.value("Lighting FIXME", false));
    r.SetField(LevelControl::LevelControlFeature::kFrequency, obj.value("Frequency FIXME", false));
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
inline std::optional<chip::BitMask<DoorLock::DlCredentialRuleMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRuleMask> r;
    r.SetField(DoorLock::DlCredentialRuleMask::kSingle, obj.value("Single FIXME", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kDual, obj.value("Dual FIXME", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kTri, obj.value("Tri FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlCredentialRulesSupport>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRulesSupport> r;
    r.SetField(DoorLock::DlCredentialRulesSupport::kSingle, obj.value("Single FIXME", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kDual, obj.value("Dual FIXME", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kTri, obj.value("Tri FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlDaysMaskMap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlDaysMaskMap> r;
    r.SetField(DoorLock::DlDaysMaskMap::kSunday, obj.value("Sun", false));
    r.SetField(DoorLock::DlDaysMaskMap::kMonday, obj.value("Mon", false));
    r.SetField(DoorLock::DlDaysMaskMap::kTuesday, obj.value("Tue", false));
    r.SetField(DoorLock::DlDaysMaskMap::kWednesday, obj.value("Wed", false));
    r.SetField(DoorLock::DlDaysMaskMap::kThursday, obj.value("Thu", false));
    r.SetField(DoorLock::DlDaysMaskMap::kFriday, obj.value("Fri", false));
    r.SetField(DoorLock::DlDaysMaskMap::kSaturday, obj.value("Sat", false));
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
        obj.value("AddUsersCredentialsSchedulesLocally FIXME", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kModifyUsersCredentialsSchedulesLocally,
        obj.value("ModifyUsersCredentialsSchedulesLocally FIXME", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kClearUsersCredentialsSchedulesLocally,
        obj.value("ClearUsersCredentialsSchedulesLocally FIXME", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kAdjustLockSettingsLocally,
        obj.value("AdjustLockSettingsLocally FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlManualOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlManualOperationEventMask> r;
    r.SetField(DoorLock::DlManualOperationEventMask::kUnknown, obj.value("Unknown FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnLock, obj.value("ThumbturnLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnUnlock, obj.value("ThumbturnUnlock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kOneTouchLock, obj.value("OneTouchLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyLock, obj.value("KeyLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyUnlock, obj.value("KeyUnlock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kAutoLock, obj.value("AutoLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleLock, obj.value("ScheduleLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleUnlock, obj.value("ScheduleUnlock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualLock, obj.value("ManualLock FIXME", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualUnlock, obj.value("ManualUnlock FIXME", false));
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
    r.SetField(DoorLock::DlSupportedOperatingModes::kNormal, obj.value("Normal FIXME", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kVacation, obj.value("Vacation FIXME", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPrivacy, obj.value("Privacy FIXME", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kNoRemoteLockUnlock, obj.value("NoRemoteLockUnlock FIXME", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPassage, obj.value("Passage FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DoorLockDayOfWeek>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DoorLockDayOfWeek> r;
    r.SetField(DoorLock::DoorLockDayOfWeek::kSunday, obj.value("Sunday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kMonday, obj.value("Monday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kTuesday, obj.value("Tuesday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kWednesday, obj.value("Wednesday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kThursday, obj.value("Thursday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kFriday, obj.value("Friday FIXME", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kSaturday, obj.value("Saturday FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DoorLockFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DoorLockFeature> r;
    r.SetField(DoorLock::DoorLockFeature::kPINCredentials, obj.value("PINCredentials FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kRFIDCredentials, obj.value("RFIDCredentials FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kFingerCredentials, obj.value("FingerCredentials FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kLogging, obj.value("Logging FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kWeekDaySchedules, obj.value("WeekDaySchedules FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kDoorPositionSensor, obj.value("DoorPositionSensor FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kFaceCredentials, obj.value("FaceCredentials FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kCredentialsOTA, obj.value("CredentialsOTA FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kUsersManagement, obj.value("UsersManagement FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kNotifications, obj.value("Notifications FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kYearDaySchedules, obj.value("YearDaySchedules FIXME", false));
    r.SetField(DoorLock::DoorLockFeature::kHolidaySchedules, obj.value("HolidaySchedules FIXME", false));
    return r;
}

template <>
inline std::optional<DoorLock::DlAlarmCode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlAlarmCode> table = {
        { "LockJammed FIXME", DoorLock::DlAlarmCode::kLockJammed },
        { "LockFactoryReset FIXME", DoorLock::DlAlarmCode::kLockFactoryReset },
        { "LockRadioPowerCycled FIXME", DoorLock::DlAlarmCode::kLockRadioPowerCycled },
        { "WrongCodeEntryLimit FIXME", DoorLock::DlAlarmCode::kWrongCodeEntryLimit },
        { "FrontEsceutcheonRemoved FIXME", DoorLock::DlAlarmCode::kFrontEsceutcheonRemoved },
        { "DoorForcedOpen FIXME", DoorLock::DlAlarmCode::kDoorForcedOpen },
        { "DoorAjar FIXME", DoorLock::DlAlarmCode::kDoorAjar },
        { "ForcedUser FIXME", DoorLock::DlAlarmCode::kForcedUser },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlCredentialRule> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlCredentialRule> table = {
        { "Single FIXME", DoorLock::DlCredentialRule::kSingle },
        { "Double FIXME", DoorLock::DlCredentialRule::kDouble },
        { "Tri FIXME", DoorLock::DlCredentialRule::kTri },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlCredentialType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlCredentialType> table = {
        { "ProgrammingPIN FIXME", DoorLock::DlCredentialType::kProgrammingPIN },
        { "PIN FIXME", DoorLock::DlCredentialType::kPin },
        { "RFID FIXME", DoorLock::DlCredentialType::kRfid },
        { "Fingerprint FIXME", DoorLock::DlCredentialType::kFingerprint },
        { "FingerVein FIXME", DoorLock::DlCredentialType::kFingerVein },
        { "Face FIXME", DoorLock::DlCredentialType::kFace },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlDataOperationType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlDataOperationType> table = {
        { "Add FIXME", DoorLock::DlDataOperationType::kAdd },
        { "Clear FIXME", DoorLock::DlDataOperationType::kClear },
        { "Modify FIXME", DoorLock::DlDataOperationType::kModify },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlDoorState> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlDoorState> table = {
        { "Open", DoorLock::DlDoorState::kDoorOpen },
        { "Closed", DoorLock::DlDoorState::kDoorClosed },
        { "ErrorJammed", DoorLock::DlDoorState::kDoorJammed },
        { "ErrorForcedOpen", DoorLock::DlDoorState::kDoorForcedOpen },
        { "ErrorUnspecified", DoorLock::DlDoorState::kDoorUnspecifiedError },
        { "Undefined", DoorLock::DlDoorState::kDoorAjar },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockDataType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlLockDataType> table = {
        { "Unspecified FIXME", DoorLock::DlLockDataType::kUnspecified },
        { "ProgrammingCode FIXME", DoorLock::DlLockDataType::kProgrammingCode },
        { "UserIndex FIXME", DoorLock::DlLockDataType::kUserIndex },
        { "WeekDaySchedule FIXME", DoorLock::DlLockDataType::kWeekDaySchedule },
        { "YearDaySchedule FIXME", DoorLock::DlLockDataType::kYearDaySchedule },
        { "HolidaySchedule FIXME", DoorLock::DlLockDataType::kHolidaySchedule },
        { "PIN FIXME", DoorLock::DlLockDataType::kPin },
        { "RFID FIXME", DoorLock::DlLockDataType::kRfid },
        { "Fingerprint FIXME", DoorLock::DlLockDataType::kFingerprint },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockOperationType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlLockOperationType> table = {
        { "Lock FIXME", DoorLock::DlLockOperationType::kLock },
        { "Unlock FIXME", DoorLock::DlLockOperationType::kUnlock },
        { "NonAccessUserEvent FIXME", DoorLock::DlLockOperationType::kNonAccessUserEvent },
        { "ForcedUserEvent FIXME", DoorLock::DlLockOperationType::kForcedUserEvent },
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
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperatingMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlOperatingMode> table = {
        { "Normal FIXME", DoorLock::DlOperatingMode::kNormal },
        { "Vacation FIXME", DoorLock::DlOperatingMode::kVacation },
        { "Privacy FIXME", DoorLock::DlOperatingMode::kPrivacy },
        { "NoRemoteLockUnlock FIXME", DoorLock::DlOperatingMode::kNoRemoteLockUnlock },
        { "Passage FIXME", DoorLock::DlOperatingMode::kPassage },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperationError> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlOperationError> table = {
        { "Unspecified FIXME", DoorLock::DlOperationError::kUnspecified },
        { "InvalidCredential FIXME", DoorLock::DlOperationError::kInvalidCredential },
        { "DisabledUserDenied FIXME", DoorLock::DlOperationError::kDisabledUserDenied },
        { "Restricted FIXME", DoorLock::DlOperationError::kRestricted },
        { "InsufficientBattery FIXME", DoorLock::DlOperationError::kInsufficientBattery },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlOperationSource> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlOperationSource> table = {
        { "Unspecified FIXME", DoorLock::DlOperationSource::kUnspecified },
        { "Manual FIXME", DoorLock::DlOperationSource::kManual },
        { "ProprietaryRemote FIXME", DoorLock::DlOperationSource::kProprietaryRemote },
        { "Keypad FIXME", DoorLock::DlOperationSource::kKeypad },
        { "Auto FIXME", DoorLock::DlOperationSource::kAuto },
        { "Button FIXME", DoorLock::DlOperationSource::kButton },
        { "Schedule FIXME", DoorLock::DlOperationSource::kSchedule },
        { "Remote FIXME", DoorLock::DlOperationSource::kRemote },
        { "RFID FIXME", DoorLock::DlOperationSource::kRfid },
        { "Biometric FIXME", DoorLock::DlOperationSource::kBiometric },
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
        { "Success FIXME", DoorLock::DlStatus::kSuccess },
        { "Failure FIXME", DoorLock::DlStatus::kFailure },
        { "Duplicate FIXME", DoorLock::DlStatus::kDuplicate },
        { "Occupied FIXME", DoorLock::DlStatus::kOccupied },
        { "InvalidField FIXME", DoorLock::DlStatus::kInvalidField },
        { "ResourceExhausted FIXME", DoorLock::DlStatus::kResourceExhausted },
        { "NotFound FIXME", DoorLock::DlStatus::kNotFound },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlUserStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlUserStatus> table = {
        { "Available FIXME", DoorLock::DlUserStatus::kAvailable },
        { "OccupiedEnabled FIXME", DoorLock::DlUserStatus::kOccupiedEnabled },
        { "OccupiedDisabled FIXME", DoorLock::DlUserStatus::kOccupiedDisabled },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlUserType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlUserType> table = {
        { "UnrestrictedUser FIXME", DoorLock::DlUserType::kUnrestrictedUser },
        { "YearDayScheduleUser FIXME", DoorLock::DlUserType::kYearDayScheduleUser },
        { "WeekDayScheduleUser FIXME", DoorLock::DlUserType::kWeekDayScheduleUser },
        { "ProgrammingUser FIXME", DoorLock::DlUserType::kProgrammingUser },
        { "NonAccessUser FIXME", DoorLock::DlUserType::kNonAccessUser },
        { "ForcedUser FIXME", DoorLock::DlUserType::kForcedUser },
        { "DisposableUser FIXME", DoorLock::DlUserType::kDisposableUser },
        { "ExpiringUser FIXME", DoorLock::DlUserType::kExpiringUser },
        { "ScheduleRestrictedUser FIXME", DoorLock::DlUserType::kScheduleRestrictedUser },
        { "RemoteOnlyUser FIXME", DoorLock::DlUserType::kRemoteOnlyUser },
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
    r.SetField(Thermostat::DayOfWeek::kAwayOrVacation, obj.value("AwayOrVacation", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ModeForSequence>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ModeForSequence> r;
    r.SetField(Thermostat::ModeForSequence::kHeatSetpointFieldPresent, obj.value("Heat", false));
    r.SetField(Thermostat::ModeForSequence::kCoolSetpointFieldPresent, obj.value("Cool", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ThermostatFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ThermostatFeature> r;
    r.SetField(Thermostat::ThermostatFeature::kHeating, obj.value("Heating FIXME", false));
    r.SetField(Thermostat::ThermostatFeature::kCooling, obj.value("Cooling FIXME", false));
    r.SetField(Thermostat::ThermostatFeature::kOccupancy, obj.value("Occupancy FIXME", false));
    r.SetField(Thermostat::ThermostatFeature::kSchedule, obj.value("Schedule FIXME", false));
    r.SetField(Thermostat::ThermostatFeature::kSetback, obj.value("Setback FIXME", false));
    r.SetField(Thermostat::ThermostatFeature::kAutomode, obj.value("Automode FIXME", false));
    return r;
}

template <>
inline std::optional<Thermostat::SetpointAdjustMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::SetpointAdjustMode> table = {
        { "Heat", Thermostat::SetpointAdjustMode::kHeatSetpoint },
        { "Cool", Thermostat::SetpointAdjustMode::kCoolSetpoint },
        { "Both", Thermostat::SetpointAdjustMode::kHeatAndCoolSetpoints },
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
        { "EmergencyHeating", Thermostat::ThermostatSystemMode::kEmergencyHeating },
        { "Precooling", Thermostat::ThermostatSystemMode::kPrecooling },
        { "FanOnly", Thermostat::ThermostatSystemMode::kFanOnly },
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
inline std::optional<chip::BitMask<FanControl::FanControlFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::FanControlFeature> r;
    r.SetField(FanControl::FanControlFeature::kMultiSpeed, obj.value("MultiSpeed FIXME", false));
    r.SetField(FanControl::FanControlFeature::kAuto, obj.value("Auto FIXME", false));
    r.SetField(FanControl::FanControlFeature::kRocking, obj.value("Rocking FIXME", false));
    r.SetField(FanControl::FanControlFeature::kWind, obj.value("Wind FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::RockSupportMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::RockSupportMask> r;
    r.SetField(FanControl::RockSupportMask::kRockLeftRight, obj.value("RockLeftRight FIXME", false));
    r.SetField(FanControl::RockSupportMask::kRockUpDown, obj.value("RockUpDown FIXME", false));
    r.SetField(FanControl::RockSupportMask::kRockRound, obj.value("RockRound FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::WindSettingMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::WindSettingMask> r;
    r.SetField(FanControl::WindSettingMask::kSleepWind, obj.value("SleepWind FIXME", false));
    r.SetField(FanControl::WindSettingMask::kNaturalWind, obj.value("NaturalWind FIXME", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::WindSupportMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::WindSupportMask> r;
    r.SetField(FanControl::WindSupportMask::kSleepWind, obj.value("SleepWind FIXME", false));
    r.SetField(FanControl::WindSupportMask::kNaturalWind, obj.value("NaturalWind FIXME", false));
    return r;
}

template <>
inline std::optional<FanControl::FanModeSequenceType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeSequenceType> table = {
        { "Off/Low/Med/High FIXME", FanControl::FanModeSequenceType::kOffLowMedHigh },
        { "Off/Low/High FIXME", FanControl::FanModeSequenceType::kOffLowHigh },
        { "Off/Low/Med/High/Auto FIXME", FanControl::FanModeSequenceType::kOffLowMedHighAuto },
        { "Off/Low/High/Auto FIXME", FanControl::FanModeSequenceType::kOffLowHighAuto },
        { "Off/On/Auto FIXME", FanControl::FanModeSequenceType::kOffOnAuto },
        { "Off/On FIXME", FanControl::FanModeSequenceType::kOffOn },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeType> table = {
        { "Off FIXME", FanControl::FanModeType::kOff },
        { "Low FIXME", FanControl::FanModeType::kLow },
        { "Medium FIXME", FanControl::FanModeType::kMedium },
        { "High FIXME", FanControl::FanModeType::kHigh },
        { "On FIXME", FanControl::FanModeType::kOn },
        { "Auto FIXME", FanControl::FanModeType::kAuto },
        { "Smart FIXME", FanControl::FanModeType::kSmart },
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
inline std::optional<chip::BitMask<ColorControl::ColorControlFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::ColorControlFeature> r;
    r.SetField(ColorControl::ColorControlFeature::kHueAndSaturation, obj.value("HueAndSaturation FIXME", false));
    r.SetField(ColorControl::ColorControlFeature::kEnhancedHue, obj.value("EnhancedHue FIXME", false));
    r.SetField(ColorControl::ColorControlFeature::kColorLoop, obj.value("ColorLoop FIXME", false));
    r.SetField(ColorControl::ColorControlFeature::kXy, obj.value("XY FIXME", false));
    r.SetField(ColorControl::ColorControlFeature::kColorTemperature, obj.value("ColorTemperature FIXME", false));
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
        { "stop FIXME", ColorControl::SaturationMoveMode::kStop },
        { "Up FIXME", ColorControl::SaturationMoveMode::kUp },
        { "Down FIXME", ColorControl::SaturationMoveMode::kDown },
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
        { "Up FIXME", ColorControl::SaturationStepMode::kUp },
        { "Down FIXME", ColorControl::SaturationStepMode::kDown },
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
inline std::optional<IlluminanceMeasurement::LightSensorType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, IlluminanceMeasurement::LightSensorType> table = {
        { "Photodiode", IlluminanceMeasurement::LightSensorType::kPhotodiode },
        { "Cmos", IlluminanceMeasurement::LightSensorType::kCmos },
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
inline std::optional<chip::BitMask<PressureMeasurement::PressureFeature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<PressureMeasurement::PressureFeature> r;
    r.SetField(PressureMeasurement::PressureFeature::kExt, obj.value("EXT FIXME", false));
    return r;
}

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
