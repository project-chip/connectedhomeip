exports.model = {
  3 : {
    name : "Identify",
    commands : {
      0 : {
        name : "Identify",
        arguments : [
          "IdentifyTime",
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
      1 : "IdentifyType",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  4 : {
    name : "Groups",
    commands : {
      0 : {
        name : "AddGroup",
        arguments : [
          "GroupID",
          "GroupName",
        ],
      },
      0 : {
        name : "AddGroupResponse",
        arguments : [
          "Status",
          "GroupID",
        ],
      },
      1 : {
        name : "ViewGroup",
        arguments : [
          "GroupID",
        ],
      },
      1 : {
        name : "ViewGroupResponse",
        arguments : [
          "Status",
          "GroupID",
          "GroupName",
        ],
      },
      2 : {
        name : "GetGroupMembership",
        arguments : [
          "GroupList",
        ],
      },
      2 : {
        name : "GetGroupMembershipResponse",
        arguments : [
          "Capacity",
          "GroupList",
        ],
      },
      3 : {
        name : "RemoveGroup",
        arguments : [
          "GroupID",
        ],
      },
      3 : {
        name : "RemoveGroupResponse",
        arguments : [
          "Status",
          "GroupID",
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
          "GroupID",
          "GroupName",
        ],
      },
    },
    attributes : {
      0 : "NameSupport",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      0 : {
        name : "AddSceneResponse",
        arguments : [
          "Status",
          "GroupID",
          "SceneID",
        ],
      },
      1 : {
        name : "ViewScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      1 : {
        name : "ViewSceneResponse",
        arguments : [
          "Status",
          "GroupID",
          "SceneID",
          "TransitionTime",
          "SceneName",
          "ExtensionFieldSets",
        ],
      },
      2 : {
        name : "RemoveScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      2 : {
        name : "RemoveSceneResponse",
        arguments : [
          "Status",
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
      3 : {
        name : "RemoveAllScenesResponse",
        arguments : [
          "Status",
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
      4 : {
        name : "StoreSceneResponse",
        arguments : [
          "Status",
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
      6 : {
        name : "GetSceneMembershipResponse",
        arguments : [
          "Status",
          "Capacity",
          "GroupID",
          "SceneList",
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
      64 : {
        name : "EnhancedAddSceneResponse",
        arguments : [
          "Status",
          "GroupID",
          "SceneID",
        ],
      },
      65 : {
        name : "EnhancedViewScene",
        arguments : [
          "GroupID",
          "SceneID",
        ],
      },
      65 : {
        name : "EnhancedViewSceneResponse",
        arguments : [
          "Status",
          "GroupID",
          "SceneID",
          "TransitionTime",
          "SceneName",
          "ExtensionFieldSets",
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
      66 : {
        name : "CopySceneResponse",
        arguments : [
          "Status",
          "GroupIdentifierFrom",
          "SceneIdentifierFrom",
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
      6 : "SceneTableSize",
      7 : "FabricSceneInfo",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  7 : {
    name : "OnOffSwitchConfiguration",
    commands : {},
    attributes : {
      0 : "SwitchType",
      16 : "SwitchActions",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  8 : {
    name : "LevelControl",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  15 : {
    name : "BinaryInputBasic",
    commands : {},
    attributes : {
      4 : "ActiveText",
      28 : "Description",
      46 : "InactiveText",
      81 : "OutOfService",
      84 : "Polarity",
      85 : "PresentValue",
      103 : "Reliability",
      111 : "StatusFlags",
      256 : "ApplicationType",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  28 : {
    name : "PulseWidthModulation",
    commands : {},
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  29 : {
    name : "Descriptor",
    commands : {},
    attributes : {
      0 : "DeviceTypeList",
      1 : "ServerList",
      2 : "ClientList",
      3 : "PartsList",
      4 : "TagList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  30 : {
    name : "Binding",
    commands : {},
    attributes : {
      0 : "Binding",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  31 : {
    name : "AccessControl",
    commands : {},
    attributes : {
      0 : "Acl",
      1 : "Extension",
      2 : "SubjectsPerAccessControlEntry",
      3 : "TargetsPerAccessControlEntry",
      4 : "AccessControlEntriesPerFabric",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  37 : {
    name : "Actions",
    commands : {
      0 : {
        name : "InstantAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      1 : {
        name : "InstantActionWithTransition",
        arguments : [
          "ActionID",
          "InvokeID",
          "TransitionTime",
        ],
      },
      2 : {
        name : "StartAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      3 : {
        name : "StartActionWithDuration",
        arguments : [
          "ActionID",
          "InvokeID",
          "Duration",
        ],
      },
      4 : {
        name : "StopAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      5 : {
        name : "PauseAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      6 : {
        name : "PauseActionWithDuration",
        arguments : [
          "ActionID",
          "InvokeID",
          "Duration",
        ],
      },
      7 : {
        name : "ResumeAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      8 : {
        name : "EnableAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      9 : {
        name : "EnableActionWithDuration",
        arguments : [
          "ActionID",
          "InvokeID",
          "Duration",
        ],
      },
      10 : {
        name : "DisableAction",
        arguments : [
          "ActionID",
          "InvokeID",
        ],
      },
      11 : {
        name : "DisableActionWithDuration",
        arguments : [
          "ActionID",
          "InvokeID",
          "Duration",
        ],
      },
    },
    attributes : {
      0 : "ActionList",
      1 : "EndpointLists",
      2 : "SetupURL",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  40 : {
    name : "BasicInformation",
    commands : {
      0 : {
        name : "MfgSpecificPing",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "DataModelRevision",
      1 : "VendorName",
      2 : "VendorID",
      3 : "ProductName",
      4 : "ProductID",
      5 : "NodeLabel",
      6 : "Location",
      7 : "HardwareVersion",
      8 : "HardwareVersionString",
      9 : "SoftwareVersion",
      10 : "SoftwareVersionString",
      11 : "ManufacturingDate",
      12 : "PartNumber",
      13 : "ProductURL",
      14 : "ProductLabel",
      15 : "SerialNumber",
      16 : "LocalConfigDisabled",
      17 : "Reachable",
      18 : "UniqueID",
      19 : "CapabilityMinima",
      20 : "ProductAppearance",
      21 : "SpecificationVersion",
      22 : "MaxPathsPerInvoke",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  41 : {
    name : "OtaSoftwareUpdateProvider",
    commands : {
      0 : {
        name : "QueryImage",
        arguments : [
          "VendorID",
          "ProductID",
          "SoftwareVersion",
          "ProtocolsSupported",
          "HardwareVersion",
          "Location",
          "RequestorCanConsent",
          "MetadataForProvider",
        ],
      },
      1 : {
        name : "QueryImageResponse",
        arguments : [
          "Status",
          "DelayedActionTime",
          "ImageURI",
          "SoftwareVersion",
          "SoftwareVersionString",
          "UpdateToken",
          "UserConsentNeeded",
          "MetadataForRequestor",
        ],
      },
      2 : {
        name : "ApplyUpdateRequest",
        arguments : [
          "UpdateToken",
          "NewVersion",
        ],
      },
      3 : {
        name : "ApplyUpdateResponse",
        arguments : [
          "Action",
          "DelayedActionTime",
        ],
      },
      4 : {
        name : "NotifyUpdateApplied",
        arguments : [
          "UpdateToken",
          "SoftwareVersion",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  42 : {
    name : "OtaSoftwareUpdateRequestor",
    commands : {
      0 : {
        name : "AnnounceOTAProvider",
        arguments : [
          "ProviderNodeID",
          "VendorID",
          "AnnouncementReason",
          "MetadataForNode",
          "Endpoint",
        ],
      },
    },
    attributes : {
      0 : "DefaultOTAProviders",
      1 : "UpdatePossible",
      2 : "UpdateState",
      3 : "UpdateStateProgress",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  43 : {
    name : "LocalizationConfiguration",
    commands : {},
    attributes : {
      0 : "ActiveLocale",
      1 : "SupportedLocales",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  44 : {
    name : "TimeFormatLocalization",
    commands : {},
    attributes : {
      0 : "HourFormat",
      1 : "ActiveCalendarType",
      2 : "SupportedCalendarTypes",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  45 : {
    name : "UnitLocalization",
    commands : {},
    attributes : {
      0 : "TemperatureUnit",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  46 : {
    name : "PowerSourceConfiguration",
    commands : {},
    attributes : {
      0 : "Sources",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  47 : {
    name : "PowerSource",
    commands : {},
    attributes : {
      0 : "Status",
      1 : "Order",
      2 : "Description",
      3 : "WiredAssessedInputVoltage",
      4 : "WiredAssessedInputFrequency",
      5 : "WiredCurrentType",
      6 : "WiredAssessedCurrent",
      7 : "WiredNominalVoltage",
      8 : "WiredMaximumCurrent",
      9 : "WiredPresent",
      10 : "ActiveWiredFaults",
      11 : "BatVoltage",
      12 : "BatPercentRemaining",
      13 : "BatTimeRemaining",
      14 : "BatChargeLevel",
      15 : "BatReplacementNeeded",
      16 : "BatReplaceability",
      17 : "BatPresent",
      18 : "ActiveBatFaults",
      19 : "BatReplacementDescription",
      20 : "BatCommonDesignation",
      21 : "BatANSIDesignation",
      22 : "BatIECDesignation",
      23 : "BatApprovedChemistry",
      24 : "BatCapacity",
      25 : "BatQuantity",
      26 : "BatChargeState",
      27 : "BatTimeToFullCharge",
      28 : "BatFunctionalWhileCharging",
      29 : "BatChargingCurrent",
      30 : "ActiveBatChargeFaults",
      31 : "EndpointList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  48 : {
    name : "GeneralCommissioning",
    commands : {
      0 : {
        name : "ArmFailSafe",
        arguments : [
          "ExpiryLengthSeconds",
          "Breadcrumb",
        ],
      },
      1 : {
        name : "ArmFailSafeResponse",
        arguments : [
          "ErrorCode",
          "DebugText",
        ],
      },
      2 : {
        name : "SetRegulatoryConfig",
        arguments : [
          "NewRegulatoryConfig",
          "CountryCode",
          "Breadcrumb",
        ],
      },
      3 : {
        name : "SetRegulatoryConfigResponse",
        arguments : [
          "ErrorCode",
          "DebugText",
        ],
      },
      4 : {
        name : "CommissioningComplete",
        arguments : [

        ],
      },
      5 : {
        name : "CommissioningCompleteResponse",
        arguments : [
          "ErrorCode",
          "DebugText",
        ],
      },
    },
    attributes : {
      0 : "Breadcrumb",
      1 : "BasicCommissioningInfo",
      2 : "RegulatoryConfig",
      3 : "LocationCapability",
      4 : "SupportsConcurrentConnection",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  49 : {
    name : "NetworkCommissioning",
    commands : {
      0 : {
        name : "ScanNetworks",
        arguments : [
          "Ssid",
          "Breadcrumb",
        ],
      },
      1 : {
        name : "ScanNetworksResponse",
        arguments : [
          "NetworkingStatus",
          "DebugText",
          "WiFiScanResults",
          "ThreadScanResults",
        ],
      },
      2 : {
        name : "AddOrUpdateWiFiNetwork",
        arguments : [
          "Ssid",
          "Credentials",
          "Breadcrumb",
          "NetworkIdentity",
          "ClientIdentifier",
          "PossessionNonce",
        ],
      },
      3 : {
        name : "AddOrUpdateThreadNetwork",
        arguments : [
          "OperationalDataset",
          "Breadcrumb",
        ],
      },
      4 : {
        name : "RemoveNetwork",
        arguments : [
          "NetworkID",
          "Breadcrumb",
        ],
      },
      5 : {
        name : "NetworkConfigResponse",
        arguments : [
          "NetworkingStatus",
          "DebugText",
          "NetworkIndex",
          "ClientIdentity",
          "PossessionSignature",
        ],
      },
      6 : {
        name : "ConnectNetwork",
        arguments : [
          "NetworkID",
          "Breadcrumb",
        ],
      },
      7 : {
        name : "ConnectNetworkResponse",
        arguments : [
          "NetworkingStatus",
          "DebugText",
          "ErrorValue",
        ],
      },
      8 : {
        name : "ReorderNetwork",
        arguments : [
          "NetworkID",
          "NetworkIndex",
          "Breadcrumb",
        ],
      },
      9 : {
        name : "QueryIdentity",
        arguments : [
          "KeyIdentifier",
          "PossessionNonce",
        ],
      },
      10 : {
        name : "QueryIdentityResponse",
        arguments : [
          "Identity",
          "PossessionSignature",
        ],
      },
    },
    attributes : {
      0 : "MaxNetworks",
      1 : "Networks",
      2 : "ScanMaxTimeSeconds",
      3 : "ConnectMaxTimeSeconds",
      4 : "InterfaceEnabled",
      5 : "LastNetworkingStatus",
      6 : "LastNetworkID",
      7 : "LastConnectErrorValue",
      8 : "SupportedWiFiBands",
      9 : "SupportedThreadFeatures",
      10 : "ThreadVersion",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  50 : {
    name : "DiagnosticLogs",
    commands : {
      0 : {
        name : "RetrieveLogsRequest",
        arguments : [
          "Intent",
          "RequestedProtocol",
          "TransferFileDesignator",
        ],
      },
      1 : {
        name : "RetrieveLogsResponse",
        arguments : [
          "Status",
          "LogContent",
          "UTCTimeStamp",
          "TimeSinceBoot",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  51 : {
    name : "GeneralDiagnostics",
    commands : {
      0 : {
        name : "TestEventTrigger",
        arguments : [
          "EnableKey",
          "EventTrigger",
        ],
      },
      1 : {
        name : "TimeSnapshot",
        arguments : [

        ],
      },
      2 : {
        name : "TimeSnapshotResponse",
        arguments : [
          "SystemTimeMs",
          "PosixTimeMs",
        ],
      },
    },
    attributes : {
      0 : "NetworkInterfaces",
      1 : "RebootCount",
      2 : "UpTime",
      3 : "TotalOperationalHours",
      4 : "BootReason",
      5 : "ActiveHardwareFaults",
      6 : "ActiveRadioFaults",
      7 : "ActiveNetworkFaults",
      8 : "TestEventTriggersEnabled",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  52 : {
    name : "SoftwareDiagnostics",
    commands : {
      0 : {
        name : "ResetWatermarks",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "ThreadMetrics",
      1 : "CurrentHeapFree",
      2 : "CurrentHeapUsed",
      3 : "CurrentHeapHighWatermark",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  53 : {
    name : "ThreadNetworkDiagnostics",
    commands : {
      0 : {
        name : "ResetCounts",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "Channel",
      1 : "RoutingRole",
      2 : "NetworkName",
      3 : "PanId",
      4 : "ExtendedPanId",
      5 : "MeshLocalPrefix",
      6 : "OverrunCount",
      7 : "NeighborTable",
      8 : "RouteTable",
      9 : "PartitionId",
      10 : "Weighting",
      11 : "DataVersion",
      12 : "StableDataVersion",
      13 : "LeaderRouterId",
      14 : "DetachedRoleCount",
      15 : "ChildRoleCount",
      16 : "RouterRoleCount",
      17 : "LeaderRoleCount",
      18 : "AttachAttemptCount",
      19 : "PartitionIdChangeCount",
      20 : "BetterPartitionAttachAttemptCount",
      21 : "ParentChangeCount",
      22 : "TxTotalCount",
      23 : "TxUnicastCount",
      24 : "TxBroadcastCount",
      25 : "TxAckRequestedCount",
      26 : "TxAckedCount",
      27 : "TxNoAckRequestedCount",
      28 : "TxDataCount",
      29 : "TxDataPollCount",
      30 : "TxBeaconCount",
      31 : "TxBeaconRequestCount",
      32 : "TxOtherCount",
      33 : "TxRetryCount",
      34 : "TxDirectMaxRetryExpiryCount",
      35 : "TxIndirectMaxRetryExpiryCount",
      36 : "TxErrCcaCount",
      37 : "TxErrAbortCount",
      38 : "TxErrBusyChannelCount",
      39 : "RxTotalCount",
      40 : "RxUnicastCount",
      41 : "RxBroadcastCount",
      42 : "RxDataCount",
      43 : "RxDataPollCount",
      44 : "RxBeaconCount",
      45 : "RxBeaconRequestCount",
      46 : "RxOtherCount",
      47 : "RxAddressFilteredCount",
      48 : "RxDestAddrFilteredCount",
      49 : "RxDuplicatedCount",
      50 : "RxErrNoFrameCount",
      51 : "RxErrUnknownNeighborCount",
      52 : "RxErrInvalidSrcAddrCount",
      53 : "RxErrSecCount",
      54 : "RxErrFcsCount",
      55 : "RxErrOtherCount",
      56 : "ActiveTimestamp",
      57 : "PendingTimestamp",
      58 : "Delay",
      59 : "SecurityPolicy",
      60 : "ChannelPage0Mask",
      61 : "OperationalDatasetComponents",
      62 : "ActiveNetworkFaultsList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  54 : {
    name : "WiFiNetworkDiagnostics",
    commands : {
      0 : {
        name : "ResetCounts",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "Bssid",
      1 : "SecurityType",
      2 : "WiFiVersion",
      3 : "ChannelNumber",
      4 : "Rssi",
      5 : "BeaconLostCount",
      6 : "BeaconRxCount",
      7 : "PacketMulticastRxCount",
      8 : "PacketMulticastTxCount",
      9 : "PacketUnicastRxCount",
      10 : "PacketUnicastTxCount",
      11 : "CurrentMaxRate",
      12 : "OverrunCount",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  55 : {
    name : "EthernetNetworkDiagnostics",
    commands : {
      0 : {
        name : "ResetCounts",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "PHYRate",
      1 : "FullDuplex",
      2 : "PacketRxCount",
      3 : "PacketTxCount",
      4 : "TxErrCount",
      5 : "CollisionCount",
      6 : "OverrunCount",
      7 : "CarrierDetect",
      8 : "TimeSinceReset",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  56 : {
    name : "TimeSynchronization",
    commands : {
      0 : {
        name : "SetUTCTime",
        arguments : [
          "UTCTime",
          "Granularity",
          "TimeSource",
        ],
      },
      1 : {
        name : "SetTrustedTimeSource",
        arguments : [
          "TrustedTimeSource",
        ],
      },
      2 : {
        name : "SetTimeZone",
        arguments : [
          "TimeZone",
        ],
      },
      3 : {
        name : "SetTimeZoneResponse",
        arguments : [
          "DSTOffsetRequired",
        ],
      },
      4 : {
        name : "SetDSTOffset",
        arguments : [
          "DSTOffset",
        ],
      },
      5 : {
        name : "SetDefaultNTP",
        arguments : [
          "DefaultNTP",
        ],
      },
    },
    attributes : {
      0 : "UTCTime",
      1 : "Granularity",
      2 : "TimeSource",
      3 : "TrustedTimeSource",
      4 : "DefaultNTP",
      5 : "TimeZone",
      6 : "DSTOffset",
      7 : "LocalTime",
      8 : "TimeZoneDatabase",
      9 : "NTPServerAvailable",
      10 : "TimeZoneListMaxSize",
      11 : "DSTOffsetListMaxSize",
      12 : "SupportsDNSResolve",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  57 : {
    name : "BridgedDeviceBasicInformation",
    commands : {},
    attributes : {
      1 : "VendorName",
      2 : "VendorID",
      3 : "ProductName",
      5 : "NodeLabel",
      7 : "HardwareVersion",
      8 : "HardwareVersionString",
      9 : "SoftwareVersion",
      10 : "SoftwareVersionString",
      11 : "ManufacturingDate",
      12 : "PartNumber",
      13 : "ProductURL",
      14 : "ProductLabel",
      15 : "SerialNumber",
      17 : "Reachable",
      18 : "UniqueID",
      20 : "ProductAppearance",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  59 : {
    name : "Switch",
    commands : {},
    attributes : {
      0 : "NumberOfPositions",
      1 : "CurrentPosition",
      2 : "MultiPressMax",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  60 : {
    name : "AdministratorCommissioning",
    commands : {
      0 : {
        name : "OpenCommissioningWindow",
        arguments : [
          "CommissioningTimeout",
          "PAKEPasscodeVerifier",
          "Discriminator",
          "Iterations",
          "Salt",
        ],
      },
      1 : {
        name : "OpenBasicCommissioningWindow",
        arguments : [
          "CommissioningTimeout",
        ],
      },
      2 : {
        name : "RevokeCommissioning",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "WindowStatus",
      1 : "AdminFabricIndex",
      2 : "AdminVendorId",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  62 : {
    name : "OperationalCredentials",
    commands : {
      0 : {
        name : "AttestationRequest",
        arguments : [
          "AttestationNonce",
        ],
      },
      1 : {
        name : "AttestationResponse",
        arguments : [
          "AttestationElements",
          "AttestationSignature",
        ],
      },
      2 : {
        name : "CertificateChainRequest",
        arguments : [
          "CertificateType",
        ],
      },
      3 : {
        name : "CertificateChainResponse",
        arguments : [
          "Certificate",
        ],
      },
      4 : {
        name : "CSRRequest",
        arguments : [
          "CSRNonce",
          "IsForUpdateNOC",
        ],
      },
      5 : {
        name : "CSRResponse",
        arguments : [
          "NOCSRElements",
          "AttestationSignature",
        ],
      },
      6 : {
        name : "AddNOC",
        arguments : [
          "NOCValue",
          "ICACValue",
          "IPKValue",
          "CaseAdminSubject",
          "AdminVendorId",
        ],
      },
      7 : {
        name : "UpdateNOC",
        arguments : [
          "NOCValue",
          "ICACValue",
        ],
      },
      8 : {
        name : "NOCResponse",
        arguments : [
          "StatusCode",
          "FabricIndex",
          "DebugText",
        ],
      },
      9 : {
        name : "UpdateFabricLabel",
        arguments : [
          "Label",
        ],
      },
      10 : {
        name : "RemoveFabric",
        arguments : [
          "FabricIndex",
        ],
      },
      11 : {
        name : "AddTrustedRootCertificate",
        arguments : [
          "RootCACertificate",
        ],
      },
    },
    attributes : {
      0 : "NOCs",
      1 : "Fabrics",
      2 : "SupportedFabrics",
      3 : "CommissionedFabrics",
      4 : "TrustedRootCertificates",
      5 : "CurrentFabricIndex",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  63 : {
    name : "GroupKeyManagement",
    commands : {
      0 : {
        name : "KeySetWrite",
        arguments : [
          "GroupKeySet",
        ],
      },
      1 : {
        name : "KeySetRead",
        arguments : [
          "GroupKeySetID",
        ],
      },
      2 : {
        name : "KeySetReadResponse",
        arguments : [
          "GroupKeySet",
        ],
      },
      3 : {
        name : "KeySetRemove",
        arguments : [
          "GroupKeySetID",
        ],
      },
      4 : {
        name : "KeySetReadAllIndices",
        arguments : [

        ],
      },
      5 : {
        name : "KeySetReadAllIndicesResponse",
        arguments : [
          "GroupKeySetIDs",
        ],
      },
    },
    attributes : {
      0 : "GroupKeyMap",
      1 : "GroupTable",
      2 : "MaxGroupsPerFabric",
      3 : "MaxGroupKeysPerFabric",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  64 : {
    name : "FixedLabel",
    commands : {},
    attributes : {
      0 : "LabelList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  65 : {
    name : "UserLabel",
    commands : {},
    attributes : {
      0 : "LabelList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  66 : {
    name : "ProxyConfiguration",
    commands : {},
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  67 : {
    name : "ProxyDiscovery",
    commands : {},
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  68 : {
    name : "ProxyValid",
    commands : {},
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  69 : {
    name : "BooleanState",
    commands : {},
    attributes : {
      0 : "StateValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  70 : {
    name : "IcdManagement",
    commands : {
      0 : {
        name : "RegisterClient",
        arguments : [
          "CheckInNodeID",
          "MonitoredSubject",
          "Key",
          "VerificationKey",
        ],
      },
      1 : {
        name : "RegisterClientResponse",
        arguments : [
          "ICDCounter",
        ],
      },
      2 : {
        name : "UnregisterClient",
        arguments : [
          "CheckInNodeID",
          "VerificationKey",
        ],
      },
      3 : {
        name : "StayActiveRequest",
        arguments : [

        ],
      },
      4 : {
        name : "StayActiveResponse",
        arguments : [
          "PromisedActiveDuration",
        ],
      },
    },
    attributes : {
      0 : "IdleModeDuration",
      1 : "ActiveModeDuration",
      2 : "ActiveModeThreshold",
      3 : "RegisteredClients",
      4 : "ICDCounter",
      5 : "ClientsSupportedPerFabric",
      6 : "UserActiveModeTriggerHint",
      7 : "UserActiveModeTriggerInstruction",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  71 : {
    name : "Timer",
    commands : {
      0 : {
        name : "SetTimer",
        arguments : [
          "NewTime",
        ],
      },
      1 : {
        name : "ResetTimer",
        arguments : [

        ],
      },
      2 : {
        name : "AddTime",
        arguments : [
          "AdditionalTime",
        ],
      },
      3 : {
        name : "ReduceTime",
        arguments : [
          "TimeReduction",
        ],
      },
    },
    attributes : {
      0 : "SetTime",
      1 : "TimeRemaining",
      2 : "TimerState",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  72 : {
    name : "OvenCavityOperationalState",
    commands : {
      0 : {
        name : "Pause",
        arguments : [

        ],
      },
      1 : {
        name : "Stop",
        arguments : [

        ],
      },
      2 : {
        name : "Start",
        arguments : [

        ],
      },
      3 : {
        name : "Resume",
        arguments : [

        ],
      },
      4 : {
        name : "OperationalCommandResponse",
        arguments : [
          "CommandResponseState",
        ],
      },
    },
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
      2 : "CountdownTime",
      3 : "OperationalStateList",
      4 : "OperationalState",
      5 : "OperationalError",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  73 : {
    name : "OvenMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      2 : "StartUpMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  74 : {
    name : "LaundryDryerControls",
    commands : {},
    attributes : {
      0 : "SupportedDrynessLevels",
      1 : "SelectedDrynessLevel",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  80 : {
    name : "ModeSelect",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
    },
    attributes : {
      0 : "Description",
      1 : "StandardNamespace",
      2 : "SupportedModes",
      3 : "CurrentMode",
      4 : "StartUpMode",
      5 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  81 : {
    name : "LaundryWasherMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      2 : "StartUpMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  82 : {
    name : "RefrigeratorAndTemperatureControlledCabinetMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      2 : "StartUpMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  83 : {
    name : "LaundryWasherControls",
    commands : {},
    attributes : {
      0 : "SpinSpeeds",
      1 : "SpinSpeedCurrent",
      2 : "NumberOfRinses",
      3 : "SupportedRinses",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  84 : {
    name : "RvcRunMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  85 : {
    name : "RvcCleanMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  86 : {
    name : "TemperatureControl",
    commands : {
      0 : {
        name : "SetTemperature",
        arguments : [
          "TargetTemperature",
          "TargetTemperatureLevel",
        ],
      },
    },
    attributes : {
      0 : "TemperatureSetpoint",
      1 : "MinTemperature",
      2 : "MaxTemperature",
      3 : "Step",
      4 : "SelectedTemperatureLevel",
      5 : "SupportedTemperatureLevels",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  87 : {
    name : "RefrigeratorAlarm",
    commands : {},
    attributes : {
      0 : "Mask",
      2 : "State",
      3 : "Supported",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  89 : {
    name : "DishwasherMode",
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
      1 : {
        name : "ChangeToModeResponse",
        arguments : [
          "Status",
          "StatusText",
        ],
      },
    },
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      2 : "StartUpMode",
      3 : "OnMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  91 : {
    name : "AirQuality",
    commands : {},
    attributes : {
      0 : "AirQuality",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  92 : {
    name : "SmokeCoAlarm",
    commands : {
      0 : {
        name : "SelfTestRequest",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "ExpressedState",
      1 : "SmokeState",
      2 : "COState",
      3 : "BatteryAlert",
      4 : "DeviceMuted",
      5 : "TestInProgress",
      6 : "HardwareFaultAlert",
      7 : "EndOfServiceAlert",
      8 : "InterconnectSmokeAlarm",
      9 : "InterconnectCOAlarm",
      10 : "ContaminationState",
      11 : "SmokeSensitivityLevel",
      12 : "ExpiryDate",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  93 : {
    name : "DishwasherAlarm",
    commands : {
      0 : {
        name : "Reset",
        arguments : [
          "Alarms",
        ],
      },
      1 : {
        name : "ModifyEnabledAlarms",
        arguments : [
          "Mask",
        ],
      },
    },
    attributes : {
      0 : "Mask",
      1 : "Latch",
      2 : "State",
      3 : "Supported",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  94 : {
    name : "MicrowaveOvenMode",
    commands : {},
    attributes : {
      0 : "SupportedModes",
      1 : "CurrentMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  95 : {
    name : "MicrowaveOvenControl",
    commands : {
      0 : {
        name : "SetCookingParameters",
        arguments : [
          "CookMode",
          "CookTime",
          "PowerSetting",
        ],
      },
      1 : {
        name : "AddMoreTime",
        arguments : [
          "TimeToAdd",
        ],
      },
    },
    attributes : {
      1 : "CookTime",
      2 : "PowerSetting",
      3 : "MinPower",
      4 : "MaxPower",
      5 : "PowerStep",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  96 : {
    name : "OperationalState",
    commands : {
      0 : {
        name : "Pause",
        arguments : [

        ],
      },
      1 : {
        name : "Stop",
        arguments : [

        ],
      },
      2 : {
        name : "Start",
        arguments : [

        ],
      },
      3 : {
        name : "Resume",
        arguments : [

        ],
      },
      4 : {
        name : "OperationalCommandResponse",
        arguments : [
          "CommandResponseState",
        ],
      },
    },
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
      2 : "CountdownTime",
      3 : "OperationalStateList",
      4 : "OperationalState",
      5 : "OperationalError",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  97 : {
    name : "RvcOperationalState",
    commands : {
      0 : {
        name : "Pause",
        arguments : [

        ],
      },
      1 : {
        name : "Stop",
        arguments : [

        ],
      },
      2 : {
        name : "Start",
        arguments : [

        ],
      },
      3 : {
        name : "Resume",
        arguments : [

        ],
      },
      4 : {
        name : "OperationalCommandResponse",
        arguments : [
          "CommandResponseState",
        ],
      },
    },
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
      2 : "CountdownTime",
      3 : "OperationalStateList",
      4 : "OperationalState",
      5 : "OperationalError",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  113 : {
    name : "HepaFilterMonitoring",
    commands : {
      0 : {
        name : "ResetCondition",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "Condition",
      1 : "DegradationDirection",
      2 : "ChangeIndication",
      3 : "InPlaceIndicator",
      4 : "LastChangedTime",
      5 : "ReplacementProductList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  114 : {
    name : "ActivatedCarbonFilterMonitoring",
    commands : {
      0 : {
        name : "ResetCondition",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "Condition",
      1 : "DegradationDirection",
      2 : "ChangeIndication",
      3 : "InPlaceIndicator",
      4 : "LastChangedTime",
      5 : "ReplacementProductList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  128 : {
    name : "BooleanStateConfiguration",
    commands : {
      0 : {
        name : "SuppressAlarm",
        arguments : [
          "AlarmsToSuppress",
        ],
      },
      1 : {
        name : "EnableDisableAlarm",
        arguments : [
          "AlarmsToEnableDisable",
        ],
      },
    },
    attributes : {
      0 : "CurrentSensitivityLevel",
      1 : "SupportedSensitivityLevels",
      2 : "DefaultSensitivityLevel",
      3 : "AlarmsActive",
      4 : "AlarmsSuppressed",
      5 : "AlarmsEnabled",
      6 : "AlarmsSupported",
      7 : "SensorFault",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  129 : {
    name : "ValveConfigurationAndControl",
    commands : {
      0 : {
        name : "Open",
        arguments : [
          "OpenDuration",
          "TargetLevel",
        ],
      },
      1 : {
        name : "Close",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "OpenDuration",
      1 : "DefaultOpenDuration",
      2 : "AutoCloseTime",
      3 : "RemainingDuration",
      4 : "CurrentState",
      5 : "TargetState",
      6 : "CurrentLevel",
      7 : "TargetLevel",
      8 : "DefaultOpenLevel",
      9 : "ValveFault",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  145 : {
    name : "ElectricalEnergyMeasurement",
    commands : {},
    attributes : {
      0 : "Accuracy",
      1 : "CumulativeEnergyImported",
      2 : "CumulativeEnergyExported",
      3 : "PeriodicEnergyImported",
      4 : "PeriodicEnergyExported",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  150 : {
    name : "DemandResponseLoadControl",
    commands : {
      0 : {
        name : "RegisterLoadControlProgramRequest",
        arguments : [
          "LoadControlProgram",
        ],
      },
      1 : {
        name : "UnregisterLoadControlProgramRequest",
        arguments : [
          "LoadControlProgramID",
        ],
      },
      2 : {
        name : "AddLoadControlEventRequest",
        arguments : [
          "Event",
        ],
      },
      3 : {
        name : "RemoveLoadControlEventRequest",
        arguments : [
          "EventID",
          "CancelControl",
        ],
      },
      4 : {
        name : "ClearLoadControlEventsRequest",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "LoadControlPrograms",
      1 : "NumberOfLoadControlPrograms",
      2 : "Events",
      3 : "ActiveEvents",
      4 : "NumberOfEventsPerProgram",
      5 : "NumberOfTransitions",
      6 : "DefaultRandomStart",
      7 : "DefaultRandomDuration",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  152 : {
    name : "DeviceEnergyManagement",
    commands : {
      0 : {
        name : "PowerAdjustRequest",
        arguments : [
          "Power",
          "Duration",
        ],
      },
      1 : {
        name : "CancelPowerAdjustRequest",
        arguments : [

        ],
      },
      2 : {
        name : "StartTimeAdjustRequest",
        arguments : [
          "RequestedStartTime",
        ],
      },
      3 : {
        name : "PauseRequest",
        arguments : [
          "Duration",
        ],
      },
      4 : {
        name : "ResumeRequest",
        arguments : [

        ],
      },
      5 : {
        name : "ModifyForecastRequest",
        arguments : [
          "ForecastId",
          "SlotAdjustments",
        ],
      },
      6 : {
        name : "RequestConstraintBasedForecast",
        arguments : [
          "Constraints",
        ],
      },
    },
    attributes : {
      0 : "ESAType",
      1 : "ESACanGenerate",
      2 : "ESAState",
      3 : "AbsMinPower",
      4 : "AbsMaxPower",
      5 : "PowerAdjustmentCapability",
      6 : "Forecast",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  153 : {
    name : "EnergyEvse",
    commands : {
      0 : {
        name : "GetTargetsResponse",
        arguments : [
          "DayOfWeekforSequence",
          "ChargingTargets",
        ],
      },
      1 : {
        name : "Disable",
        arguments : [

        ],
      },
      2 : {
        name : "EnableCharging",
        arguments : [
          "ChargingEnabledUntil",
          "MinimumChargeCurrent",
          "MaximumChargeCurrent",
        ],
      },
      3 : {
        name : "EnableDischarging",
        arguments : [
          "DischargingEnabledUntil",
          "MaximumDischargeCurrent",
        ],
      },
      4 : {
        name : "StartDiagnostics",
        arguments : [

        ],
      },
      5 : {
        name : "SetTargets",
        arguments : [
          "DayOfWeekforSequence",
          "ChargingTargets",
        ],
      },
      6 : {
        name : "GetTargets",
        arguments : [
          "DaysToReturn",
        ],
      },
      7 : {
        name : "ClearTargets",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "State",
      1 : "SupplyState",
      2 : "FaultState",
      3 : "ChargingEnabledUntil",
      4 : "DischargingEnabledUntil",
      5 : "CircuitCapacity",
      6 : "MinimumChargeCurrent",
      7 : "MaximumChargeCurrent",
      8 : "MaximumDischargeCurrent",
      9 : "UserMaximumChargeCurrent",
      10 : "RandomizationDelayWindow",
      33 : "NumberOfWeeklyTargets",
      34 : "NumberOfDailyTargets",
      35 : "NextChargeStartTime",
      36 : "NextChargeTargetTime",
      37 : "NextChargeRequiredEnergy",
      38 : "NextChargeTargetSoC",
      39 : "ApproximateEVEfficiency",
      48 : "StateOfCharge",
      49 : "BatteryCapacity",
      50 : "VehicleID",
      64 : "SessionID",
      65 : "SessionDuration",
      66 : "SessionEnergyCharged",
      67 : "SessionEnergyDischarged",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  155 : {
    name : "EnergyPreference",
    commands : {},
    attributes : {
      0 : "EnergyBalances",
      1 : "CurrentEnergyBalance",
      2 : "EnergyPriorities",
      3 : "LowPowerModeSensitivities",
      4 : "CurrentLowPowerModeSensitivity",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  257 : {
    name : "DoorLock",
    commands : {
      0 : {
        name : "LockDoor",
        arguments : [
          "PINCode",
        ],
      },
      1 : {
        name : "UnlockDoor",
        arguments : [
          "PINCode",
        ],
      },
      3 : {
        name : "UnlockWithTimeout",
        arguments : [
          "Timeout",
          "PINCode",
        ],
      },
      11 : {
        name : "SetWeekDaySchedule",
        arguments : [
          "WeekDayIndex",
          "UserIndex",
          "DaysMask",
          "StartHour",
          "StartMinute",
          "EndHour",
          "EndMinute",
        ],
      },
      12 : {
        name : "GetWeekDaySchedule",
        arguments : [
          "WeekDayIndex",
          "UserIndex",
        ],
      },
      12 : {
        name : "GetWeekDayScheduleResponse",
        arguments : [
          "WeekDayIndex",
          "UserIndex",
          "Status",
          "DaysMask",
          "StartHour",
          "StartMinute",
          "EndHour",
          "EndMinute",
        ],
      },
      13 : {
        name : "ClearWeekDaySchedule",
        arguments : [
          "WeekDayIndex",
          "UserIndex",
        ],
      },
      14 : {
        name : "SetYearDaySchedule",
        arguments : [
          "YearDayIndex",
          "UserIndex",
          "LocalStartTime",
          "LocalEndTime",
        ],
      },
      15 : {
        name : "GetYearDaySchedule",
        arguments : [
          "YearDayIndex",
          "UserIndex",
        ],
      },
      15 : {
        name : "GetYearDayScheduleResponse",
        arguments : [
          "YearDayIndex",
          "UserIndex",
          "Status",
          "LocalStartTime",
          "LocalEndTime",
        ],
      },
      16 : {
        name : "ClearYearDaySchedule",
        arguments : [
          "YearDayIndex",
          "UserIndex",
        ],
      },
      17 : {
        name : "SetHolidaySchedule",
        arguments : [
          "HolidayIndex",
          "LocalStartTime",
          "LocalEndTime",
          "OperatingMode",
        ],
      },
      18 : {
        name : "GetHolidaySchedule",
        arguments : [
          "HolidayIndex",
        ],
      },
      18 : {
        name : "GetHolidayScheduleResponse",
        arguments : [
          "HolidayIndex",
          "Status",
          "LocalStartTime",
          "LocalEndTime",
          "OperatingMode",
        ],
      },
      19 : {
        name : "ClearHolidaySchedule",
        arguments : [
          "HolidayIndex",
        ],
      },
      26 : {
        name : "SetUser",
        arguments : [
          "OperationType",
          "UserIndex",
          "UserName",
          "UserUniqueID",
          "UserStatus",
          "UserType",
          "CredentialRule",
        ],
      },
      27 : {
        name : "GetUser",
        arguments : [
          "UserIndex",
        ],
      },
      28 : {
        name : "GetUserResponse",
        arguments : [
          "UserIndex",
          "UserName",
          "UserUniqueID",
          "UserStatus",
          "UserType",
          "CredentialRule",
          "Credentials",
          "CreatorFabricIndex",
          "LastModifiedFabricIndex",
          "NextUserIndex",
        ],
      },
      29 : {
        name : "ClearUser",
        arguments : [
          "UserIndex",
        ],
      },
      34 : {
        name : "SetCredential",
        arguments : [
          "OperationType",
          "Credential",
          "CredentialData",
          "UserIndex",
          "UserStatus",
          "UserType",
        ],
      },
      35 : {
        name : "SetCredentialResponse",
        arguments : [
          "Status",
          "UserIndex",
          "NextCredentialIndex",
        ],
      },
      36 : {
        name : "GetCredentialStatus",
        arguments : [
          "Credential",
        ],
      },
      37 : {
        name : "GetCredentialStatusResponse",
        arguments : [
          "CredentialExists",
          "UserIndex",
          "CreatorFabricIndex",
          "LastModifiedFabricIndex",
          "NextCredentialIndex",
        ],
      },
      38 : {
        name : "ClearCredential",
        arguments : [
          "Credential",
        ],
      },
      39 : {
        name : "UnboltDoor",
        arguments : [
          "PINCode",
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
      27 : "CredentialRulesSupport",
      28 : "NumberOfCredentialsSupportedPerUser",
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
      44 : "LocalProgrammingFeatures",
      48 : "WrongCodeEntryLimit",
      49 : "UserCodeTemporaryDisableTime",
      50 : "SendPINOverTheAir",
      51 : "RequirePINforRemoteOperation",
      53 : "ExpiringUserTimeout",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  258 : {
    name : "WindowCovering",
    commands : {
      0 : {
        name : "UpOrOpen",
        arguments : [

        ],
      },
      1 : {
        name : "DownOrClose",
        arguments : [

        ],
      },
      2 : {
        name : "StopMotion",
        arguments : [

        ],
      },
      4 : {
        name : "GoToLiftValue",
        arguments : [
          "LiftValue",
        ],
      },
      5 : {
        name : "GoToLiftPercentage",
        arguments : [
          "LiftPercent100thsValue",
        ],
      },
      7 : {
        name : "GoToTiltValue",
        arguments : [
          "TiltValue",
        ],
      },
      8 : {
        name : "GoToTiltPercentage",
        arguments : [
          "TiltPercent100thsValue",
        ],
      },
    },
    attributes : {
      0 : "Type",
      1 : "PhysicalClosedLimitLift",
      2 : "PhysicalClosedLimitTilt",
      3 : "CurrentPositionLift",
      4 : "CurrentPositionTilt",
      5 : "NumberOfActuationsLift",
      6 : "NumberOfActuationsTilt",
      7 : "ConfigStatus",
      8 : "CurrentPositionLiftPercentage",
      9 : "CurrentPositionTiltPercentage",
      10 : "OperationalStatus",
      11 : "TargetPositionLiftPercent100ths",
      12 : "TargetPositionTiltPercent100ths",
      13 : "EndProductType",
      14 : "CurrentPositionLiftPercent100ths",
      15 : "CurrentPositionTiltPercent100ths",
      16 : "InstalledOpenLimitLift",
      17 : "InstalledClosedLimitLift",
      18 : "InstalledOpenLimitTilt",
      19 : "InstalledClosedLimitTilt",
      23 : "Mode",
      26 : "SafetyStatus",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  259 : {
    name : "BarrierControl",
    commands : {
      0 : {
        name : "BarrierControlGoToPercent",
        arguments : [
          "PercentOpen",
        ],
      },
      1 : {
        name : "BarrierControlStop",
        arguments : [

        ],
      },
    },
    attributes : {
      1 : "BarrierMovingState",
      2 : "BarrierSafetyStatus",
      3 : "BarrierCapabilities",
      4 : "BarrierOpenEvents",
      5 : "BarrierCloseEvents",
      6 : "BarrierCommandOpenEvents",
      7 : "BarrierCommandCloseEvents",
      8 : "BarrierOpenPeriod",
      9 : "BarrierClosePeriod",
      10 : "BarrierPosition",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  512 : {
    name : "PumpConfigurationAndControl",
    commands : {},
    attributes : {
      0 : "MaxPressure",
      1 : "MaxSpeed",
      2 : "MaxFlow",
      3 : "MinConstPressure",
      4 : "MaxConstPressure",
      5 : "MinCompPressure",
      6 : "MaxCompPressure",
      7 : "MinConstSpeed",
      8 : "MaxConstSpeed",
      9 : "MinConstFlow",
      10 : "MaxConstFlow",
      11 : "MinConstTemp",
      12 : "MaxConstTemp",
      16 : "PumpStatus",
      17 : "EffectiveOperationMode",
      18 : "EffectiveControlMode",
      19 : "Capacity",
      20 : "Speed",
      21 : "LifetimeRunningHours",
      22 : "Power",
      23 : "LifetimeEnergyConsumed",
      32 : "OperationMode",
      33 : "ControlMode",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  513 : {
    name : "Thermostat",
    commands : {
      0 : {
        name : "SetpointRaiseLower",
        arguments : [
          "Mode",
          "Amount",
        ],
      },
      0 : {
        name : "GetWeeklyScheduleResponse",
        arguments : [
          "NumberOfTransitionsForSequence",
          "DayOfWeekForSequence",
          "ModeForSequence",
          "Transitions",
        ],
      },
      1 : {
        name : "SetWeeklySchedule",
        arguments : [
          "NumberOfTransitionsForSequence",
          "DayOfWeekForSequence",
          "ModeForSequence",
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
      71 : "ACCapacityformat",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  514 : {
    name : "FanControl",
    commands : {
      0 : {
        name : "Step",
        arguments : [
          "Direction",
          "Wrap",
          "LowestOff",
        ],
      },
    },
    attributes : {
      0 : "FanMode",
      1 : "FanModeSequence",
      2 : "PercentSetting",
      3 : "PercentCurrent",
      4 : "SpeedMax",
      5 : "SpeedSetting",
      6 : "SpeedCurrent",
      7 : "RockSupport",
      8 : "RockSetting",
      9 : "WindSupport",
      10 : "WindSetting",
      11 : "AirflowDirection",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  516 : {
    name : "ThermostatUserInterfaceConfiguration",
    commands : {},
    attributes : {
      0 : "TemperatureDisplayMode",
      1 : "KeypadLockout",
      2 : "ScheduleProgrammingVisibility",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  769 : {
    name : "BallastConfiguration",
    commands : {},
    attributes : {
      0 : "PhysicalMinLevel",
      1 : "PhysicalMaxLevel",
      2 : "BallastStatus",
      16 : "MinLevel",
      17 : "MaxLevel",
      20 : "IntrinsicBallastFactor",
      21 : "BallastFactorAdjustment",
      32 : "LampQuantity",
      48 : "LampType",
      49 : "LampManufacturer",
      50 : "LampRatedHours",
      51 : "LampBurnHours",
      52 : "LampAlarmMode",
      53 : "LampBurnHoursTripPoint",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1028 : {
    name : "FlowMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1029 : {
    name : "RelativeHumidityMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "Tolerance",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1036 : {
    name : "CarbonMonoxideConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1037 : {
    name : "CarbonDioxideConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1043 : {
    name : "NitrogenDioxideConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1045 : {
    name : "OzoneConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1066 : {
    name : "Pm25ConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1067 : {
    name : "FormaldehydeConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1068 : {
    name : "Pm1ConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1069 : {
    name : "Pm10ConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1070 : {
    name : "TotalVolatileOrganicCompoundsConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1071 : {
    name : "RadonConcentrationMeasurement",
    commands : {},
    attributes : {
      0 : "MeasuredValue",
      1 : "MinMeasuredValue",
      2 : "MaxMeasuredValue",
      3 : "PeakMeasuredValue",
      4 : "PeakMeasuredValueWindow",
      5 : "AverageMeasuredValue",
      6 : "AverageMeasuredValueWindow",
      7 : "Uncertainty",
      8 : "MeasurementUnit",
      9 : "MeasurementMedium",
      10 : "LevelValue",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1283 : {
    name : "WakeOnLan",
    commands : {},
    attributes : {
      0 : "MACAddress",
      1 : "LinkLocalAddress",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1284 : {
    name : "Channel",
    commands : {
      0 : {
        name : "ChangeChannel",
        arguments : [
          "Match",
        ],
      },
      1 : {
        name : "ChangeChannelResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
      2 : {
        name : "ChangeChannelByNumber",
        arguments : [
          "MajorNumber",
          "MinorNumber",
        ],
      },
      3 : {
        name : "SkipChannel",
        arguments : [
          "Count",
        ],
      },
      4 : {
        name : "GetProgramGuide",
        arguments : [
          "StartTime",
          "EndTime",
          "ChannelList",
          "PageToken",
          "RecordingFlag",
          "ExternalIDList",
          "Data",
        ],
      },
      5 : {
        name : "ProgramGuideResponse",
        arguments : [
          "Paging",
          "ProgramList",
        ],
      },
      6 : {
        name : "RecordProgram",
        arguments : [
          "ProgramIdentifier",
          "ShouldRecordSeries",
          "ExternalIDList",
          "Data",
        ],
      },
      7 : {
        name : "CancelRecordProgram",
        arguments : [
          "ProgramIdentifier",
          "ShouldRecordSeries",
          "ExternalIDList",
          "Data",
        ],
      },
    },
    attributes : {
      0 : "ChannelList",
      1 : "Lineup",
      2 : "CurrentChannel",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1285 : {
    name : "TargetNavigator",
    commands : {
      0 : {
        name : "NavigateTarget",
        arguments : [
          "Target",
          "Data",
        ],
      },
      1 : {
        name : "NavigateTargetResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
    },
    attributes : {
      0 : "TargetList",
      1 : "CurrentTarget",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1286 : {
    name : "MediaPlayback",
    commands : {
      0 : {
        name : "Play",
        arguments : [

        ],
      },
      1 : {
        name : "Pause",
        arguments : [

        ],
      },
      2 : {
        name : "Stop",
        arguments : [

        ],
      },
      3 : {
        name : "StartOver",
        arguments : [

        ],
      },
      4 : {
        name : "Previous",
        arguments : [

        ],
      },
      5 : {
        name : "Next",
        arguments : [

        ],
      },
      6 : {
        name : "Rewind",
        arguments : [
          "AudioAdvanceUnmuted",
        ],
      },
      7 : {
        name : "FastForward",
        arguments : [
          "AudioAdvanceUnmuted",
        ],
      },
      8 : {
        name : "SkipForward",
        arguments : [
          "DeltaPositionMilliseconds",
        ],
      },
      9 : {
        name : "SkipBackward",
        arguments : [
          "DeltaPositionMilliseconds",
        ],
      },
      10 : {
        name : "PlaybackResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
      11 : {
        name : "Seek",
        arguments : [
          "Position",
        ],
      },
      12 : {
        name : "ActivateAudioTrack",
        arguments : [
          "TrackID",
          "AudioOutputIndex",
        ],
      },
      13 : {
        name : "ActivateTextTrack",
        arguments : [
          "TrackID",
        ],
      },
      14 : {
        name : "DeactivateTextTrack",
        arguments : [

        ],
      },
    },
    attributes : {
      0 : "CurrentState",
      1 : "StartTime",
      2 : "Duration",
      3 : "SampledPosition",
      4 : "PlaybackSpeed",
      5 : "SeekRangeEnd",
      6 : "SeekRangeStart",
      7 : "ActiveAudioTrack",
      8 : "AvailableAudioTracks",
      9 : "ActiveTextTrack",
      10 : "AvailableTextTracks",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1287 : {
    name : "MediaInput",
    commands : {
      0 : {
        name : "SelectInput",
        arguments : [
          "Index",
        ],
      },
      1 : {
        name : "ShowInputStatus",
        arguments : [

        ],
      },
      2 : {
        name : "HideInputStatus",
        arguments : [

        ],
      },
      3 : {
        name : "RenameInput",
        arguments : [
          "Index",
          "Name",
        ],
      },
    },
    attributes : {
      0 : "InputList",
      1 : "CurrentInput",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1288 : {
    name : "LowPower",
    commands : {
      0 : {
        name : "Sleep",
        arguments : [

        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1289 : {
    name : "KeypadInput",
    commands : {
      0 : {
        name : "SendKey",
        arguments : [
          "KeyCode",
        ],
      },
      1 : {
        name : "SendKeyResponse",
        arguments : [
          "Status",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1290 : {
    name : "ContentLauncher",
    commands : {
      0 : {
        name : "LaunchContent",
        arguments : [
          "Search",
          "AutoPlay",
          "Data",
          "PlaybackPreferences",
          "UseCurrentContext",
        ],
      },
      1 : {
        name : "LaunchURL",
        arguments : [
          "ContentURL",
          "DisplayString",
          "BrandingInformation",
        ],
      },
      2 : {
        name : "LauncherResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
    },
    attributes : {
      0 : "AcceptHeader",
      1 : "SupportedStreamingProtocols",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1291 : {
    name : "AudioOutput",
    commands : {
      0 : {
        name : "SelectOutput",
        arguments : [
          "Index",
        ],
      },
      1 : {
        name : "RenameOutput",
        arguments : [
          "Index",
          "Name",
        ],
      },
    },
    attributes : {
      0 : "OutputList",
      1 : "CurrentOutput",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1292 : {
    name : "ApplicationLauncher",
    commands : {
      0 : {
        name : "LaunchApp",
        arguments : [
          "Application",
          "Data",
        ],
      },
      1 : {
        name : "StopApp",
        arguments : [
          "Application",
        ],
      },
      2 : {
        name : "HideApp",
        arguments : [
          "Application",
        ],
      },
      3 : {
        name : "LauncherResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
    },
    attributes : {
      0 : "CatalogList",
      1 : "CurrentApp",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1293 : {
    name : "ApplicationBasic",
    commands : {},
    attributes : {
      0 : "VendorName",
      1 : "VendorID",
      2 : "ApplicationName",
      3 : "ProductID",
      4 : "Application",
      5 : "Status",
      6 : "ApplicationVersion",
      7 : "AllowedVendorList",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1294 : {
    name : "AccountLogin",
    commands : {
      0 : {
        name : "GetSetupPIN",
        arguments : [
          "TempAccountIdentifier",
        ],
      },
      1 : {
        name : "GetSetupPINResponse",
        arguments : [
          "SetupPIN",
        ],
      },
      2 : {
        name : "Login",
        arguments : [
          "TempAccountIdentifier",
          "SetupPIN",
          "Node",
        ],
      },
      3 : {
        name : "Logout",
        arguments : [
          "Node",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1295 : {
    name : "ContentControl",
    commands : {
      0 : {
        name : "UpdatePIN",
        arguments : [
          "OldPIN",
          "NewPIN",
        ],
      },
      1 : {
        name : "ResetPIN",
        arguments : [

        ],
      },
      2 : {
        name : "ResetPINResponse",
        arguments : [
          "PINCode",
        ],
      },
      3 : {
        name : "Enable",
        arguments : [

        ],
      },
      4 : {
        name : "Disable",
        arguments : [

        ],
      },
      5 : {
        name : "AddBonusTime",
        arguments : [
          "PINCode",
          "BonusTime",
        ],
      },
      6 : {
        name : "SetScreenDailyTime",
        arguments : [
          "ScreenTime",
        ],
      },
      7 : {
        name : "BlockUnratedContent",
        arguments : [

        ],
      },
      8 : {
        name : "UnblockUnratedContent",
        arguments : [

        ],
      },
      9 : {
        name : "SetOnDemandRatingThreshold",
        arguments : [
          "Rating",
        ],
      },
      10 : {
        name : "SetScheduledContentRatingThreshold",
        arguments : [
          "Rating",
        ],
      },
    },
    attributes : {
      0 : "Enabled",
      1 : "OnDemandRatings",
      2 : "OnDemandRatingThreshold",
      3 : "ScheduledContentRatings",
      4 : "ScheduledContentRatingThreshold",
      5 : "ScreenDailyTime",
      6 : "RemainingScreenTime",
      7 : "BlockUnrated",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  1296 : {
    name : "ContentAppObserver",
    commands : {
      0 : {
        name : "ContentAppMessage",
        arguments : [
          "Data",
          "EncodingHint",
        ],
      },
      1 : {
        name : "ContentAppMessageResponse",
        arguments : [
          "Status",
          "Data",
          "EncodingHint",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  2820 : {
    name : "ElectricalMeasurement",
    commands : {
      0 : {
        name : "GetProfileInfoResponseCommand",
        arguments : [
          "ProfileCount",
          "ProfileIntervalPeriod",
          "MaxNumberOfIntervals",
          "ListOfAttributes",
        ],
      },
      0 : {
        name : "GetProfileInfoCommand",
        arguments : [

        ],
      },
      1 : {
        name : "GetMeasurementProfileResponseCommand",
        arguments : [
          "StartTime",
          "Status",
          "ProfileIntervalPeriod",
          "NumberOfIntervalsDelivered",
          "AttributeId",
          "Intervals",
        ],
      },
      1 : {
        name : "GetMeasurementProfileCommand",
        arguments : [
          "AttributeId",
          "StartTime",
          "NumberOfIntervals",
        ],
      },
    },
    attributes : {
      0 : "MeasurementType",
      256 : "DcVoltage",
      257 : "DcVoltageMin",
      258 : "DcVoltageMax",
      259 : "DcCurrent",
      260 : "DcCurrentMin",
      261 : "DcCurrentMax",
      262 : "DcPower",
      263 : "DcPowerMin",
      264 : "DcPowerMax",
      512 : "DcVoltageMultiplier",
      513 : "DcVoltageDivisor",
      514 : "DcCurrentMultiplier",
      515 : "DcCurrentDivisor",
      516 : "DcPowerMultiplier",
      517 : "DcPowerDivisor",
      768 : "AcFrequency",
      769 : "AcFrequencyMin",
      770 : "AcFrequencyMax",
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
      1024 : "AcFrequencyMultiplier",
      1025 : "AcFrequencyDivisor",
      1026 : "PowerMultiplier",
      1027 : "PowerDivisor",
      1028 : "HarmonicCurrentMultiplier",
      1029 : "PhaseHarmonicCurrentMultiplier",
      1280 : "InstantaneousVoltage",
      1281 : "InstantaneousLineCurrent",
      1282 : "InstantaneousActiveCurrent",
      1283 : "InstantaneousReactiveCurrent",
      1284 : "InstantaneousPower",
      1285 : "RmsVoltage",
      1286 : "RmsVoltageMin",
      1287 : "RmsVoltageMax",
      1288 : "RmsCurrent",
      1289 : "RmsCurrentMin",
      1290 : "RmsCurrentMax",
      1291 : "ActivePower",
      1292 : "ActivePowerMin",
      1293 : "ActivePowerMax",
      1294 : "ReactivePower",
      1295 : "ApparentPower",
      1296 : "PowerFactor",
      1297 : "AverageRmsVoltageMeasurementPeriod",
      1299 : "AverageRmsUnderVoltageCounter",
      1300 : "RmsExtremeOverVoltagePeriod",
      1301 : "RmsExtremeUnderVoltagePeriod",
      1302 : "RmsVoltageSagPeriod",
      1303 : "RmsVoltageSwellPeriod",
      1536 : "AcVoltageMultiplier",
      1537 : "AcVoltageDivisor",
      1538 : "AcCurrentMultiplier",
      1539 : "AcCurrentDivisor",
      1540 : "AcPowerMultiplier",
      1541 : "AcPowerDivisor",
      1792 : "OverloadAlarmsMask",
      1793 : "VoltageOverload",
      1794 : "CurrentOverload",
      2048 : "AcOverloadAlarmsMask",
      2049 : "AcVoltageOverload",
      2050 : "AcCurrentOverload",
      2051 : "AcActivePowerOverload",
      2052 : "AcReactivePowerOverload",
      2053 : "AverageRmsOverVoltage",
      2054 : "AverageRmsUnderVoltage",
      2055 : "RmsExtremeOverVoltage",
      2056 : "RmsExtremeUnderVoltage",
      2057 : "RmsVoltageSag",
      2058 : "RmsVoltageSwell",
      2305 : "LineCurrentPhaseB",
      2306 : "ActiveCurrentPhaseB",
      2307 : "ReactiveCurrentPhaseB",
      2309 : "RmsVoltagePhaseB",
      2310 : "RmsVoltageMinPhaseB",
      2311 : "RmsVoltageMaxPhaseB",
      2312 : "RmsCurrentPhaseB",
      2313 : "RmsCurrentMinPhaseB",
      2314 : "RmsCurrentMaxPhaseB",
      2315 : "ActivePowerPhaseB",
      2316 : "ActivePowerMinPhaseB",
      2317 : "ActivePowerMaxPhaseB",
      2318 : "ReactivePowerPhaseB",
      2319 : "ApparentPowerPhaseB",
      2320 : "PowerFactorPhaseB",
      2321 : "AverageRmsVoltageMeasurementPeriodPhaseB",
      2322 : "AverageRmsOverVoltageCounterPhaseB",
      2323 : "AverageRmsUnderVoltageCounterPhaseB",
      2324 : "RmsExtremeOverVoltagePeriodPhaseB",
      2325 : "RmsExtremeUnderVoltagePeriodPhaseB",
      2326 : "RmsVoltageSagPeriodPhaseB",
      2327 : "RmsVoltageSwellPeriodPhaseB",
      2561 : "LineCurrentPhaseC",
      2562 : "ActiveCurrentPhaseC",
      2563 : "ReactiveCurrentPhaseC",
      2565 : "RmsVoltagePhaseC",
      2566 : "RmsVoltageMinPhaseC",
      2567 : "RmsVoltageMaxPhaseC",
      2568 : "RmsCurrentPhaseC",
      2569 : "RmsCurrentMinPhaseC",
      2570 : "RmsCurrentMaxPhaseC",
      2571 : "ActivePowerPhaseC",
      2572 : "ActivePowerMinPhaseC",
      2573 : "ActivePowerMaxPhaseC",
      2574 : "ReactivePowerPhaseC",
      2575 : "ApparentPowerPhaseC",
      2576 : "PowerFactorPhaseC",
      2577 : "AverageRmsVoltageMeasurementPeriodPhaseC",
      2578 : "AverageRmsOverVoltageCounterPhaseC",
      2579 : "AverageRmsUnderVoltageCounterPhaseC",
      2580 : "RmsExtremeOverVoltagePeriodPhaseC",
      2581 : "RmsExtremeUnderVoltagePeriodPhaseC",
      2582 : "RmsVoltageSagPeriodPhaseC",
      2583 : "RmsVoltageSwellPeriodPhaseC",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  4294048773 : {
    name : "UnitTesting",
    commands : {
      0 : {
        name : "Test",
        arguments : [

        ],
      },
      0 : {
        name : "TestSpecificResponse",
        arguments : [
          "ReturnValue",
        ],
      },
      1 : {
        name : "TestNotHandled",
        arguments : [

        ],
      },
      1 : {
        name : "TestAddArgumentsResponse",
        arguments : [
          "ReturnValue",
        ],
      },
      2 : {
        name : "TestSpecific",
        arguments : [

        ],
      },
      2 : {
        name : "TestSimpleArgumentResponse",
        arguments : [
          "ReturnValue",
        ],
      },
      3 : {
        name : "TestUnknownCommand",
        arguments : [

        ],
      },
      3 : {
        name : "TestStructArrayArgumentResponse",
        arguments : [
          "Arg1",
          "Arg2",
          "Arg3",
          "Arg4",
          "Arg5",
          "Arg6",
        ],
      },
      4 : {
        name : "TestAddArguments",
        arguments : [
          "Arg1",
          "Arg2",
        ],
      },
      4 : {
        name : "TestListInt8UReverseResponse",
        arguments : [
          "Arg1",
        ],
      },
      5 : {
        name : "TestSimpleArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      5 : {
        name : "TestEnumsResponse",
        arguments : [
          "Arg1",
          "Arg2",
        ],
      },
      6 : {
        name : "TestStructArrayArgumentRequest",
        arguments : [
          "Arg1",
          "Arg2",
          "Arg3",
          "Arg4",
          "Arg5",
          "Arg6",
        ],
      },
      6 : {
        name : "TestNullableOptionalResponse",
        arguments : [
          "WasPresent",
          "WasNull",
          "Value",
          "OriginalValue",
        ],
      },
      7 : {
        name : "TestStructArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      7 : {
        name : "TestComplexNullableOptionalResponse",
        arguments : [
          "NullableIntWasNull",
          "NullableIntValue",
          "OptionalIntWasPresent",
          "OptionalIntValue",
          "NullableOptionalIntWasPresent",
          "NullableOptionalIntWasNull",
          "NullableOptionalIntValue",
          "NullableStringWasNull",
          "NullableStringValue",
          "OptionalStringWasPresent",
          "OptionalStringValue",
          "NullableOptionalStringWasPresent",
          "NullableOptionalStringWasNull",
          "NullableOptionalStringValue",
          "NullableStructWasNull",
          "NullableStructValue",
          "OptionalStructWasPresent",
          "OptionalStructValue",
          "NullableOptionalStructWasPresent",
          "NullableOptionalStructWasNull",
          "NullableOptionalStructValue",
          "NullableListWasNull",
          "NullableListValue",
          "OptionalListWasPresent",
          "OptionalListValue",
          "NullableOptionalListWasPresent",
          "NullableOptionalListWasNull",
          "NullableOptionalListValue",
        ],
      },
      8 : {
        name : "TestNestedStructArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      8 : {
        name : "BooleanResponse",
        arguments : [
          "Value",
        ],
      },
      9 : {
        name : "TestListStructArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      9 : {
        name : "SimpleStructResponse",
        arguments : [
          "Arg1",
        ],
      },
      10 : {
        name : "TestListInt8UArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      10 : {
        name : "TestEmitTestEventResponse",
        arguments : [
          "Value",
        ],
      },
      11 : {
        name : "TestNestedStructListArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      11 : {
        name : "TestEmitTestFabricScopedEventResponse",
        arguments : [
          "Value",
        ],
      },
      12 : {
        name : "TestListNestedStructListArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      13 : {
        name : "TestListInt8UReverseRequest",
        arguments : [
          "Arg1",
        ],
      },
      14 : {
        name : "TestEnumsRequest",
        arguments : [
          "Arg1",
          "Arg2",
        ],
      },
      15 : {
        name : "TestNullableOptionalRequest",
        arguments : [
          "Arg1",
        ],
      },
      16 : {
        name : "TestComplexNullableOptionalRequest",
        arguments : [
          "NullableInt",
          "OptionalInt",
          "NullableOptionalInt",
          "NullableString",
          "OptionalString",
          "NullableOptionalString",
          "NullableStruct",
          "OptionalStruct",
          "NullableOptionalStruct",
          "NullableList",
          "OptionalList",
          "NullableOptionalList",
        ],
      },
      17 : {
        name : "SimpleStructEchoRequest",
        arguments : [
          "Arg1",
        ],
      },
      18 : {
        name : "TimedInvokeRequest",
        arguments : [

        ],
      },
      19 : {
        name : "TestSimpleOptionalArgumentRequest",
        arguments : [
          "Arg1",
        ],
      },
      20 : {
        name : "TestEmitTestEventRequest",
        arguments : [
          "Arg1",
          "Arg2",
          "Arg3",
        ],
      },
      21 : {
        name : "TestEmitTestFabricScopedEventRequest",
        arguments : [
          "Arg1",
        ],
      },
    },
    attributes : {
      0 : "Boolean",
      1 : "Bitmap8",
      2 : "Bitmap16",
      3 : "Bitmap32",
      4 : "Bitmap64",
      5 : "Int8u",
      6 : "Int16u",
      7 : "Int24u",
      8 : "Int32u",
      9 : "Int40u",
      10 : "Int48u",
      11 : "Int56u",
      12 : "Int64u",
      13 : "Int8s",
      14 : "Int16s",
      15 : "Int24s",
      16 : "Int32s",
      17 : "Int40s",
      18 : "Int48s",
      19 : "Int56s",
      20 : "Int64s",
      21 : "Enum8",
      22 : "Enum16",
      23 : "FloatSingle",
      24 : "FloatDouble",
      25 : "OctetString",
      26 : "ListInt8u",
      27 : "ListOctetString",
      28 : "ListStructOctetString",
      29 : "LongOctetString",
      30 : "CharString",
      31 : "LongCharString",
      32 : "EpochUs",
      33 : "EpochS",
      34 : "VendorId",
      35 : "ListNullablesAndOptionalsStruct",
      36 : "EnumAttr",
      37 : "StructAttr",
      38 : "RangeRestrictedInt8u",
      39 : "RangeRestrictedInt8s",
      40 : "RangeRestrictedInt16u",
      41 : "RangeRestrictedInt16s",
      42 : "ListLongOctetString",
      43 : "ListFabricScoped",
      48 : "TimedWriteBoolean",
      49 : "GeneralErrorBoolean",
      50 : "ClusterErrorBoolean",
      255 : "Unsupported",
      16384 : "NullableBoolean",
      16385 : "NullableBitmap8",
      16386 : "NullableBitmap16",
      16387 : "NullableBitmap32",
      16388 : "NullableBitmap64",
      16389 : "NullableInt8u",
      16390 : "NullableInt16u",
      16391 : "NullableInt24u",
      16392 : "NullableInt32u",
      16393 : "NullableInt40u",
      16394 : "NullableInt48u",
      16395 : "NullableInt56u",
      16396 : "NullableInt64u",
      16397 : "NullableInt8s",
      16398 : "NullableInt16s",
      16399 : "NullableInt24s",
      16400 : "NullableInt32s",
      16401 : "NullableInt40s",
      16402 : "NullableInt48s",
      16403 : "NullableInt56s",
      16404 : "NullableInt64s",
      16405 : "NullableEnum8",
      16406 : "NullableEnum16",
      16407 : "NullableFloatSingle",
      16408 : "NullableFloatDouble",
      16409 : "NullableOctetString",
      16414 : "NullableCharString",
      16420 : "NullableEnumAttr",
      16421 : "NullableStruct",
      16422 : "NullableRangeRestrictedInt8u",
      16423 : "NullableRangeRestrictedInt8s",
      16424 : "NullableRangeRestrictedInt16u",
      16425 : "NullableRangeRestrictedInt16s",
      16426 : "WriteOnlyInt8u",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  4294048774 : {
    name : "FaultInjection",
    commands : {
      0 : {
        name : "FailAtFault",
        arguments : [
          "Type",
          "Id",
          "NumCallsToSkip",
          "NumCallsToFail",
          "TakeMutex",
        ],
      },
      1 : {
        name : "FailRandomlyAtFault",
        arguments : [
          "Type",
          "Id",
          "Percentage",
        ],
      },
    },
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  4294048800 : {
    name : "SampleMei",
    commands : {
      0 : {
        name : "Ping",
        arguments : [

        ],
      },
      1 : {
        name : "AddArgumentsResponse",
        arguments : [
          "ReturnValue",
        ],
      },
      2 : {
        name : "AddArguments",
        arguments : [
          "Arg1",
          "Arg2",
        ],
      },
    },
    attributes : {
      0 : "FlipFlop",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },

  enums : {
    "ACCapacityFormatEnum" : [
      "BTUh",
    ], //
    "ACCompressorTypeEnum" : [
      "Unknown",
      "T1",
      "T2",
      "T3",
    ], //
    "ACLouverPositionEnum" : [
      "Closed",
      "Open",
      "Quarter",
      "Half",
      "ThreeQuarters",
    ], //
    "ACRefrigerantTypeEnum" : [
      "Unknown",
      "R22",
      "R410a",
      "R407c",
    ], //
    "ACTypeEnum" : [
      "Unknown",
      "CoolingFixed",
      "HeatPumpFixed",
      "CoolingInverter",
      "HeatPumpInverter",
    ], //
    "AccessControlEntryAuthModeEnum" : [
      "Pase",
      "Case",
      "Group",
    ], //
    "AccessControlEntryPrivilegeEnum" : [
      "View",
      "ProxyView",
      "Operate",
      "Manage",
      "Administer",
    ], //
    "ActionErrorEnum" : [
      "Unknown",
      "Interrupted",
    ], //
    "ActionStateEnum" : [
      "Inactive",
      "Active",
      "Paused",
      "Disabled",
    ], //
    "ActionTypeEnum" : [
      "Other",
      "Scene",
      "Sequence",
      "Automation",
      "Exception",
      "Notification",
      "Alarm",
    ], //
    "AirQualityEnum" : [
      "Unknown",
      "Good",
      "Fair",
      "Moderate",
      "Poor",
      "VeryPoor",
      "ExtremelyPoor",
    ], //
    "AirflowDirectionEnum" : [
      "Forward",
      "Reverse",
    ], //
    "AlarmCodeEnum" : [
      "LockJammed",
      "LockFactoryReset",
      "LockRadioPowerCycled",
      "WrongCodeEntryLimit",
      "FrontEsceutcheonRemoved",
      "DoorForcedOpen",
      "DoorAjar",
      "ForcedUser",
    ], //
    "AlarmStateEnum" : [
      "Normal",
      "Warning",
      "Critical",
    ], //
    "AnnouncementReasonEnum" : [
      "SimpleAnnouncement",
      "UpdateAvailable",
      "UrgentUpdateAvailable",
    ], //
    "ApplicationStatusEnum" : [
      "Stopped",
      "ActiveVisibleFocus",
      "ActiveHidden",
      "ActiveVisibleNotFocus",
    ], //
    "ApplyUpdateActionEnum" : [
      "Proceed",
      "AwaitNextAction",
      "Discontinue",
    ], //
    "AssociationFailureCauseEnum" : [
      "Unknown",
      "AssociationFailed",
      "AuthenticationFailed",
      "SsidNotFound",
    ], //
    "BatApprovedChemistryEnum" : [
      "Unspecified",
      "Alkaline",
      "LithiumCarbonFluoride",
      "LithiumChromiumOxide",
      "LithiumCopperOxide",
      "LithiumIronDisulfide",
      "LithiumManganeseDioxide",
      "LithiumThionylChloride",
      "Magnesium",
      "MercuryOxide",
      "NickelOxyhydride",
      "SilverOxide",
      "ZincAir",
      "ZincCarbon",
      "ZincChloride",
      "ZincManganeseDioxide",
      "LeadAcid",
      "LithiumCobaltOxide",
      "LithiumIon",
      "LithiumIonPolymer",
      "LithiumIronPhosphate",
      "LithiumSulfur",
      "LithiumTitanate",
      "NickelCadmium",
      "NickelHydrogen",
      "NickelIron",
      "NickelMetalHydride",
      "NickelZinc",
      "SilverZinc",
      "SodiumIon",
      "SodiumSulfur",
      "ZincBromide",
      "ZincCerium",
    ], //
    "BatChargeFaultEnum" : [
      "Unspecified",
      "AmbientTooHot",
      "AmbientTooCold",
      "BatteryTooHot",
      "BatteryTooCold",
      "BatteryAbsent",
      "BatteryOverVoltage",
      "BatteryUnderVoltage",
      "ChargerOverVoltage",
      "ChargerUnderVoltage",
      "SafetyTimeout",
    ], //
    "BatChargeLevelEnum" : [
      "Ok",
      "Warning",
      "Critical",
    ], //
    "BatChargeStateEnum" : [
      "Unknown",
      "IsCharging",
      "IsAtFullCharge",
      "IsNotCharging",
    ], //
    "BatCommonDesignationEnum" : [
      "Unspecified",
      "Aaa",
      "Aa",
      "C",
      "D",
      "4v5",
      "6v0",
      "9v0",
      "12aa",
      "Aaaa",
      "A",
      "B",
      "F",
      "N",
      "No6",
      "SubC",
      "A23",
      "A27",
      "Ba5800",
      "Duplex",
      "4sr44",
      "523",
      "531",
      "15v0",
      "22v5",
      "30v0",
      "45v0",
      "67v5",
      "J",
      "Cr123a",
      "Cr2",
      "2cr5",
      "CrP2",
      "CrV3",
      "Sr41",
      "Sr43",
      "Sr44",
      "Sr45",
      "Sr48",
      "Sr54",
      "Sr55",
      "Sr57",
      "Sr58",
      "Sr59",
      "Sr60",
      "Sr63",
      "Sr64",
      "Sr65",
      "Sr66",
      "Sr67",
      "Sr68",
      "Sr69",
      "Sr516",
      "Sr731",
      "Sr712",
      "Lr932",
      "A5",
      "A10",
      "A13",
      "A312",
      "A675",
      "Ac41e",
      "10180",
      "10280",
      "10440",
      "14250",
      "14430",
      "14500",
      "14650",
      "15270",
      "16340",
      "Rcr123a",
      "17500",
      "17670",
      "18350",
      "18500",
      "18650",
      "19670",
      "25500",
      "26650",
      "32600",
    ], //
    "BatFaultEnum" : [
      "Unspecified",
      "OverTemp",
      "UnderTemp",
    ], //
    "BatReplaceabilityEnum" : [
      "Unspecified",
      "NotReplaceable",
      "UserReplaceable",
      "FactoryReplaceable",
    ], //
    "BootReasonEnum" : [
      "Unspecified",
      "PowerOnReboot",
      "BrownOutReset",
      "SoftwareWatchdogReset",
      "HardwareWatchdogReset",
      "SoftwareUpdateCompleted",
      "SoftwareReset",
    ], //
    "CECKeyCodeEnum" : [
      "Select",
      "Up",
      "Down",
      "Left",
      "Right",
      "RightUp",
      "RightDown",
      "LeftUp",
      "LeftDown",
      "RootMenu",
      "SetupMenu",
      "ContentsMenu",
      "FavoriteMenu",
      "Exit",
      "MediaTopMenu",
      "MediaContextSensitiveMenu",
      "NumberEntryMode",
      "Number11",
      "Number12",
      "Number0OrNumber10",
      "Numbers1",
      "Numbers2",
      "Numbers3",
      "Numbers4",
      "Numbers5",
      "Numbers6",
      "Numbers7",
      "Numbers8",
      "Numbers9",
      "Dot",
      "Enter",
      "Clear",
      "NextFavorite",
      "ChannelUp",
      "ChannelDown",
      "PreviousChannel",
      "SoundSelect",
      "InputSelect",
      "DisplayInformation",
      "Help",
      "PageUp",
      "PageDown",
      "Power",
      "VolumeUp",
      "VolumeDown",
      "Mute",
      "Play",
      "Stop",
      "Pause",
      "Record",
      "Rewind",
      "FastForward",
      "Eject",
      "Forward",
      "Backward",
      "StopRecord",
      "PauseRecord",
      "Reserved",
      "Angle",
      "SubPicture",
      "VideoOnDemand",
      "ElectronicProgramGuide",
      "TimerProgramming",
      "InitialConfiguration",
      "SelectBroadcastType",
      "SelectSoundPresentation",
      "PlayFunction",
      "PausePlayFunction",
      "RecordFunction",
      "PauseRecordFunction",
      "StopFunction",
      "MuteFunction",
      "RestoreVolumeFunction",
      "TuneFunction",
      "SelectMediaFunction",
      "SelectAvInputFunction",
      "SelectAudioInputFunction",
      "PowerToggleFunction",
      "PowerOffFunction",
      "PowerOnFunction",
      "F1Blue",
      "F2Red",
      "F3Green",
      "F4Yellow",
      "F5",
      "Data",
    ], //
    "CalendarTypeEnum" : [
      "Buddhist",
      "Chinese",
      "Coptic",
      "Ethiopian",
      "Gregorian",
      "Hebrew",
      "Indian",
      "Islamic",
      "Japanese",
      "Korean",
      "Persian",
      "Taiwanese",
      "UseActiveLocale",
    ], //
    "CauseEnum" : [
      "NormalCompletion",
      "Offline",
      "Fault",
      "UserOptOut",
    ], //
    "CertificateChainTypeEnum" : [
      "DACCertificate",
      "PAICertificate",
    ], //
    "ChangeIndicationEnum" : [
      "Ok",
      "Warning",
      "Critical",
    ], //
    "ChangeReasonEnum" : [
      "Unknown",
      "Success",
      "Failure",
      "TimeOut",
      "DelayByProvider",
    ], //
    "ChangeTypeEnum" : [
      "Changed",
      "Added",
      "Removed",
    ], //
    "ChannelTypeEnum" : [
      "Satellite",
      "Cable",
      "Terrestrial",
    ], //
    "CharacteristicEnum" : [
      "ForcedSubtitles",
      "DescribesVideo",
      "EasyToRead",
      "FrameBased",
      "MainProgram",
      "OriginalContent",
      "VoiceOverTranslation",
      "Caption",
      "Subtitle",
      "Alternate",
      "Supplementary",
      "Commentary",
      "DubbedTranslation",
      "Description",
      "Metadata",
      "EnhancedAudioIntelligibility",
      "Emergency",
      "Karaoke",
    ], //
    "CharacteristicEnum" : [
      "ForcedSubtitles",
      "DescribesVideo",
      "EasyToRead",
      "FrameBased",
      "MainProgram",
      "OriginalContent",
      "VoiceOverTranslation",
      "Caption",
      "Subtitle",
      "Alternate",
      "Supplementary",
      "Commentary",
      "DubbedTranslation",
      "Description",
      "Metadata",
      "EnhancedAudioIntelligibility",
      "Emergency",
      "Karaoke",
    ], //
    "ColorEnum" : [
      "Black",
      "Navy",
      "Green",
      "Teal",
      "Maroon",
      "Purple",
      "Olive",
      "Gray",
      "Blue",
      "Lime",
      "Aqua",
      "Red",
      "Fuchsia",
      "Yellow",
      "White",
      "Nickel",
      "Chrome",
      "Brass",
      "Copper",
      "Silver",
      "Gold",
    ], //
    "ColorEnum" : [
      "Black",
      "Navy",
      "Green",
      "Teal",
      "Maroon",
      "Purple",
      "Olive",
      "Gray",
      "Blue",
      "Lime",
      "Aqua",
      "Red",
      "Fuchsia",
      "Yellow",
      "White",
      "Nickel",
      "Chrome",
      "Brass",
      "Copper",
      "Silver",
      "Gold",
    ], //
    "ColorLoopAction" : [
      "Deactivate",
      "ActivateFromColorLoopStartEnhancedHue",
      "ActivateFromEnhancedCurrentHue",
    ], //
    "ColorLoopDirection" : [
      "DecrementHue",
      "IncrementHue",
    ], //
    "ColorMode" : [
      "CurrentHueAndCurrentSaturation",
      "CurrentXAndCurrentY",
      "ColorTemperature",
    ], //
    "CommissioningErrorEnum" : [
      "Ok",
      "ValueOutsideRange",
      "InvalidAuthentication",
      "NoFailSafe",
      "BusyWithOtherAdmin",
    ], //
    "CommissioningWindowStatusEnum" : [
      "WindowNotOpen",
      "EnhancedWindowOpen",
      "BasicWindowOpen",
    ], //
    "ConnectionStatusEnum" : [
      "Connected",
      "NotConnected",
    ], //
    "ConnectionStatusEnum" : [
      "Connected",
      "NotConnected",
    ], //
    "ContaminationStateEnum" : [
      "Normal",
      "Low",
      "Warning",
      "Critical",
    ], //
    "ControlModeEnum" : [
      "ConstantSpeed",
      "ConstantPressure",
      "ProportionalPressure",
      "ConstantFlow",
      "ConstantTemperature",
      "Automatic",
    ], //
    "ControlSequenceOfOperationEnum" : [
      "CoolingOnly",
      "CoolingWithReheat",
      "HeatingOnly",
      "HeatingWithReheat",
      "CoolingAndHeating",
      "CoolingAndHeatingWithReheat",
    ], //
    "CostTypeEnum" : [
      "Financial",
      "GHGEmissions",
      "Comfort",
      "Temperature",
    ], //
    "CredentialRuleEnum" : [
      "Single",
      "Dual",
      "Tri",
    ], //
    "CredentialTypeEnum" : [
      "ProgrammingPIN",
      "Pin",
      "Rfid",
      "Fingerprint",
      "FingerVein",
      "Face",
    ], //
    "CriticalityLevelEnum" : [
      "Unknown",
      "Green",
      "Level1",
      "Level2",
      "Level3",
      "Level4",
      "Level5",
      "Emergency",
      "PlannedOutage",
      "ServiceDisconnect",
    ], //
    "DataOperationTypeEnum" : [
      "Add",
      "Clear",
      "Modify",
    ], //
    "DegradationDirectionEnum" : [
      "Up",
      "Down",
    ], //
    "DelayedAllOffEffectVariantEnum" : [
      "DelayedOffFastFade",
      "NoFade",
      "DelayedOffSlowFade",
    ], //
    "DlLockState" : [
      "NotFullyLocked",
      "Locked",
      "Unlocked",
      "Unlatched",
    ], //
    "DlLockType" : [
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
      "Eurocylinder",
    ], //
    "DlStatus" : [
      "Success",
      "Failure",
      "Duplicate",
      "Occupied",
      "InvalidField",
      "ResourceExhausted",
      "NotFound",
    ], //
    "DoorLockOperationEventCode" : [
      "UnknownOrMfgSpecific",
      "Lock",
      "Unlock",
      "LockInvalidPinOrId",
      "LockInvalidSchedule",
      "UnlockInvalidPinOrId",
      "UnlockInvalidSchedule",
      "OneTouchLock",
      "KeyLock",
      "KeyUnlock",
      "AutoLock",
      "ScheduleLock",
      "ScheduleUnlock",
      "ManualLock",
      "ManualUnlock",
    ], //
    "DoorLockProgrammingEventCode" : [
      "UnknownOrMfgSpecific",
      "MasterCodeChanged",
      "PinAdded",
      "PinDeleted",
      "PinChanged",
      "IdAdded",
      "IdDeleted",
    ], //
    "DoorLockSetPinOrIdStatus" : [
      "Success",
      "GeneralFailure",
      "MemoryFull",
      "DuplicateCodeError",
    ], //
    "DoorLockUserStatus" : [
      "Available",
      "OccupiedEnabled",
      "OccupiedDisabled",
      "NotSupported",
    ], //
    "DoorLockUserType" : [
      "Unrestricted",
      "YearDayScheduleUser",
      "WeekDayScheduleUser",
      "MasterUser",
      "NonAccessUser",
      "NotSupported",
    ], //
    "DoorStateEnum" : [
      "DoorOpen",
      "DoorClosed",
      "DoorJammed",
      "DoorForcedOpen",
      "DoorUnspecifiedError",
      "DoorAjar",
    ], //
    "DownloadProtocolEnum" : [
      "BDXSynchronous",
      "BDXAsynchronous",
      "Https",
      "VendorSpecific",
    ], //
    "DrynessLevelEnum" : [
      "Low",
      "Normal",
      "Extra",
      "Max",
    ], //
    "DyingLightEffectVariantEnum" : [
      "DyingLightFadeOff",
    ], //
    "ESAStateEnum" : [
      "Offline",
      "Online",
      "Fault",
      "UserOptOut",
      "PowerAdjustActive",
      "Paused",
    ], //
    "ESATypeEnum" : [
      "Evse",
      "SpaceHeating",
      "WaterHeating",
      "SpaceCooling",
      "SpaceHeatingCooling",
      "BatteryStorage",
      "SolarPV",
      "FridgeFreezer",
      "WashingMachine",
      "Dishwasher",
      "Cooking",
      "HomeWaterPump",
      "IrrigationWaterPump",
      "PoolPump",
      "Other",
    ], //
    "EffectIdentifierEnum" : [
      "DelayedAllOff",
      "DyingLight",
    ], //
    "EffectIdentifierEnum" : [
      "Blink",
      "Breathe",
      "Okay",
      "ChannelChange",
      "FinishEffect",
      "StopEffect",
    ], //
    "EffectVariantEnum" : [
      "Default",
    ], //
    "EndOfServiceEnum" : [
      "Normal",
      "Expired",
    ], //
    "EndProductType" : [
      "RollerShade",
      "RomanShade",
      "BalloonShade",
      "WovenWood",
      "PleatedShade",
      "CellularShade",
      "LayeredShade",
      "LayeredShade2D",
      "SheerShade",
      "TiltOnlyInteriorBlind",
      "InteriorBlind",
      "VerticalBlindStripCurtain",
      "InteriorVenetianBlind",
      "ExteriorVenetianBlind",
      "LateralLeftCurtain",
      "LateralRightCurtain",
      "CentralCurtain",
      "RollerShutter",
      "ExteriorVerticalScreen",
      "AwningTerracePatio",
      "AwningVerticalScreen",
      "TiltOnlyPergola",
      "SwingingShutter",
      "SlidingShutter",
      "Unknown",
    ], //
    "EndpointListTypeEnum" : [
      "Other",
      "Room",
      "Zone",
    ], //
    "EnergyPriorityEnum" : [
      "Comfort",
      "Speed",
      "Efficiency",
      "WaterConsumption",
    ], //
    "EnergyTransferStoppedReasonEnum" : [
      "EVStopped",
      "EVSEStopped",
      "Other",
    ], //
    "ErrorStateEnum" : [
      "NoError",
      "UnableToStartOrResume",
      "UnableToCompleteOperation",
      "CommandInvalidInState",
    ], //
    "ErrorStateEnum" : [
      "FailedToFindChargingDock",
      "Stuck",
      "DustBinMissing",
      "DustBinFull",
      "WaterTankEmpty",
      "WaterTankMissing",
      "WaterTankLidOpen",
      "MopCleaningPadMissing",
    ], //
    "ExpressedStateEnum" : [
      "Normal",
      "SmokeAlarm",
      "COAlarm",
      "BatteryAlert",
      "Testing",
      "HardwareFault",
      "EndOfService",
      "InterconnectSmoke",
      "InterconnectCO",
    ], //
    "FanModeEnum" : [
      "Off",
      "Low",
      "Medium",
      "High",
      "On",
      "Auto",
      "Smart",
    ], //
    "FanModeSequenceEnum" : [
      "OffLowMedHigh",
      "OffLowHigh",
      "OffLowMedHighAuto",
      "OffLowHighAuto",
      "OffHighAuto",
      "OffHigh",
    ], //
    "FaultStateEnum" : [
      "NoError",
      "MeterFailure",
      "OverVoltage",
      "UnderVoltage",
      "OverCurrent",
      "ContactWetFailure",
      "ContactDryFailure",
      "GroundFault",
      "PowerLoss",
      "PowerQuality",
      "PilotShortCircuit",
      "EmergencyStop",
      "EVDisconnected",
      "WrongPowerSupply",
      "LiveNeutralSwap",
      "OverTemperature",
      "Other",
    ], //
    "FaultType" : [
      "Unspecified",
      "SystemFault",
      "InetFault",
      "ChipFault",
      "CertFault",
    ], //
    "GranularityEnum" : [
      "NoTimeGranularity",
      "MinutesGranularity",
      "SecondsGranularity",
      "MillisecondsGranularity",
      "MicrosecondsGranularity",
    ], //
    "GroupKeySecurityPolicyEnum" : [
      "TrustFirst",
      "CacheAndSync",
    ], //
    "HardwareFaultEnum" : [
      "Unspecified",
      "Radio",
      "Sensor",
      "ResettableOverTemp",
      "NonResettableOverTemp",
      "PowerSource",
      "VisualDisplayFault",
      "AudioOutputFault",
      "UserInterfaceFault",
      "NonVolatileMemoryError",
      "TamperDetected",
    ], //
    "HeatingSourceEnum" : [
      "Any",
      "Electric",
      "NonElectric",
    ], //
    "HourFormatEnum" : [
      "12hr",
      "24hr",
      "UseActiveLocale",
    ], //
    "HueDirection" : [
      "ShortestDistance",
      "LongestDistance",
      "Up",
      "Down",
    ], //
    "HueMoveMode" : [
      "Stop",
      "Up",
      "Down",
    ], //
    "HueStepMode" : [
      "Up",
      "Down",
    ], //
    "IdentifyTypeEnum" : [
      "None",
      "LightOutput",
      "VisibleIndicator",
      "AudibleBeep",
      "Display",
      "Actuator",
    ], //
    "InputTypeEnum" : [
      "Internal",
      "Aux",
      "Coax",
      "Composite",
      "Hdmi",
      "Input",
      "Line",
      "Optical",
      "Video",
      "Scart",
      "Usb",
      "Other",
    ], //
    "IntentEnum" : [
      "EndUserSupport",
      "NetworkDiag",
      "CrashLogs",
    ], //
    "InterfaceTypeEnum" : [
      "Unspecified",
      "WiFi",
      "Ethernet",
      "Cellular",
      "Thread",
    ], //
    "KeypadLockoutEnum" : [
      "NoLockout",
      "Lockout1",
      "Lockout2",
      "Lockout3",
      "Lockout4",
      "Lockout5",
    ], //
    "LevelValueEnum" : [
      "Unknown",
      "Low",
      "Medium",
      "High",
      "Critical",
    ], //
    "LightSensorTypeEnum" : [
      "Photodiode",
      "Cmos",
    ], //
    "LineupInfoTypeEnum" : [
      "Mso",
    ], //
    "LoadControlEventChangeSourceEnum" : [
      "Automatic",
      "UserAction",
    ], //
    "LoadControlEventStatusEnum" : [
      "Unknown",
      "Received",
      "InProgress",
      "Completed",
      "OptedOut",
      "OptedIn",
      "Canceled",
      "Superseded",
      "PartialOptedOut",
      "PartialOptedIn",
      "NoParticipation",
      "Unavailable",
      "Failed",
    ], //
    "LockDataTypeEnum" : [
      "Unspecified",
      "ProgrammingCode",
      "UserIndex",
      "WeekDaySchedule",
      "YearDaySchedule",
      "HolidaySchedule",
      "Pin",
      "Rfid",
      "Fingerprint",
      "FingerVein",
      "Face",
    ], //
    "LockOperationTypeEnum" : [
      "Lock",
      "Unlock",
      "NonAccessUserEvent",
      "ForcedUserEvent",
      "Unlatch",
    ], //
    "MeasurementMediumEnum" : [
      "Air",
      "Water",
      "Soil",
    ], //
    "MeasurementTypeEnum" : [
      "Unspecified",
      "Voltage",
      "ActiveCurrent",
      "ReactiveCurrent",
      "ApparentCurrent",
      "ActivePower",
      "ReactivePower",
      "ApparentPower",
      "RMSVoltage",
      "RMSCurrent",
      "RMSPower",
      "Frequency",
      "PowerFactor",
      "NeutralCurrent",
      "ElectricalEnergy",
    ], //
    "MeasurementUnitEnum" : [
      "Ppm",
      "Ppb",
      "Ppt",
      "Mgm3",
      "Ugm3",
      "Ngm3",
      "Pm3",
      "Bqm3",
    ], //
    "MetricTypeEnum" : [
      "Pixels",
      "Percentage",
    ], //
    "ModeTag" : [
      "Normal",
      "Defrost",
    ], //
    "ModeTag" : [
      "Normal",
      "Heavy",
      "Light",
    ], //
    "ModeTag" : [
      "Bake",
      "Convection",
      "Grill",
      "Roast",
      "Clean",
      "ConvectionBake",
      "ConvectionRoast",
      "Warming",
      "Proofing",
    ], //
    "ModeTag" : [
      "Normal",
      "Delicate",
      "Heavy",
      "Whites",
    ], //
    "ModeTag" : [
      "RapidCool",
      "RapidFreeze",
    ], //
    "ModeTag" : [
      "DeepClean",
      "Vacuum",
      "Mop",
    ], //
    "ModeTag" : [
      "Idle",
      "Cleaning",
    ], //
    "MoveModeEnum" : [
      "Up",
      "Down",
    ], //
    "MuteStateEnum" : [
      "NotMuted",
      "Muted",
    ], //
    "NetworkCommissioningStatusEnum" : [
      "Success",
      "OutOfRange",
      "BoundsExceeded",
      "NetworkIDNotFound",
      "DuplicateNetworkID",
      "NetworkNotFound",
      "RegulatoryError",
      "AuthFailure",
      "UnsupportedSecurity",
      "OtherConnectionFailure",
      "IPV6Failed",
      "IPBindFailed",
      "UnknownError",
    ], //
    "NetworkFaultEnum" : [
      "Unspecified",
      "HardwareFailure",
      "NetworkJammed",
      "ConnectionFailed",
    ], //
    "NetworkFaultEnum" : [
      "Unspecified",
      "LinkDown",
      "HardwareFailure",
      "NetworkJammed",
    ], //
    "NodeOperationalCertStatusEnum" : [
      "Ok",
      "InvalidPublicKey",
      "InvalidNodeOpId",
      "InvalidNOC",
      "MissingCsr",
      "TableFull",
      "InvalidAdminSubject",
      "FabricConflict",
      "LabelConflict",
      "InvalidFabricIndex",
    ], //
    "NumberOfRinsesEnum" : [
      "None",
      "Normal",
      "Extra",
      "Max",
    ], //
    "OccupancySensorTypeEnum" : [
      "Pir",
      "Ultrasonic",
      "PIRAndUltrasonic",
      "PhysicalContact",
    ], //
    "OperatingModeEnum" : [
      "Normal",
      "Vacation",
      "Privacy",
      "NoRemoteLockUnlock",
      "Passage",
    ], //
    "OperationErrorEnum" : [
      "Unspecified",
      "InvalidCredential",
      "DisabledUserDenied",
      "Restricted",
      "InsufficientBattery",
    ], //
    "OperationModeEnum" : [
      "Normal",
      "Minimum",
      "Maximum",
      "Local",
    ], //
    "OperationSourceEnum" : [
      "Unspecified",
      "Manual",
      "ProprietaryRemote",
      "Keypad",
      "Auto",
      "Button",
      "Schedule",
      "Remote",
      "Rfid",
      "Biometric",
    ], //
    "OperationalStateEnum" : [
      "Stopped",
      "Running",
      "Paused",
      "Error",
    ], //
    "OperationalStateEnum" : [
      "SeekingCharger",
      "Charging",
      "Docked",
    ], //
    "OutputTypeEnum" : [
      "Hdmi",
      "Bt",
      "Optical",
      "Headphone",
      "Internal",
      "Other",
    ], //
    "PHYRateEnum" : [
      "Rate10M",
      "Rate100M",
      "Rate1G",
      "Rate25g",
      "Rate5G",
      "Rate10G",
      "Rate40G",
      "Rate100G",
      "Rate200G",
      "Rate400G",
    ], //
    "ParameterEnum" : [
      "Actor",
      "Channel",
      "Character",
      "Director",
      "Event",
      "Franchise",
      "Genre",
      "League",
      "Popularity",
      "Provider",
      "Sport",
      "SportsTeam",
      "Type",
      "Video",
      "Season",
      "Episode",
      "Any",
    ], //
    "PlaybackStateEnum" : [
      "Playing",
      "Paused",
      "NotPlaying",
      "Buffering",
    ], //
    "PowerSourceStatusEnum" : [
      "Unspecified",
      "Active",
      "Standby",
      "Unavailable",
    ], //
    "ProductFinishEnum" : [
      "Other",
      "Matte",
      "Satin",
      "Polished",
      "Rugged",
      "Fabric",
    ], //
    "ProductFinishEnum" : [
      "Other",
      "Matte",
      "Satin",
      "Polished",
      "Rugged",
      "Fabric",
    ], //
    "ProductIdentifierTypeEnum" : [
      "Upc",
      "Gtin8",
      "Ean",
      "Gtin14",
      "Oem",
    ], //
    "RadioFaultEnum" : [
      "Unspecified",
      "WiFiFault",
      "CellularFault",
      "ThreadFault",
      "NFCFault",
      "BLEFault",
      "EthernetFault",
    ], //
    "RegulatoryLocationTypeEnum" : [
      "Indoor",
      "Outdoor",
      "IndoorOutdoor",
    ], //
    "RoutingRoleEnum" : [
      "Unspecified",
      "Unassigned",
      "SleepyEndDevice",
      "EndDevice",
      "Reed",
      "Router",
      "Leader",
    ], //
    "SaturationMoveMode" : [
      "Stop",
      "Up",
      "Down",
    ], //
    "SaturationStepMode" : [
      "Up",
      "Down",
    ], //
    "ScheduleProgrammingVisibilityEnum" : [
      "ScheduleProgrammingPermitted",
      "ScheduleProgrammingDenied",
    ], //
    "SecurityTypeEnum" : [
      "Unspecified",
      "None",
      "Wep",
      "Wpa",
      "Wpa2",
      "Wpa3",
    ], //
    "SensitivityEnum" : [
      "High",
      "Standard",
      "Low",
    ], //
    "SetpointChangeSourceEnum" : [
      "Manual",
      "Schedule",
      "External",
    ], //
    "SetpointRaiseLowerModeEnum" : [
      "Heat",
      "Cool",
      "Both",
    ], //
    "SimpleEnum" : [
      "Unspecified",
      "ValueA",
      "ValueB",
      "ValueC",
    ], //
    "StartOfWeekEnum" : [
      "Sunday",
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday",
    ], //
    "StartUpOnOffEnum" : [
      "Off",
      "On",
      "Toggle",
    ], //
    "StateEnum" : [
      "NotPluggedIn",
      "PluggedInNoDemand",
      "PluggedInDemand",
      "PluggedInCharging",
      "PluggedInDischarging",
      "SessionEnding",
      "Fault",
    ], //
    "StatusCode" : [
      "CleaningInProgress",
    ], //
    "StatusCode" : [
      "Stuck",
      "DustBinMissing",
      "DustBinFull",
      "WaterTankEmpty",
      "WaterTankMissing",
      "WaterTankLidOpen",
      "MopCleaningPadMissing",
      "BatteryLow",
    ], //
    "StatusCode" : [
      "Busy",
      "PAKEParameterError",
      "WindowNotOpen",
    ], //
    "StatusCode" : [
      "TimeNotAccepted",
    ], //
    "StatusCodeEnum" : [
      "FailureDueToFault",
    ], //
    "StatusEnum" : [
      "Success",
      "UnsupportedKey",
      "InvalidKeyInCurrentState",
    ], //
    "StatusEnum" : [
      "Success",
      "TargetNotFound",
      "NotAllowed",
    ], //
    "StatusEnum" : [
      "Success",
      "UnexpectedData",
    ], //
    "StatusEnum" : [
      "Success",
      "AppNotAvailable",
      "SystemBusy",
    ], //
    "StatusEnum" : [
      "Success",
      "Exhausted",
      "NoLogs",
      "Busy",
      "Denied",
    ], //
    "StatusEnum" : [
      "Success",
      "URLNotAvailable",
      "AuthFailed",
      "TextTrackNotAvailable",
      "AudioTrackNotAvailable",
    ], //
    "StatusEnum" : [
      "Success",
      "InvalidStateForCommand",
      "NotAllowed",
      "NotActive",
      "SpeedOutOfRange",
      "SeekOutOfRange",
    ], //
    "StatusEnum" : [
      "Success",
      "MultipleMatches",
      "NoMatches",
    ], //
    "StatusEnum" : [
      "UpdateAvailable",
      "Busy",
      "NotAvailable",
      "DownloadProtocolNotSupported",
    ], //
    "StepDirectionEnum" : [
      "Increase",
      "Decrease",
    ], //
    "StepModeEnum" : [
      "Up",
      "Down",
    ], //
    "SupplyStateEnum" : [
      "Disabled",
      "ChargingEnabled",
      "DischargingEnabled",
      "DisabledError",
      "DisabledDiagnostics",
    ], //
    "SystemModeEnum" : [
      "Off",
      "Auto",
      "Cool",
      "Heat",
      "EmergencyHeat",
      "Precooling",
      "FanOnly",
      "Dry",
      "Sleep",
    ], //
    "TempUnitEnum" : [
      "Fahrenheit",
      "Celsius",
      "Kelvin",
    ], //
    "TemperatureDisplayModeEnum" : [
      "Celsius",
      "Fahrenheit",
    ], //
    "TemperatureSetpointHoldEnum" : [
      "SetpointHoldOff",
      "SetpointHoldOn",
    ], //
    "ThermostatRunningModeEnum" : [
      "Off",
      "Cool",
      "Heat",
    ], //
    "TimeSourceEnum" : [
      "None",
      "Unknown",
      "Admin",
      "NodeTimeCluster",
      "NonMatterSNTP",
      "NonMatterNTP",
      "MatterSNTP",
      "MatterNTP",
      "MixedNTP",
      "NonMatterSNTPNTS",
      "NonMatterNTPNTS",
      "MatterSNTPNTS",
      "MatterNTPNTS",
      "MixedNTPNTS",
      "CloudSource",
      "Ptp",
      "Gnss",
    ], //
    "TimeZoneDatabaseEnum" : [
      "Full",
      "Partial",
      "None",
    ], //
    "TimerStatusEnum" : [
      "Running",
      "Paused",
      "Expired",
      "Ready",
    ], //
    "TransferProtocolEnum" : [
      "ResponsePayload",
      "Bdx",
    ], //
    "Type" : [
      "RollerShade",
      "RollerShade2Motor",
      "RollerShadeExterior",
      "RollerShadeExterior2Motor",
      "Drapery",
      "Awning",
      "Shutter",
      "TiltBlindTiltOnly",
      "TiltBlindLiftAndTilt",
      "ProjectorScreen",
      "Unknown",
    ], //
    "UpdateStateEnum" : [
      "Unknown",
      "Idle",
      "Querying",
      "DelayedOnQuery",
      "Downloading",
      "Applying",
      "DelayedOnApply",
      "RollingBack",
      "DelayedOnUserConsent",
    ], //
    "UserStatusEnum" : [
      "Available",
      "OccupiedEnabled",
      "OccupiedDisabled",
    ], //
    "UserTypeEnum" : [
      "UnrestrictedUser",
      "YearDayScheduleUser",
      "WeekDayScheduleUser",
      "ProgrammingUser",
      "NonAccessUser",
      "ForcedUser",
      "DisposableUser",
      "ExpiringUser",
      "ScheduleRestrictedUser",
      "RemoteOnlyUser",
    ], //
    "ValveStateEnum" : [
      "Closed",
      "Open",
      "Transitioning",
    ], //
    "WiFiBandEnum" : [
      "2g4",
      "3g65",
      "5g",
      "6g",
      "60g",
      "1g",
    ], //
    "WiFiVersionEnum" : [
      "A",
      "B",
      "G",
      "N",
      "Ac",
      "Ax",
      "Ah",
    ], //
    "WiredCurrentTypeEnum" : [
      "Ac",
      "Dc",
    ], //
    "WiredFaultEnum" : [
      "Unspecified",
      "OverVoltage",
      "UnderVoltage",
    ], //
  },

  bitmaps : {
    "ACErrorCodeBitmap" : {
      1 : "CompressorFail",
      2 : "RoomSensorFail",
      4 : "OutdoorSensorFail",
      8 : "CoilSensorFail",
      16 : "FanFail",
    },
    "AlarmMap" : {
      1 : "InflowError",
      2 : "DrainError",
      4 : "DoorError",
      8 : "TempTooLow",
      16 : "TempTooHigh",
      32 : "WaterLevelError",
    },
    "AlarmMap" : {
      1 : "DoorOpen",
    },
    "AlarmModeBitmap" : {
      1 : "Visual",
      2 : "Audible",
    },
    "BallastStatusBitmap" : {
      1 : "BallastNonOperational",
      2 : "LampFailure",
    },
    "BarrierControlCapabilities" : {
      1 : "PartialBarrier",
    },
    "BarrierControlSafetyStatus" : {
      1 : "RemoteLockout",
      2 : "TemperDetected",
      4 : "FailedCommunication",
      8 : "PositionFailure",
    },
    "Bitmap16MaskMap" : {
      1 : "MaskVal1",
      2 : "MaskVal2",
      4 : "MaskVal3",
      16384 : "MaskVal4",
    },
    "Bitmap32MaskMap" : {
      1 : "MaskVal1",
      2 : "MaskVal2",
      4 : "MaskVal3",
      1073741824 : "MaskVal4",
    },
    "Bitmap64MaskMap" : {
      1 : "MaskVal1",
      2 : "MaskVal2",
      4 : "MaskVal3",
      4611686018427388000 : "MaskVal4",
    },
    "Bitmap8MaskMap" : {
      1 : "MaskVal1",
      2 : "MaskVal2",
      4 : "MaskVal3",
      64 : "MaskVal4",
    },
    "CancelControlBitmap" : {
      1 : "RandomEnd",
    },
    "ColorCapabilities" : {
      1 : "HueSaturationSupported",
      2 : "EnhancedHueSupported",
      4 : "ColorLoopSupported",
      8 : "XYAttributesSupported",
      16 : "ColorTemperatureSupported",
    },
    "ColorLoopUpdateFlags" : {
      1 : "UpdateAction",
      2 : "UpdateDirection",
      4 : "UpdateTime",
      8 : "UpdateStartHue",
    },
    "CommandBits" : {
      1 : "InstantAction",
      2 : "InstantActionWithTransition",
      4 : "StartAction",
      8 : "StartActionWithDuration",
      16 : "StopAction",
      32 : "PauseAction",
      64 : "PauseActionWithDuration",
      128 : "ResumeAction",
      256 : "EnableAction",
      512 : "EnableActionWithDuration",
      1024 : "DisableAction",
      2048 : "DisableActionWithDuration",
    },
    "ConfigStatus" : {
      1 : "Operational",
      2 : "OnlineReserved",
      4 : "LiftMovementReversed",
      8 : "LiftPositionAware",
      16 : "TiltPositionAware",
      32 : "LiftEncoderControlled",
      64 : "TiltEncoderControlled",
    },
    "CopyModeBitmap" : {
      1 : "CopyAllScenes",
    },
    "DaysMaskMap" : {
      1 : "Sunday",
      2 : "Monday",
      4 : "Tuesday",
      8 : "Wednesday",
      16 : "Thursday",
      32 : "Friday",
      64 : "Saturday",
    },
    "DeviceClassBitmap" : {
      1 : "Hvac",
      2 : "StripHeater",
      4 : "WaterHeater",
      8 : "PoolPump",
      16 : "SmartAppliance",
      32 : "IrrigationPump",
      64 : "CommercialLoad",
      128 : "ResidentialLoad",
      256 : "ExteriorLighting",
      512 : "InteriorLighting",
      1024 : "Ev",
      2048 : "GenerationSystem",
      4096 : "SmartInverter",
      8192 : "Evse",
      16384 : "Resu",
      32768 : "Ems",
      65536 : "Sem",
    },
    "DlCredentialRuleMask" : {
      1 : "Single",
      2 : "Dual",
      4 : "Tri",
    },
    "DlCredentialRulesSupport" : {
      1 : "Single",
      2 : "Dual",
      4 : "Tri",
    },
    "DlDefaultConfigurationRegister" : {
      1 : "EnableLocalProgrammingEnabled",
      2 : "KeypadInterfaceDefaultAccessEnabled",
      4 : "RemoteInterfaceDefaultAccessIsEnabled",
      32 : "SoundEnabled",
      64 : "AutoRelockTimeSet",
      128 : "LEDSettingsSet",
    },
    "DlKeypadOperationEventMask" : {
      1 : "Unknown",
      2 : "Lock",
      4 : "Unlock",
      8 : "LockInvalidPIN",
      16 : "LockInvalidSchedule",
      32 : "UnlockInvalidCode",
      64 : "UnlockInvalidSchedule",
      128 : "NonAccessUserOpEvent",
    },
    "DlKeypadProgrammingEventMask" : {
      1 : "Unknown",
      2 : "ProgrammingPINChanged",
      4 : "PINAdded",
      8 : "PINCleared",
      16 : "PINChanged",
    },
    "DlLocalProgrammingFeatures" : {
      1 : "AddUsersCredentialsSchedulesLocally",
      2 : "ModifyUsersCredentialsSchedulesLocally",
      4 : "ClearUsersCredentialsSchedulesLocally",
      8 : "AdjustLockSettingsLocally",
    },
    "DlManualOperationEventMask" : {
      1 : "Unknown",
      2 : "ThumbturnLock",
      4 : "ThumbturnUnlock",
      8 : "OneTouchLock",
      16 : "KeyLock",
      32 : "KeyUnlock",
      64 : "AutoLock",
      128 : "ScheduleLock",
      256 : "ScheduleUnlock",
      512 : "ManualLock",
      1024 : "ManualUnlock",
    },
    "DlRFIDOperationEventMask" : {
      1 : "Unknown",
      2 : "Lock",
      4 : "Unlock",
      8 : "LockInvalidRFID",
      16 : "LockInvalidSchedule",
      32 : "UnlockInvalidRFID",
      64 : "UnlockInvalidSchedule",
    },
    "DlRFIDProgrammingEventMask" : {
      1 : "Unknown",
      32 : "RFIDCodeAdded",
      64 : "RFIDCodeCleared",
    },
    "DlRemoteOperationEventMask" : {
      1 : "Unknown",
      2 : "Lock",
      4 : "Unlock",
      8 : "LockInvalidCode",
      16 : "LockInvalidSchedule",
      32 : "UnlockInvalidCode",
      64 : "UnlockInvalidSchedule",
    },
    "DlRemoteProgrammingEventMask" : {
      1 : "Unknown",
      2 : "ProgrammingPINChanged",
      4 : "PINAdded",
      8 : "PINCleared",
      16 : "PINChanged",
      32 : "RFIDCodeAdded",
      64 : "RFIDCodeCleared",
    },
    "DlSupportedOperatingModes" : {
      1 : "Normal",
      2 : "Vacation",
      4 : "Privacy",
      8 : "NoRemoteLockUnlock",
      16 : "Passage",
    },
    "DoorLockDayOfWeek" : {
      1 : "Sunday",
      2 : "Monday",
      4 : "Tuesday",
      8 : "Wednesday",
      16 : "Thursday",
      32 : "Friday",
      64 : "Saturday",
    },
    "EventControlBitmap" : {
      1 : "RandomStart",
    },
    "EventTransitionControlBitmap" : {
      1 : "RandomDuration",
      2 : "IgnoreOptOut",
    },
    "Feature" : {
      1 : "OnOff",
    },
    "Feature" : {
      1 : "Fair",
      2 : "Moderate",
      4 : "VeryPoor",
      8 : "ExtremelyPoor",
    },
    "Feature" : {
      1 : "Spin",
      2 : "Rinse",
    },
    "Feature" : {
      1 : "TagList",
    },
    "Feature" : {
      1 : "NavigationKeyCodes",
      2 : "LocationKeys",
      4 : "NumberKeys",
    },
    "Feature" : {
      1 : "OnOff",
    },
    "Feature" : {
      1 : "TemperatureNumber",
      2 : "TemperatureLevel",
      4 : "TemperatureStep",
    },
    "Feature" : {
      1 : "TemperatureUnit",
    },
    "Feature" : {
      1 : "Reset",
    },
    "Feature" : {
      1 : "Visual",
      2 : "Audible",
      4 : "AlarmSuppress",
      8 : "SensitivityLevel",
    },
    "Feature" : {
      1 : "ImportedEnergy",
      2 : "ExportedEnergy",
      4 : "CumulativeEnergy",
      8 : "PeriodicEnergy",
    },
    "Feature" : {
      1 : "PacketCounts",
      2 : "ErrorCounts",
    },
    "Feature" : {
      1 : "Reset",
    },
    "Feature" : {
      1 : "Extended",
    },
    "Feature" : {
      1 : "Watermarks",
    },
    "Feature" : {
      1 : "TimeSync",
      2 : "Level",
    },
    "Feature" : {
      1 : "CalendarFormat",
    },
    "Feature" : {
      1 : "ApplicationPlatform",
    },
    "Feature" : {
      1 : "MultiSpeed",
      2 : "Auto",
      4 : "Rocking",
      8 : "Wind",
      16 : "Step",
      32 : "AirflowDirection",
    },
    "Feature" : {
      1 : "NameUpdates",
    },
    "Feature" : {
      1 : "Condition",
      2 : "Warning",
      4 : "ReplacementProductList",
    },
    "Feature" : {
      1 : "PacketCounts",
      2 : "ErrorCounts",
    },
    "Feature" : {
      1 : "Lighting",
      2 : "DeadFrontBehavior",
      4 : "OffOnly",
    },
    "Feature" : {
      1 : "NameUpdates",
    },
    "Feature" : {
      1 : "PowerAdjustment",
      2 : "PowerForecastReporting",
      4 : "StateForecastReporting",
      8 : "ForecastAdjustment",
    },
    "Feature" : {
      1 : "ContentSearch",
      2 : "URLPlayback",
      3 : "AdvancedSeek",
      4 : "TextTracks",
      5 : "AudioTracks",
    },
    "Feature" : {
      1 : "EnergyBalance",
      2 : "LowPowerModeSensitivity",
    },
    "Feature" : {
      1 : "LatchingSwitch",
      2 : "MomentarySwitch",
      4 : "MomentarySwitchRelease",
      8 : "MomentarySwitchLongPress",
      16 : "MomentarySwitchMultiPress",
    },
    "Feature" : {
      1 : "SmokeAlarm",
      2 : "CoAlarm",
    },
    "Feature" : {
      1 : "EnrollmentGroups",
      2 : "TemperatureOffset",
      4 : "TemperatureSetpoint",
      8 : "LoadAdjustment",
      16 : "DutyCycle",
      32 : "PowerSavings",
      64 : "HeatingSource",
    },
    "Feature" : {
      1 : "Basic",
    },
    "Feature" : {
      1 : "ScreenTime",
      2 : "PINManagement",
      3 : "BlockUnrated",
      4 : "OnDemandContentRating",
      5 : "ScheduledContentRating",
    },
    "Feature" : {
      1 : "CheckInProtocolSupport",
      2 : "UserActiveModeTrigger",
      4 : "LongIdleTimeSupport",
    },
    "Feature" : {
      1 : "CacheAndSync",
    },
    "Feature" : {
      1 : "AdvancedSeek",
      2 : "VariableSpeed",
      3 : "TextTracks",
      4 : "AudioTracks",
      5 : "AudioAdvance",
    },
    "Feature" : {
      1 : "TimeZone",
      2 : "NTPClient",
      4 : "NTPServer",
      8 : "TimeSyncClient",
    },
    "Feature" : {
      1 : "Wired",
      2 : "Battery",
      4 : "Rechargeable",
      8 : "Replaceable",
    },
    "Feature" : {
      1 : "ChannelList",
      2 : "LineupInfo",
      3 : "ElectronicGuide",
      4 : "RecordProgram",
    },
    "Feature" : {
      1 : "Lift",
      2 : "Tilt",
      4 : "PositionAwareLift",
      8 : "AbsolutePosition",
      16 : "PositionAwareTilt",
    },
    "Feature" : {
      1 : "GroupNames",
    },
    "Feature" : {
      1 : "ChargingPreferences",
      2 : "SoCReporting",
      4 : "PlugAndCharge",
      8 : "Rfid",
      16 : "V2x",
    },
    "Feature" : {
      1 : "OnOff",
      2 : "Lighting",
      4 : "Frequency",
    },
    "Feature" : {
      1 : "ConstantPressure",
      2 : "CompensatedPressure",
      4 : "ConstantFlow",
      8 : "ConstantSpeed",
      16 : "ConstantTemperature",
      32 : "Automatic",
      64 : "LocalOperation",
    },
    "Feature" : {
      1 : "PacketCounts",
      2 : "ErrorCounts",
      4 : "MLECounts",
      8 : "MACCounts",
    },
    "Feature" : {
      1 : "WiFiNetworkInterface",
      2 : "ThreadNetworkInterface",
      4 : "EthernetNetworkInterface",
      8 : "PerDeviceCredentials",
    },
    "Feature" : {
      1 : "HueAndSaturation",
      2 : "EnhancedHue",
      4 : "ColorLoop",
      8 : "Xy",
      16 : "ColorTemperature",
    },
    "Feature" : {
      1 : "SceneNames",
      2 : "Explicit",
      4 : "TableSize",
      8 : "FabricScenes",
    },
    "Feature" : {
      1 : "Heating",
      2 : "Cooling",
      4 : "Occupancy",
      8 : "ScheduleConfiguration",
      16 : "Setback",
      32 : "AutoMode",
      64 : "LocalTemperatureNotExposed",
    },
    "Feature" : {
      1 : "NumericMeasurement",
      2 : "LevelIndication",
      4 : "MediumLevel",
      8 : "CriticalLevel",
      16 : "PeakMeasurement",
      32 : "AverageMeasurement",
    },
    "Feature" : {
      1 : "PinCredential",
      2 : "RfidCredential",
      4 : "FingerCredentials",
      8 : "Logging",
      16 : "WeekDayAccessSchedules",
      32 : "DoorPositionSensor",
      64 : "FaceCredentials",
      128 : "CredentialsOverTheAirAccess",
      256 : "User",
      512 : "Notification",
      1024 : "YearDayAccessSchedules",
      2048 : "HolidaySchedules",
      4096 : "Unbolt",
    },
    "HVACSystemTypeBitmap" : {
      3 : "CoolingStage",
      12 : "HeatingStage",
      16 : "HeatingIsHeatPump",
      32 : "HeatingUsesFuel",
    },
    "LampAlarmModeBitmap" : {
      1 : "LampBurnHours",
    },
    "Mode" : {
      1 : "MotorDirectionReversed",
      2 : "CalibrationMode",
      4 : "MaintenanceMode",
      8 : "LedFeedback",
    },
    "NameSupportBitmap" : {
      128 : "GroupNames",
    },
    "NameSupportBitmap" : {
      128 : "SceneNames",
    },
    "OccupancyBitmap" : {
      1 : "Occupied",
    },
    "OccupancySensorTypeBitmap" : {
      1 : "Pir",
      2 : "Ultrasonic",
      4 : "PhysicalContact",
    },
    "OnOffControlBitmap" : {
      1 : "AcceptOnlyWhenOn",
    },
    "OperationalStatus" : {
      3 : "Global",
      12 : "Lift",
      48 : "Tilt",
    },
    "OptionsBitmap" : {
      1 : "ExecuteIfOff",
      2 : "CoupleColorTempToLevel",
    },
    "ProgrammingOperationModeBitmap" : {
      1 : "ScheduleActive",
      2 : "AutoRecovery",
      4 : "Economy",
    },
    "PumpStatusBitmap" : {
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
    "RecordingFlagBitmap" : {
      1 : "Scheduled",
      2 : "RecordSeries",
      3 : "Recorded",
    },
    "RelayStateBitmap" : {
      1 : "Heat",
      2 : "Cool",
      4 : "Fan",
      8 : "HeatStage2",
      16 : "CoolStage2",
      32 : "FanStage2",
      64 : "FanStage3",
    },
    "RemoteSensingBitmap" : {
      1 : "LocalTemperature",
      2 : "OutdoorTemperature",
      4 : "Occupancy",
    },
    "RockBitmap" : {
      1 : "RockLeftRight",
      2 : "RockUpDown",
      4 : "RockRound",
    },
    "SafetyStatus" : {
      1 : "RemoteLockout",
      2 : "TamperDetection",
      4 : "FailedCommunication",
      8 : "PositionFailure",
      16 : "ThermalProtection",
      32 : "ObstacleDetected",
      64 : "Power",
      128 : "StopInput",
      256 : "MotorJammed",
      512 : "HardwareFailure",
      1024 : "ManualOperation",
      2048 : "Protection",
    },
    "ScheduleDayOfWeekBitmap" : {
      1 : "Sunday",
      2 : "Monday",
      4 : "Tuesday",
      8 : "Wednesday",
      16 : "Thursday",
      32 : "Friday",
      64 : "Saturday",
      128 : "Away",
    },
    "ScheduleModeBitmap" : {
      1 : "HeatSetpointPresent",
      2 : "CoolSetpointPresent",
    },
    "SensorFaultBitmap" : {
      1 : "GeneralFault",
    },
    "SimpleBitmap" : {
      1 : "ValueA",
      2 : "ValueB",
      4 : "ValueC",
    },
    "SupportedProtocolsBitmap" : {
      1 : "Dash",
      2 : "Hls",
      2 : "WebRTC",
    },
    "TargetDayOfWeekBitmap" : {
      1 : "Sunday",
      2 : "Monday",
      4 : "Tuesday",
      8 : "Wednesday",
      16 : "Thursday",
      32 : "Friday",
      64 : "Saturday",
    },
    "ThreadCapabilitiesBitmap" : {
      1 : "IsBorderRouterCapable",
      2 : "IsRouterCapable",
      4 : "IsSleepyEndDeviceCapable",
      8 : "IsFullThreadDevice",
      16 : "IsSynchronizedSleepyEndDeviceCapable",
    },
    "UserActiveModeTriggerBitmap" : {
      1 : "PowerCycle",
      2 : "SettingsMenu",
      4 : "CustomInstruction",
      8 : "DeviceManual",
      16 : "ActuateSensor",
      32 : "ActuateSensorSeconds",
      64 : "ActuateSensorTimes",
      128 : "ActuateSensorLightsBlink",
      256 : "ResetButton",
      512 : "ResetButtonLightsBlink",
      1024 : "ResetButtonSeconds",
      2048 : "ResetButtonTimes",
      4096 : "SetupButton",
      8192 : "SetupButtonSeconds",
      16384 : "SetupButtonLightsBlink",
      32768 : "SetupButtonTimes",
      65536 : "AppDefinedButton",
    },
    "ValveFaultBitmap" : {
      1 : "GeneralFault",
      2 : "Blocked",
      4 : "Leaking",
      8 : "NotConnected",
      16 : "ShortCircuit",
      32 : "CurrentExceeded",
    },
    "WiFiSecurityBitmap" : {
      1 : "Unencrypted",
      2 : "Wep",
      4 : "WpaPersonal",
      8 : "Wpa2Personal",
      16 : "Wpa3Personal",
      32 : "Wpa3MatterPdc",
    },
    "WindBitmap" : {
      1 : "SleepWind",
      2 : "NaturalWind",
    },
    "Bitmap16" : {},
    "Bitmap32" : {},
    "Bitmap64" : {},
    "Bitmap8" : {},
  }
}
