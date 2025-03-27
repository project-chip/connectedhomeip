{
  "fileFormat": 2,
  "featureLevel": 106,
  "creator": "zap",
  "keyValuePairs": [
    {
      "key": "commandDiscovery",
      "value": "1"
    },
    {
      "key": "defaultResponsePolicy",
      "value": "always"
    },
    {
      "key": "manufacturerCodes",
      "value": "0x1002"
    }
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
      "category": "matter",
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
        "name": "MA-rootdevice",
        "deviceTypeOrder": 0
      },
      "deviceTypes": [
        {
          "code": 22,
          "profileId": 259,
          "label": "MA-rootdevice",
          "name": "MA-rootdevice",
          "deviceTypeOrder": 0
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
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Identify",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TriggerEffect",
              "code": 64,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Groups",
          "code": 4,
          "mfgCode": null,
          "define": "GROUPS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "AddGroup",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddGroupResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ViewGroup",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ViewGroupResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "GetGroupMembership",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetGroupMembershipResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveGroup",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveGroupResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveAllGroups",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddGroupIfIdentifying",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "On/Off",
          "code": 6,
          "mfgCode": null,
          "define": "ON_OFF_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Off",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "On",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Toggle",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OffWithEffect",
              "code": 64,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OnWithRecallGlobalScene",
              "code": 65,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OnWithTimedOff",
              "code": 66,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Level Control",
          "code": 8,
          "mfgCode": null,
          "define": "LEVEL_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "MoveToLevel",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Move",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Step",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Stop",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveToLevelWithOnOff",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveWithOnOff",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StepWithOnOff",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StopWithOnOff",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Descriptor",
          "code": 29,
          "mfgCode": null,
          "define": "DESCRIPTOR_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Binding",
          "code": 30,
          "mfgCode": null,
          "define": "BINDING_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Access Control",
          "code": 31,
          "mfgCode": null,
          "define": "ACCESS_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Actions",
          "code": 37,
          "mfgCode": null,
          "define": "ACTIONS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "InstantAction",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "InstantActionWithTransition",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StartAction",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StartActionWithDuration",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StopAction",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "PauseAction",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "PauseActionWithDuration",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ResumeAction",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnableAction",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnableActionWithDuration",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DisableAction",
              "code": 10,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DisableActionWithDuration",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Basic Information",
          "code": 40,
          "mfgCode": null,
          "define": "BASIC_INFORMATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "OTA Software Update Provider",
          "code": 41,
          "mfgCode": null,
          "define": "OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "QueryImage",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "QueryImageResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ApplyUpdateRequest",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ApplyUpdateResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "NotifyUpdateApplied",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "OTA Software Update Requestor",
          "code": 42,
          "mfgCode": null,
          "define": "OTA_SOFTWARE_UPDATE_REQUESTOR_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "AnnounceOTAProvider",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Localization Configuration",
          "code": 43,
          "mfgCode": null,
          "define": "LOCALIZATION_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Time Format Localization",
          "code": 44,
          "mfgCode": null,
          "define": "TIME_FORMAT_LOCALIZATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Unit Localization",
          "code": 45,
          "mfgCode": null,
          "define": "UNIT_LOCALIZATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Power Source Configuration",
          "code": 46,
          "mfgCode": null,
          "define": "POWER_SOURCE_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Power Source",
          "code": 47,
          "mfgCode": null,
          "define": "POWER_SOURCE_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "General Commissioning",
          "code": 48,
          "mfgCode": null,
          "define": "GENERAL_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ArmFailSafe",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ArmFailSafeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetRegulatoryConfig",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetRegulatoryConfigResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CommissioningComplete",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CommissioningCompleteResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Network Commissioning",
          "code": 49,
          "mfgCode": null,
          "define": "NETWORK_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ScanNetworks",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ScanNetworksResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddOrUpdateWiFiNetwork",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddOrUpdateThreadNetwork",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveNetwork",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "NetworkConfigResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ConnectNetwork",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ConnectNetworkResponse",
              "code": 7,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ReorderNetwork",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Diagnostic Logs",
          "code": 50,
          "mfgCode": null,
          "define": "DIAGNOSTIC_LOGS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "RetrieveLogsRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RetrieveLogsResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "General Diagnostics",
          "code": 51,
          "mfgCode": null,
          "define": "GENERAL_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "TestEventTrigger",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Software Diagnostics",
          "code": 52,
          "mfgCode": null,
          "define": "SOFTWARE_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetWatermarks",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Thread Network Diagnostics",
          "code": 53,
          "mfgCode": null,
          "define": "THREAD_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetCounts",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Wi-Fi Network Diagnostics",
          "code": 54,
          "mfgCode": null,
          "define": "WIFI_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetCounts",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Ethernet Network Diagnostics",
          "code": 55,
          "mfgCode": null,
          "define": "ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ResetCounts",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Time Synchronization",
          "code": 56,
          "mfgCode": null,
          "define": "TIME_SYNCHRONIZATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SetUTCTime",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetTrustedTimeSource",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetTimeZone",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetTimeZoneResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetDSTOffset",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetDefaultNTP",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Bridged Device Basic Information",
          "code": 57,
          "mfgCode": null,
          "define": "BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Switch",
          "code": 59,
          "mfgCode": null,
          "define": "SWITCH_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Administrator Commissioning",
          "code": 60,
          "mfgCode": null,
          "define": "ADMINISTRATOR_COMMISSIONING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "OpenCommissioningWindow",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OpenBasicCommissioningWindow",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RevokeCommissioning",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Operational Credentials",
          "code": 62,
          "mfgCode": null,
          "define": "OPERATIONAL_CREDENTIALS_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "AttestationRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AttestationResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CertificateChainRequest",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CertificateChainResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CSRRequest",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CSRResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddNOC",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UpdateNOC",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "NOCResponse",
              "code": 8,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UpdateFabricLabel",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveFabric",
              "code": 10,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddTrustedRootCertificate",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Group Key Management",
          "code": 63,
          "mfgCode": null,
          "define": "GROUP_KEY_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "KeySetWrite",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "KeySetRead",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "KeySetReadResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "KeySetRemove",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "KeySetReadAllIndices",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "KeySetReadAllIndicesResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Fixed Label",
          "code": 64,
          "mfgCode": null,
          "define": "FIXED_LABEL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "User Label",
          "code": 65,
          "mfgCode": null,
          "define": "USER_LABEL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Boolean State",
          "code": 69,
          "mfgCode": null,
          "define": "BOOLEAN_STATE_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "ICD Management",
          "code": 70,
          "mfgCode": null,
          "define": "ICD_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "RegisterClient",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RegisterClientResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "UnregisterClient",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StayActiveRequest",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StayActiveResponse",
              "code": 4,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Timer",
          "code": 71,
          "mfgCode": null,
          "define": "TIMER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "SetTimer",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddTime",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Oven Cavity Operational State",
          "code": 72,
          "mfgCode": null,
          "define": "OPERATIONAL_STATE_OVEN_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Oven Mode",
          "code": 73,
          "mfgCode": null,
          "define": "OVEN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Laundry Dryer Controls",
          "code": 74,
          "mfgCode": null,
          "define": "LAUNDRY_DRYER_CONTROLS_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Mode Select",
          "code": 80,
          "mfgCode": null,
          "define": "MODE_SELECT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Laundry Washer Mode",
          "code": 81,
          "mfgCode": null,
          "define": "LAUNDRY_WASHER_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Refrigerator And Temperature Controlled Cabinet Mode",
          "code": 82,
          "mfgCode": null,
          "define": "REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Laundry Washer Controls",
          "code": 83,
          "mfgCode": null,
          "define": "LAUNDRY_WASHER_CONTROLS_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "RVC Run Mode",
          "code": 84,
          "mfgCode": null,
          "define": "RVC_RUN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "RVC Clean Mode",
          "code": 85,
          "mfgCode": null,
          "define": "RVC_CLEAN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Temperature Control",
          "code": 86,
          "mfgCode": null,
          "define": "TEMPERATURE_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SetTemperature",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Refrigerator Alarm",
          "code": 87,
          "mfgCode": null,
          "define": "REFRIGERATOR_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Dishwasher Mode",
          "code": 89,
          "mfgCode": null,
          "define": "DISHWASHER_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Air Quality",
          "code": 91,
          "mfgCode": null,
          "define": "AIR_QUALITY_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Smoke CO Alarm",
          "code": 92,
          "mfgCode": null,
          "define": "SMOKE_CO_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SelfTestRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Dishwasher Alarm",
          "code": 93,
          "mfgCode": null,
          "define": "DISHWASHER_ALARM_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Reset",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Microwave Oven Mode",
          "code": 94,
          "mfgCode": null,
          "define": "MICROWAVE_OVEN_MODE_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Microwave Oven Control",
          "code": 95,
          "mfgCode": null,
          "define": "MICROWAVE_OVEN_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SetCookingParameters",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Operational State",
          "code": 96,
          "mfgCode": null,
          "define": "OPERATIONAL_STATE_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "RVC Operational State",
          "code": 97,
          "mfgCode": null,
          "define": "OPERATIONAL_STATE_RVC_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Pause",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Resume",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OperationalCommandResponse",
              "code": 4,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Scenes Management",
          "code": 98,
          "mfgCode": null,
          "define": "SCENES_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "AddScene",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddSceneResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ViewScene",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ViewSceneResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveScene",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveSceneResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveAllScenes",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveAllScenesResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "StoreScene",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StoreSceneResponse",
              "code": 4,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RecallScene",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetSceneMembership",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetSceneMembershipResponse",
              "code": 6,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "HEPA Filter Monitoring",
          "code": 113,
          "mfgCode": null,
          "define": "HEPA_FILTER_MONITORING_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Activated Carbon Filter Monitoring",
          "code": 114,
          "mfgCode": null,
          "define": "ACTIVATED_CARBON_FILTER_MONITORING_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Electrical Power Measurement",
          "code": 144,
          "mfgCode": null,
          "define": "ELECTRICAL_POWER_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Electrical Energy Measurement",
          "code": 145,
          "mfgCode": null,
          "define": "ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Water Heater Management",
          "code": 148,
          "mfgCode": null,
          "define": "WATER_HEATER_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "Boost",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CancelBoost",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Device Energy Management",
          "code": 152,
          "mfgCode": null,
          "define": "DEVICE_ENERGY_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional"
        },
        {
          "name": "Energy EVSE",
          "code": 153,
          "mfgCode": null,
          "define": "ENERGY_EVSE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Disable",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnableCharging",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Power Topology",
          "code": 156,
          "mfgCode": null,
          "define": "POWER_TOPOLOGY_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Energy EVSE Mode",
          "code": 157,
          "mfgCode": null,
          "define": "ENERGY_EVSE_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Water Heater Mode",
          "code": 158,
          "mfgCode": null,
          "define": "WATER_HEATER_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Device Energy Management Mode",
          "code": 159,
          "mfgCode": null,
          "define": "DEVICE_ENERGY_MANAGEMENT_MODE_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "ChangeToMode",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeToModeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Door Lock",
          "code": 257,
          "mfgCode": null,
          "define": "DOOR_LOCK_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "LockDoor",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UnlockDoor",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UnlockWithTimeout",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetWeekDaySchedule",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetWeekDaySchedule",
              "code": 12,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetWeekDayScheduleResponse",
              "code": 12,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ClearWeekDaySchedule",
              "code": 13,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetYearDaySchedule",
              "code": 14,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetYearDaySchedule",
              "code": 15,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetYearDayScheduleResponse",
              "code": 15,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ClearYearDaySchedule",
              "code": 16,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetHolidaySchedule",
              "code": 17,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetHolidaySchedule",
              "code": 18,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetHolidayScheduleResponse",
              "code": 18,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ClearHolidaySchedule",
              "code": 19,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetUser",
              "code": 26,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetUser",
              "code": 27,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetUserResponse",
              "code": 28,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ClearUser",
              "code": 29,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetCredential",
              "code": 34,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetCredentialResponse",
              "code": 35,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "GetCredentialStatus",
              "code": 36,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetCredentialStatusResponse",
              "code": 37,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ClearCredential",
              "code": 38,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Window Covering",
          "code": 258,
          "mfgCode": null,
          "define": "WINDOW_COVERING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "UpOrOpen",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "DownOrClose",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StopMotion",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GoToLiftValue",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GoToLiftPercentage",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GoToTiltValue",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GoToTiltPercentage",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Pump Configuration and Control",
          "code": 512,
          "mfgCode": null,
          "define": "PUMP_CONFIGURATION_AND_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Thermostat",
          "code": 513,
          "mfgCode": null,
          "define": "THERMOSTAT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SetpointRaiseLower",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetWeeklyScheduleResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetWeeklySchedule",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetWeeklySchedule",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ClearWeeklySchedule",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Fan Control",
          "code": 514,
          "mfgCode": null,
          "define": "FAN_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Thermostat User Interface Configuration",
          "code": 516,
          "mfgCode": null,
          "define": "THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Color Control",
          "code": 768,
          "mfgCode": null,
          "define": "COLOR_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "MoveToHue",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveHue",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StepHue",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveToSaturation",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveSaturation",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StepSaturation",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveToHueAndSaturation",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveToColor",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveColor",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StepColor",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveToColorTemperature",
              "code": 10,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnhancedMoveToHue",
              "code": 64,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnhancedMoveHue",
              "code": 65,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnhancedStepHue",
              "code": 66,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "EnhancedMoveToHueAndSaturation",
              "code": 67,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ColorLoopSet",
              "code": 68,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StopMoveStep",
              "code": 71,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "MoveColorTemperature",
              "code": 75,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StepColorTemperature",
              "code": 76,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Ballast Configuration",
          "code": 769,
          "mfgCode": null,
          "define": "BALLAST_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional"
        },
        {
          "name": "Illuminance Measurement",
          "code": 1024,
          "mfgCode": null,
          "define": "ILLUMINANCE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Temperature Measurement",
          "code": 1026,
          "mfgCode": null,
          "define": "TEMPERATURE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Pressure Measurement",
          "code": 1027,
          "mfgCode": null,
          "define": "PRESSURE_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Flow Measurement",
          "code": 1028,
          "mfgCode": null,
          "define": "FLOW_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Relative Humidity Measurement",
          "code": 1029,
          "mfgCode": null,
          "define": "RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Occupancy Sensing",
          "code": 1030,
          "mfgCode": null,
          "define": "OCCUPANCY_SENSING_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Carbon Monoxide Concentration Measurement",
          "code": 1036,
          "mfgCode": null,
          "define": "CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Carbon Dioxide Concentration Measurement",
          "code": 1037,
          "mfgCode": null,
          "define": "CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Nitrogen Dioxide Concentration Measurement",
          "code": 1043,
          "mfgCode": null,
          "define": "NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Ozone Concentration Measurement",
          "code": 1045,
          "mfgCode": null,
          "define": "OZONE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "PM2.5 Concentration Measurement",
          "code": 1066,
          "mfgCode": null,
          "define": "PM2_5_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Formaldehyde Concentration Measurement",
          "code": 1067,
          "mfgCode": null,
          "define": "FORMALDEHYDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "PM1 Concentration Measurement",
          "code": 1068,
          "mfgCode": null,
          "define": "PM1_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "PM10 Concentration Measurement",
          "code": 1069,
          "mfgCode": null,
          "define": "PM10_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Total Volatile Organic Compounds Concentration Measurement",
          "code": 1070,
          "mfgCode": null,
          "define": "TVOC_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Radon Concentration Measurement",
          "code": 1071,
          "mfgCode": null,
          "define": "RADON_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Thread Border Router Management",
          "code": 1106,
          "mfgCode": null,
          "define": "THREAD_BORDER_ROUTER_MANAGEMENT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "GetActiveDatasetRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetPendingDatasetRequest",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetPendingDatasetRequest",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Thread Network Directory",
          "code": 1107,
          "mfgCode": null,
          "define": "THREAD_NETWORK_DIRECTORY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "AddNetwork",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveNetwork",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetOperationalDataset",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "OperationalDatasetResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Wake on LAN",
          "code": 1283,
          "mfgCode": null,
          "define": "WAKE_ON_LAN_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Channel",
          "code": 1284,
          "mfgCode": null,
          "define": "CHANNEL_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "ChangeChannel",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ChangeChannelResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ChangeChannelByNumber",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SkipChannel",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Target Navigator",
          "code": 1285,
          "mfgCode": null,
          "define": "TARGET_NAVIGATOR_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "NavigateTarget",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "NavigateTargetResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Media Playback",
          "code": 1286,
          "mfgCode": null,
          "define": "MEDIA_PLAYBACK_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Play",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Pause",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Stop",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StartOver",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Previous",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Next",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Rewind",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "FastForward",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SkipForward",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SkipBackward",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "PlaybackResponse",
              "code": 10,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "Seek",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Media Input",
          "code": 1287,
          "mfgCode": null,
          "define": "MEDIA_INPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SelectInput",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ShowInputStatus",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "HideInputStatus",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RenameInput",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Low Power",
          "code": 1288,
          "mfgCode": null,
          "define": "LOW_POWER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Sleep",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Keypad Input",
          "code": 1289,
          "mfgCode": null,
          "define": "KEYPAD_INPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SendKey",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SendKeyResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Content Launcher",
          "code": 1290,
          "mfgCode": null,
          "define": "CONTENT_LAUNCHER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "LaunchContent",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "LaunchURL",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "LauncherResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Audio Output",
          "code": 1291,
          "mfgCode": null,
          "define": "AUDIO_OUTPUT_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "SelectOutput",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RenameOutput",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Application Launcher",
          "code": 1292,
          "mfgCode": null,
          "define": "APPLICATION_LAUNCHER_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "LaunchApp",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "StopApp",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "HideApp",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "LauncherResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Application Basic",
          "code": 1293,
          "mfgCode": null,
          "define": "APPLICATION_BASIC_CLUSTER",
          "side": "client",
          "enabled": 1
        },
        {
          "name": "Account Login",
          "code": 1294,
          "mfgCode": null,
          "define": "ACCOUNT_LOGIN_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "GetSetupPIN",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "GetSetupPINResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "Login",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "Logout",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Chime",
          "code": 1366,
          "mfgCode": null,
          "define": "CHIME_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "provisional",
          "commands": [
            {
              "name": "PlayChimeSound",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Unit Testing",
          "code": 4294048773,
          "mfgCode": null,
          "define": "UNIT_TESTING_CLUSTER",
          "side": "client",
          "enabled": 1,
          "apiMaturity": "internal",
          "commands": [
            {
              "name": "Test",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestSpecificResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestNotHandled",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestAddArgumentsResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestSpecific",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestUnknownCommand",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestAddArguments",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestListInt8UReverseResponse",
              "code": 4,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestEnumsResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestNullableOptionalResponse",
              "code": 6,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestStructArgumentRequest",
              "code": 7,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestNestedStructArgumentRequest",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "BooleanResponse",
              "code": 8,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestListStructArgumentRequest",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SimpleStructResponse",
              "code": 9,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestListInt8UArgumentRequest",
              "code": 10,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestEmitTestEventResponse",
              "code": 10,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "TestNestedStructListArgumentRequest",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestListNestedStructListArgumentRequest",
              "code": 12,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestListInt8UReverseRequest",
              "code": 13,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestEnumsRequest",
              "code": 14,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestNullableOptionalRequest",
              "code": 15,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SimpleStructEchoRequest",
              "code": 17,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TimedInvokeRequest",
              "code": 18,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestSimpleOptionalArgumentRequest",
              "code": 19,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "TestEmitTestEventRequest",
              "code": 20,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ]
        },
        {
          "name": "Sample MEI",
          "code": 4294048800,
          "mfgCode": null,
          "define": "SAMPLE_MEI_CLUSTER",
          "side": "client",
          "enabled": 1,
          "commands": [
            {
              "name": "Ping",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddArgumentsResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddArguments",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 0,
              "isEnabled": 1
            }
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
      "networkId": 0,
      "parentEndpointIdentifier": null
    }
  ]
}