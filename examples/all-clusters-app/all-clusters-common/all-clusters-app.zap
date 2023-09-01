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
      "path": "../../../src/app/zap-templates/app-templates.json",
      "type": "gen-templates-json",
      "version": "chip-v1"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../src/app/zap-templates/zcl/zcl-with-test-extensions.json",
      "type": "zcl-properties",
      "category": "matter",
      "version": 1,
      "description": "Matter SDK ZCL data with some extensions"
    }
  ],
  "endpointTypes": [
    {
      "id": 1,
      "name": "MA-rootdevice",
      "deviceTypeRef": {
        "code": 17,
        "profileId": 259,
        "label": "MA-powersource",
        "name": "MA-powersource"
      },
      "deviceTypes": [
        {
          "code": 17,
          "profileId": 259,
          "label": "MA-powersource",
          "name": "MA-powersource"
        },
        {
          "code": 22,
          "profileId": 259,
          "label": "MA-rootdevice",
          "name": "MA-rootdevice"
        }
      ],
      "deviceVersions": [
        1,
        1
      ],
      "deviceIdentifiers": [
        17,
        22
      ],
      "deviceTypeName": "MA-powersource",
      "deviceTypeCode": 17,
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
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                     |               65344 |                     => IdentifyType [IdentifyTypeEnum]",
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
            "0x0000 |         | server |        1 |       1 => AddGroupResponse",
            "0x0001 |         | server |        1 |       1 => ViewGroupResponse",
            "0x0002 |         | server |        1 |       1 => GetGroupMembershipResponse",
            "0x0003 |         | server |        1 |       1 => RemoveGroupResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [NameSupportBitmap]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartsList [array]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TagList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Binding",
          "code": "0x001e",
          "define": "BINDING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Binding [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => ACL [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => Extension [array]",
            "    0x0002 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => SubjectsPerAccessControlEntry [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => TargetsPerAccessControlEntry [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => AccessControlEntriesPerFabric [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0014 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => ProductAppearance [ProductAppearanceStruct]",
            "    0xfff8 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM | singleton |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Unit Localization",
          "code": "0x002d",
          "define": "UNIT_LOCALIZATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => TemperatureUnit [TempUnitEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                  0x1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Power Source Configuration",
          "code": "0x002e",
          "define": "POWER_SOURCE_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Sources [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Power Source",
          "code": "0x002f",
          "define": "POWER_SOURCE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => Status [PowerSourceStatusEnum]",
            "    0x0001 |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => Order [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                   B1 |          1 |                   1 |               65534 |                     => Description [char_string]",
            "    0x000e |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => BatChargeLevel [BatChargeLevelEnum]",
            "    0x000f |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplacementNeeded [boolean]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplaceability [BatReplaceabilityEnum]",
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0003 |         | server |        1 |       1 => SetRegulatoryConfigResponse",
            "0x0005 |         | server |        0 |       1 => CommissioningCompleteResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => Breadcrumb [int64u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => BasicCommissioningInfo [BasicCommissioningInfo]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => RegulatoryConfig [RegulatoryLocationTypeEnum]",
            "    0x0003 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LocationCapability [RegulatoryLocationTypeEnum]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => SupportsConcurrentConnection [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0005 |         | server |        1 |       1 => NetworkConfigResponse",
            "0x0007 |         | server |        1 |       1 => ConnectNetworkResponse"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
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
            "    0x0008 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TestEventTriggersEnabled [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => SoftwareFault"
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
            "    0x0005 |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => BeaconLostCount [int32u]",
            "    0x0006 |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => BeaconRxCount [int32u]",
            "    0x0007 |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => PacketMulticastRxCount [int32u]",
            "    0x0008 |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => PacketMulticastTxCount [int32u]",
            "    0x0009 |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => PacketUnicastRxCount [int32u]",
            "    0x000a |         | server |           Ext |           |         |           0x00000000 |          1 |                     |               65344 |                     => PacketUnicastTxCount [int32u]",
            "    0x000b |         | server |           Ext |           |         |   0x0000000000000000 |          1 |                     |               65344 |                     => CurrentMaxRate [int64u]",
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
          "name": "Time Synchronization",
          "code": "0x0038",
          "define": "TIME_SYNCHRONIZATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0003 |         | server |        0 |       1 => SetTimeZoneResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => UTCTime [epoch_us]",
            "    0x0001 |         | server |           Ext |           |         |                 0x00 |          1 |                   1 |               65534 |                     => Granularity [GranularityEnum]",
            "    0x0002 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => TimeSource [TimeSourceEnum]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TrustedTimeSource [TrustedTimeSourceStruct]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DefaultNTP [char_string]",
            "    0x0005 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => TimeZone [array]",
            "    0x0006 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DSTOffset [array]",
            "    0x0007 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LocalTime [epoch_us]",
            "    0x0008 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => TimeZoneDatabase [TimeZoneDatabaseEnum]",
            "    0x000a |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => TimeZoneListMaxSize [int8u]",
            "    0x000b |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DSTOffsetListMaxSize [int8u]",
            "    0x000c |         | server |           RAM |           |         |                false |          1 |                   1 |               65534 |                     => SupportsDNSResolve [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                 0x0B |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => DSTTableEmpty",
            "0x0001 |         | server => DSTStatus",
            "0x0002 |         | server => TimeZoneStatus",
            "0x0003 |         | server => TimeFailure",
            "0x0004 |         | server => MissingTrustedTimeSource"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0001 |         | server |        0 |       1 => AttestationResponse",
            "0x0003 |         | server |        0 |       1 => CertificateChainResponse",
            "0x0005 |         | server |        0 |       1 => CSRResponse",
            "0x0008 |         | server |        0 |       1 => NOCResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => NOCs [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Fabrics [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SupportedFabrics [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CommissionedFabrics [int8u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => TrustedRootCertificates [array]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentFabricIndex [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => GroupKeyMap [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => GroupTable [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupsPerFabric [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupKeysPerFabric [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
        },
        {
          "name": "ICD Management",
          "code": "0x0046",
          "define": "ICD_MANAGEMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => RegisterClientResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                  500 |          1 |                   1 |               65534 |                     => IdleModeInterval [int32u]",
            "    0x0001 |         | server |           Ext |           |         |                  300 |          1 |                   1 |               65534 |                     => ActiveModeInterval [int32u]",
            "    0x0002 |         | server |           Ext |           |         |                  300 |          1 |                   1 |               65534 |                     => ActiveModeThreshold [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => RegisteredClients [array]",
            "    0x0004 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ICDCounter [int32u]",
            "    0x0005 |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => ClientsSupportedPerFabric [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Relative Humidity Measurement",
          "code": "0x0405",
          "define": "RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |               0x2710 |          1 |                     |               65344 |                     => MaxMeasuredValue [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Fault Injection",
          "code": "0xfff1fc06",
          "define": "FAULT_INJECTION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 2,
      "name": "MA-onofflight",
      "deviceTypeRef": {
        "code": 256,
        "profileId": 259,
        "label": "MA-onofflight",
        "name": "MA-onofflight"
      },
      "deviceTypes": [
        {
          "code": 256,
          "profileId": 259,
          "label": "MA-onofflight",
          "name": "MA-onofflight"
        },
        {
          "code": 17,
          "profileId": 259,
          "label": "MA-powersource",
          "name": "MA-powersource"
        }
      ],
      "deviceVersions": [
        1,
        1
      ],
      "deviceIdentifiers": [
        256,
        17
      ],
      "deviceTypeName": "MA-onofflight",
      "deviceTypeCode": 256,
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
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                     |               65344 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0000 |         | server |        1 |       1 => AddGroupResponse",
            "0x0001 |         | server |        1 |       1 => ViewGroupResponse",
            "0x0002 |         | server |        1 |       1 => GetGroupMembershipResponse",
            "0x0003 |         | server |        1 |       1 => RemoveGroupResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [NameSupportBitmap]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0000 |         | server |        1 |       1 => AddSceneResponse",
            "0x0001 |         | server |        1 |       1 => ViewSceneResponse",
            "0x0002 |         | server |        1 |       1 => RemoveSceneResponse",
            "0x0003 |         | server |        1 |       1 => RemoveAllScenesResponse",
            "0x0004 |         | server |        1 |       1 => StoreSceneResponse",
            "0x0006 |         | server |        1 |       1 => GetSceneMembershipResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneCount [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentScene [int8u]",
            "    0x0002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => CurrentGroup [group_id]",
            "    0x0003 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneValid [boolean]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [bitmap8]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LastConfiguredBy [node_id]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SceneTableSize [int16u]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => RemainingCapacity [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "0x0040 |         | client |        1 |       0 => OffWithEffect",
            "0x0041 |         | client |        1 |       0 => OnWithRecallGlobalScene",
            "0x0042 |         | client |        1 |       0 => OnWithTimedOff"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "On/off Switch Configuration",
          "code": "0x0007",
          "define": "ON_OFF_SWITCH_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => switch type [enum8]",
            "    0x0010 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => switch actions [enum8]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           NVM |           |         |                 0xFE |          1 |                     |               65344 |                     => CurrentLevel [int8u]",
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
          "name": "Binary Input (Basic)",
          "code": "0x000f",
          "define": "BINARY_INPUT_BASIC_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0051 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => out of service [boolean]",
            "    0x0055 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => present value [boolean]",
            "    0x006f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => status flags [bitmap8]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartsList [array]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TagList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Binding",
          "code": "0x001e",
          "define": "BINDING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Binding [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Actions",
          "code": "0x0025",
          "define": "ACTIONS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActionList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => EndpointLists [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SetupURL [long_char_string]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Power Source",
          "code": "0x002f",
          "define": "POWER_SOURCE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => Status [PowerSourceStatusEnum]",
            "    0x0001 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => Order [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                   B2 |          1 |                     |               65344 |                     => Description [char_string]",
            "    0x000e |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => BatChargeLevel [BatChargeLevelEnum]",
            "    0x000f |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplacementNeeded [boolean]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplaceability [BatReplaceabilityEnum]",
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0001 |         | server => BatFaultChange"
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
            "    0x0002 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => MultiPressMax [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => SwitchLatched"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => LabelList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
        },
        {
          "name": "Boolean State",
          "code": "0x0045",
          "define": "BOOLEAN_STATE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => StateValue [boolean]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Mode Select",
          "code": "0x0050",
          "define": "MODE_SELECT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |               Coffee |          1 |                   1 |               65534 |                     => Description [char_string]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => StandardNamespace [enum16]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0003 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0004 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0005 |         | server |           NVM |           |         |                  255 |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]",
            "0xfff10001 |         | server |           RAM |           |         |                  255 |          1 |                   1 |               65534 |                     => ManufacturerExtension [int8u]"
          ]
        },
        {
          "name": "Laundry Washer Mode",
          "code": "0x0051",
          "define": "LAUNDRY_WASHER_MODE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ChangeToModeResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Refrigerator And Temperature Controlled Cabinet Mode",
          "code": "0x0052",
          "define": "REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ChangeToModeResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Laundry Washer Controls",
          "code": "0x0053",
          "define": "LAUNDRY_WASHER_CONTROLS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SpinSpeeds [array]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SpinSpeedCurrent [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => NumberOfRinses [NumberOfRinsesEnum]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedRinses [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "RVC Run Mode",
          "code": "0x0054",
          "define": "RVC_RUN_MODE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ChangeToModeResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "RVC Clean Mode",
          "code": "0x0055",
          "define": "RVC_CLEAN_MODE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ChangeToModeResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Temperature Control",
          "code": "0x0056",
          "define": "TEMPERATURE_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0004 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => SelectedTemperatureLevel [int8u]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedTemperatureLevels [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Refrigerator Alarm",
          "code": "0x0057",
          "define": "REFRIGERATOR_ALARM_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => Mask [AlarmMap]",
            "    0x0002 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => State [AlarmMap]",
            "    0x0003 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => Supported [AlarmMap]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => Notify"
          ]
        },
        {
          "name": "Dishwasher Mode",
          "code": "0x0059",
          "define": "DISHWASHER_MODE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        1 |       1 => ChangeToModeResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OnMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Air Quality",
          "code": "0x005b",
          "define": "AIR_QUALITY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AirQuality [AirQualityEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   15 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Smoke CO Alarm",
          "code": "0x005c",
          "define": "SMOKE_CO_ALARM_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => ExpressedState [ExpressedStateEnum]",
            "    0x0001 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => SmokeState [AlarmStateEnum]",
            "    0x0002 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => COState [AlarmStateEnum]",
            "    0x0003 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => BatteryAlert [AlarmStateEnum]",
            "    0x0004 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => DeviceMuted [MuteStateEnum]",
            "    0x0005 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => TestInProgress [boolean]",
            "    0x0006 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => HardwareFaultAlert [boolean]",
            "    0x0007 |         | server |           NVM |           |         |                    0 |          1 |                   1 |               65534 |                     => EndOfServiceAlert [EndOfServiceEnum]",
            "    0x0008 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => InterconnectSmokeAlarm [AlarmStateEnum]",
            "    0x0009 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => InterconnectCOAlarm [AlarmStateEnum]",
            "    0x000a |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => ContaminationState [ContaminationStateEnum]",
            "    0x000b |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => SmokeSensitivityLevel [SensitivityEnum]",
            "    0x000c |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => ExpiryDate [epoch_s]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => SmokeAlarm",
            "0x0001 |         | server => COAlarm",
            "0x0002 |         | server => LowBattery",
            "0x0003 |         | server => HardwareFault",
            "0x0004 |         | server => EndOfService",
            "0x0005 |         | server => SelfTestComplete",
            "0x0006 |         | server => AlarmMuted",
            "0x0007 |         | server => MuteEnded",
            "0x0008 |         | server => InterconnectSmokeAlarm",
            "0x0009 |         | server => InterconnectCOAlarm",
            "0x000a |         | server => AllClear"
          ]
        },
        {
          "name": "Dishwasher Alarm",
          "code": "0x005d",
          "define": "DISHWASHER_ALARM_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => Mask [AlarmMap]",
            "    0x0001 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => Latch [AlarmMap]",
            "    0x0002 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => State [AlarmMap]",
            "    0x0003 |         | server |           RAM |           |         |                   15 |          1 |                   1 |               65534 |                     => Supported [AlarmMap]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => Notify"
          ]
        },
        {
          "name": "Operational State",
          "code": "0x0060",
          "define": "OPERATIONAL_STATE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0004 |         | server |        0 |       1 => OperationalCommandResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PhaseList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentPhase [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CountdownTime [elapsed_s]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OperationalStateList [array]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OperationalState [OperationalStateEnum]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => OperationalError [ErrorStateStruct]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => OperationalError",
            "0x0001 |         | server => OperationCompletion"
          ]
        },
        {
          "name": "RVC Operational State",
          "code": "0x0061",
          "define": "OPERATIONAL_STATE_RVC_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0004 |         | server |        0 |       1 => OperationalCommandResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PhaseList [array]",
            "    0x0001 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => CurrentPhase [int8u]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => CountdownTime [elapsed_s]",
            "    0x0003 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => OperationalStateList [array]",
            "    0x0004 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => OperationalState [enum8]",
            "    0x0005 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => OperationalError [ErrorStateStruct]",
            "    0xfff8 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => OperationalError",
            "0x0001 |         | server => OperationCompletion"
          ]
        },
        {
          "name": "HEPA Filter Monitoring",
          "code": "0x0071",
          "define": "HEPA_FILTER_MONITORING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Condition [percent]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DegradationDirection [DegradationDirectionEnum]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ChangeIndication [ChangeIndicationEnum]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => InPlaceIndicator [boolean]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LastChangedTime [epoch_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ReplacementProductList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Activated Carbon Filter Monitoring",
          "code": "0x0072",
          "define": "ACTIVATED_CARBON_FILTER_MONITORING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Condition [percent]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => DegradationDirection [DegradationDirectionEnum]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ChangeIndication [ChangeIndicationEnum]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => InPlaceIndicator [boolean]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LastChangedTime [epoch_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ReplacementProductList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Door Lock",
          "code": "0x0101",
          "define": "DOOR_LOCK_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x001c |         | server |        0 |       1 => GetUserResponse",
            "0x0023 |         | server |        0 |       1 => SetCredentialResponse",
            "0x0025 |         | server |        0 |       1 => GetCredentialStatusResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => LockState [DlLockState]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => LockType [DlLockType]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ActuatorEnabled [boolean]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => DoorState [DoorStateEnum]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => DoorOpenEvents [int32u]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => DoorClosedEvents [int32u]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OpenPeriod [int16u]",
            "    0x0011 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => NumberOfTotalUsersSupported [int16u]",
            "    0x0012 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => NumberOfPINUsersSupported [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => NumberOfRFIDUsersSupported [int16u]",
            "    0x0014 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => NumberOfWeekDaySchedulesSupportedPerUser [int8u]",
            "    0x0015 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => NumberOfYearDaySchedulesSupportedPerUser [int8u]",
            "    0x0016 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => NumberOfHolidaySchedulesSupported [int8u]",
            "    0x0017 |         | server |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => MaxPINCodeLength [int8u]",
            "    0x0018 |         | server |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => MinPINCodeLength [int8u]",
            "    0x0019 |         | server |           RAM |           |         |                   20 |          1 |                   1 |               65534 |                     => MaxRFIDCodeLength [int8u]",
            "    0x001a |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => MinRFIDCodeLength [int8u]",
            "    0x001b |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => CredentialRulesSupport [DlCredentialRuleMask]",
            "    0x001c |         | server |           RAM |           |         |                    5 |          1 |                   1 |               65534 |                     => NumberOfCredentialsSupportedPerUser [int8u]",
            "    0x0021 |         | server |           RAM |           |         |                   en |          1 |                     |               65344 |                     => Language [char_string]",
            "    0x0023 |         | server |           RAM |           |         |                   60 |          1 |                     |               65344 |                     => AutoRelockTime [int32u]",
            "    0x0024 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SoundVolume [int8u]",
            "    0x0025 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => OperatingMode [OperatingModeEnum]",
            "    0x0026 |         | server |           RAM |           |         |               0xFFF6 |          1 |                   1 |               65534 |                     => SupportedOperatingModes [DlSupportedOperatingModes]",
            "    0x0027 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => DefaultConfigurationRegister [DlDefaultConfigurationRegister]",
            "    0x0029 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => EnableOneTouchLocking [boolean]",
            "    0x002a |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => EnableInsideStatusLED [boolean]",
            "    0x002b |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => EnablePrivacyModeButton [boolean]",
            "    0x0030 |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => WrongCodeEntryLimit [int8u]",
            "    0x0031 |         | server |           RAM |           |         |                   10 |          1 |                     |               65344 |                     => UserCodeTemporaryDisableTime [int8u]",
            "    0x0033 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => RequirePINforRemoteOperation [boolean]",
            "    0xfffc |         | server |           RAM |           |         |               0x1D13 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => DoorLockAlarm",
            "0x0002 |         | server => LockOperation",
            "0x0003 |         | server => LockOperationError",
            "0x0004 |         | server => LockUserChange"
          ]
        },
        {
          "name": "Window Covering",
          "code": "0x0102",
          "define": "WINDOW_COVERING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x08 |          1 |                     |               65344 |                     => Type [Type]",
            "    0x0001 |         | server |           RAM |           |         |               0xFFFF |          1 |                     |               65545 |                     => PhysicalClosedLimitLift [int16u]",
            "    0x0002 |         | server |           RAM |           |         |               0xFFFF |          1 |                     |               65545 |                     => PhysicalClosedLimitTilt [int16u]",
            "    0x0003 |         | server |           NVM |           |         |               0x7FFF |          1 |                     |               65344 |                     => CurrentPositionLift [int16u]",
            "    0x0004 |         | server |           NVM |           |         |               0x7FFF |          1 |                     |               65344 |                     => CurrentPositionTilt [int16u]",
            "    0x0005 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65545 |                     => NumberOfActuationsLift [int16u]",
            "    0x0006 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65545 |                     => NumberOfActuationsTilt [int16u]",
            "    0x0007 |         | server |           NVM |           |         |                 0x03 |          1 |                     |               65344 |                     => ConfigStatus [ConfigStatus]",
            "    0x0008 |         | server |           NVM |           |         |                   50 |          1 |                     |                 100 |                     => CurrentPositionLiftPercentage [Percent]",
            "    0x0009 |         | server |           NVM |           |         |                   50 |          1 |                     |                 100 |                     => CurrentPositionTiltPercentage [Percent]",
            "    0x000a |         | server |           RAM |           |         |                 0x00 |          1 |                     |                 127 |                     => OperationalStatus [OperationalStatus]",
            "    0x000b |         | server |           RAM |           |         |                 5000 |          1 |                     |               10000 |                     => TargetPositionLiftPercent100ths [Percent100ths]",
            "    0x000c |         | server |           RAM |           |         |                 5000 |          1 |                     |               10000 |                     => TargetPositionTiltPercent100ths [Percent100ths]",
            "    0x000d |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => EndProductType [EndProductType]",
            "    0x000e |         | server |           NVM |           |         |                 5000 |          1 |                     |               10000 |                     => CurrentPositionLiftPercent100ths [Percent100ths]",
            "    0x000f |         | server |           NVM |           |         |                 5000 |          1 |                     |               10000 |                     => CurrentPositionTiltPercent100ths [Percent100ths]",
            "    0x0010 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65344 |                     => InstalledOpenLimitLift [int16u]",
            "    0x0011 |         | server |           NVM |           |         |               0xFFFF |          1 |                     |               65344 |                     => InstalledClosedLimitLift [int16u]",
            "    0x0012 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65344 |                     => InstalledOpenLimitTilt [int16u]",
            "    0x0013 |         | server |           NVM |           |         |               0xFFFF |          1 |                     |               65344 |                     => InstalledClosedLimitTilt [int16u]",
            "    0x0017 |         | server |           NVM |           |         |                 0x00 |          1 |                     |                  15 |                     => Mode [Mode]",
            "    0x001a |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SafetyStatus [SafetyStatus]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                 0x17 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Barrier Control",
          "code": "0x0103",
          "define": "BARRIER_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => barrier moving state [enum8]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => barrier safety status [bitmap16]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => barrier capabilities [bitmap8]",
            "    0x000a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => barrier position [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Pump Configuration and Control",
          "code": "0x0200",
          "define": "PUMP_CONFIGURATION_AND_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MaxPressure [int16s]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MaxSpeed [int16u]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MaxFlow [int16u]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinConstPressure [int16s]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxConstPressure [int16s]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinCompPressure [int16s]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxCompPressure [int16s]",
            "    0x0007 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinConstSpeed [int16u]",
            "    0x0008 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxConstSpeed [int16u]",
            "    0x0009 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinConstFlow [int16u]",
            "    0x000a |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxConstFlow [int16u]",
            "    0x000b |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinConstTemp [int16s]",
            "    0x000c |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxConstTemp [int16s]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => PumpStatus [PumpStatusBitmap]",
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => EffectiveOperationMode [OperationModeEnum]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => EffectiveControlMode [ControlModeEnum]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Capacity [int16s]",
            "    0x0014 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Speed [int16u]",
            "    0x0015 |         | server |           RAM |           |         |             0x000000 |          1 |                   1 |               65534 |                     => LifetimeRunningHours [int24u]",
            "    0x0016 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Power [int24u]",
            "    0x0017 |         | server |           RAM |           |         |           0x00000000 |          1 |                   1 |               65534 |                     => LifetimeEnergyConsumed [int32u]",
            "    0x0020 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => OperationMode [OperationModeEnum]",
            "    0x0021 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => ControlMode [ControlModeEnum]",
            "    0xfffc |         | server |           RAM |           |         |                 0x1F |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thermostat",
          "code": "0x0201",
          "define": "THERMOSTAT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | server |        1 |       0 => GetWeeklyScheduleResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => LocalTemperature [int16s]",
            "    0x0003 |         | server |           RAM |           |         |               0x02BC |          1 |                     |               65344 |                     => AbsMinHeatSetpointLimit [int16s]",
            "    0x0004 |         | server |           RAM |           |         |               0x0BB8 |          1 |                     |               65344 |                     => AbsMaxHeatSetpointLimit [int16s]",
            "    0x0005 |         | server |           RAM |           |         |               0x0640 |          1 |                     |               65344 |                     => AbsMinCoolSetpointLimit [int16s]",
            "    0x0006 |         | server |           RAM |           |         |               0x0C80 |          1 |                     |               65344 |                     => AbsMaxCoolSetpointLimit [int16s]",
            "    0x0011 |         | server |           RAM |           |         |               0x0A28 |          1 |                     |               65344 |                     => OccupiedCoolingSetpoint [int16s]",
            "    0x0012 |         | server |           RAM |           |         |               0x07D0 |          1 |                     |               65344 |                     => OccupiedHeatingSetpoint [int16s]",
            "    0x0015 |         | server |           RAM |           |         |               0x02BC |          1 |                     |               65344 |                     => MinHeatSetpointLimit [int16s]",
            "    0x0016 |         | server |           RAM |           |         |               0x0BB8 |          1 |                     |               65344 |                     => MaxHeatSetpointLimit [int16s]",
            "    0x0017 |         | server |           RAM |           |         |               0x0640 |          1 |                     |               65344 |                     => MinCoolSetpointLimit [int16s]",
            "    0x0018 |         | server |           RAM |           |         |               0x0C80 |          1 |                     |               65344 |                     => MaxCoolSetpointLimit [int16s]",
            "    0x0019 |         | server |           RAM |           |         |                 0x19 |          1 |                   1 |               65534 |                     => MinSetpointDeadBand [int8s]",
            "    0x001b |         | server |           RAM |           |         |                 0x04 |          1 |                     |               65344 |                     => ControlSequenceOfOperation [ThermostatControlSequence]",
            "    0x001c |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => SystemMode [enum8]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |               0x0023 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Fan Control",
          "code": "0x0202",
          "define": "FAN_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => FanMode [FanModeEnum]",
            "    0x0001 |         | server |           RAM |           |         |                 0x02 |          1 |                   1 |               65534 |                     => FanModeSequence [FanModeSequenceEnum]",
            "    0x0002 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => PercentSetting [Percent]",
            "    0x0003 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => PercentCurrent [Percent]",
            "    0x0004 |         | server |           RAM |           |         |                  100 |          1 |                   1 |               65534 |                     => SpeedMax [int8u]",
            "    0x0005 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => SpeedSetting [int8u]",
            "    0x0006 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => SpeedCurrent [int8u]",
            "    0x0007 |         | server |           RAM |           |         |                 0x03 |          1 |                   1 |               65534 |                     => RockSupport [RockBitmap]",
            "    0x0008 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => RockSetting [RockBitmap]",
            "    0x0009 |         | server |           RAM |           |         |                 0x03 |          1 |                   1 |               65534 |                     => WindSupport [WindBitmap]",
            "    0x000a |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => WindSetting [WindBitmap]",
            "    0x000b |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => AirflowDirection [AirflowDirectionEnum]",
            "    0xfffc |         | server |           RAM |           |         |                 0x3F |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thermostat User Interface Configuration",
          "code": "0x0204",
          "define": "THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => TemperatureDisplayMode [enum8]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => KeypadLockout [enum8]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ScheduleProgrammingVisibility [enum8]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => DriftCompensation [enum8]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => CompensationText [char_string]",
            "    0x0007 |         | server |           NVM |           |         |               0x00FA |          1 |                     |               65344 |                     => ColorTemperatureMireds [int16u]",
            "    0x0008 |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => ColorMode [enum8]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Options [bitmap8]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NumberOfPrimaries [int8u]",
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary1X [int16u]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary1Y [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary1Intensity [int8u]",
            "    0x0015 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary2X [int16u]",
            "    0x0016 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary2Y [int16u]",
            "    0x0017 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary2Intensity [int8u]",
            "    0x0019 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary3X [int16u]",
            "    0x001a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary3Y [int16u]",
            "    0x001b |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary3Intensity [int8u]",
            "    0x0020 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary4X [int16u]",
            "    0x0021 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary4Y [int16u]",
            "    0x0022 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary4Intensity [int8u]",
            "    0x0024 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary5X [int16u]",
            "    0x0025 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary5Y [int16u]",
            "    0x0026 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary5Intensity [int8u]",
            "    0x0028 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary6X [int16u]",
            "    0x0029 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary6Y [int16u]",
            "    0x002a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Primary6Intensity [int8u]",
            "    0x0030 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WhitePointX [int16u]",
            "    0x0031 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WhitePointY [int16u]",
            "    0x0032 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointRX [int16u]",
            "    0x0033 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointRY [int16u]",
            "    0x0034 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointRIntensity [int8u]",
            "    0x0036 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointGX [int16u]",
            "    0x0037 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointGY [int16u]",
            "    0x0038 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointGIntensity [int8u]",
            "    0x003a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointBX [int16u]",
            "    0x003b |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointBY [int16u]",
            "    0x003c |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ColorPointBIntensity [int8u]",
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
          "name": "Ballast Configuration",
          "code": "0x0301",
          "define": "BALLAST_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => PhysicalMinLevel [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0xFE |          1 |                   1 |               65534 |                     => PhysicalMaxLevel [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => BallastStatus [bitmap8]",
            "    0x0010 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => MinLevel [int8u]",
            "    0x0011 |         | server |           RAM |           |         |                 0xFE |          1 |                   1 |               65534 |                     => MaxLevel [int8u]",
            "    0x0014 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => IntrinsicBallastFactor [int8u]",
            "    0x0015 |         | server |           RAM |           |         |                 0xFF |          1 |                   1 |               65534 |                     => BallastFactorAdjustment [int8u]",
            "    0x0020 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LampQuantity [int8u]",
            "    0x0030 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LampType [char_string]",
            "    0x0031 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LampManufacturer [char_string]",
            "    0x0032 |         | server |           RAM |           |         |             0xFFFFFF |          1 |                   1 |               65534 |                     => LampRatedHours [int24u]",
            "    0x0033 |         | server |           RAM |           |         |             0x000000 |          1 |                   1 |               65534 |                     => LampBurnHours [int24u]",
            "    0x0034 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => LampAlarmMode [bitmap8]",
            "    0x0035 |         | server |           RAM |           |         |             0xFFFFFF |          1 |                   1 |               65534 |                     => LampBurnHoursTripPoint [int24u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Illuminance Measurement",
          "code": "0x0400",
          "define": "ILLUMINANCE_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |               0xFFFE |          1 |                   1 |               65534 |                     => MaxMeasuredValue [int16u]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Tolerance [int16u]",
            "    0x0004 |         | server |           RAM |           |         |                 0xFF |          1 |                   1 |               65534 |                     => LightSensorType [LightSensorTypeEnum]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Temperature Measurement",
          "code": "0x0402",
          "define": "TEMPERATURE_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |               0x8000 |          1 |                     |               65344 |                     => MeasuredValue [int16s]",
            "    0x0001 |         | server |           RAM |           |         |               0x8000 |          1 |                     |               65344 |                     => MinMeasuredValue [int16s]",
            "    0x0002 |         | server |           RAM |           |         |               0x8000 |          1 |                     |               65344 |                     => MaxMeasuredValue [int16s]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Tolerance [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Pressure Measurement",
          "code": "0x0403",
          "define": "PRESSURE_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => MeasuredValue [int16s]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MinMeasuredValue [int16s]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MaxMeasuredValue [int16s]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Flow Measurement",
          "code": "0x0404",
          "define": "FLOW_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MaxMeasuredValue [int16u]",
            "    0x0003 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => Tolerance [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Relative Humidity Measurement",
          "code": "0x0405",
          "define": "RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |               0x2710 |          1 |                     |               65344 |                     => MaxMeasuredValue [int16u]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Tolerance [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
        },
        {
          "name": "Carbon Monoxide Concentration Measurement",
          "code": "0x040c",
          "define": "CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                  401 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                   50 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                 1500 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                  511 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                  213 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                   10 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Carbon Dioxide Concentration Measurement",
          "code": "0x040d",
          "define": "CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                  458 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                  300 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                 2000 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                  523 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                  421 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    5 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Nitrogen Dioxide Concentration Measurement",
          "code": "0x0413",
          "define": "NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  150 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                  120 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                  120 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Ozone Concentration Measurement",
          "code": "0x0415",
          "define": "OZONE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                   10 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  300 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   50 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                   20 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM2.5 Concentration Measurement",
          "code": "0x042a",
          "define": "PM2_5_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                   42 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  400 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   90 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                   35 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Formaldehyde Concentration Measurement",
          "code": "0x042b",
          "define": "FORMALDEHYDE_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                   10 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  200 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   10 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 7200 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 7200 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM1 Concentration Measurement",
          "code": "0x042c",
          "define": "PM1_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                   39 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  400 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   70 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                   41 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "PM10 Concentration Measurement",
          "code": "0x042d",
          "define": "PM10_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    7 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  400 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   49 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                   43 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    4 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Total Volatile Organic Compounds Concentration Measurement",
          "code": "0x042e",
          "define": "TVOC_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                    5 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  100 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                    8 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Radon Concentration Measurement",
          "code": "0x042f",
          "define": "RADON_CONCENTRATION_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                   10 |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                    5 |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                  100 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                   36 |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                   20 |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                 3600 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0007 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => Uncertainty [single]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    3 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                   63 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Wake on LAN",
          "code": "0x0503",
          "define": "WAKE_ON_LAN_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MACAddress [char_string]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Channel",
          "code": "0x0504",
          "define": "CHANNEL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ChannelList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Target Navigator",
          "code": "0x0505",
          "define": "TARGET_NAVIGATOR_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        0 |       1 => NavigateTargetResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => TargetList [array]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentTarget [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Media Playback",
          "code": "0x0506",
          "define": "MEDIA_PLAYBACK_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentState [PlaybackStateEnum]",
            "    0x0001 |         | server |           RAM |           |         |                 0xFF |          1 |                   1 |               65534 |                     => StartTime [epoch_us]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Duration [int64u]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => PlaybackSpeed [single]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SeekRangeEnd [int64u]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SeekRangeStart [int64u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Media Input",
          "code": "0x0507",
          "define": "MEDIA_INPUT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => InputList [array]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentInput [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Low Power",
          "code": "0x0508",
          "define": "LOW_POWER_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Keypad Input",
          "code": "0x0509",
          "define": "KEYPAD_INPUT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        0 |       1 => SendKeyResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Content Launcher",
          "code": "0x050a",
          "define": "CONTENT_LAUNCHER_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => AcceptHeader [array]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => SupportedStreamingProtocols [bitmap32]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Audio Output",
          "code": "0x050b",
          "define": "AUDIO_OUTPUT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OutputList [array]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentOutput [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Application Launcher",
          "code": "0x050c",
          "define": "APPLICATION_LAUNCHER_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CatalogList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Application Basic",
          "code": "0x050d",
          "define": "APPLICATION_BASIC_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => VendorName [char_string]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => VendorID [vendor_id]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ApplicationName [char_string]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ProductID [int16u]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Status [ApplicationStatusEnum]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ApplicationVersion [char_string]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AllowedVendorList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Account Login",
          "code": "0x050e",
          "define": "ACCOUNT_LOGIN_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Electrical Measurement",
          "code": "0x0b04",
          "define": "ELECTRICAL_MEASUREMENT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |             0x000000 |          1 |                     |               65344 |                     => measurement type [bitmap32]",
            "    0x0304 |         | server |           RAM |           |         |             0x000000 |          1 |                     |               65344 |                     => total active power [int32s]",
            "    0x0505 |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => rms voltage [int16u]",
            "    0x0506 |         | server |           RAM |           |         |               0x8000 |          1 |                     |               65344 |                     => rms voltage min [int16u]",
            "    0x0507 |         | server |           RAM |           |         |               0x8000 |          1 |                     |               65344 |                     => rms voltage max [int16u]",
            "    0x0508 |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => rms current [int16u]",
            "    0x0509 |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => rms current min [int16u]",
            "    0x050a |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => rms current max [int16u]",
            "    0x050b |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => active power [int16s]",
            "    0x050c |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => active power min [int16s]",
            "    0x050d |         | server |           RAM |           |         |               0xffff |          1 |                     |               65344 |                     => active power max [int16s]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Unit Testing",
          "code": "0xfff1fc05",
          "define": "UNIT_TESTING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | server |        1 |       1 => TestSpecificResponse",
            "0x0001 |         | server |        0 |       1 => TestAddArgumentsResponse",
            "0x0004 |         | server |        0 |       1 => TestListInt8UReverseResponse",
            "0x0005 |         | server |        0 |       1 => TestEnumsResponse",
            "0x0006 |         | server |        0 |       1 => TestNullableOptionalResponse",
            "0x0009 |         | server |        0 |       1 => SimpleStructResponse",
            "0x000a |         | server |        0 |       1 => TestEmitTestEventResponse",
            "0x000b |         | server |        0 |       1 => TestEmitTestFabricScopedEventResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                false |          1 |                     |               65344 |                     => boolean [boolean]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => bitmap8 [Bitmap8MaskMap]",
            "    0x0002 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => bitmap16 [Bitmap16MaskMap]",
            "    0x0003 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => bitmap32 [Bitmap32MaskMap]",
            "    0x0004 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => bitmap64 [Bitmap64MaskMap]",
            "    0x0005 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int8u [int8u]",
            "    0x0006 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int16u [int16u]",
            "    0x0007 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int24u [int24u]",
            "    0x0008 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int32u [int32u]",
            "    0x0009 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int40u [int40u]",
            "    0x000a |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int48u [int48u]",
            "    0x000b |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int56u [int56u]",
            "    0x000c |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int64u [int64u]",
            "    0x000d |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int8s [int8s]",
            "    0x000e |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int16s [int16s]",
            "    0x000f |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int24s [int24s]",
            "    0x0010 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int32s [int32s]",
            "    0x0011 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int40s [int40s]",
            "    0x0012 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int48s [int48s]",
            "    0x0013 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => int56s [int56s]",
            "    0x0014 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => int64s [int64s]",
            "    0x0015 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => enum8 [enum8]",
            "    0x0016 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => enum16 [enum16]",
            "    0x0017 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => float_single [single]",
            "    0x0018 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => float_double [double]",
            "    0x0019 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => octet_string [octet_string]",
            "    0x001a |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => list_int8u [array]",
            "    0x001b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => list_octet_string [array]",
            "    0x001c |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => list_struct_octet_string [array]",
            "    0x001d |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => long_octet_string [long_octet_string]",
            "    0x001e |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => char_string [char_string]",
            "    0x001f |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => long_char_string [long_char_string]",
            "    0x0020 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => epoch_us [epoch_us]",
            "    0x0021 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => epoch_s [epoch_s]",
            "    0x0022 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => vendor_id [vendor_id]",
            "    0x0023 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => list_nullables_and_optionals_struct [array]",
            "    0x0024 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => enum_attr [SimpleEnum]",
            "    0x0025 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => struct_attr [SimpleStruct]",
            "    0x0026 |         | server |           RAM |           |         |                   70 |          1 |                   1 |               65534 |                     => range_restricted_int8u [int8u]",
            "    0x0027 |         | server |           RAM |           |         |                  -20 |          1 |                   1 |               65534 |                     => range_restricted_int8s [int8s]",
            "    0x0028 |         | server |           RAM |           |         |                  200 |          1 |                   1 |               65534 |                     => range_restricted_int16u [int16u]",
            "    0x0029 |         | server |           RAM |           |         |                 -100 |          1 |                   1 |               65534 |                     => range_restricted_int16s [int16s]",
            "    0x002a |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => list_long_octet_string [array]",
            "    0x002b |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => list_fabric_scoped [array]",
            "    0x0030 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => timed_write_boolean [boolean]",
            "    0x0031 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => general_error_boolean [boolean]",
            "    0x0032 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => cluster_error_boolean [boolean]",
            "    0x4000 |         | server |           RAM |           |         |                false |          1 |                   1 |               65534 |                     => nullable_boolean [boolean]",
            "    0x4001 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_bitmap8 [Bitmap8MaskMap]",
            "    0x4002 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_bitmap16 [Bitmap16MaskMap]",
            "    0x4003 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_bitmap32 [Bitmap32MaskMap]",
            "    0x4004 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_bitmap64 [Bitmap64MaskMap]",
            "    0x4005 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int8u [int8u]",
            "    0x4006 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int16u [int16u]",
            "    0x4007 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int24u [int24u]",
            "    0x4008 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int32u [int32u]",
            "    0x4009 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int40u [int40u]",
            "    0x400a |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int48u [int48u]",
            "    0x400b |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int56u [int56u]",
            "    0x400c |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int64u [int64u]",
            "    0x400d |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int8s [int8s]",
            "    0x400e |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int16s [int16s]",
            "    0x400f |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int24s [int24s]",
            "    0x4010 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int32s [int32s]",
            "    0x4011 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int40s [int40s]",
            "    0x4012 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int48s [int48s]",
            "    0x4013 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int56s [int56s]",
            "    0x4014 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_int64s [int64s]",
            "    0x4015 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_enum8 [enum8]",
            "    0x4016 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_enum16 [enum16]",
            "    0x4017 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_float_single [single]",
            "    0x4018 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => nullable_float_double [double]",
            "    0x4019 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => nullable_octet_string [octet_string]",
            "    0x401e |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => nullable_char_string [char_string]",
            "    0x4024 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => nullable_enum_attr [SimpleEnum]",
            "    0x4025 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => nullable_struct [SimpleStruct]",
            "    0x4026 |         | server |           RAM |           |         |                   70 |          1 |                   1 |               65534 |                     => nullable_range_restricted_int8u [int8u]",
            "    0x4027 |         | server |           RAM |           |         |                  -20 |          1 |                   1 |               65534 |                     => nullable_range_restricted_int8s [int8s]",
            "    0x4028 |         | server |           RAM |           |         |                  200 |          1 |                   1 |               65534 |                     => nullable_range_restricted_int16u [int16u]",
            "    0x4029 |         | server |           RAM |           |         |                 -100 |          1 |                   1 |               65534 |                     => nullable_range_restricted_int16s [int16s]",
            "    0x402a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => write_only_int8u [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0001 |         | server => TestEvent",
            "0x0002 |         | server => TestFabricScopedEvent"
          ]
        }
      ]
    },
    {
      "id": 3,
      "name": "MA-onofflight",
      "deviceTypeRef": {
        "code": 256,
        "profileId": 259,
        "label": "MA-onofflight",
        "name": "MA-onofflight"
      },
      "deviceTypes": [
        {
          "code": 256,
          "profileId": 259,
          "label": "MA-onofflight",
          "name": "MA-onofflight"
        },
        {
          "code": 17,
          "profileId": 259,
          "label": "MA-powersource",
          "name": "MA-powersource"
        }
      ],
      "deviceVersions": [
        1,
        1
      ],
      "deviceIdentifiers": [
        256,
        17
      ],
      "deviceTypeName": "MA-onofflight",
      "deviceTypeCode": 256,
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
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0000 |         | server |        1 |       1 => AddGroupResponse",
            "0x0001 |         | server |        1 |       1 => ViewGroupResponse",
            "0x0002 |         | server |        1 |       1 => GetGroupMembershipResponse",
            "0x0003 |         | server |        1 |       1 => RemoveGroupResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [NameSupportBitmap]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "0x0000 |         | server |        1 |       1 => AddSceneResponse",
            "0x0001 |         | server |        1 |       1 => ViewSceneResponse",
            "0x0002 |         | server |        1 |       1 => RemoveSceneResponse",
            "0x0003 |         | server |        1 |       1 => RemoveAllScenesResponse",
            "0x0004 |         | server |        1 |       1 => StoreSceneResponse",
            "0x0006 |         | server |        1 |       1 => GetSceneMembershipResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneCount [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => CurrentScene [int8u]",
            "    0x0002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => CurrentGroup [group_id]",
            "    0x0003 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SceneValid [boolean]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => NameSupport [bitmap8]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SceneTableSize [int16u]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => RemainingCapacity [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => OnOff [boolean]",
            "    0x4000 |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => GlobalSceneControl [boolean]",
            "    0x4001 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => OnTime [int16u]",
            "    0x4002 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => OffWaitTime [int16u]",
            "    0x4003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => StartUpOnOff [OnOffStartUpOnOff]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
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
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TagList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Power Source",
          "code": "0x002f",
          "define": "POWER_SOURCE_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => Status [PowerSourceStatusEnum]",
            "    0x0001 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => Order [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                   B3 |          1 |                   1 |               65534 |                     => Description [char_string]",
            "    0x000e |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => BatChargeLevel [BatChargeLevelEnum]",
            "    0x000f |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplacementNeeded [boolean]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => BatReplaceability [BatReplaceabilityEnum]",
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
    },
    {
      "id": 4,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 61442,
        "profileId": 259,
        "label": "MA-secondary-network-commissioning",
        "name": "MA-secondary-network-commissioning"
      },
      "deviceTypes": [
        {
          "code": 61442,
          "profileId": 259,
          "label": "MA-secondary-network-commissioning",
          "name": "MA-secondary-network-commissioning"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        61442
      ],
      "deviceTypeName": "MA-secondary-network-commissioning",
      "deviceTypeCode": 61442,
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
            "    0xfffd |         | server |           Ext |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0001 |         | server |        0 |       1 => ScanNetworksResponse",
            "0x0005 |         | server |        0 |       1 => NetworkConfigResponse",
            "0x0007 |         | server |        0 |       1 => ConnectNetworkResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxNetworks [int8u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Networks [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ScanMaxTimeSeconds [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => ConnectMaxTimeSeconds [int8u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => InterfaceEnabled [boolean]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LastNetworkingStatus [NetworkCommissioningStatusEnum]",
            "    0x0006 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LastNetworkID [octet_string]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LastConnectErrorValue [int32s]",
            "    0xfffc |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
      "endpointTypeName": "MA-onofflight",
      "endpointTypeIndex": 1,
      "profileId": 259,
      "endpointId": 1,
      "networkId": 0
    },
    {
      "endpointTypeName": "MA-onofflight",
      "endpointTypeIndex": 2,
      "profileId": 259,
      "endpointId": 2,
      "networkId": 0
    },
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 3,
      "profileId": 259,
      "endpointId": 65534,
      "networkId": 0
    }
  ]
}