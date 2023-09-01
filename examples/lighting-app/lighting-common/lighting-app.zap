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
      "path": "../../../src/app/zap-templates/zcl/zcl.json",
      "type": "zcl-properties",
      "category": "matter",
      "version": 1,
      "description": "Matter SDK ZCL data"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../src/app/zap-templates/app-templates.json",
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
          "name": "Descriptor",
          "code": "0x001d",
          "define": "DESCRIPTOR_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PartsList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Access Control",
          "code": "0x001f",
          "define": "ACCESS_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ACL [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Extension [array]",
            "    0x0002 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => SubjectsPerAccessControlEntry [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => TargetsPerAccessControlEntry [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => AccessControlEntriesPerFabric [int16u]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => AccessControlEntryChanged",
            "0x0001 |         | server => AccessControlExtensionChanged"
          ]
        },
        {
          "name": "Basic Information",
          "code": "0x0028",
          "define": "BASIC_INFORMATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext | singleton |         |                   10 |          1 |                     |               65344 |                     => DataModelRevision [int16u]",
            "    0x0001 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => VendorName [char_string]",
            "    0x0002 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => VendorID [vendor_id]",
            "    0x0003 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => ProductName [char_string]",
            "    0x0004 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => ProductID [int16u]",
            "    0x0005 |         | server |           NVM | singleton |         |                      |          1 |                     |               65344 |                     => NodeLabel [char_string]",
            "    0x0006 |         | server |           Ext | singleton |         |                   XX |          1 |                     |               65344 |                     => Location [char_string]",
            "    0x0007 |         | server |           Ext | singleton |         |                    0 |          1 |                     |               65344 |                     => HardwareVersion [int16u]",
            "    0x0008 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => HardwareVersionString [char_string]",
            "    0x0009 |         | server |           Ext | singleton |         |                    0 |          1 |                     |               65344 |                     => SoftwareVersion [int32u]",
            "    0x000a |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => SoftwareVersionString [char_string]",
            "    0x000b |         | server |           Ext | singleton |         |     20210614123456ZZ |          1 |                     |               65344 |                     => ManufacturingDate [char_string]",
            "    0x000c |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => PartNumber [char_string]",
            "    0x000d |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => ProductURL [long_char_string]",
            "    0x000e |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => ProductLabel [char_string]",
            "    0x000f |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => SerialNumber [char_string]",
            "    0x0010 |         | server |           NVM | singleton |         |                    0 |          1 |                     |               65344 |                     => LocalConfigDisabled [boolean]",
            "    0x0012 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => UniqueID [char_string]",
            "    0x0013 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CapabilityMinima [CapabilityMinimaStruct]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM | singleton |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => StartUp",
            "0x0001 |         | server => ShutDown",
            "0x0002 |         | server => Leave"
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
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => DefaultOTAProviders [array]",
            "    0x0001 |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => UpdatePossible [boolean]",
            "    0x0002 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => UpdateState [OTAUpdateStateEnum]",
            "    0x0003 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => UpdateStateProgress [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => StateTransition",
            "0x0001 |         | server => VersionApplied",
            "0x0002 |         | server => DownloadError"
          ]
        },
        {
          "name": "Localization Configuration",
          "code": "0x002b",
          "define": "LOCALIZATION_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                en-US |          1 |                   1 |               65534 |                     => ActiveLocale [char_string]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedLocales [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Time Format Localization",
          "code": "0x002c",
          "define": "TIME_FORMAT_LOCALIZATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => HourFormat [HourFormatEnum]",
            "    0x0001 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => ActiveCalendarType [CalendarTypeEnum]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedCalendarTypes [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "General Commissioning",
          "code": "0x0030",
          "define": "GENERAL_COMMISSIONING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ArmFailSafeResponse",
            "0x0003 |         | server |        0 |       1 => SetRegulatoryConfigResponse",
            "0x0005 |         | server |        1 |       1 => CommissioningCompleteResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => Breadcrumb [int64u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => BasicCommissioningInfo [BasicCommissioningInfo]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => RegulatoryConfig [RegulatoryLocationTypeEnum]",
            "    0x0003 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LocationCapability [RegulatoryLocationTypeEnum]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => SupportsConcurrentConnection [boolean]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Network Commissioning",
          "code": "0x0031",
          "define": "NETWORK_COMMISSIONING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ScanNetworksResponse",
            "0x0005 |         | server |        0 |       1 => NetworkConfigResponse",
            "0x0007 |         | server |        0 |       1 => ConnectNetworkResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxNetworks [int8u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Networks [array]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ScanMaxTimeSeconds [int8u]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ConnectMaxTimeSeconds [int8u]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => InterfaceEnabled [boolean]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LastNetworkingStatus [NetworkCommissioningStatusEnum]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LastNetworkID [octet_string]",
            "    0x0007 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LastConnectErrorValue [int32s]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Diagnostic Logs",
          "code": "0x0032",
          "define": "DIAGNOSTIC_LOGS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       0 => RetrieveLogsRequest"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "General Diagnostics",
          "code": "0x0033",
          "define": "GENERAL_DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       0 => TestEventTrigger"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => NetworkInterfaces [array]",
            "    0x0001 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RebootCount [int16u]",
            "    0x0002 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => UpTime [int64u]",
            "    0x0003 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => TotalOperationalHours [int32u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => BootReason [BootReasonEnum]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ActiveHardwareFaults [array]",
            "    0x0006 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ActiveRadioFaults [array]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ActiveNetworkFaults [array]",
            "    0x0008 |         | server |           Ext |           |         |                false |          1 |                   1 |               65534 |                     => TestEventTriggersEnabled [boolean]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => HardwareFaultChange",
            "0x0001 |         | server => RadioFaultChange",
            "0x0002 |         | server => NetworkFaultChange",
            "0x0003 |         | server => BootReason"
          ]
        },
        {
          "name": "Software Diagnostics",
          "code": "0x0034",
          "define": "SOFTWARE_DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ThreadMetrics [array]",
            "    0x0001 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => CurrentHeapFree [int64u]",
            "    0x0002 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => CurrentHeapUsed [int64u]",
            "    0x0003 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => CurrentHeapHighWatermark [int64u]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thread Network Diagnostics",
          "code": "0x0035",
          "define": "THREAD_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Channel [int16u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => RoutingRole [RoutingRoleEnum]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => NetworkName [char_string]",
            "    0x0003 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => PanId [int16u]",
            "    0x0004 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => ExtendedPanId [int64u]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => MeshLocalPrefix [octet_string]",
            "    0x0006 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => OverrunCount [int64u]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => NeighborTable [array]",
            "    0x0008 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => RouteTable [array]",
            "    0x0009 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartitionId [int32u]",
            "    0x000a |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Weighting [int8u]",
            "    0x000b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DataVersion [int8u]",
            "    0x000c |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => StableDataVersion [int8u]",
            "    0x000d |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => LeaderRouterId [int8u]",
            "    0x000e |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => DetachedRoleCount [int16u]",
            "    0x000f |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => ChildRoleCount [int16u]",
            "    0x0010 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RouterRoleCount [int16u]",
            "    0x0011 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => LeaderRoleCount [int16u]",
            "    0x0012 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => AttachAttemptCount [int16u]",
            "    0x0013 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => PartitionIdChangeCount [int16u]",
            "    0x0014 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => BetterPartitionAttachAttemptCount [int16u]",
            "    0x0015 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => ParentChangeCount [int16u]",
            "    0x0016 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxTotalCount [int32u]",
            "    0x0017 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxUnicastCount [int32u]",
            "    0x0018 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxBroadcastCount [int32u]",
            "    0x0019 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxAckRequestedCount [int32u]",
            "    0x001a |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxAckedCount [int32u]",
            "    0x001b |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxNoAckRequestedCount [int32u]",
            "    0x001c |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxDataCount [int32u]",
            "    0x001d |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxDataPollCount [int32u]",
            "    0x001e |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxBeaconCount [int32u]",
            "    0x001f |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxBeaconRequestCount [int32u]",
            "    0x0020 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxOtherCount [int32u]",
            "    0x0021 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxRetryCount [int32u]",
            "    0x0022 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxDirectMaxRetryExpiryCount [int32u]",
            "    0x0023 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxIndirectMaxRetryExpiryCount [int32u]",
            "    0x0024 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxErrCcaCount [int32u]",
            "    0x0025 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxErrAbortCount [int32u]",
            "    0x0026 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => TxErrBusyChannelCount [int32u]",
            "    0x0027 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxTotalCount [int32u]",
            "    0x0028 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxUnicastCount [int32u]",
            "    0x0029 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxBroadcastCount [int32u]",
            "    0x002a |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxDataCount [int32u]",
            "    0x002b |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxDataPollCount [int32u]",
            "    0x002c |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxBeaconCount [int32u]",
            "    0x002d |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxBeaconRequestCount [int32u]",
            "    0x002e |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxOtherCount [int32u]",
            "    0x002f |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxAddressFilteredCount [int32u]",
            "    0x0030 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxDestAddrFilteredCount [int32u]",
            "    0x0031 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxDuplicatedCount [int32u]",
            "    0x0032 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrNoFrameCount [int32u]",
            "    0x0033 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrUnknownNeighborCount [int32u]",
            "    0x0034 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrInvalidSrcAddrCount [int32u]",
            "    0x0035 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrSecCount [int32u]",
            "    0x0036 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrFcsCount [int32u]",
            "    0x0037 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RxErrOtherCount [int32u]",
            "    0x0038 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => ActiveTimestamp [int64u]",
            "    0x0039 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => PendingTimestamp [int64u]",
            "    0x003a |         | server |           Ext |           |         |               0x0000 |          1 |                   1 |               65534 |                     => Delay [int32u]",
            "    0x003b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SecurityPolicy [SecurityPolicy]",
            "    0x003c |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => ChannelPage0Mask [octet_string]",
            "    0x003d |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OperationalDatasetComponents [OperationalDatasetComponents]",
            "    0x003e |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveNetworkFaultsList [array]",
            "    0xfffc |         | server |           RAM |           |         |               0x000F |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "WiFi Network Diagnostics",
          "code": "0x0036",
          "define": "WIFI_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => BSSID [octet_string]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SecurityType [SecurityTypeEnum]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => WiFiVersion [WiFiVersionEnum]",
            "    0x0003 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => ChannelNumber [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => RSSI [int8s]",
            "    0x0005 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => BeaconLostCount [int32u]",
            "    0x0006 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => BeaconRxCount [int32u]",
            "    0x0007 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => PacketMulticastRxCount [int32u]",
            "    0x0008 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => PacketMulticastTxCount [int32u]",
            "    0x0009 |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => PacketUnicastRxCount [int32u]",
            "    0x000a |         | server |           Ext |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => PacketUnicastTxCount [int32u]",
            "    0x000b |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => CurrentMaxRate [int64u]",
            "    0x000c |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => OverrunCount [int64u]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => Disconnection",
            "0x0001 |         | server => AssociationFailure",
            "0x0002 |         | server => ConnectionStatus"
          ]
        },
        {
          "name": "Ethernet Network Diagnostics",
          "code": "0x0037",
          "define": "ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PHYRate [PHYRateEnum]",
            "    0x0001 |         | server |           Ext |           |         |                 0x00 |          1 |                   1 |               65534 |                     => FullDuplex [boolean]",
            "    0x0002 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => PacketRxCount [int64u]",
            "    0x0003 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => PacketTxCount [int64u]",
            "    0x0004 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => TxErrCount [int64u]",
            "    0x0005 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => CollisionCount [int64u]",
            "    0x0006 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => OverrunCount [int64u]",
            "    0x0007 |         | server |           Ext |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CarrierDetect [boolean]",
            "    0x0008 |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => TimeSinceReset [int64u]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Switch",
          "code": "0x003b",
          "define": "SWITCH_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => NumberOfPositions [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => CurrentPosition [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => SwitchLatched"
          ]
        },
        {
          "name": "Administrator Commissioning",
          "code": "0x003c",
          "define": "ADMINISTRATOR_COMMISSIONING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => WindowStatus [CommissioningWindowStatusEnum]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AdminFabricIndex [fabric_idx]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AdminVendorId [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Operational Credentials",
          "code": "0x003e",
          "define": "OPERATIONAL_CREDENTIALS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => AttestationResponse",
            "0x0003 |         | server |        1 |       1 => CertificateChainResponse",
            "0x0005 |         | server |        1 |       1 => CSRResponse",
            "0x0008 |         | server |        1 |       1 => NOCResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => NOCs [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Fabrics [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SupportedFabrics [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CommissionedFabrics [int8u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => TrustedRootCertificates [array]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentFabricIndex [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Group Key Management",
          "code": "0x003f",
          "define": "GROUP_KEY_MANAGEMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0002 |         | server |        0 |       1 => KeySetReadResponse",
            "0x0005 |         | server |        0 |       1 => KeySetReadAllIndicesResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GroupKeyMap [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GroupTable [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupsPerFabric [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupKeysPerFabric [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Fixed Label",
          "code": "0x0040",
          "define": "FIXED_LABEL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LabelList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "User Label",
          "code": "0x0041",
          "define": "USER_LABEL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LabelList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 2,
      "name": "MA-dimmablelight",
      "deviceTypeRef": {
        "code": 257,
        "profileId": 259,
        "label": "MA-dimmablelight",
        "name": "MA-dimmablelight"
      },
      "deviceTypes": [
        {
          "code": 257,
          "profileId": 259,
          "label": "MA-dimmablelight",
          "name": "MA-dimmablelight"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        257
      ],
      "deviceTypeName": "MA-dimmablelight",
      "deviceTypeCode": 257,
      "deviceTypeProfileId": 259,
      "clusters": [
        {
          "name": "Identify",
          "code": "0x0003",
          "define": "IDENTIFY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Groups",
          "code": "0x0004",
          "define": "GROUPS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | server |        0 |       1 => AddGroupResponse",
            "0x0001 |         | server |        0 |       1 => ViewGroupResponse",
            "0x0002 |         | server |        0 |       1 => GetGroupMembershipResponse",
            "0x0003 |         | server |        0 |       1 => RemoveGroupResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [NameSupportBitmap]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Scenes",
          "code": "0x0005",
          "define": "SCENES_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | server |        0 |       1 => AddSceneResponse",
            "0x0001 |         | server |        0 |       1 => ViewSceneResponse",
            "0x0002 |         | server |        0 |       1 => RemoveSceneResponse",
            "0x0003 |         | server |        0 |       1 => RemoveAllScenesResponse",
            "0x0004 |         | server |        0 |       1 => StoreSceneResponse",
            "0x0006 |         | server |        0 |       1 => GetSceneMembershipResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneCount [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentScene [int8u]",
            "    0x0002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => CurrentGroup [group_id]",
            "    0x0003 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneValid [boolean]",
            "    0x0004 |         | server |           RAM |           |         |                 0x80 |          1 |                     |               65344 |                     => NameSupport [bitmap8]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LastConfiguredBy [node_id]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SceneTableSize [int16u]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => RemainingCapacity [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "On/Off",
          "code": "0x0006",
          "define": "ON_OFF_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                 0x00 |          1 |                     |               65344 |                     => OnOff [boolean]",
            "    0x4000 |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => GlobalSceneControl [boolean]",
            "    0x4001 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => OnTime [int16u]",
            "    0x4002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => OffWaitTime [int16u]",
            "    0x4003 |         | server |           NVM |           |         |                 0xFF |          1 |                     |               65344 |                     => StartUpOnOff [OnOffStartUpOnOff]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Level Control",
          "code": "0x0008",
          "define": "LEVEL_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                 0x01 |          1 |                     |               65344 |                     => CurrentLevel [int8u]",
            "    0x0001 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => RemainingTime [int16u]",
            "    0x0002 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => MinLevel [int8u]",
            "    0x0003 |         | server |           RAM |           |         |                 0xFE |          1 |                   1 |               65534 |                     => MaxLevel [int8u]",
            "    0x0004 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => CurrentFrequency [int16u]",
            "    0x0005 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => MinFrequency [int16u]",
            "    0x0006 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => MaxFrequency [int16u]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Options [LevelControlOptions]",
            "    0x0010 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => OnOffTransitionTime [int16u]",
            "    0x0011 |         | server |           RAM |           |         |                 0xFF |          1 |                   1 |               65534 |                     => OnLevel [int8u]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OnTransitionTime [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OffTransitionTime [int16u]",
            "    0x0014 |         | server |           RAM |           |         |                   50 |          1 |                   1 |               65534 |                     => DefaultMoveRate [int8u]",
            "    0x4000 |         | server |           NVM |           |         |                  255 |          1 |                     |               65344 |                     => StartUpCurrentLevel [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Descriptor",
          "code": "0x001d",
          "define": "DESCRIPTOR_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PartsList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Color Control",
          "code": "0x0300",
          "define": "COLOR_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentHue [int8u]",
            "    0x0001 |         | server |           NVM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentSaturation [int8u]",
            "    0x0002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => RemainingTime [int16u]",
            "    0x0003 |         | server |           NVM |           |         |               0x616B |          1 |                     |               65344 |                     => CurrentX [int16u]",
            "    0x0004 |         | server |           NVM |           |         |               0x607D |          1 |                     |               65344 |                     => CurrentY [int16u]",
            "    0x0007 |         | server |           NVM |           |         |               0x00FA |          1 |                     |               65344 |                     => ColorTemperatureMireds [int16u]",
            "    0x0008 |         | server |           NVM |           |         |                 0x01 |          1 |                     |               65344 |                     => ColorMode [enum8]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Options [bitmap8]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NumberOfPrimaries [int8u]",
            "    0x4000 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65344 |                     => EnhancedCurrentHue [int16u]",
            "    0x4001 |         | server |           NVM |           |         |                 0x01 |          1 |                     |               65344 |                     => EnhancedColorMode [enum8]",
            "    0x4002 |         | server |           NVM |           |         |                 0x00 |          1 |                     |               65344 |                     => ColorLoopActive [int8u]",
            "    0x4003 |         | server |           NVM |           |         |                 0x00 |          1 |                     |               65344 |                     => ColorLoopDirection [int8u]",
            "    0x4004 |         | server |           NVM |           |         |               0x0019 |          1 |                     |               65344 |                     => ColorLoopTime [int16u]",
            "    0x4005 |         | server |           RAM |           |         |               0x2300 |          1 |                     |               65344 |                     => ColorLoopStartEnhancedHue [int16u]",
            "    0x4006 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => ColorLoopStoredEnhancedHue [int16u]",
            "    0x400a |         | server |           RAM |           |         |                 0x1F |          1 |                     |               65344 |                     => ColorCapabilities [bitmap16]",
            "    0x400b |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => ColorTempPhysicalMinMireds [int16u]",
            "    0x400c |         | server |           RAM |           |         |               0xFEFF |          1 |                     |               65344 |                     => ColorTempPhysicalMaxMireds [int16u]",
            "    0x400d |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => CoupleColorTempToLevelMinMireds [int16u]",
            "    0x4010 |         | server |           NVM |           |         |                      |          1 |                     |               65344 |                     => StartUpColorTemperatureMireds [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                 0x1F |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Occupancy Sensing",
          "code": "0x0406",
          "define": "OCCUPANCY_SENSING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Occupancy [OccupancyBitmap]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => OccupancySensorType [OccupancySensorTypeEnum]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => OccupancySensorTypeBitmap [OccupancySensorTypeBitmap]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
      "endpointId": 0,
      "networkId": 0
    },
    {
      "endpointTypeName": "MA-dimmablelight",
      "endpointTypeIndex": 1,
      "profileId": 259,
      "endpointId": 1,
      "networkId": 0
    }
  ]
}