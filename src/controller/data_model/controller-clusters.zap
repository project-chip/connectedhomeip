{
  "fileFormat": 2,
  "featureLevel": 98,
  "creator": "zap",
  "keyValuePairs": [
    "commandDiscovery = 1",
    "defaultResponsePolicy = always",
    "manufacturerCodes = 0x1002"
  ],
  "package": [
    {
      "pathRelativity": "relativeToZap",
      "path": "../../app/zap-templates/zcl/zcl.json",
      "type": "zcl-properties",
      "category": "matter",
      "version": 1,
      "description": "Matter SDK ZCL data"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../app/zap-templates/app-templates.json",
      "type": "gen-templates-json",
      "version": "chip-v1"
    }
  ],
  "endpointTypes": [
    {
      "id": 1,
      "name": "MA-rootdevice",
      "deviceTypeRef": {
        "code": 22,
        "profileId": 259,
        "label": "MA-rootdevice",
        "name": "MA-rootdevice"
      },
      "deviceTypes": [
        {
          "code": 22,
          "profileId": 259,
          "label": "MA-rootdevice",
          "name": "MA-rootdevice"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        22
      ],
      "deviceTypeName": "MA-rootdevice",
      "deviceTypeCode": 22,
      "deviceTypeProfileId": 259,
      "clusters": [
        {
          "name": "Identify",
          "code": "0x0003",
          "define": "IDENTIFY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => Identify",
            "0x0040 |         | client |        0 |       1 => TriggerEffect"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Groups",
          "code": "0x0004",
          "define": "GROUPS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => AddGroup",
            "0x0001 |         | client |        1 |       1 => ViewGroup",
            "0x0002 |         | client |        1 |       1 => GetGroupMembership",
            "0x0003 |         | client |        1 |       1 => RemoveGroup",
            "0x0004 |         | client |        1 |       1 => RemoveAllGroups",
            "0x0005 |         | client |        1 |       1 => AddGroupIfIdentifying"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Scenes",
          "code": "0x0005",
          "define": "SCENES_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => AddScene",
            "0x0001 |         | client |        1 |       1 => ViewScene",
            "0x0002 |         | client |        1 |       1 => RemoveScene",
            "0x0003 |         | client |        1 |       1 => RemoveAllScenes",
            "0x0004 |         | client |        1 |       1 => StoreScene",
            "0x0005 |         | client |        1 |       1 => RecallScene",
            "0x0006 |         | client |        1 |       1 => GetSceneMembership"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "On/Off",
          "code": "0x0006",
          "define": "ON_OFF_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => Off",
            "0x0001 |         | client |        1 |       1 => On",
            "0x0002 |         | client |        1 |       1 => Toggle",
            "0x0040 |         | client |        0 |       1 => OffWithEffect",
            "0x0041 |         | client |        0 |       1 => OnWithRecallGlobalScene",
            "0x0042 |         | client |        0 |       1 => OnWithTimedOff"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "On/off Switch Configuration",
          "code": "0x0007",
          "define": "ON_OFF_SWITCH_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Level Control",
          "code": "0x0008",
          "define": "LEVEL_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => MoveToLevel",
            "0x0001 |         | client |        1 |       1 => Move",
            "0x0002 |         | client |        1 |       1 => Step",
            "0x0003 |         | client |        1 |       1 => Stop",
            "0x0004 |         | client |        1 |       1 => MoveToLevelWithOnOff",
            "0x0005 |         | client |        1 |       1 => MoveWithOnOff",
            "0x0006 |         | client |        1 |       1 => StepWithOnOff",
            "0x0007 |         | client |        1 |       1 => StopWithOnOff"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Binary Input (Basic)",
          "code": "0x000f",
          "define": "BINARY_INPUT_BASIC_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Descriptor",
          "code": "0x001d",
          "define": "DESCRIPTOR_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Binding",
          "code": "0x001e",
          "define": "BINDING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Access Control",
          "code": "0x001f",
          "define": "ACCESS_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Actions",
          "code": "0x0025",
          "define": "ACTIONS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => InstantAction",
            "0x0001 |         | client |        1 |       1 => InstantActionWithTransition",
            "0x0002 |         | client |        1 |       1 => StartAction",
            "0x0003 |         | client |        1 |       1 => StartActionWithDuration",
            "0x0004 |         | client |        1 |       1 => StopAction",
            "0x0005 |         | client |        1 |       1 => PauseAction",
            "0x0006 |         | client |        1 |       1 => PauseActionWithDuration",
            "0x0007 |         | client |        1 |       1 => ResumeAction",
            "0x0008 |         | client |        1 |       1 => EnableAction",
            "0x0009 |         | client |        1 |       1 => EnableActionWithDuration",
            "0x000a |         | client |        1 |       1 => DisableAction",
            "0x000b |         | client |        1 |       1 => DisableActionWithDuration"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Basic Information",
          "code": "0x0028",
          "define": "BASIC_INFORMATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM | singleton |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "OTA Software Update Provider",
          "code": "0x0029",
          "define": "OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => QueryImage",
            "0x0002 |         | client |        0 |       1 => ApplyUpdateRequest",
            "0x0004 |         | client |        0 |       1 => NotifyUpdateApplied"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "OTA Software Update Requestor",
          "code": "0x002a",
          "define": "OTA_SOFTWARE_UPDATE_REQUESTOR_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => AnnounceOTAProvider"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Localization Configuration",
          "code": "0x002b",
          "define": "LOCALIZATION_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Time Format Localization",
          "code": "0x002c",
          "define": "TIME_FORMAT_LOCALIZATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Unit Localization",
          "code": "0x002d",
          "define": "UNIT_LOCALIZATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Power Source Configuration",
          "code": "0x002e",
          "define": "POWER_SOURCE_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Power Source",
          "code": "0x002f",
          "define": "POWER_SOURCE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "General Commissioning",
          "code": "0x0030",
          "define": "GENERAL_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => ArmFailSafe",
            "0x0002 |         | client |        1 |       1 => SetRegulatoryConfig",
            "0x0004 |         | client |        0 |       1 => CommissioningComplete"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Network Commissioning",
          "code": "0x0031",
          "define": "NETWORK_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => ScanNetworks",
            "0x0002 |         | client |        1 |       1 => AddOrUpdateWiFiNetwork",
            "0x0003 |         | client |        1 |       1 => AddOrUpdateThreadNetwork",
            "0x0004 |         | client |        1 |       1 => RemoveNetwork",
            "0x0006 |         | client |        1 |       1 => ConnectNetwork",
            "0x0008 |         | client |        1 |       1 => ReorderNetwork"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Diagnostic Logs",
          "code": "0x0032",
          "define": "DIAGNOSTIC_LOGS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => RetrieveLogsRequest"
          ]
        },
        {
          "name": "General Diagnostics",
          "code": "0x0033",
          "define": "GENERAL_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => TestEventTrigger"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Software Diagnostics",
          "code": "0x0034",
          "define": "SOFTWARE_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ResetWatermarks"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thread Network Diagnostics",
          "code": "0x0035",
          "define": "THREAD_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ResetCounts"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "WiFi Network Diagnostics",
          "code": "0x0036",
          "define": "WIFI_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ResetCounts"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Ethernet Network Diagnostics",
          "code": "0x0037",
          "define": "ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ResetCounts"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Time Synchronization",
          "code": "0x0038",
          "define": "TIME_SYNCHRONIZATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => SetUTCTime",
            "0x0001 |         | client |        1 |       1 => SetTrustedTimeSource",
            "0x0002 |         | client |        1 |       1 => SetTimeZone",
            "0x0004 |         | client |        1 |       1 => SetDSTOffset",
            "0x0005 |         | client |        1 |       1 => SetDefaultNTP"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Bridged Device Basic Information",
          "code": "0x0039",
          "define": "BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Switch",
          "code": "0x003b",
          "define": "SWITCH_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Administrator Commissioning",
          "code": "0x003c",
          "define": "ADMINISTRATOR_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => OpenCommissioningWindow",
            "0x0001 |         | client |        0 |       1 => OpenBasicCommissioningWindow",
            "0x0002 |         | client |        0 |       1 => RevokeCommissioning"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Operational Credentials",
          "code": "0x003e",
          "define": "OPERATIONAL_CREDENTIALS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => AttestationRequest",
            "0x0002 |         | client |        0 |       1 => CertificateChainRequest",
            "0x0004 |         | client |        0 |       1 => CSRRequest",
            "0x0006 |         | client |        0 |       1 => AddNOC",
            "0x0007 |         | client |        0 |       1 => UpdateNOC",
            "0x0009 |         | client |        0 |       1 => UpdateFabricLabel",
            "0x000a |         | client |        0 |       1 => RemoveFabric",
            "0x000b |         | client |        0 |       1 => AddTrustedRootCertificate"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Group Key Management",
          "code": "0x003f",
          "define": "GROUP_KEY_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => KeySetWrite",
            "0x0001 |         | client |        0 |       1 => KeySetRead",
            "0x0003 |         | client |        0 |       1 => KeySetRemove",
            "0x0004 |         | client |        0 |       1 => KeySetReadAllIndices"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Fixed Label",
          "code": "0x0040",
          "define": "FIXED_LABEL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "User Label",
          "code": "0x0041",
          "define": "USER_LABEL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Boolean State",
          "code": "0x0045",
          "define": "BOOLEAN_STATE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "ICD Management",
          "code": "0x0046",
          "define": "ICD_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => RegisterClient",
            "0x0002 |         | client |        0 |       1 => UnregisterClient",
            "0x0003 |         | client |        0 |       1 => StayActiveRequest"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Mode Select",
          "code": "0x0050",
          "define": "MODE_SELECT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Laundry Washer Mode",
          "code": "0x0051",
          "define": "LAUNDRY_WASHER_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Refrigerator And Temperature Controlled Cabinet Mode",
          "code": "0x0052",
          "define": "REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Laundry Washer Controls",
          "code": "0x0053",
          "define": "LAUNDRY_WASHER_CONTROLS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "RVC Run Mode",
          "code": "0x0054",
          "define": "RVC_RUN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "RVC Clean Mode",
          "code": "0x0055",
          "define": "RVC_CLEAN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Temperature Control",
          "code": "0x0056",
          "define": "TEMPERATURE_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SetTemperature"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Refrigerator Alarm",
          "code": "0x0057",
          "define": "REFRIGERATOR_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Dishwasher Mode",
          "code": "0x0059",
          "define": "DISHWASHER_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeToMode"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Air Quality",
          "code": "0x005b",
          "define": "AIR_QUALITY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Smoke CO Alarm",
          "code": "0x005c",
          "define": "SMOKE_CO_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SelfTestRequest"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Dishwasher Alarm",
          "code": "0x005d",
          "define": "DISHWASHER_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => Reset"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Operational State",
          "code": "0x0060",
          "define": "OPERATIONAL_STATE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "RVC Operational State",
          "code": "0x0061",
          "define": "OPERATIONAL_STATE_RVC_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => Pause",
            "0x0001 |         | client |        0 |       1 => Stop",
            "0x0002 |         | client |        0 |       1 => Start",
            "0x0003 |         | client |        0 |       1 => Resume"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "HEPA Filter Monitoring",
          "code": "0x0071",
          "define": "HEPA_FILTER_MONITORING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Activated Carbon Filter Monitoring",
          "code": "0x0072",
          "define": "ACTIVATED_CARBON_FILTER_MONITORING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Door Lock",
          "code": "0x0101",
          "define": "DOOR_LOCK_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => LockDoor",
            "0x0001 |         | client |        1 |       1 => UnlockDoor",
            "0x0003 |         | client |        0 |       1 => UnlockWithTimeout",
            "0x000b |         | client |        1 |       1 => SetWeekDaySchedule",
            "0x000c |         | client |        1 |       1 => GetWeekDaySchedule",
            "0x000d |         | client |        0 |       1 => ClearWeekDaySchedule",
            "0x000e |         | client |        1 |       1 => SetYearDaySchedule",
            "0x000f |         | client |        1 |       1 => GetYearDaySchedule",
            "0x0010 |         | client |        1 |       1 => ClearYearDaySchedule",
            "0x0011 |         | client |        1 |       1 => SetHolidaySchedule",
            "0x0012 |         | client |        1 |       1 => GetHolidaySchedule",
            "0x0013 |         | client |        1 |       1 => ClearHolidaySchedule",
            "0x001a |         | client |        0 |       1 => SetUser",
            "0x001b |         | client |        0 |       1 => GetUser",
            "0x001d |         | client |        0 |       1 => ClearUser",
            "0x0022 |         | client |        0 |       1 => SetCredential",
            "0x0024 |         | client |        0 |       1 => GetCredentialStatus",
            "0x0026 |         | client |        0 |       1 => ClearCredential"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Window Covering",
          "code": "0x0102",
          "define": "WINDOW_COVERING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => UpOrOpen",
            "0x0001 |         | client |        0 |       1 => DownOrClose",
            "0x0002 |         | client |        0 |       1 => StopMotion",
            "0x0004 |         | client |        0 |       1 => GoToLiftValue",
            "0x0005 |         | client |        0 |       1 => GoToLiftPercentage",
            "0x0007 |         | client |        0 |       1 => GoToTiltValue",
            "0x0008 |         | client |        0 |       1 => GoToTiltPercentage"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Barrier Control",
          "code": "0x0103",
          "define": "BARRIER_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => BarrierControlGoToPercent",
            "0x0001 |         | client |        1 |       1 => BarrierControlStop"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Pump Configuration and Control",
          "code": "0x0200",
          "define": "PUMP_CONFIGURATION_AND_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thermostat",
          "code": "0x0201",
          "define": "THERMOSTAT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SetpointRaiseLower",
            "0x0001 |         | client |        0 |       1 => SetWeeklySchedule",
            "0x0002 |         | client |        0 |       1 => GetWeeklySchedule",
            "0x0003 |         | client |        0 |       1 => ClearWeeklySchedule"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Fan Control",
          "code": "0x0202",
          "define": "FAN_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Thermostat User Interface Configuration",
          "code": "0x0204",
          "define": "THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Color Control",
          "code": "0x0300",
          "define": "COLOR_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => MoveToHue",
            "0x0001 |         | client |        1 |       1 => MoveHue",
            "0x0002 |         | client |        1 |       1 => StepHue",
            "0x0003 |         | client |        1 |       1 => MoveToSaturation",
            "0x0004 |         | client |        1 |       1 => MoveSaturation",
            "0x0005 |         | client |        1 |       1 => StepSaturation",
            "0x0006 |         | client |        1 |       1 => MoveToHueAndSaturation",
            "0x0007 |         | client |        1 |       1 => MoveToColor",
            "0x0008 |         | client |        1 |       1 => MoveColor",
            "0x0009 |         | client |        1 |       1 => StepColor",
            "0x000a |         | client |        1 |       1 => MoveToColorTemperature",
            "0x0040 |         | client |        0 |       1 => EnhancedMoveToHue",
            "0x0041 |         | client |        0 |       1 => EnhancedMoveHue",
            "0x0042 |         | client |        0 |       1 => EnhancedStepHue",
            "0x0043 |         | client |        0 |       1 => EnhancedMoveToHueAndSaturation",
            "0x0044 |         | client |        0 |       1 => ColorLoopSet",
            "0x0047 |         | client |        1 |       1 => StopMoveStep",
            "0x004b |         | client |        1 |       1 => MoveColorTemperature",
            "0x004c |         | client |        1 |       1 => StepColorTemperature"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Ballast Configuration",
          "code": "0x0301",
          "define": "BALLAST_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Illuminance Measurement",
          "code": "0x0400",
          "define": "ILLUMINANCE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Temperature Measurement",
          "code": "0x0402",
          "define": "TEMPERATURE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Pressure Measurement",
          "code": "0x0403",
          "define": "PRESSURE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Flow Measurement",
          "code": "0x0404",
          "define": "FLOW_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Relative Humidity Measurement",
          "code": "0x0405",
          "define": "RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Occupancy Sensing",
          "code": "0x0406",
          "define": "OCCUPANCY_SENSING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Carbon Monoxide Concentration Measurement",
          "code": "0x040c",
          "define": "CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Carbon Dioxide Concentration Measurement",
          "code": "0x040d",
          "define": "CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Nitrogen Dioxide Concentration Measurement",
          "code": "0x0413",
          "define": "NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Ozone Concentration Measurement",
          "code": "0x0415",
          "define": "OZONE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM2.5 Concentration Measurement",
          "code": "0x042a",
          "define": "PM2_5_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Formaldehyde Concentration Measurement",
          "code": "0x042b",
          "define": "FORMALDEHYDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM1 Concentration Measurement",
          "code": "0x042c",
          "define": "PM1_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM10 Concentration Measurement",
          "code": "0x042d",
          "define": "PM10_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Total Volatile Organic Compounds Concentration Measurement",
          "code": "0x042e",
          "define": "TVOC_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Radon Concentration Measurement",
          "code": "0x042f",
          "define": "RADON_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Wake on LAN",
          "code": "0x0503",
          "define": "WAKE_ON_LAN_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Channel",
          "code": "0x0504",
          "define": "CHANNEL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => ChangeChannel",
            "0x0002 |         | client |        0 |       1 => ChangeChannelByNumber",
            "0x0003 |         | client |        0 |       1 => SkipChannel"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Target Navigator",
          "code": "0x0505",
          "define": "TARGET_NAVIGATOR_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => NavigateTarget"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Media Playback",
          "code": "0x0506",
          "define": "MEDIA_PLAYBACK_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => Play",
            "0x0001 |         | client |        0 |       1 => Pause",
            "0x0002 |         | client |        0 |       1 => Stop",
            "0x0003 |         | client |        0 |       1 => StartOver",
            "0x0004 |         | client |        0 |       1 => Previous",
            "0x0005 |         | client |        0 |       1 => Next",
            "0x0006 |         | client |        0 |       1 => Rewind",
            "0x0007 |         | client |        0 |       1 => FastForward",
            "0x0008 |         | client |        0 |       1 => SkipForward",
            "0x0009 |         | client |        0 |       1 => SkipBackward",
            "0x000b |         | client |        0 |       1 => Seek"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Media Input",
          "code": "0x0507",
          "define": "MEDIA_INPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SelectInput",
            "0x0001 |         | client |        0 |       1 => ShowInputStatus",
            "0x0002 |         | client |        0 |       1 => HideInputStatus",
            "0x0003 |         | client |        0 |       1 => RenameInput"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Low Power",
          "code": "0x0508",
          "define": "LOW_POWER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => Sleep"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Keypad Input",
          "code": "0x0509",
          "define": "KEYPAD_INPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SendKey"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Content Launcher",
          "code": "0x050a",
          "define": "CONTENT_LAUNCHER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => LaunchContent",
            "0x0001 |         | client |        0 |       1 => LaunchURL"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Audio Output",
          "code": "0x050b",
          "define": "AUDIO_OUTPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => SelectOutput",
            "0x0001 |         | client |        0 |       1 => RenameOutput"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Application Launcher",
          "code": "0x050c",
          "define": "APPLICATION_LAUNCHER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => LaunchApp",
            "0x0001 |         | client |        0 |       1 => StopApp",
            "0x0002 |         | client |        0 |       1 => HideApp"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Application Basic",
          "code": "0x050d",
          "define": "APPLICATION_BASIC_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Account Login",
          "code": "0x050e",
          "define": "ACCOUNT_LOGIN_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => GetSetupPIN",
            "0x0002 |         | client |        0 |       1 => Login",
            "0x0003 |         | client |        0 |       1 => Logout"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Electrical Measurement",
          "code": "0x0b04",
          "define": "ELECTRICAL_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Unit Testing",
          "code": "0xfff1fc05",
          "define": "UNIT_TESTING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        0 |       1 => Test",
            "0x0001 |         | client |        0 |       1 => TestNotHandled",
            "0x0002 |         | client |        0 |       1 => TestSpecific",
            "0x0003 |         | client |        0 |       1 => TestUnknownCommand",
            "0x0004 |         | client |        0 |       1 => TestAddArguments",
            "0x0007 |         | client |        0 |       1 => TestStructArgumentRequest",
            "0x0008 |         | client |        0 |       1 => TestNestedStructArgumentRequest",
            "0x0009 |         | client |        0 |       1 => TestListStructArgumentRequest",
            "0x000a |         | client |        0 |       1 => TestListInt8UArgumentRequest",
            "0x000b |         | client |        0 |       1 => TestNestedStructListArgumentRequest",
            "0x000c |         | client |        0 |       1 => TestListNestedStructListArgumentRequest",
            "0x000d |         | client |        0 |       1 => TestListInt8UReverseRequest",
            "0x000e |         | client |        0 |       1 => TestEnumsRequest",
            "0x000f |         | client |        0 |       1 => TestNullableOptionalRequest",
            "0x0011 |         | client |        0 |       1 => SimpleStructEchoRequest",
            "0x0012 |         | client |        0 |       1 => TimedInvokeRequest",
            "0x0013 |         | client |        0 |       1 => TestSimpleOptionalArgumentRequest",
            "0x0014 |         | client |        0 |       1 => TestEmitTestEventRequest"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    }
  ],
  "endpoints": [
    {
      "endpointTypeName": "MA-rootdevice",
      "endpointTypeIndex": 0,
      "profileId": 259,
      "endpointId": 1,
      "networkId": 0
    }
  ]
}