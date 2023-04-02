const bitmaps_unify_to_matter = {
    "BallastConfigurationBallastStatus" : "BallastStatus",
    "BallastConfigurationLampAlarmMode" : "",  // FIXME
    "BarrierControlCapabilities" : "BarrierControlCapabilities",
    "BarrierControlSafetyStatus" : "BarrierControlSafetyStatus",
    "BasicAlarmMask" : "AlarmMask",
    "BasicDisableLocalConfig" : "",  // Not used
    "BatteryAlarmMask" : "BatteryAlarmMask",
    "BatteryAlarmState" : "",  // FIXME
    "CCColorOptions" : "ColorControlOptions",
    "CCStepMode": "SaturationStepMode",
    "CCMoveMode": "SaturationMoveMode",
    "ColorControlColorCapabilities" : "ColorCapabilities",
    "ColorLoopSetUpdateFlags" : "ColorLoopUpdateFlags",
    "CopySceneMode" : "ScenesCopyMode",
    "DeviceTemperatureConfigurationDeviceTempAlarmMask" : "DeviceTempAlarmMask",
    "DoorLockAlarmMask" : "",  // FIXME
    "DoorLockDefaultConfigurationRegister" : "DlDefaultConfigurationRegister",
    "DoorLockKeypadOperationEventMask" : "DlKeypadOperationEventMask",
    "DoorLockKeypadProgrammingEventMask" : "DlKeypadProgrammingEventMask",
    "DoorLockManualOperationEventMask" : "DlManualOperationEventMask",
    "DoorLockRFIDOperationEventMask" : "DlRFIDOperationEventMask",
    "DoorLockRFIDProgrammingEventMask" : "DlRFIDProgrammingEventMask",
    "DoorLockRFOperationEventMask" : "DlRemoteOperationEventMask",
    "DoorLockRFProgrammingEventMask" : "DlRemoteProgrammingEventMask",
    "DoorLockSupportedOperatingModes" : "DlSupportedOperatingModes",
    "DrlkDaysMask" : "DaysMaskMap",
    "ElectricalMeasurementACAlarmsMask" : "",  // FIXME
    "ElectricalMeasurementDCOverloadAlarmsMask" : "",  // FIXME
    "ElectricalMeasurementMeasurementType" : "",  // FIXME
    "GroupsNameSupport" : "",  // FIXME
    "IasZoneStatus" : "IasZoneStatus",
    "ImageBlockRequestFieldControl" : "",  // Not used
    "ImagePageRequestFieldControl" : "",  // Not used
    "LevelOptions" : "LevelControlOptions",
    "OccupancySensingOccupancy" : "OccupancyBitmap",
    "OccupancySensingOccupancySensorTypeBitmap" : "OccupancySensorTypeBitmap",
    "OnWithTimedOffOnOffControl" : "OnOffControl",
    "PowerConfigurationMainsAlarmMask" : "MainsAlarmMask",
    "PumpConfigurationAndControlAlarmMask" : "",  // FIXME
    "PumpConfigurationAndControlPumpStatus" : "PumpStatus",
    "QueryNextImageRequestFieldControl" : "",  // Not used
    "ResetStartupParametersOptions" : "",  // Not used
    "RestartDeviceOptions" : "",  // Not used
    "ScanResponseKeyBitmask" : "",  // Not used
    "ScenesNameSupport" : "",  // FIXME
    "ShadeConfigurationStatus" : "",  // FIXME
    "SquawkSquawkConfiguration" : "",  // Not used
    "StartWarningSirenConfiguration" : "",  // Not used
    "TLTouchlinkInformation" : "",  // Not used
    "TLVersion" : "",  // Not used
    "TLZigbeeInformation" : "",  // Not used
    "ThermostatACErrorCode" : "",  // FIXME
    "ThermostatAlarmMask" : "ThermostatAlarmMask",
    "ThermostatHVACSystemTypeConfiguration" : "",  // FIXME
    "ThermostatOccupancy" : "ThermostatOccupancy",
    "ThermostatRemoteSensing" : "ThermostatSensing",
    "ThermostatThermostatProgrammingOperationMode" : "",  // FIXME
    "ThermostatThermostatRunningState" : "ThermostatRunningState",
    "TimeTimeStatus" : "TimeStatusMask",
    "TstatScheduleDOW" : "DayOfWeek",
    "TstatScheduleMode" : "ModeForSequence",
    "WindowCoveringConfigOrStatus" : "ConfigStatus",
    "WindowCoveringMode" : "Mode",
    "map16" : "",  // FIXME
    "map24" : "",  // FIXME
    "map32" : "",  // FIXME
    "map40" : "",  // FIXME
    "map48" : "",  // FIXME
    "map56" : "",  // FIXME
    "map64" : "",  // FIXME
    "map8" : "",  // FIXME
}
var bitmaps_matter_to_unify = {
} // Leave this empty, will be filled by unify_bitmap_name function


