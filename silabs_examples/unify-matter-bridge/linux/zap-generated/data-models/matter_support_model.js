exports.model = {
  3 : {
    name : "Identify",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  6 : {
    name : "OnOff",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      81 : "OutOfService",
      85 : "PresentValue",
      111 : "StatusFlags",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "DeviceTypeList",
      1 : "ServerList",
      2 : "ClientList",
      3 : "PartsList",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Acl",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "ActionList",
      1 : "EndpointLists",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
      19 : "CapabilityMinima",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "HourFormat",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Status",
      1 : "Order",
      2 : "Description",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "MaxNetworks",
      1 : "Networks",
      4 : "InterfaceEnabled",
      5 : "LastNetworkingStatus",
      6 : "LastNetworkID",
      7 : "LastConnectErrorValue",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "NetworkInterfaces",
      1 : "RebootCount",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Channel",
      1 : "RoutingRole",
      2 : "NetworkName",
      3 : "PanId",
      4 : "ExtendedPanId",
      5 : "MeshLocalPrefix",
      7 : "NeighborTable",
      8 : "RouteTable",
      9 : "PartitionId",
      10 : "Weighting",
      11 : "DataVersion",
      12 : "StableDataVersion",
      13 : "LeaderRouterId",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Bssid",
      1 : "SecurityType",
      2 : "WiFiVersion",
      3 : "ChannelNumber",
      4 : "Rssi",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SetUTCTime",
        arguments : [
          "UTCTime",
          "Granularity",
          "TimeSource",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "UTCTime",
      1 : "Granularity",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      17 : "Reachable",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "NumberOfPositions",
      1 : "CurrentPosition",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
      2 : {
        name : "RevokeCommissioning",
        arguments : [

        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "IdleModeDuration",
      1 : "ActiveModeDuration",
      2 : "ActiveModeThreshold",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SetTimer",
        arguments : [
          "NewTime",
        ],
      },
      2 : {
        name : "AddTime",
        arguments : [
          "AdditionalTime",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  74 : {
    name : "LaundryDryerControls",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "ChangeToMode",
        arguments : [
          "NewMode",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Description",
      1 : "StandardNamespace",
      2 : "SupportedModes",
      3 : "CurrentMode",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  82 : {
    name : "RefrigeratorAndTemperatureControlledCabinetMode",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  83 : {
    name : "LaundryWasherControls",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  85 : {
    name : "RvcCleanMode",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  86 : {
    name : "TemperatureControl",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SetTemperature",
        arguments : [
          "TargetTemperature",
          "TargetTemperatureLevel",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  91 : {
    name : "AirQuality",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "ExpressedState",
      3 : "BatteryAlert",
      5 : "TestInProgress",
      6 : "HardwareFaultAlert",
      7 : "EndOfServiceAlert",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  94 : {
    name : "MicrowaveOvenMode",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SetCookingParameters",
        arguments : [
          "CookMode",
          "CookTime",
          "PowerSetting",
          "WattSettingIndex",
          "StartAfterSetting",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "CookTime",
      1 : "MaxCookTime",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "PhaseList",
      1 : "CurrentPhase",
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
  98 : {
    name : "ScenesManagement",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      1 : "SceneTableSize",
      2 : "FabricSceneInfo",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      2 : "ChangeIndication",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      2 : "ChangeIndication",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "OpenDuration",
      1 : "DefaultOpenDuration",
      3 : "RemainingDuration",
      4 : "CurrentState",
      5 : "TargetState",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  144 : {
    name : "ElectricalPowerMeasurement",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "PowerMode",
      1 : "NumberOfMeasurementTypes",
      2 : "Accuracy",
      8 : "ActivePower",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Accuracy",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  151 : {
    name : "Messages",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "PresentMessagesRequest",
        arguments : [
          "MessageID",
          "Priority",
          "MessageControl",
          "StartTime",
          "Duration",
          "MessageText",
          "Responses",
        ],
      },
      1 : {
        name : "CancelMessagesRequest",
        arguments : [
          "MessageIDs",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Messages",
      1 : "ActiveMessageIDs",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "ESAType",
      1 : "ESACanGenerate",
      2 : "ESAState",
      3 : "AbsMinPower",
      4 : "AbsMaxPower",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "State",
      1 : "SupplyState",
      2 : "FaultState",
      3 : "ChargingEnabledUntil",
      5 : "CircuitCapacity",
      6 : "MinimumChargeCurrent",
      7 : "MaximumChargeCurrent",
      64 : "SessionID",
      65 : "SessionDuration",
      66 : "SessionEnergyCharged",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  156 : {
    name : "PowerTopology",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
  },
  157 : {
    name : "EnergyEvseMode",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  159 : {
    name : "DeviceEnergyManagementMode",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
  257 : {
    name : "DoorLock",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "LockState",
      1 : "LockType",
      2 : "ActuatorEnabled",
      3 : "DoorState",
      35 : "AutoRelockTime",
      37 : "OperatingMode",
      38 : "SupportedOperatingModes",
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
    },
    events : {
      0 : "DoorLockAlarm",
      1 : "DoorStateChange",
      2 : "LockOperation",
      3 : "LockOperationError",
      4 : "LockUserChange",
    },
  },
  258 : {
    name : "WindowCovering",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Type",
      7 : "ConfigStatus",
      10 : "OperationalStatus",
      13 : "EndProductType",
      23 : "Mode",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      1 : "BarrierMovingState",
      2 : "BarrierSafetyStatus",
      3 : "BarrierCapabilities",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "MaxPressure",
      1 : "MaxSpeed",
      2 : "MaxFlow",
      17 : "EffectiveOperationMode",
      18 : "EffectiveControlMode",
      19 : "Capacity",
      32 : "OperationMode",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SetpointRaiseLower",
        arguments : [
          "Mode",
          "Amount",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "LocalTemperature",
      17 : "OccupiedCoolingSetpoint",
      18 : "OccupiedHeatingSetpoint",
      21 : "MinHeatSetpointLimit",
      22 : "MaxHeatSetpointLimit",
      23 : "MinCoolSetpointLimit",
      24 : "MaxCoolSetpointLimit",
      27 : "ControlSequenceOfOperation",
      28 : "SystemMode",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "FanMode",
      1 : "FanModeSequence",
      2 : "PercentSetting",
      3 : "PercentCurrent",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "TemperatureDisplayMode",
      1 : "KeypadLockout",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "CurrentHue",
      1 : "CurrentSaturation",
      2 : "RemainingTime",
      3 : "CurrentX",
      4 : "CurrentY",
      5 : "DriftCompensation",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "PhysicalMinLevel",
      1 : "PhysicalMaxLevel",
      16 : "MinLevel",
      17 : "MaxLevel",
      32 : "LampQuantity",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Occupancy",
      1 : "OccupancySensorType",
      2 : "OccupancySensorTypeBitmap",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "TargetList",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
      10 : {
        name : "PlaybackResponse",
        arguments : [
          "Status",
          "Data",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "CurrentState",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "Sleep",
        arguments : [

        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "SelectOutput",
        arguments : [
          "Index",
        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      2 : "ApplicationName",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
      0 : "Enabled",
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {},
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
    attributes : {
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
    commands : {
      0 : {
        name : "Test",
        arguments : [

        ],
      },
      1 : {
        name : "TestNotHandled",
        arguments : [

        ],
      },
      2 : {
        name : "TestSpecific",
        arguments : [

        ],
      },
    },
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
      65528 : "GeneratedCommandList",
      65529 : "AcceptedCommandList",
      65530 : "EventList",
      65531 : "AttributeList",
      65532 : "FeatureMap",
      65533 : "ClusterRevision",
      4294070017 : "MeiInt8u",
    },
  },
  4294048774 : {
    name : "FaultInjection",
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory commands, hence only mapping of those as listed here.*/
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
    /*currently we only support mandatory and feature mandatory attributes, hence only mapping of those as listed here.*/
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
    "AdjustmentCauseEnum" : [
      "LocalOptimization",
      "GridOptimization",
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
      "Cancelled",
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
      "Ott",
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
      "AliroCredentialIssuerKey",
      "AliroEvictableEndpointKey",
      "AliroNonEvictableEndpointKey",
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
    "ForecastUpdateReasonEnum" : [
      "InternalOptimization",
      "LocalOptimization",
      "GridOptimization",
    ], //
    "FutureMessagePreferenceEnum" : [
      "Allowed",
      "Increased",
      "Reduced",
      "Disallowed",
      "Banned",
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
      "AliroCredentialIssuerKey",
      "AliroEvictableEndpointKey",
      "AliroNonEvictableEndpointKey",
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
    "MessagePriorityEnum" : [
      "Low",
      "Medium",
      "High",
      "Critical",
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
      "NoOptimization",
      "DeviceOptimization",
      "LocalOptimization",
      "GridOptimization",
    ], //
    "ModeTag" : [
      "Manual",
      "TimeOfUse",
      "SolarCharging",
    ], //
    "ModeTag" : [
      "Normal",
      "Delicate",
      "Heavy",
      "Whites",
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
      "Mapping",
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
      "Sit",
      "Lit",
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
      "Aliro",
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
    "OptOutStateEnum" : [
      "NoOptOut",
      "LocalOptOut",
      "GridOptOut",
      "OptOut",
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
    "PowerModeEnum" : [
      "Unknown",
      "Dc",
      "Ac",
    ], //
    "PowerSourceStatusEnum" : [
      "Unspecified",
      "Active",
      "Standby",
      "Unavailable",
    ], //
    "PresetScenarioEnum" : [
      "Unspecified",
      "Occupied",
      "Unoccupied",
      "Sleep",
      "Wake",
      "Vacation",
      "UserDefined",
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
    "AlarmBitmap" : {
      1 : "InflowError",
      2 : "DrainError",
      4 : "DoorError",
      8 : "TempTooLow",
      16 : "TempTooHigh",
      32 : "WaterLevelError",
    },
    "AlarmBitmap" : {
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
      1 : "NodeTopology",
      2 : "TreeTopology",
      4 : "SetTopology",
      8 : "DynamicPowerFlow",
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
      0 : "NoFeatures",
    },
    "Feature" : {
      0 : "NoFeatures",
    },
    "Feature" : {
      1 : "TemperatureUnit",
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
      1 : "Reset",
    },
    "Feature" : {
      1 : "Extended",
    },
    "Feature" : {
      1 : "PacketCounts",
      2 : "ErrorCounts",
    },
    "Feature" : {
      1 : "Reset",
    },
    "Feature" : {
      1 : "Watermarks",
    },
    "Feature" : {
      1 : "PowerAsNumber",
      2 : "PowerInWatts",
      4 : "PowerNumberLimits",
    },
    "Feature" : {
      1 : "CalendarFormat",
    },
    "Feature" : {
      1 : "ApplicationPlatform",
    },
    "Feature" : {
      1 : "TimeSync",
      2 : "Level",
    },
    "Feature" : {
      1 : "ReceivedConfirmation",
      2 : "ConfirmationResponse",
      4 : "ConfirmationReply",
      8 : "ProtectedMessages",
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
      1 : "DirectCurrent",
      2 : "AlternatingCurrent",
      4 : "PolyphasePower",
      8 : "Harmonics",
      16 : "PowerQuality",
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
      4 : "BlockUnrated",
      8 : "OnDemandContentRating",
      16 : "ScheduledContentRating",
    },
    "Feature" : {
      1 : "DataModelTest",
    },
    "Feature" : {
      1 : "PowerAdjustment",
      2 : "PowerForecastReporting",
      4 : "StateForecastReporting",
      8 : "StartTimeAdjustment",
      16 : "Pausable",
      32 : "ForecastAdjustment",
      64 : "ConstraintBasedAdjustment",
    },
    "Feature" : {
      1 : "CacheAndSync",
    },
    "Feature" : {
      1 : "CheckInProtocolSupport",
      2 : "UserActiveModeTrigger",
      4 : "LongIdleTimeSupport",
    },
    "Feature" : {
      1 : "AdvancedSeek",
      2 : "VariableSpeed",
      4 : "TextTracks",
      8 : "AudioTracks",
      16 : "AudioAdvance",
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
      1 : "ChargingPreferences",
      2 : "SoCReporting",
      4 : "PlugAndCharge",
      8 : "Rfid",
      16 : "V2x",
    },
    "Feature" : {
      1 : "ChannelList",
      2 : "LineupInfo",
      4 : "ElectronicGuide",
      8 : "RecordProgram",
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
      1 : "SceneNames",
    },
    "Feature" : {
      1 : "HueAndSaturation",
      2 : "EnhancedHue",
      4 : "ColorLoop",
      8 : "Xy",
      16 : "ColorTemperature",
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
      1 : "Heating",
      2 : "Cooling",
      4 : "Occupancy",
      8 : "ScheduleConfiguration",
      16 : "Setback",
      32 : "AutoMode",
      64 : "LocalTemperatureNotExposed",
      128 : "MatterScheduleConfiguration",
      256 : "Presets",
      512 : "Setpoints",
      1024 : "QueuedPresetsSupported",
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
      8192 : "AliroProvisioning",
      16384 : "AliroBLEUWB",
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
    "MessageControlBitmap" : {
      1 : "ConfirmationRequired",
      2 : "ResponseRequired",
      4 : "ReplyMessage",
      8 : "MessageConfirmed",
      16 : "MessageProtected",
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
    "PresetTypeFeaturesBitmap" : {
      1 : "Automatic",
      2 : "SupportsNames",
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
      4 : "Recorded",
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
    "ScheduleTypeFeaturesBitmap" : {
      1 : "SupportsPresets",
      2 : "SupportsSetpoints",
      4 : "SupportsNames",
      8 : "SupportsOff",
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
    "TemperatureSetpointHoldPolicyBitmap" : {
      1 : "HoldDurationElapsed",
      2 : "HoldDurationElapsedOrPresetChanged",
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
