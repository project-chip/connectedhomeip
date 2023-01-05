exports.model = {
  0 : {
    name : "Basic",
    commands : {
      0 : {
        name : "ResetToFactoryDefaults",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "ZCLVersion",
      1 : "ApplicationVersion",
      2 : "StackVersion",
      3 : "HWVersion",
      4 : "ManufacturerName",
      5 : "ModelIdentifier",
      6 : "DateCode",
      7 : "PowerSource",
      8 : "GenericDeviceClass",
      9 : "GenericDeviceType",
      10 : "ProductCode",
      11 : "ProductURL",
      12 : "ManufacturerVersionDetails",
      13 : "SerialNumber",
      14 : "ProductLabel",
      16 : "LocationDescription",
      17 : "PhysicalEnvironment",
      18 : "DeviceEnabled",
      19 : "AlarmMask",
      20 : "DisableLocalConfig",
      16384 : "SWBuildID",
    },
  },
  3 : {
    name : "Identify",
    commands : {
      0 : {
        name : "Identify",
        arguments : [
          "IdentifyTime",
        ],
      },
      1 : {
        name : "IdentifyQuery",
        arguments : [

        ],
      },
      64 : {
        name : "TriggerEffect",
        arguments : [
          "EffectIdentifier",
          "EffectVariant",
        ],
      },
    },
    attributes : {
      0 : "IdentifyTime",
    },
  },
  4 : {
    name : "Groups",
    commands : {
      0 : {
        name : "AddGroup",
        arguments : [
          "GroupId",
          "GroupName",
        ],
      },
      1 : {
        name : "ViewGroup",
        arguments : [
          "GroupId",
        ],
      },
      2 : {
        name : "GetGroupMembership",
        arguments : [
          "GroupList",
        ],
      },
      3 : {
        name : "RemoveGroup",
        arguments : [
          "GroupId",
        ],
      },
      4 : {
        name : "RemoveAllGroups",
        arguments : [

        ],
      },
      5 : {
        name : "AddGroupIfIdentifying",
        arguments : [
          "GroupId",
          "GroupName",
        ],
      },
    },
    attributes : {
      0 : "NameSupport",
    },
  },
  5 : {
    name : "Scenes",
    commands : {
      0 : {
        name : "AddScene",
        arguments : [
          "GroupID",
          "SceneID",
          "TransitionTime",
          "SceneName",
          "ExtensionFieldSets",
        ],
      },
      1 : {
        name : "ViewScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      2 : {
        name : "RemoveScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      3 : {
        name : "RemoveAllScenes",
        arguments : [
          "GroupID",
        ],
      },
      4 : {
        name : "StoreScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      5 : {
        name : "RecallScene",
        arguments : [
          "GroupID",
          "SceneID",
          "TransitionTime",
        ],
      },
      6 : {
        name : "GetSceneMembership",
        arguments : [
          "GroupID",
        ],
      },
      64 : {
        name : "EnhancedAddScene",
        arguments : [
          "GroupID",
          "SceneID",
          "TransitionTime",
          "SceneName",
          "ExtensionFieldSets",
        ],
      },
      65 : {
        name : "EnhancedViewScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      66 : {
        name : "CopyScene",
        arguments : [
          "Mode",
          "GroupIdentifierFrom",
          "SceneIdentifierFrom",
          "GroupIdentifierTo",
          "SceneIdentifierTo",
        ],
      },
    },
    attributes : {
      0 : "SceneCount",
      1 : "CurrentScene",
      2 : "CurrentGroup",
      3 : "SceneValid",
      4 : "NameSupport",
      5 : "LastConfiguredBy",
    },
  },
  6 : {
    name : "OnOff",
    commands : {
      0 : {
        name : "Off",
        arguments : [

        ],
      },
      1 : {
        name : "On",
        arguments : [

        ],
      },
      2 : {
        name : "Toggle",
        arguments : [

        ],
      },
      64 : {
        name : "OffWithEffect",
        arguments : [
          "EffectIdentifier",
          "EffectVariant",
        ],
      },
      65 : {
        name : "OnWithRecallGlobalScene",
        arguments : [

        ],
      },
      66 : {
        name : "OnWithTimedOff",
        arguments : [
          "OnOffControl",
          "OnTime",
          "OffWaitTime",
        ],
      },
    },
    attributes : {
      0 : "OnOff",
      16384 : "GlobalSceneControl",
      16385 : "OnTime",
      16386 : "OffWaitTime",
      16387 : "StartUpOnOff",
    },
  },
  8 : {
    name : "Level",
    commands : {
      0 : {
        name : "MoveToLevel",
        arguments : [
          "Level",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      1 : {
        name : "Move",
        arguments : [
          "MoveMode",
          "Rate",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      2 : {
        name : "Step",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      3 : {
        name : "Stop",
        arguments : [
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      4 : {
        name : "MoveToLevelWithOnOff",
        arguments : [
          "Level",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      5 : {
        name : "MoveWithOnOff",
        arguments : [
          "MoveMode",
          "Rate",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      6 : {
        name : "StepWithOnOff",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      7 : {
        name : "StopWithOnOff",
        arguments : [
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      8 : {
        name : "MoveToClosestFrequency",
        arguments : [
          "Frequency",
        ],
      },
    },
    attributes : {
      0 : "CurrentLevel",
      1 : "RemainingTime",
      2 : "MinLevel",
      3 : "MaxLevel",
      4 : "CurrentFrequency",
      5 : "MinFrequency",
      6 : "MaxFrequency",
      15 : "Options",
      16 : "OnOffTransitionTime",
      17 : "OnLevel",
      18 : "OnTransitionTime",
      19 : "OffTransitionTime",
      20 : "DefaultMoveRate",
      16384 : "StartUpCurrentLevel",
    },
  },
  9 : {
    name : "Alarms",
    commands : {
      0 : {
        name : "ResetAlarm",
        arguments : [
          "AlarmCode",
          "ClusterIdentifier",
        ],
      },
      1 : {
        name : "ResetAllAlarms",
        arguments : [

        ],
      },
      2 : {
        name : "GetAlarm",
        arguments : [

        ],
      },
      3 : {
        name : "ResetAlarmLog",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "AlarmCount",
    },
  },
  10 : {
    name : "Time",
    commands : {},
    attributes : {
      0 : "Time",
      1 : "TimeStatus",
      2 : "TimeZone",
      3 : "DstStart",
      4 : "DstEnd",
      5 : "DstShift",
      6 : "StandardTime",
      7 : "LocalTime",
      8 : "LastSetTime",
      9 : "ValidUntilTime",
    },
  },
  257 : {
    name : "DoorLock",
    commands : {
      0 : {
        name : "LockDoor",
        arguments : [
          "PINOrRFIDCode",
        ],
      },
      1 : {
        name : "UnlockDoor",
        arguments : [
          "PINOrRFIDCode",
        ],
      },
      2 : {
        name : "Toggle",
        arguments : [
          "PINOrRFIDCode",
        ],
      },
      3 : {
        name : "UnlockWithTimeout",
        arguments : [
          "TimeoutInSeconds",
          "PINOrRFIDCode",
        ],
      },
      4 : {
        name : "GetLogRecord",
        arguments : [
          "LogIndex",
        ],
      },
      5 : {
        name : "SetPINCode",
        arguments : [
          "UserID",
          "UserStatus",
          "UserType",
          "Pin",
        ],
      },
      6 : {
        name : "GetPINCode",
        arguments : [
          "UserID",
        ],
      },
      7 : {
        name : "ClearPINCode",
        arguments : [
          "UserID",
        ],
      },
      8 : {
        name : "ClearAllPINCodes",
        arguments : [

        ],
      },
      9 : {
        name : "SetUserStatus",
        arguments : [
          "UserID",
          "UserStatus",
        ],
      },
      10 : {
        name : "GetUserStatus",
        arguments : [
          "UserID",
        ],
      },
      11 : {
        name : "SetWeekdaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
          "DaysMask",
          "StartHour",
          "StartMinute",
          "EndHour",
          "EndMinute",
        ],
      },
      12 : {
        name : "GetWeekdaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
        ],
      },
      13 : {
        name : "ClearWeekdaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
        ],
      },
      14 : {
        name : "SetYearDaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
          "LocalStartTime",
          "LocalEndTime",
        ],
      },
      15 : {
        name : "GetYearDaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
        ],
      },
      16 : {
        name : "ClearYearDaySchedule",
        arguments : [
          "ScheduleID",
          "UserID",
        ],
      },
      17 : {
        name : "SetHolidaySchedule",
        arguments : [
          "HolidayScheduleID",
          "LocalStartTime",
          "LocalEndTime",
          "OperatingModeDuringHoliday",
        ],
      },
      18 : {
        name : "GetHolidaySchedule",
        arguments : [
          "HolidayScheduleID",
        ],
      },
      19 : {
        name : "ClearHolidaySchedule",
        arguments : [
          "HolidayScheduleID",
        ],
      },
      20 : {
        name : "SetUserType",
        arguments : [
          "UserID",
          "UserType",
        ],
      },
      21 : {
        name : "GetUserType",
        arguments : [
          "UserID",
        ],
      },
      22 : {
        name : "SetRFIDCode",
        arguments : [
          "UserID",
          "UserStatus",
          "UserType",
          "RFIDCode",
        ],
      },
      23 : {
        name : "GetRFIDCode",
        arguments : [
          "UserID",
        ],
      },
      24 : {
        name : "ClearRFIDCode",
        arguments : [
          "UserID",
        ],
      },
      25 : {
        name : "ClearAllRFIDCodes",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "LockState",
      1 : "LockType",
      2 : "ActuatorEnabled",
      3 : "DoorState",
      4 : "DoorOpenEvents",
      5 : "DoorClosedEvents",
      6 : "OpenPeriod",
      16 : "NumberOfLogRecordsSupported",
      17 : "NumberOfTotalUsersSupported",
      18 : "NumberOfPINUsersSupported",
      19 : "NumberOfRFIDUsersSupported",
      20 : "NumberOfWeekDaySchedulesSupportedPerUser",
      21 : "NumberOfYearDaySchedulesSupportedPerUser",
      22 : "NumberOfHolidaySchedulesSupported",
      23 : "MaxPINCodeLength",
      24 : "MinPINCodeLength",
      25 : "MaxRFIDCodeLength",
      26 : "MinRFIDCodeLength",
      32 : "EnableLogging",
      33 : "Language",
      34 : "LEDSettings",
      35 : "AutoRelockTime",
      36 : "SoundVolume",
      37 : "OperatingMode",
      38 : "SupportedOperatingModes",
      39 : "DefaultConfigurationRegister",
      40 : "EnableLocalProgramming",
      41 : "EnableOneTouchLocking",
      42 : "EnableInsideStatusLED",
      43 : "EnablePrivacyModeButton",
      48 : "WrongCodeEntryLimit",
      49 : "UserCodeTemporaryDisableTime",
      50 : "SendPINOverTheAir",
      51 : "RequirePINforRFOperation",
      52 : "SecurityLevel",
      64 : "AlarmMask",
      65 : "KeypadOperationEventMask",
      66 : "RFOperationEventMask",
      67 : "ManualOperationEventMask",
      68 : "RFIDOperationEventMask",
      69 : "KeypadProgrammingEventMask",
      70 : "RFProgrammingEventMask",
      71 : "RFIDProgrammingEventMask",
    },
  },
  259 : {
    name : "BarrierControl",
    commands : {
      0 : {
        name : "GoToPercent",
        arguments : [
          "PercentOpen",
        ],
      },
      1 : {
        name : "Stop",
        arguments : [

        ],
      },
    },
    attributes : {
      1 : "MovingState",
      2 : "SafetyStatus",
      3 : "Capabilities",
      4 : "OpenEvents",
      5 : "CloseEvents",
      6 : "CommandOpenEvents",
      7 : "CommandCloseEvents",
      8 : "OpenPeriod",
      9 : "ClosePeriod",
      10 : "BarrierPosition",
    },
  },
  513 : {
    name : "Thermostat",
    commands : {
      0 : {
        name : "SetpointRaiseOrLower",
        arguments : [
          "Mode",
          "Amount",
        ],
      },
      1 : {
        name : "SetWeeklySchedule",
        arguments : [
          "NumberOfTransitions",
          "DayOfWeek",
          "Mode",
          "Transitions",
        ],
      },
      2 : {
        name : "GetWeeklySchedule",
        arguments : [
          "DaysToReturn",
          "ModeToReturn",
        ],
      },
      3 : {
        name : "ClearWeeklySchedule",
        arguments : [

        ],
      },
      4 : {
        name : "GetRelayStatusLog",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "LocalTemperature",
      1 : "OutdoorTemperature",
      2 : "Occupancy",
      3 : "AbsMinHeatSetpointLimit",
      4 : "AbsMaxHeatSetpointLimit",
      5 : "AbsMinCoolSetpointLimit",
      6 : "AbsMaxCoolSetpointLimit",
      7 : "PICoolingDemand",
      8 : "PIHeatingDemand",
      9 : "HVACSystemTypeConfiguration",
      16 : "LocalTemperatureCalibration",
      17 : "OccupiedCoolingSetpoint",
      18 : "OccupiedHeatingSetpoint",
      19 : "UnoccupiedCoolingSetpoint",
      20 : "UnoccupiedHeatingSetpoint",
      21 : "MinHeatSetpointLimit",
      22 : "MaxHeatSetpointLimit",
      23 : "MinCoolSetpointLimit",
      24 : "MaxCoolSetpointLimit",
      25 : "MinSetpointDeadBand",
      26 : "RemoteSensing",
      27 : "ControlSequenceOfOperation",
      28 : "SystemMode",
      29 : "AlarmMask",
      30 : "ThermostatRunningMode",
      32 : "StartOfWeek",
      33 : "NumberOfWeeklyTransitions",
      34 : "NumberOfDailyTransitions",
      35 : "TemperatureSetpointHold",
      36 : "TemperatureSetpointHoldDuration",
      37 : "ThermostatProgrammingOperationMode",
      41 : "ThermostatRunningState",
      48 : "SetpointChangeSource",
      49 : "SetpointChangeAmount",
      50 : "SetpointChangeSourceTimestamp",
      52 : "OccupiedSetback",
      53 : "OccupiedSetbackMin",
      54 : "OccupiedSetbackMax",
      55 : "UnoccupiedSetback",
      56 : "UnoccupiedSetbackMin",
      57 : "UnoccupiedSetbackMax",
      58 : "EmergencyHeatDelta",
      64 : "ACType",
      65 : "ACCapacity",
      66 : "ACRefrigerantType",
      67 : "ACCompressorType",
      68 : "ACErrorCode",
      69 : "ACLouverPosition",
      70 : "ACCoilTemperature",
      71 : "ACCapacityFormat",
    },
  },
  514 : {
    name : "FanControl",
    commands : {},
    attributes : {
      0 : "FanMode",
      1 : "FanModeSequence",
    },
  },
  516 : {
    name : "ThermostatUserInterfaceConfiguration",
    commands : {},
    attributes : {
      0 : "TemperatureDisplayMode",
      1 : "KeypadLockout",
      2 : "ScheduleProgrammingVisibility",
    },
  },
  768 : {
    name : "ColorControl",
    commands : {
      0 : {
        name : "MoveToHue",
        arguments : [
          "Hue",
          "Direction",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      1 : {
        name : "MoveHue",
        arguments : [
          "MoveMode",
          "Rate",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      2 : {
        name : "StepHue",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      3 : {
        name : "MoveToSaturation",
        arguments : [
          "Saturation",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      4 : {
        name : "MoveSaturation",
        arguments : [
          "MoveMode",
          "Rate",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      5 : {
        name : "StepSaturation",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      6 : {
        name : "MoveToHueAndSaturation",
        arguments : [
          "Hue",
          "Saturation",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      7 : {
        name : "MoveToColor",
        arguments : [
          "ColorX",
          "ColorY",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      8 : {
        name : "MoveColor",
        arguments : [
          "RateX",
          "RateY",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      9 : {
        name : "StepColor",
        arguments : [
          "StepX",
          "StepY",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      10 : {
        name : "MoveToColorTemperature",
        arguments : [
          "ColorTemperatureMireds",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      64 : {
        name : "EnhancedMoveToHue",
        arguments : [
          "EnhancedHue",
          "Direction",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      65 : {
        name : "EnhancedMoveHue",
        arguments : [
          "MoveMode",
          "Rate",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      66 : {
        name : "EnhancedStepHue",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      67 : {
        name : "EnhancedMoveToHueAndSaturation",
        arguments : [
          "EnhancedHue",
          "Saturation",
          "TransitionTime",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      68 : {
        name : "ColorLoopSet",
        arguments : [
          "UpdateFlags",
          "Action",
          "Direction",
          "Time",
          "StartHue",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      71 : {
        name : "StopMoveStep",
        arguments : [
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      75 : {
        name : "MoveColorTemperature",
        arguments : [
          "MoveMode",
          "Rate",
          "ColorTemperatureMinimumMireds",
          "ColorTemperatureMaximumMireds",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
      76 : {
        name : "StepColorTemperature",
        arguments : [
          "StepMode",
          "StepSize",
          "TransitionTime",
          "ColorTemperatureMinimumMireds",
          "ColorTemperatureMaximumMireds",
          "OptionsMask",
          "OptionsOverride",
        ],
      },
    },
    attributes : {
      0 : "CurrentHue",
      1 : "CurrentSaturation",
      2 : "RemainingTime",
      3 : "CurrentX",
      4 : "CurrentY",
      5 : "DriftCompensation",
      6 : "CompensationText",
      7 : "ColorTemperatureMireds",
      8 : "ColorMode",
      15 : "Options",
      16 : "NumberOfPrimaries",
      17 : "Primary1X",
      18 : "Primary1Y",
      19 : "Primary1Intensity",
      21 : "Primary2X",
      22 : "Primary2Y",
      23 : "Primary2Intensity",
      25 : "Primary3X",
      26 : "Primary3Y",
      27 : "Primary3Intensity",
      32 : "Primary4X",
      33 : "Primary4Y",
      34 : "Primary4Intensity",
      36 : "Primary5X",
      37 : "Primary5Y",
      38 : "Primary5Intensity",
      40 : "Primary6X",
      41 : "Primary6Y",
      42 : "Primary6Intensity",
      48 : "WhitePointX",
      49 : "WhitePointY",
      50 : "ColorPointRX",
      51 : "ColorPointRY",
      52 : "ColorPointRIntensity",
      54 : "ColorPointGX",
      55 : "ColorPointGY",
      56 : "ColorPointGIntensity",
      58 : "ColorPointBX",
      59 : "ColorPointBY",
      60 : "ColorPointBIntensity",
      16384 : "EnhancedCurrentHue",
      16385 : "EnhancedColorMode",
      16386 : "ColorLoopActive",
      16387 : "ColorLoopDirection",
      16388 : "ColorLoopTime",
      16389 : "ColorLoopStartEnhancedHue",
      16390 : "ColorLoopStoredEnhancedHue",
      16394 : "ColorCapabilities",
      16395 : "ColorTempPhysicalMinMireds",
      16396 : "ColorTempPhysicalMaxMireds",
      16397 : "CoupleColorTempToLevelMinMireds",
      16400 : "StartUpColorTemperatureMireds",
    },
  },
  1024 : {
    name : "IlluminanceMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
      4 : "LightSensorType",
    },
  },
  1026 : {
    name : "TemperatureMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
    },
  },
  1027 : {
    name : "PressureMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
      16 : "ScaledValue",
      17 : "MinScaledValue",
      18 : "MaxScaledValue",
      19 : "ScaledTolerance",
      20 : "Scale",
    },
  },
  1029 : {
    name : "RelativityHumidity",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
    },
  },
  1030 : {
    name : "OccupancySensing",
    commands : {},
    attributes : {
      0 : "Occupancy",
      1 : "OccupancySensorType",
      2 : "OccupancySensorTypeBitmap",
      16 : "PIROccupiedToUnoccupiedDelay",
      17 : "PIRUnoccupiedToOccupiedDelay",
      18 : "PIRUnoccupiedToOccupiedThreshold",
      32 : "UltrasonicOccupiedToUnoccupiedDelay",
      33 : "UltrasonicUnoccupiedToOccupiedDelay",
      34 : "UltrasonicUnoccupiedToOccupiedThreshold",
      48 : "PhysicalContactOccupiedToUnoccupiedDelay",
      49 : "PhysicalContactUnoccupiedToOccupiedDelay",
      50 : "PhysicalContactUnoccupiedToOccupiedThreshold",
    },
  },
  2820 : {
    name : "ElectricalMeasurement",
    commands : {
      0 : {
        name : "GetProfileInfoResponse",
        arguments : [
          "ProfileCount",
          "ProfileIntervalPeriod",
          "MaxNumberOfIntervals",
          "ListOfAttributes",
        ],
      },
      1 : {
        name : "GetMeasurementProfileResponse",
        arguments : [
          "StartTime",
          "Status",
          "ProfileIntervalPeriod",
          "NumberOfIntervalsDelivered",
          "AttributeId",
          "Intervals",
        ],
      },
    },
    attributes : {
      0 : "MeasurementType",
      256 : "DCVoltage",
      257 : "DCVoltageMin",
      258 : "DCVoltageMax",
      259 : "DCCurrent",
      260 : "DCCurrentMin",
      261 : "DCCurrentMax",
      262 : "DCPower",
      263 : "DCPowerMin",
      264 : "DCPowerMax",
      512 : "DCVoltageMultiplier",
      513 : "DCVoltageDivisor",
      514 : "DCCurrentMultiplier",
      515 : "DCCurrentDivisor",
      516 : "DCPowerMultiplier",
      517 : "DCPowerDivisor",
      768 : "ACFrequency",
      769 : "ACFrequencyMin",
      770 : "ACFrequencyMax",
      771 : "NeutralCurrent",
      772 : "TotalActivePower",
      773 : "TotalReactivePower",
      774 : "TotalApparentPower",
      775 : "Measured1stHarmonicCurrent",
      776 : "Measured3rdHarmonicCurrent",
      777 : "Measured5thHarmonicCurrent",
      778 : "Measured7thHarmonicCurrent",
      779 : "Measured9thHarmonicCurrent",
      780 : "Measured11thHarmonicCurrent",
      781 : "MeasuredPhase1stHarmonicCurrent",
      782 : "MeasuredPhase3rdHarmonicCurrent",
      783 : "MeasuredPhase5thHarmonicCurrent",
      784 : "MeasuredPhase7thHarmonicCurrent",
      785 : "MeasuredPhase9thHarmonicCurrent",
      786 : "MeasuredPhase11thHarmonicCurrent",
      1024 : "ACFrequencyMultiplier",
      1025 : "ACFrequencyDivisor",
      1026 : "PowerMultiplier",
      1027 : "PowerDivisor",
      1028 : "HarmonicCurrentMultiplier",
      1029 : "PhaseHarmonicCurrentMultiplier",
      1281 : "LineCurrent",
      1282 : "ActiveCurrent",
      1283 : "ReactiveCurrent",
      1285 : "RMSVoltage",
      1286 : "RMSVoltageMin",
      1287 : "RMSVoltageMax",
      1288 : "RMSCurrent",
      1289 : "RMSCurrentMin",
      1290 : "RMSCurrentMax",
      1291 : "ActivePower",
      1292 : "ActivePowerMin",
      1293 : "ActivePowerMax",
      1294 : "ReactivePower",
      1295 : "ApparentPower",
      1296 : "PowerFactor",
      1297 : "AverageRMSVoltageMeasurementPeriod",
      1298 : "AverageRMSOverVoltageCounter",
      1299 : "AverageRMSUnderVoltageCounter",
      1300 : "RMSExtremeOverVoltagePeriod",
      1301 : "RMSExtremeUnderVoltagePeriod",
      1302 : "RMSVoltageSagPeriod",
      1303 : "RMSVoltageSwellPeriod",
      1536 : "ACVoltageMultiplier",
      1537 : "ACVoltageDivisor",
      1538 : "ACCurrentMultiplier",
      1539 : "ACCurrentDivisor",
      1540 : "ACPowerMultiplier",
      1541 : "ACPowerDivisor",
      1792 : "DCOverloadAlarmsMask",
      1793 : "DCVoltageOverload",
      1794 : "DCCurrentOverload",
      2048 : "ACAlarmsMask",
      2049 : "ACVoltageOverload",
      2050 : "ACCurrentOverload",
      2051 : "ACActivePowerOverload",
      2052 : "ACReactivePowerOverload",
      2053 : "AverageRMSOverVoltage",
      2054 : "AverageRMSUnderVoltage",
      2055 : "RMSExtremeOverVoltage",
      2056 : "RMSExtremeUnderVoltage",
      2057 : "RMSVoltageSag",
      2058 : "RMSVoltageSwell",
      2305 : "LineCurrentPhB",
      2306 : "ActiveCurrentPhB",
      2307 : "ReactiveCurrentPhB",
      2309 : "RMSVoltagePhB",
      2310 : "RMSVoltageMinPhB",
      2311 : "RMSVoltageMaxPhB",
      2312 : "RMSCurrentPhB",
      2313 : "RMSCurrentMinPhB",
      2314 : "RMSCurrentMaxPhB",
      2315 : "ActivePowerPhB",
      2316 : "ActivePowerMinPhB",
      2317 : "ActivePowerMaxPhB",
      2318 : "ReactivePowerPhB",
      2319 : "ApparentPowerPhB",
      2320 : "PowerFactorPhB",
      2321 : "AverageRMSVoltageMeasurementPeriodPhB",
      2322 : "AverageRMSOverVoltageCounterPhB",
      2323 : "AverageRMSUnderVoltageCounterPhB",
      2324 : "RMSExtremeOverVoltagePeriodPhB",
      2325 : "RMSExtremeUnderVoltagePeriodPhB",
      2326 : "RMSVoltageSagPeriodPhB",
      2327 : "RMSVoltageSwellPeriodPhB",
      2561 : "LineCurrentPhC",
      2562 : "ActiveCurrentPhC",
      2563 : "ReactiveCurrentPhC",
      2565 : "RMSVoltagePhC",
      2566 : "RMSVoltageMinPhC",
      2567 : "RMSVoltageMaxPhC",
      2568 : "RMSCurrentPhC",
      2569 : "RMSCurrentMinPhC",
      2570 : "RMSCurrentMaxPhC",
      2571 : "ActivePowerPhC",
      2572 : "ActivePowerMinPhC",
      2573 : "ActivePowerMaxPhC",
      2574 : "ReactivePowerPhC",
      2575 : "ApparentPowerPhC",
      2576 : "PowerFactorPhC",
      2577 : "AverageRMSVoltageMeasurementPeriodPhC",
      2578 : "AverageRMSOverVoltageCounterPhC",
      2579 : "AverageRMSUnderVoltageCounterPhC",
      2580 : "RMSExtremeOverVoltagePeriodPhC",
      2581 : "RMSExtremeUnderVoltagePeriodPhC",
      2582 : "RMSVoltageSagPeriodPhC",
      2583 : "RMSVoltageSwellPeriodPhC",
    },
  },

  enums : {
    "AoXLocatorAntennaMode" : [
      "Array4x4Ura",
      "Array3x3Ura",
      "Array1x4Ula",
      "Array4x4DpUra",
      "ArrayCorehw15x15Dp",
      "ArrayCorehw12x12Dp",
    ], //
    "AoXLocatorAoXMode" : [
      "OneShotBasic",
      "OneShotBasicLightweight",
      "OneShotFastResponse",
      "OneShotHighAccuracy",
      "OneShotBasicAzimuthOnly",
      "OneShotFastResponseAzimuthOnly",
      "OneShotHighAccuracyAzimuthOnly",
      "RealTimeFastResponse",
      "RealTimeBasic",
      "RealTimeHighAccuracy",
    ], //
    "AoXLocatorCTEMode" : [
      "SiliconLabs",
      "Connection",
      "Connectionless",
    ], //
    "AoXLocatorReportingMode" : [
      "IQReport",
      "AngleReport",
    ], //
    "ArmArmMode" : [
      "Disarm",
      "ArmDayHomeZonesOnly",
      "ArmNightSleepZonesOnly",
      "ArmAllZones",
    ], //
    "ArmResponseArmNotification" : [
      "AllZonesDisarmed",
      "OnlyDayHomeZonesArmed",
      "OnlyNightSleepZonesArmed",
      "AllZonesArmed",
      "InvalidArmDisarmCode",
      "NotReadyToArm",
      "AlreadyDisarmed",
    ], //
    "BarrierControlMovingState" : [
      "Stopped",
      "Closing",
      "Opening",
    ], //
    "BasicGenericDeviceClass" : [
      "Lighting",
    ], //
    "BasicGenericDeviceType" : [
      "Incandescent",
      "SpotlightHalogen",
      "HalogenBulb",
      "Cfl",
      "LinearFluorescent",
      "LEDBulb",
      "SpotlightLED",
      "LEDStrip",
      "LEDTube",
      "GenericIndoorLuminaireOrLightFixture",
      "GenericOutdoorLuminaireOrLightFixture",
      "PendantLuminaireOrLightFixture",
      "FloorStandingLuminaireOrLightFixture",
      "GenericController",
      "WallSwitch",
      "PortableRemoteController",
      "MotionSensorOrLightSensor",
      "GenericActuator",
      "WallSocket",
      "GatewayOrBridge",
      "PlugInUnit",
      "RetrofitActuator",
      "Unspecified",
    ], //
    "BasicPhysicalEnvironment" : [
      "UnspecifiedEnvironment",
      "Deprecated",
      "Bar",
      "Courtyard",
      "Bathroom",
      "Bedroom",
      "BilliardRoom",
      "UtilityRoom",
      "Cellar",
      "StorageCloset",
      "Theater",
      "Office",
      "Deck",
      "Den",
      "DiningRoom",
      "ElectricalRoom",
      "Elevator",
      "Entry",
      "FamilyRoom",
      "MainFloor",
      "Upstairs",
      "Downstairs",
      "BasementLowerLevel",
      "Gallery",
      "GameRoom",
      "Garage",
      "Gym",
      "Hallway",
      "House",
      "Kitchen",
      "LaundryRoom",
      "Library",
      "MasterBedroom",
      "MudRoom",
      "Nursery",
      "Pantry",
      "SecondaryOffice",
      "Outside",
      "Pool",
      "Porch",
      "SewingRoom",
      "SittingRoom",
      "Stairway",
      "Yard",
      "Attic",
      "HotTub",
      "LivingRoom",
      "Sauna",
      "ShopOrWorkshop",
      "GuestBedroom",
      "GuestBath",
      "PowderRoom",
      "BackYard",
      "FrontYard",
      "Patio",
      "Driveway",
      "SunRoom",
      "SecondaryLivingRoom",
      "Spa",
      "Whirlpool",
      "Shed",
      "EquipmentStorage",
      "HobbyOrCraftRoom",
      "Fountain",
      "Pond",
      "ReceptionRoom",
      "BreakfastRoom",
      "Nook",
      "Garden",
      "Balcony",
      "PanicRoom",
      "Terrace",
      "Roof",
      "Toilet",
      "ToiletMain",
      "OutsideToilet",
      "ShowerRoom",
      "Study",
      "FrontGarden",
      "BackGarden",
      "Kettle",
      "Television",
      "Stove",
      "Microwave",
      "Toaster",
      "Vacuum",
      "Appliance",
      "FrontDoor",
      "BackDoor",
      "FridgeDoor",
      "MedicationCabinetDoor",
      "WardrobeDoor",
      "FrontCupboardDoor",
      "OtherDoor",
      "WaitingRoom",
      "TriageRoom",
      "DoctorsOffice",
      "PatientsPrivateRoom",
      "ConsultationRoom",
      "NurseStation",
      "Ward",
      "Corridor",
      "OperatingTheatre",
      "DentalSurgeryRoom",
      "MedicalImagingRoom",
      "DecontaminationRoom",
      "Atrium",
      "Mirror",
      "UnknownEnvironment",
    ], //
    "BasicPowerSource" : [
      "Unknown",
      "SinglePhaseMains",
      "ThreePhaseMains",
      "Battery",
      "DCSource",
      "EmergencyMainsConstantlyPowered",
      "EmergencyMainsAndTransferSwitch",
      "UnknownWithBatteryBackup",
      "SinglePhaseMainsWithBatteryBackup",
      "ThreePhaseMainsWithBatteryBackup",
      "BatteryWithBatteryBackup",
      "DCSourceWithBatteryBackup",
      "EmergencyMainsConstantlyPoweredWithBatteryBackup",
      "EmergencyMainsAndTransferSwitchWithBatteryBackup",
    ], //
    "BatterySize" : [
      "NoBattery",
      "BuiltIn",
      "Other",
      "Aa",
      "Aaa",
      "C",
      "D",
      "Cr2",
      "Cr123a",
      "Unknown",
    ], //
    "BypassResponseZoneIDBypassResult" : [
      "ZoneBypassed",
      "ZoneNotBypassed",
      "NotAllowed",
      "InvalidZoneID",
      "UnknownZoneID",
      "InvalidArmDisarmCode",
    ], //
    "CCColorLoopDirection" : [
      "DecrementEnhancedCurrentHue",
      "IncrementEnhancedCurrentHue",
    ], //
    "CCDirection" : [
      "ShortestDistance",
      "LongestDistance",
      "Up",
      "Down",
    ], //
    "CCMoveMode" : [
      "Stop",
      "Up",
      "Down",
    ], //
    "CCStepMode" : [
      "Up",
      "Down",
    ], //
    "ColorControlColorLoopActive" : [
      "ColorLoopInactive",
      "ColorLoopActive",
    ], //
    "ColorControlColorMode" : [
      "CurrentHueAndCurrentSaturation",
      "CurrentXAndCurrentY",
      "ColorTemperatureMireds",
    ], //
    "ColorControlDriftCompensation" : [
      "None",
      "OtherOrUnknown",
      "TemperatureMonitoring",
      "OpticalLuminanceMonitoringAndFeedback",
      "OpticalColorMonitoringAndFeedback",
    ], //
    "ColorControlEnhancedColorMode" : [
      "CurrentHueAndCurrentSaturation",
      "CurrentXAndCurrentY",
      "ColorTemperatureMireds",
      "EnhancedCurrentHueAndCurrentSaturation",
    ], //
    "ColorLoopSetAction" : [
      "DeactivateColorLoop",
      "ActivateColorLoopFromColorLoopStartEnhancedHue",
      "ActivateColorLoopFromEnhancedCurrentHue",
    ], //
    "CommissioningNetworkKeyType" : [
      "StandardKey",
    ], //
    "CommissioningProtocolVersion" : [
      "Zigbee2006OrLater",
    ], //
    "CommissioningStackProfile" : [
      "ZigbeeStackProfile",
      "ZigbeeProStackProfile",
    ], //
    "CommissioningStartupControl" : [
      "OnANetwork",
      "FormNetwork",
      "RejoinNetwork",
      "JoinUsingMACAssociation",
    ], //
    "DehumidificationControlDehumidificationLockout" : [
      "DehumidificationNotAllowed",
      "DehumidificationAllowed",
    ], //
    "DehumidificationControlRelativeHumidityDisplay" : [
      "NotDisplayed",
      "Displayed",
    ], //
    "DehumidificationControlRelativeHumidityMode" : [
      "MeasuredLocally",
      "UpdatedOverTheNetwork",
    ], //
    "DoorLockDoorState" : [
      "Open",
      "Closed",
      "ErrorJammed",
      "ErrorForcedOpen",
      "ErrorUnspecified",
      "Undefined",
    ], //
    "DoorLockLEDSettings" : [
      "NeverUseLED",
      "UseLEDExceptForAccessAllowed",
      "UseLEDForAllEvents",
    ], //
    "DoorLockLockState" : [
      "NotFullyLocked",
      "Locked",
      "Unlocked",
      "Undefined",
    ], //
    "DoorLockLockType" : [
      "DeadBolt",
      "Magnetic",
      "Other",
      "Mortise",
      "Rim",
      "LatchBolt",
      "CylindricalLock",
      "TubularLock",
      "InterconnectedLock",
      "DeadLatch",
      "DoorFurniture",
    ], //
    "DoorLockSecurityLevel" : [
      "Network",
      "Aps",
    ], //
    "DoorLockSoundVolume" : [
      "SilentMode",
      "LowVolume",
      "HighVolume",
    ], //
    "DrlkOperEventSource" : [
      "Keypad",
      "Rf",
      "Manual",
      "Rfid",
      "Indeterminate",
    ], //
    "DrlkOperMode" : [
      "Normal",
      "Vacation",
      "Privacy",
      "NoRFLockOrUnlock",
      "Passage",
    ], //
    "DrlkPassFailStatus" : [
      "Pass",
      "Fail",
    ], //
    "DrlkSetCodeStatus" : [
      "Success",
      "GeneralFailure",
      "MemoryFull",
      "DuplicateCode",
    ], //
    "DrlkSettableUserStatus" : [
      "OccupiedEnabled",
      "OccupiedDisabled",
    ], //
    "DrlkUserStatus" : [
      "Available",
      "OccupiedEnabled",
      "OccupiedDisabled",
      "NotSupported",
    ], //
    "DrlkUserType" : [
      "UnrestrictedUser",
      "YearDayScheduleUser",
      "WeekDayScheduleUser",
      "MasterUser",
      "NonAccessUser",
      "NotSupported",
    ], //
    "FanControlFanMode" : [
      "Off",
      "Low",
      "Medium",
      "High",
      "On",
      "Auto",
      "Smart",
    ], //
    "FanControlFanModeSequence" : [
      "LowMedHigh",
      "LowHigh",
      "LowMedHighAuto",
      "LowHighAuto",
      "OnAuto",
    ], //
    "GetLogRecordResponseEventType" : [
      "Operation",
      "Programming",
      "Alarm",
    ], //
    "GetMeasurementProfileResponseStatus" : [
      "Success",
      "AttributeProfileNotSupported",
      "InvalidStartTime",
      "MoreIntervalsRequestedThanCanBeReturned",
      "NoIntervalsAvailableForTheRequestedTime",
    ], //
    "HVACSystemTypeConfigurationCoolingSystemStage" : [
      "CoolStage1",
      "CoolStage2",
      "CoolStage3",
    ], //
    "HVACSystemTypeConfigurationHeatingFuelSource" : [
      "ElectricOrB",
      "GasOrO",
    ], //
    "HVACSystemTypeConfigurationHeatingSystemStage" : [
      "HeatStage1",
      "HeatStage2",
      "HeatStage3",
    ], //
    "HVACSystemTypeConfigurationHeatingSystemType" : [
      "Conventional",
      "HeatPump",
    ], //
    "IASZoneZoneState" : [
      "NotEnrolled",
      "Enrolled",
    ], //
    "IasZoneType" : [
      "StandardCIE",
      "MotionSensor",
      "ContactSwitch",
      "DoorOrWindowHandle",
      "FireSensor",
      "WaterSensor",
      "CarbonMonoxideSensor",
      "PersonalEmergencyDevice",
      "VibrationOrMovementSensor",
      "RemoteControl",
      "KeyFob",
      "Keypad",
      "StandardWarningDevice",
      "GlassBreakSensor",
      "SecurityRepeater",
      "Invalid",
    ], //
    "IasacPanelStatus" : [
      "PanelDisarmedReadyToArm",
      "ArmedStay",
      "ArmedNight",
      "ArmedAway",
      "ExitDelay",
      "EntryDelay",
      "NotReadyToArm",
      "InAlarm",
      "ArmingStay",
      "ArmingNight",
      "ArmingAway",
    ], //
    "IasaceAlarmStatus" : [
      "NoAlarm",
      "Burgler",
      "Fire",
      "Emergency",
      "PolicePanic",
      "FirePanic",
      "EmergencyPanic",
    ], //
    "IasaceAudibleNotification" : [
      "Mute",
      "DefaultSound",
    ], //
    "IaswdLevel" : [
      "LowLevel",
      "MediumLevel",
      "HighLevel",
      "VeryHighLevel",
    ], //
    "IlluminanceLevelSensingLevelStatus" : [
      "IlluminanceOnTarget",
      "IlluminanceBelowTarget",
      "IlluminanceAboveTarget",
    ], //
    "IlluminanceLevelSensingLightSensorType" : [
      "Photodiode",
      "Cmos",
      "Unknown",
    ], //
    "IlluminanceMeasurementLightSensorType" : [
      "Photodiode",
      "Cmos",
      "Unknown",
    ], //
    "ImageNotifyPayloadType" : [
      "QueryJitter",
      "QueryJitterAndManufacturerCode",
      "QueryJitterManufacturerCodeAndImageType",
      "QueryJitterManufacturerCodeImageTypeAndNewFileVersion",
    ], //
    "LoggingLevelEnum" : [
      "Debug",
      "Info",
      "Warning",
      "Error",
      "Critical",
    ], //
    "MeteringEnergyCarrierUnitOfMeasure" : [
      "KilowattHoursOrKilowatts",
      "CubicMeterOrCubicMeterPerHour",
      "CubicFeetOrCubicFeetPerHour",
      "CentumCubicFeetOrCentum",
      "USGallonsOrUSGalonsPerHour",
      "ImperialGallonsOrImperialGallonsPerhour",
      "BTUsOrBTUPerhour",
      "LitersOrLitersPerHour",
      "KPAGaugein",
      "KPAabsolute",
      "McfCubicFeetOrMcfPerHour",
      "Unitless",
      "MegaJouleMegaJoulePerSecond",
    ], //
    "MeteringMeteringDeviceType" : [
      "ElectricMetering",
      "GasMetering",
      "WaterMetering",
      "ThermalMetering",
      "PressureMetering",
      "HeatMetering",
      "CoolingMetering",
      "MirroredGasMetering",
      "MirroredWaterMetering",
      "MirroredThermalMetering",
      "MirroredPressureMetering",
      "MirroredHeatMetering",
      "MirroredCoolingMetering",
    ], //
    "MeteringSupplyStatus" : [
      "SupplyOFF",
      "SupplyOFFOrARMED",
      "SupplyON",
    ], //
    "MeteringTemperatureUnitOfMeasure" : [
      "DegreesKelvin",
      "DegreesCelsius",
      "DegreesFahrenheit",
    ], //
    "MeteringUnitofMeasure" : [
      "KilowattHoursOrKilowatts",
      "CubicMeterOrCubicMeterPerHour",
      "CubicFeetOrCubicFeetPerHour",
      "CentumCubicFeetOrCentum",
      "USGallonsOrUSGalonsPerHour",
      "ImperialGallonsOrImperialGallonsPerhour",
      "BTUsOrBTUPerhour",
      "LitersOrLitersPerHour",
      "KPAGaugein",
      "KPAabsolute",
      "McfCubicFeetOrMcfPerHour",
      "Unitless",
      "MegaJouleMegaJoulePerSecond",
    ], //
    "MoveStepMode" : [
      "Up",
      "Down",
    ], //
    "NmState" : [
      "Idle",
      "AddNode",
      "RemoveNode",
      "JoinNetwork",
      "LeaveNetwork",
      "NetworkRepair",
      "NetworkUpdate",
      "Reset",
      "ScanMode",
    ], //
    "NodeStateNetworkStatus" : [
      "OnlineFunctional",
      "OnlineInterviewing",
      "OnlineNonFunctional",
      "Unavailable",
      "Offline",
    ], //
    "NodeStateSecurity" : [
      "None",
      "ZWaveS0",
      "ZWaveS2Unauthenticated",
      "ZWaveS2Authenticated",
      "ZWaveS2AccessControl",
      "ZigbeeZ3",
    ], //
    "OTADeviceSpecificImageType" : [
      "ClientSecurityCredentials",
      "ClientConfiguration",
      "ServerLog",
      "Picture",
    ], //
    "OTAUpgradeImageUpgradeStatus" : [
      "Normal",
      "DownloadInProgress",
      "DownloadComplete",
      "WaitingToUpgrade",
      "CountDown",
      "WaitForMore",
      "WaitingToUpgradeViaExternalEvent",
    ], //
    "OTAUpgradeUpgradeActivationPolicy" : [
      "OTAServerActivationAllowed",
      "OutOfBandActivationOnly",
    ], //
    "OTAUpgradeUpgradeTimeoutPolicy" : [
      "ApplyUpgradeAfterTimeout",
      "DoNotApplyUpgradeAfterTimeout",
    ], //
    "OccupancySensingOccupancySensorType" : [
      "Pir",
      "Ultrasonic",
      "PIRAndUltrasonic",
      "PhysicalContact",
    ], //
    "OffWithEffectEffectIdentifier" : [
      "DelayedAllOff",
      "DyingLight",
    ], //
    "OnOffStartUpOnOff" : [
      "SetOnOffTo0",
      "SetOnOffTo1",
      "TogglePreviousOnOff",
      "SetPreviousOnOff",
    ], //
    "OperatingEventNotificationOperationEventCode" : [
      "UnknownOrMS",
      "Lock",
      "Unlock",
      "LockFailureInvalidPINOrID",
      "LockFailureInvalidSchedule",
      "UnlockFailureInvalidPINOrID",
      "UnlockFailureInvalidSchedule",
      "OneTouchLock",
      "KeyLock",
      "KeyUnlock",
      "AutoLock",
      "ScheduleLock",
      "ScheduleUnlock",
      "ManualLock",
      "ManualUnlock",
      "NonAccessUserOperationalEvent",
    ], //
    "OptionsStartupMode" : [
      "RestartUsingStartupParameters",
      "RestartUsingCurrentState",
    ], //
    "ProfileIntervalPeriod" : [
      "Daily",
      "60Minutes",
      "30Minutes",
      "15Minutes",
      "10Minutes",
      "7dot5Minutes",
      "5Minutes",
      "2dot5Minutes",
    ], //
    "ProgrammingEventNotificationProgramEventCode" : [
      "UnknownOrMS",
      "MasterCodeChanged",
      "PINCodeAdded",
      "PINCodeDeleted",
      "PINCodeChanged",
      "RFIDCodeAdded",
      "RFIDCodeDeleted",
    ], //
    "ProgrammingEventNotificationProgramEventSource" : [
      "Keypad",
      "Rf",
      "Rfid",
      "Indeterminate",
    ], //
    "PumpControlMode" : [
      "ConstantSpeed",
      "ConstantPressure",
      "ProportionalPressure",
      "ConstantFlow",
      "ConstantTemperature",
      "Automatic",
    ], //
    "PumpOperationMode" : [
      "Normal",
      "Minimum",
      "Maximum",
      "Local",
    ], //
    "SHDCFGDirection" : [
      "Closing",
      "Opening",
    ], //
    "SetpointRaiseOrLowerMode" : [
      "Heat",
      "Cool",
      "Both",
    ], //
    "ShadeConfigurationMode" : [
      "Normal",
      "Configure",
    ], //
    "SirenConfigurationStrobe" : [
      "NoStrobe",
      "UseStrobe",
    ], //
    "SirenConfigurationWarningMode" : [
      "Stop",
      "Burgler",
      "Fire",
      "Emergency",
      "PolicePanic",
      "FirePanic",
      "EmergencyPanic",
    ], //
    "SquawkConfigurationSquawkMode" : [
      "SoundForSystemIsArmed",
      "SoundForSystemIsDisarmed",
    ], //
    "ThermostatACCapacityFormat" : [
      "BTUh",
    ], //
    "ThermostatACCompressorType" : [
      "T1",
      "T2",
      "T3",
    ], //
    "ThermostatACLouverPosition" : [
      "FullyClosed",
      "FullyOpen",
      "QuarterOpen",
      "HalfOpen",
      "ThreeQuartersOpen",
    ], //
    "ThermostatACRefrigerantType" : [
      "R22",
      "R410a",
      "R407c",
    ], //
    "ThermostatACType" : [
      "CoolingAndFixedSpeed",
      "HeatPumpAndFixedSpeed",
      "CoolingAndInverter",
      "HeatPumpAndInverter",
    ], //
    "ThermostatControlSequenceOfOperation" : [
      "CoolingOnly",
      "CoolingWithReheat",
      "HeatingOnly",
      "HeatingWithReheat",
      "CoolingAndHeating4Pipes",
      "CoolingAndHeating4PipesWithReheat",
    ], //
    "ThermostatProgrammingOperationModeProgrammingMode" : [
      "SimpleOrSetpoint",
      "Schedule",
    ], //
    "ThermostatSetpointChangeSource" : [
      "Manual",
      "ScheduleOrInternalProgramming",
      "External",
    ], //
    "ThermostatStartOfWeek" : [
      "Sunday",
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday",
    ], //
    "ThermostatSystemMode" : [
      "Off",
      "Auto",
      "Cool",
      "Heat",
      "EmergencyHeating",
      "Precooling",
      "FanOnly",
      "Dry",
      "Sleep",
    ], //
    "ThermostatTemperatureSetpointHold" : [
      "SetpointHoldOff",
      "SetpointHoldOn",
    ], //
    "ThermostatThermostatRunningMode" : [
      "Off",
      "Cool",
      "Heat",
    ], //
    "ThermostatUserInterfaceConfigurationKeypadLockout" : [
      "None",
      "Level1",
      "Level2",
      "Level3",
      "Level4",
      "Level5",
    ], //
    "ThermostatUserInterfaceConfigurationScheduleProgrammingVisibility" : [
      "LocalEnabled",
      "LocalDisabled",
    ], //
    "ThermostatUserInterfaceConfigurationTemperatureDisplayMode" : [
      "Celsius",
      "Fahrenheit",
    ], //
    "TriggerEffectEffectIdentifier" : [
      "Blink",
      "Breathe",
      "Okay",
      "ChannelChange",
      "FinishEffect",
      "StopEffect",
    ], //
    "TriggerEffectEffectVariant" : [
      "Default",
    ], //
    "TxReportTransmissionSpeed" : [
      "ZWave9600BitsPerSecond",
      "ZWave40KbitsPerSecond",
      "ZWave100KbitsPerSecond",
      "ZWaveLongRange100KbitsPerSecond",
      "Other",
      "Unknown",
    ], //
    "WindowCoveringWindowCoveringType" : [
      "Rollershade",
      "Rollershade2Motor",
      "RollershadeExterior",
      "RollershadeExterior2Motor",
      "Drapery",
      "Awning",
      "Shutter",
      "TiltBlindTiltOnly",
      "TiltBlindLiftAndTilt",
      "ProjectorScreen",
    ], //
    "ZoneEnrollResponseEnrollResponseCode" : [
      "Success",
      "NotSupported",
      "NoEnrollPermit",
      "TooManyZones",
    ], //
    "AttributeReportingStatus" : [
      "Pending",
      "Complete",
    ], //
    "ZclStatus" : [
      "Success",
      "Failure",
      "NotAuthorized",
      "MalformedCommand",
      "UnsupClusterCommand",
      "UnsupGeneralCommand",
      "UnsupManufClusterCommand",
      "UnsupManufGeneralCommand",
      "InvalidField",
      "UnsupportedAttribute",
      "InvalidValue",
      "ReadOnly",
      "InsufficientSpace",
      "DuplicateExists",
      "NotFound",
      "UnreportableAttribute",
      "InvalidDataType",
      "InvalidSelector",
      "WriteOnly",
      "InconsistentStartupState",
      "DefinedOutOfBand",
      "Inconsistent",
      "ActionDenied",
      "Timeout",
      "Abort",
      "InvalidImage",
      "WaitForData",
      "NoImageAvailable",
      "RequireMoreImage",
      "NotificationPending",
      "HardwareFailure",
      "SoftwareFailure",
      "CalibrationError",
      "UnsupportedCluster",
      "LimitReached",
    ], //
    "ZclType" : [
      "Nodata",
      "Data8",
      "Data16",
      "Data24",
      "Data32",
      "Data40",
      "Data48",
      "Data56",
      "Data64",
      "Bool",
      "Map8",
      "Map16",
      "Map24",
      "Map32",
      "Map40",
      "Map48",
      "Map56",
      "Map64",
      "Uint8",
      "Uint16",
      "Uint24",
      "Uint32",
      "Uint40",
      "Uint48",
      "Uint56",
      "Uint64",
      "Int8",
      "Int16",
      "Int24",
      "Int32",
      "Int40",
      "Int48",
      "Int56",
      "Int64",
      "Enum8",
      "Enum16",
      "Semi",
      "Single",
      "Double",
      "Octstr",
      "String",
      "Octstr16",
      "String16",
      "Array",
      "Struct",
      "Set",
      "Bag",
      "ToD",
      "Date",
      "Utc",
      "ClusterId",
      "AttribId",
      "BacOID",
      "Eui64",
      "Key128",
      "Unk",
    ], //
  },

  bitmaps : {
    "BallastConfigurationBallastStatus" : {
      1 : "BallastNonOperational",
      2 : "LampFailure",
    },
    "BallastConfigurationLampAlarmMode" : {
      1 : "LampBurnHours",
    },
    "BarrierControlCapabilities" : {
      1 : "PartialBarrier",
    },
    "BarrierControlSafetyStatus" : {
      1 : "RemoteLockout",
      2 : "TamperDetected",
      4 : "FailedCommunication",
      8 : "PositionFailure",
    },
    "BasicAlarmMask" : {
      1 : "GeneralHardwareFault",
      2 : "GeneralSoftwareFault",
    },
    "BasicDisableLocalConfig" : {
      1 : "DisableResetToFactoryDefaults",
      2 : "DisableDeviceConfiguration",
    },
    "BatteryAlarmMask" : {
      1 : "BatteryVoltageTooLow",
      2 : "BatteryAlarm1",
      4 : "BatteryAlarm2",
      8 : "BatteryAlarm3",
    },
    "BatteryAlarmState" : {
      1 : "Battery1MinThreshold",
      2 : "Battery1Threshold1",
      4 : "Battery1Threshold2",
      8 : "Battery1Threshold3",
      1024 : "Battery2MinThreshold",
      2048 : "Battery2Threshold1",
      4096 : "Battery2Threshold2",
      8192 : "Battery2Threshold3",
      1048576 : "Battery3MinThreshold",
      2097152 : "Battery3Threshold1",
      4194304 : "Battery3Threshold2",
      8388608 : "Battery3Threshold3",
      1073741824 : "MainsPowerSupplyLost",
    },
    "CCColorOptions" : {
      1 : "ExecuteIfOff",
    },
    "ColorControlColorCapabilities" : {
      1 : "HueSaturationSupported",
      2 : "EnhancedHueSupported",
      4 : "ColorLoopSupported",
      8 : "XYSupported",
      16 : "ColorTemperatureSupported",
    },
    "ColorLoopSetUpdateFlags" : {
      1 : "UpdateAction",
      2 : "UpdateDirection",
      4 : "UpdateTime",
      8 : "UpdateStartHue",
    },
    "CopySceneMode" : {
      1 : "CopyAllScenes",
    },
    "DeviceTemperatureConfigurationDeviceTempAlarmMask" : {
      1 : "DeviceTemperatureTooLow",
      2 : "DeviceTemperatureTooHigh",
    },
    "DoorLockAlarmMask" : {
      1 : "DeadboltJammed",
      2 : "LockResetToFactoryDefaults",
      8 : "RFPowerModuleCycled",
      16 : "TamperAlarmWrongCodeEntryLimit",
      32 : "TamperAlarmFrontEscutcheonRemovedFromMain",
      64 : "ForcedDoorOpenUnderDoorLockedCondition",
    },
    "DoorLockDefaultConfigurationRegister" : {
      1 : "DefaultEnableLocalProgrammingAttributeIsEnabled",
      2 : "DefaultKeypadInterfaceIsEnabled",
      4 : "DefaultRFInterfaceIsEnabled",
      32 : "DefaultSoundVolumeIsEnabled",
      64 : "DefaultAutoRelockTimeIsEnabled",
      128 : "DefaultLEDSettingsIsEnabled",
    },
    "DoorLockKeypadOperationEventMask" : {
      1 : "KeypadOpUnknownOrMS",
      2 : "KeypadOpLock",
      4 : "KeypadOpUnlock",
      8 : "KeypadOpLockErrorInvalidPIN",
      16 : "KeypadOpLockErrorInvalidSchedule",
      32 : "KeypadOpUnlockInvalidPIN",
      64 : "KeypadOpUnlockInvalidSchedule",
      128 : "KeypadOpNonAccessUser",
    },
    "DoorLockKeypadProgrammingEventMask" : {
      1 : "KeypadProgUnknownOrMS",
      2 : "KeypadProgMasterCodeChanged",
      4 : "KeypadProgPINAdded",
      8 : "KeypadProgPINDeleted",
      16 : "KeypadProgPINChanged",
    },
    "DoorLockManualOperationEventMask" : {
      1 : "ManualOpUnknownOrMS",
      2 : "ManualOpThumbturnLock",
      4 : "ManualOpThumbturnUnlock",
      8 : "ManualOpOneTouchLock",
      16 : "ManualOpKeyLock",
      32 : "ManualOpKeyUnlock",
      64 : "ManualOpAutoLock",
      128 : "ManualOpScheduleLock",
      256 : "ManualOpScheduleUnlock",
      512 : "ManualOpLock",
      1024 : "ManualOpUnlock",
    },
    "DoorLockRFIDOperationEventMask" : {
      1 : "RFIDOpUnknownOrMS",
      2 : "RFIDOpLock",
      4 : "RFIDOpUnlock",
      8 : "RFIDOpLockErrorInvalidRFID",
      16 : "RFIDOpLockErrorInvalidSchedule",
      32 : "RFIDOpUnlockErrorInvalidRFID",
      64 : "RFIDOpUnlockErrorInvalidSchedule",
    },
    "DoorLockRFIDProgrammingEventMask" : {
      1 : "RFIDProgUnknownOrMS",
      32 : "RFIDProgRFIDAdded",
      64 : "RFIDProgRFIDDeleted",
    },
    "DoorLockRFOperationEventMask" : {
      1 : "RFOpUnknownOrMS",
      2 : "RFOpLock",
      4 : "RFOpUnlock",
      8 : "RFOpLockErrorInvalidCode",
      16 : "RFOpLockErrorInvalidSchedule",
      32 : "RFOpUnlockInvalidCode",
      64 : "RFOpUnlockInvalidSchedule",
    },
    "DoorLockRFProgrammingEventMask" : {
      1 : "RFProgUnknownOrMS",
      4 : "RFProgPINAdded",
      8 : "RFProgPINDeleted",
      16 : "RFProgPINChanged",
      32 : "RFProgRFIDAdded",
      64 : "RFProgRFIDDeleted",
    },
    "DoorLockSupportedOperatingModes" : {
      1 : "NormalModeSupported",
      2 : "VacationModeSupported",
      4 : "PrivacyModeSupported",
      8 : "NoRFLockOrUnlockModeSupported",
      16 : "PassageModeSupported",
    },
    "DrlkDaysMask" : {
      1 : "Sun",
      2 : "Mon",
      4 : "Tue",
      8 : "Wed",
      16 : "Thu",
      32 : "Fri",
      64 : "Sat",
      128 : "Enable",
    },
    "ElectricalMeasurementACAlarmsMask" : {
      1 : "VoltageOverload",
      2 : "CurrentOverload",
      4 : "ActivePowerOverload",
      8 : "ReactivePowerOverload",
      16 : "AverageRMSOverVoltage",
      32 : "AverageRMSUnderVoltage",
      64 : "RMSExtremeOverVoltage",
      128 : "RMSExtremeUnderVoltage",
      256 : "RMSVoltageSag",
      512 : "RMSVoltageSwell",
    },
    "ElectricalMeasurementDCOverloadAlarmsMask" : {
      1 : "VoltageOverload",
      2 : "CurrentOverload",
    },
    "ElectricalMeasurementMeasurementType" : {
      1 : "ActiveMeasurementAC",
      2 : "ReactiveMeasurementAC",
      4 : "ApparentMeasurementAC",
      8 : "PhaseAMeasurement",
      16 : "PhaseBMeasurement",
      32 : "PhaseCMeasurement",
      64 : "DCMeasurement",
      128 : "HarmonicsMeasurement",
      256 : "PowerQualityMeasurement",
    },
    "GroupsNameSupport" : {
      128 : "Supported",
    },
    "IasZoneStatus" : {
      1 : "Alarm1",
      2 : "Alarm2",
      4 : "Tamper",
      8 : "BatteryLow",
      16 : "SupervisionNotify",
      32 : "RestoreNotify",
      64 : "Trouble",
      128 : "ACMainsFault",
      256 : "Test",
      512 : "BatteryDefect",
    },
    "ImageBlockRequestFieldControl" : {
      1 : "RequestNodeIEEEAddressPresent",
      2 : "MinimumBlockPeriodPresent",
    },
    "ImagePageRequestFieldControl" : {
      1 : "RequestNodeIEEEAddressPresent",
    },
    "LevelOptions" : {
      1 : "ExecuteIfOff",
      2 : "CoupleColorTempToLevel",
    },
    "MeteringDemandFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "MeteringEnergyCarrierDemandFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "MeteringEnergyCarrierSummationFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "MeteringHistoricalConsumptionFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "MeteringSummationFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "MeteringTemperatureFormatting" : {
      1 : "SuppressLeadingZeros",
      224 : "NumberOfDigitsToTheRightOfDecimalPoint",
      30 : "NumberOfDigitsToTheLeftOfTheDecimalPoint",
    },
    "OccupancySensingOccupancy" : {
      1 : "SensedOccupancy",
    },
    "OccupancySensingOccupancySensorTypeBitmap" : {
      1 : "Pir",
      2 : "Ultrasonic",
      4 : "PhysicalContact",
    },
    "OnWithTimedOffOnOffControl" : {
      1 : "AcceptOnlyWhenOn",
    },
    "PowerConfigurationMainsAlarmMask" : {
      1 : "MainsVoltageTooLow",
      2 : "MainsVoltageTooHigh",
      4 : "MainsPowerSupplyLost",
    },
    "PumpConfigurationAndControlAlarmMask" : {
      1 : "SupplyVoltageTooLow",
      2 : "SupplyVoltageTooHigh",
      4 : "PowerMissingPhase",
      8 : "SystemPressureTooLow",
      16 : "SystemPressureTooHigh",
      32 : "DryRunning",
      64 : "MotorTemperatureTooHigh",
      128 : "PumpMotorHasFatalFailure",
      256 : "ElectronicTemperatureTooHigh",
      512 : "PumpBlocked",
      1024 : "SensorFailure",
      2048 : "ElectronicNonFatalFailure",
      4096 : "ElectronicFatalFailure",
      8192 : "GeneralFault",
    },
    "PumpConfigurationAndControlPumpStatus" : {
      1 : "DeviceFault",
      2 : "SupplyFault",
      4 : "SpeedLow",
      8 : "SpeedHigh",
      16 : "LocalOverride",
      32 : "Running",
      64 : "RemotePressure",
      128 : "RemoteFlow",
      256 : "RemoteTemperature",
    },
    "QueryNextImageRequestFieldControl" : {
      1 : "HardwareVersionPresent",
    },
    "ResetStartupParametersOptions" : {
      1 : "ResetCurrent",
      2 : "ResetAll",
      4 : "EraseIndex",
    },
    "RestartDeviceOptions" : {
      7 : "StartupMode",
      8 : "Immediate",
    },
    "ScenesNameSupport" : {
      128 : "SceneNamesSupported",
    },
    "ShadeConfigurationStatus" : {
      1 : "ShadeOperational",
      2 : "ShadeAdjusting",
      4 : "ShadeDirection",
      8 : "ForwardDirectionOfMotor",
    },
    "SquawkSquawkConfiguration" : {
      3 : "SquawkLevel",
      8 : "SquawkStrobeActive",
      240 : "SquawkMode",
    },
    "StartWarningSirenConfiguration" : {
      3 : "SirenLevel",
      12 : "Strobe",
      240 : "WarningMode",
    },
    "ThermostatACErrorCode" : {
      1 : "CompressorFailureOrRefrigerantLeakage",
      2 : "RoomTemperatureSensorFailure",
      4 : "OutdoorTemperatureSensorFailure",
      8 : "IndoorCoilTemperatureSensorFailure",
      16 : "FanFailure",
    },
    "ThermostatAlarmMask" : {
      1 : "InitializationFailure",
      2 : "HardwareFailure",
      4 : "SelfCalibrationFailure",
    },
    "ThermostatHVACSystemTypeConfiguration" : {
      3 : "CoolingSystemStage",
      12 : "HeatingSystemStage",
      16 : "HeatingSystemType",
      32 : "HeatingFuelSource",
    },
    "ThermostatOccupancy" : {
      1 : "Occupied",
    },
    "ThermostatRemoteSensing" : {
      1 : "LocalTemperatureRemote",
      2 : "OutdoorTemperatureRemote",
      4 : "OccupancyRemote",
    },
    "ThermostatThermostatProgrammingOperationMode" : {
      1 : "ProgrammingMode",
      2 : "AutoOrRecovery",
      4 : "EconomyOrEnergyStar",
    },
    "ThermostatThermostatRunningState" : {
      1 : "HeatOn",
      2 : "CoolOn",
      4 : "FanOn",
      8 : "HeatSecondStageOn",
      16 : "CoolSecondStageOn",
      32 : "FanSecondStageOn",
      64 : "FanThirdStageOn",
    },
    "TimeTimeStatus" : {
      1 : "Master",
      2 : "Synchronized",
      4 : "MasterZoneDst",
      8 : "Superseding",
    },
    "TstatScheduleDOW" : {
      1 : "Sunday",
      2 : "Monday",
      4 : "Tuesday",
      8 : "Wednesday",
      16 : "Thursday",
      32 : "Friday",
      64 : "Saturday",
      128 : "AwayOrVacation",
    },
    "TstatScheduleMode" : {
      1 : "Heat",
      2 : "Cool",
    },
    "WindowCoveringConfigOrStatus" : {
      1 : "Operational",
      2 : "Online",
      4 : "OpenAndUpCommandsReversed",
      8 : "LiftClosedLoop",
      16 : "TiltClosedLoop",
      32 : "LiftEncoderControlled",
      64 : "TiltEncoderControlled",
    },
    "WindowCoveringMode" : {
      1 : "MotorDirectionReversed",
      2 : "CalibrationMode",
      4 : "MaintenanceMode",
      8 : "LEDFeedback",
    },
    "Map16" : {},
    "Map24" : {},
    "Map32" : {},
    "Map40" : {},
    "Map48" : {},
    "Map56" : {},
    "Map64" : {},
    "Map8" : {},
  }
}