const enums_unify_to_matter = {
    "ArmArmMode" : "",  // Not used
    "ArmResponseArmNotification" : "",  // Not used
    "AttributeReportingStatus" : "",  // Not used
    "BarrierControlMovingState" : "BarrierControlMovingState",
    "BasicGenericDevice-Class" : "",  // Not used
    "BasicGenericDevice-Type" : "",  // Not used
    "BasicPhysicalEnvironment" : "",  // Not used
    "BasicPowerSource" : "PowerSource",
    "BatterySize" : "BatterySize",
    "BypassResponseZoneIDBypassResult" : "",  // Not used
    "CCColorLoopDirection" : "ColorLoopDirection",
    "CCDirection" : "HueDirection",
    "CCMoveMode" : "HueMoveMode",
    "CCStepMode" : "HueStepMode",
    "ColorControlColorLoopActive" : "",  // FIXME
    "ColorControlColorMode" : "ColorMode",
    "ColorControlDriftCompensation" : "",  // FIXME
    "ColorControlEnhancedColorMode" : "EnhancedColorMode",
    "ColorLoopSetAction" : "ColorLoopAction",
    "CommissioningNetworkKeyType" : "",  // Not used
    "CommissioningProtocolVersion" : "",  // Not used
    "CommissioningStackProfile" : "",  // Not used
    "CommissioningStartupControl" : "",  // Not used
    "DehumidificationControlDehumidificationLockout" : "DehumidifcationLockout",
    "DehumidificationControlRelativeHumidityDisplay" : "RelativeHumidityDisplay",
    "DehumidificationControlRelativeHumidityMode" : "RelativeHumidityMode",
    "DoorLockDoorState" : "DoorStateEnum",
    "DoorLockLEDSettings" : "",  // FIXME
    "DoorLockLockState" : "DlLockState",
    "DoorLockLockType" : "DlLockType",
    "DoorLockSecurityLevel" : "DoorLockSecurityLevel",
    "DoorLockSoundVolume" : "DoorLockSoundVolume",
    "DrlkOperEventSource" : "DoorLockEventSource",
    "DrlkOperMode" : "DoorLockOperatingMode",
    "DrlkPassFailStatus" : "",  // FIXME
    "DrlkSetCodeStatus" : "DoorLockSetPinOrIdStatus",
    "DrlkSettableUserStatus" : "",  // FIXME
    "DrlkUserStatus" : "DoorLockUserStatus",
    "DrlkUserType" : "DoorLockUserType",
    "FanControlFanMode" : "FanModeType",
    "FanControlFanModeSequence" : "FanModeSequence",
    "GetLogRecordResponseEventType" : "DoorLockEventType",
    "GetMeasurementProfileResponseStatus" : "",  // FIXME
    "HVACSystemTypeConfigurationCoolingSystemStage" : "",  // FIXME
    "HVACSystemTypeConfigurationHeatingFuelSource" : "",  // FIXME
    "HVACSystemTypeConfigurationHeatingSystemStage" : "",  // FIXME
    "HVACSystemTypeConfigurationHeatingSystemType" : "",  // FIXME
    "IASZoneZoneState" : "",  // Not used
    "IasZoneType" : "",  // Not used
    "IasacPanelStatus" : "",  // Not used
    "IasaceAlarmStatus" : "",  // Not used
    "IasaceAudibleNotification" : "",  // Not used
    "IaswdLevel" : "",  // Not used
    "IlluminanceLevelSensingLevelStatus" : "LevelStatus",
    "IlluminanceLevelSensingLightSensorType" : "LightSensorType",
    "IlluminanceMeasurementLightSensorType" : "LightSensorType",
    "ImageNotifyPayloadType" : "",  // Not used
    "MoveStepMode" : ["MoveMode", "StepMode"],
    "OTADeviceSpecificImageType" : "",  // Not used
    "OTAUpgradeImageUpgradeStatus" : "",  // Not used
    "OTAUpgradeUpgradeActivationPolicy" : "",  // Not used
    "OTAUpgradeUpgradeTimeoutPolicy" : "",  // Not used
    "OccupancySensingOccupancySensorType" : "OccupancySensorTypeEnum",
    "OffWithEffectEffectIdentifier" : "OnOffEffectIdentifier",
    "OnOffStartUpOnOff" : "OnOffStartUpOnOff",
    "OperatingEventNotificationOperationEventCode" : "DoorLockOperationEventCode",
    "OptionsStartupMode" : "",  // Not used
    "ProfileIntervalPeriod" : "",  // FIXME
    "ProgrammingEventNotificationProgramEventCode" : "DoorLockProgrammingEventCode",
    "ProgrammingEventNotificationProgramEventSource" : "DoorLockEventSource",
    "PumpControlMode" : "PumpControlMode",
    "PumpOperationMode" : "PumpOperationMode",
    "SHDCFGDirection" : "",  // FIXME
    "SetpointRaiseOrLowerMode" : "SetpointAdjustMode",
    "ShadeConfigurationMode" : "OperatingMode",
    "SirenConfigurationStrobe" : "SquawkStobe",
    "SirenConfigurationWarningMode" : "WarningMode",
    "SquawkConfigurationSquawkMode" : "SquawkMode",
    "TLKeyIndex" : "",  // Not used
    "TLStatus" : "",  // Not used
    "TLZigbeeInformationLogicalType" : "",  // Not used
    "ThermostatACCapacityFormat" : "",  // FIXME
    "ThermostatACCompressorType" : "",  // FIXME
    "ThermostatACLouverPosition" : "",  // FIXME
    "ThermostatACRefrigerantType" : "",  // FIXME
    "ThermostatACType" : "",  // FIXME
    "ThermostatControlSequenceOfOperation" : "ThermostatControlSequence",
    "ThermostatProgrammingOperationModeProgrammingMode" : "",  // FIXME
    "ThermostatSetpointChangeSource" : "",  // FIXME
    "ThermostatStartOfWeek" : "StartOfWeek",
    "ThermostatSystemMode" : "ThermostatSystemMode",
    "ThermostatTemperatureSetpointHold" : "TemperatureSetpointHold",
    "ThermostatThermostatRunningMode" : "ThermostatRunningMode",
    "ThermostatUserInterfaceConfigurationKeypadLockout" : "KeypadLockout",
    "ThermostatUserInterfaceConfigurationScheduleProgrammingVisibility" : "",  // FIXME
    "ThermostatUserInterfaceConfigurationTemperatureDisplayMode" : "TemperatureDisplayMode",
    "TriggerEffectEffectIdentifier" : "IdentifyEffectIdentifier",
    "TriggerEffectEffectVariant" : "IdentifyEffectVariant",
    "WindowCoveringWindowCoveringType" : "Type",
    "ZoneEnrollResponseEnrollResponseCode" : "",  // Not used
    "attributeReportingStatus" : "",  // FIXME
    "reportingRole" : "",  // FIXME
    "zclStatus" : "Status",
    "zclType" : "",  // FIXME
}

var enums_matter_to_unify = {} // Leave this empty, will be filled by unify_enum_name function

// Get Unify bitmap name from Matter bitmap name
function unify_bitmap_name(matter_bitmap_name) {
    // Check bitmaps_matter_to_unify is initialized or initialize it
    if (Object.keys(bitmaps_matter_to_unify).length == 0) {
        for (const key in bitmaps_unify_to_matter) {
            const value = bitmaps_unify_to_matter[key]
            if (value != "") {
                if (Array.isArray(value)) {
                    value.forEach(elem => {bitmaps_matter_to_unify[elem] = key})
                }
                bitmaps_matter_to_unify[value] = key
            }
        }
    }
    if (bitmaps_matter_to_unify.hasOwnProperty(matter_bitmap_name)) {
        return bitmaps_matter_to_unify[matter_bitmap_name]
    } else {
        return matter_bitmap_name
    }
}

// Get Matter bitmap name from Unify bitmap name
function matter_bitmap_name(unify_bitmap_name) {
    if (bitmaps_unify_to_matter.hasOwnProperty(unify_bitmap_name) && bitmaps_unify_to_matter[unify_bitmap_name] != "") {
        if (Array.isArray(bitmaps_unify_to_matter[unify_bitmap_name])) {
            // If it is an array, return first type, as they should map to the same
            return bitmaps_unify_to_matter[unify_bitmap_name][0]
        }
        return bitmaps_unify_to_matter[unify_bitmap_name]
    } else {
        return unify_bitmap_name
    }
}

// Get Unify enum name from Matter enum name
function unify_enum_name(matter_enum_name) {
    if(matter_enum_name == "SaturationMoveMode") return "CCMoveMode";
    if(matter_enum_name == "SaturationStepMode") return "CCStepMode";

    // Check bitmaps_matter_to_unify is initialized or initialize it
    if (Object.keys(enums_matter_to_unify).length == 0) {
        for (const key in enums_unify_to_matter) {
            const value = enums_unify_to_matter[key]
            if (value != "") {
                if (Array.isArray(value)) {
                    value.forEach(elem => {enums_matter_to_unify[elem] = key})
                }
                enums_matter_to_unify[value] = key
            }
        }
    }
    if (enums_matter_to_unify.hasOwnProperty(matter_enum_name)) {
        return enums_matter_to_unify[matter_enum_name]
    } else {
        return matter_enum_name
    }
}

// Get Matter enum name from Unify enum name
function matter_enum_name(unify_enum_name) {
    if (enums_unify_to_matter.hasOwnProperty(unify_enum_name) && enums_unify_to_matter[unify_bitmap_name] != "") {
        if (Array.isArray(enums_unify_to_matter[unify_enum_name])) {
            // If it is an array, return first type, as they should map to the same
            return enums_unify_to_matter[unify_enum_name][0]
        }
        return enums_unify_to_matter[unify_enum_name]
    } else {
        return unify_enum_name
    }
}


exports.unify_bitmap_name = unify_bitmap_name
exports.matter_bitmap_name = matter_bitmap_name
exports.unify_enum_name = unify_enum_name
exports.matter_enum_name = matter_enum_name