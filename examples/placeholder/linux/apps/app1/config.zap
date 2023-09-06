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
      "path": "../../../../../src/app/zap-templates/zcl/zcl.json",
      "type": "zcl-properties",
      "category": "matter",
      "version": 1,
      "description": "Matter SDK ZCL data"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../../../src/app/zap-templates/app-templates.json",
      "type": "gen-templates-json",
      "version": "chip-v1"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SubjectsPerAccessControlEntry [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TargetsPerAccessControlEntry [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AccessControlEntriesPerFabric [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => AccessControlEntryChanged"
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
            "    0xfff8 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
          "name": "Localization Configuration",
          "code": "0x002b",
          "define": "LOCALIZATION_CONFIGURATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ActiveLocale [char_string]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => HourFormat [HourFormatEnum]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveCalendarType [CalendarTypeEnum]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SupportedCalendarTypes [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Status [PowerSourceStatusEnum]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Order [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Description [char_string]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredAssessedInputVoltage [int32u]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredAssessedInputFrequency [int16u]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredCurrentType [WiredCurrentTypeEnum]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredAssessedCurrent [int32u]",
            "    0x0007 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredNominalVoltage [int32u]",
            "    0x0008 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredMaximumCurrent [int32u]",
            "    0x0009 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WiredPresent [boolean]",
            "    0x000a |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveWiredFaults [array]",
            "    0x000b |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatVoltage [int32u]",
            "    0x000c |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatPercentRemaining [int8u]",
            "    0x000d |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatTimeRemaining [int32u]",
            "    0x000e |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatChargeLevel [BatChargeLevelEnum]",
            "    0x000f |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatReplacementNeeded [boolean]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatReplaceability [BatReplaceabilityEnum]",
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatPresent [boolean]",
            "    0x0012 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveBatFaults [array]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatReplacementDescription [char_string]",
            "    0x0014 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatCommonDesignation [BatCommonDesignationEnum]",
            "    0x0015 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatANSIDesignation [char_string]",
            "    0x0016 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatIECDesignation [char_string]",
            "    0x0017 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatApprovedChemistry [BatApprovedChemistryEnum]",
            "    0x0018 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatCapacity [int32u]",
            "    0x0019 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatQuantity [int8u]",
            "    0x001a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatChargeState [BatChargeStateEnum]",
            "    0x001b |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatTimeToFullCharge [int32u]",
            "    0x001c |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatFunctionalWhileCharging [boolean]",
            "    0x001d |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => BatChargingCurrent [int32u]",
            "    0x001e |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveBatChargeFaults [array]",
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "0x0004 |         | client |        1 |       1 => CommissioningComplete"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0001 |         | server |        0 |       1 => ArmFailSafeResponse",
            "0x0003 |         | server |        0 |       1 => SetRegulatoryConfigResponse",
            "0x0005 |         | server |        0 |       1 => CommissioningCompleteResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |   0x0000000000000000 |          1 |                   1 |               65534 |                     => Breadcrumb [int64u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => BasicCommissioningInfo [BasicCommissioningInfo]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => RegulatoryConfig [RegulatoryLocationTypeEnum]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => LocationCapability [RegulatoryLocationTypeEnum]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => SupportsConcurrentConnection [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
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
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       0 => ResetCounts"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => Channel [int16u]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => RoutingRole [RoutingRoleEnum]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => NetworkName [char_string]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PanId [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => ExtendedPanId [int64u]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => MeshLocalPrefix [octet_string]",
            "    0x0006 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => OverrunCount [int64u]",
            "    0x0007 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => NeighborTable [array]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => RouteTable [array]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => PartitionId [int32u]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => Weighting [int8u]",
            "    0x000b |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => DataVersion [int8u]",
            "    0x000c |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => StableDataVersion [int8u]",
            "    0x000d |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => LeaderRouterId [int8u]",
            "    0x000e |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => DetachedRoleCount [int16u]",
            "    0x000f |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => ChildRoleCount [int16u]",
            "    0x0010 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => RouterRoleCount [int16u]",
            "    0x0011 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => LeaderRoleCount [int16u]",
            "    0x0012 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => AttachAttemptCount [int16u]",
            "    0x0013 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => PartitionIdChangeCount [int16u]",
            "    0x0014 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => BetterPartitionAttachAttemptCount [int16u]",
            "    0x0015 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => ParentChangeCount [int16u]",
            "    0x0016 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => TxTotalCount [int32u]",
            "    0x0017 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxUnicastCount [int32u]",
            "    0x0018 |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => TxBroadcastCount [int32u]",
            "    0x0019 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxAckRequestedCount [int32u]",
            "    0x001a |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => TxAckedCount [int32u]",
            "    0x001b |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxNoAckRequestedCount [int32u]",
            "    0x001c |         | server |           Ext |           |         |                    0 |          1 |                     |               65534 |                     => TxDataCount [int32u]",
            "    0x001d |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxDataPollCount [int32u]",
            "    0x001e |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxBeaconCount [int32u]",
            "    0x001f |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxBeaconRequestCount [int32u]",
            "    0x0020 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxOtherCount [int32u]",
            "    0x0021 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxRetryCount [int32u]",
            "    0x0022 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxDirectMaxRetryExpiryCount [int32u]",
            "    0x0023 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxIndirectMaxRetryExpiryCount [int32u]",
            "    0x0024 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxErrCcaCount [int32u]",
            "    0x0025 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxErrAbortCount [int32u]",
            "    0x0026 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => TxErrBusyChannelCount [int32u]",
            "    0x0027 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxTotalCount [int32u]",
            "    0x0028 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxUnicastCount [int32u]",
            "    0x0029 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxBroadcastCount [int32u]",
            "    0x002a |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxDataCount [int32u]",
            "    0x002b |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxDataPollCount [int32u]",
            "    0x002c |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxBeaconCount [int32u]",
            "    0x002d |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxBeaconRequestCount [int32u]",
            "    0x002e |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxOtherCount [int32u]",
            "    0x002f |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxAddressFilteredCount [int32u]",
            "    0x0030 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxDestAddrFilteredCount [int32u]",
            "    0x0031 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxDuplicatedCount [int32u]",
            "    0x0032 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrNoFrameCount [int32u]",
            "    0x0033 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrUnknownNeighborCount [int32u]",
            "    0x0034 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrInvalidSrcAddrCount [int32u]",
            "    0x0035 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrSecCount [int32u]",
            "    0x0036 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrFcsCount [int32u]",
            "    0x0037 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => RxErrOtherCount [int32u]",
            "    0x0038 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => ActiveTimestamp [int64u]",
            "    0x0039 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => PendingTimestamp [int64u]",
            "    0x003a |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => Delay [int32u]",
            "    0x003b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SecurityPolicy [SecurityPolicy]",
            "    0x003c |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ChannelPage0Mask [octet_string]",
            "    0x003d |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OperationalDatasetComponents [OperationalDatasetComponents]",
            "    0x003e |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveNetworkFaultsList [array]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
          "name": "Bridged Device Basic Information",
          "code": "0x0039",
          "define": "BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0001 |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => VendorName [char_string]",
            "    0x0002 |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => VendorID [vendor_id]",
            "    0x0003 |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => ProductName [char_string]",
            "    0x0005 |         | server |           NVM | singleton |         |                      |          1 |                     |               65344 |                     => NodeLabel [char_string]",
            "    0x0007 |         | server |           RAM | singleton |         |                    0 |          1 |                     |               65344 |                     => HardwareVersion [int16u]",
            "    0x0008 |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => HardwareVersionString [char_string]",
            "    0x0009 |         | server |           RAM | singleton |         |                    0 |          1 |                     |               65344 |                     => SoftwareVersion [int32u]",
            "    0x000a |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => SoftwareVersionString [char_string]",
            "    0x000b |         | server |           RAM | singleton |         |     20210614123456ZZ |          1 |                     |               65344 |                     => ManufacturingDate [char_string]",
            "    0x000c |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => PartNumber [char_string]",
            "    0x000d |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => ProductURL [long_char_string]",
            "    0x000e |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => ProductLabel [char_string]",
            "    0x000f |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => SerialNumber [char_string]",
            "    0x0011 |         | server |           RAM | singleton |         |                    1 |          1 |                     |               65344 |                     => Reachable [boolean]",
            "    0x0012 |         | server |           RAM | singleton |         |                      |          1 |                     |               65344 |                     => UniqueID [char_string]",
            "    0x0014 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65344 |                     => ProductAppearance [ProductAppearanceStruct]",
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
          "name": "Switch",
          "code": "0x003b",
          "define": "SWITCH_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => NumberOfPositions [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => CurrentPosition [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => MultiPressMax [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => AttestationRequest",
            "0x0002 |         | client |        1 |       1 => CertificateChainRequest",
            "0x0004 |         | client |        1 |       1 => CSRRequest",
            "0x0006 |         | client |        1 |       1 => AddNOC",
            "0x0007 |         | client |        1 |       1 => UpdateNOC",
            "0x0009 |         | client |        1 |       1 => UpdateFabricLabel",
            "0x000a |         | client |        1 |       1 => RemoveFabric",
            "0x000b |         | client |        1 |       1 => AddTrustedRootCertificate"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Fabrics [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SupportedFabrics [int8u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CommissionedFabrics [int8u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TrustedRootCertificates [array]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CurrentFabricIndex [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
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
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       1 => ChangeToMode"
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
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Description [char_string]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => StandardNamespace [enum16]",
            "    0x0002 |         | server |           Ext |           |         |                  255 |          1 |                   1 |               65534 |                     => SupportedModes [array]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => CurrentMode [int8u]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => StartUpMode [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0003 |         | client |        1 |       1 => UnlockWithTimeout",
            "0x000b |         | client |        1 |       1 => SetWeekDaySchedule",
            "0x000c |         | client |        0 |       1 => GetWeekDaySchedule",
            "0x000d |         | client |        1 |       1 => ClearWeekDaySchedule",
            "0x000e |         | client |        1 |       1 => SetYearDaySchedule",
            "0x000f |         | client |        0 |       1 => GetYearDaySchedule",
            "0x0010 |         | client |        1 |       1 => ClearYearDaySchedule",
            "0x0011 |         | client |        1 |       1 => SetHolidaySchedule",
            "0x0012 |         | client |        0 |       1 => GetHolidaySchedule",
            "0x0013 |         | client |        1 |       1 => ClearHolidaySchedule",
            "0x001a |         | client |        1 |       1 => SetUser",
            "0x001b |         | client |        0 |       1 => GetUser",
            "0x0022 |         | client |        0 |       1 => SetCredential",
            "0x0024 |         | client |        0 |       1 => GetCredentialStatus",
            "0x0026 |         | client |        1 |       1 => ClearCredential"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x000c |         | server |        1 |       0 => GetWeekDayScheduleResponse",
            "0x000f |         | server |        1 |       0 => GetYearDayScheduleResponse",
            "0x0012 |         | server |        1 |       0 => GetHolidayScheduleResponse",
            "0x0023 |         | server |        1 |       0 => SetCredentialResponse",
            "0x0025 |         | server |        1 |       0 => GetCredentialStatusResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => LockState [DlLockState]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => LockType [DlLockType]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => ActuatorEnabled [boolean]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => DoorState [DoorStateEnum]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => DoorOpenEvents [int32u]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => DoorClosedEvents [int32u]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => OpenPeriod [int16u]",
            "    0x0011 |         | server |           RAM |           |         |                    0 |          1 |                     |               65534 |                     => NumberOfTotalUsersSupported [int16u]",
            "    0x0012 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => NumberOfPINUsersSupported [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                    0 |          1 |                     |               65534 |                     => NumberOfRFIDUsersSupported [int16u]",
            "    0x0014 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => NumberOfWeekDaySchedulesSupportedPerUser [int8u]",
            "    0x0015 |         | server |           RAM |           |         |                    0 |          1 |                     |               65534 |                     => NumberOfYearDaySchedulesSupportedPerUser [int8u]",
            "    0x0016 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => NumberOfHolidaySchedulesSupported [int8u]",
            "    0x0017 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => MaxPINCodeLength [int8u]",
            "    0x0018 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MinPINCodeLength [int8u]",
            "    0x0019 |         | server |           RAM |           |         |                      |          1 |                     |               65534 |                     => MaxRFIDCodeLength [int8u]",
            "    0x001a |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => MinRFIDCodeLength [int8u]",
            "    0x001b |         | server |           RAM |           |         |                    1 |          1 |                     |               65534 |                     => CredentialRulesSupport [DlCredentialRuleMask]",
            "    0x001c |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => NumberOfCredentialsSupportedPerUser [int8u]",
            "    0x0021 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Language [char_string]",
            "    0x0022 |         | server |           RAM |           |         |                    0 |          1 |                     |               65534 |                     => LEDSettings [int8u]",
            "    0x0023 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => AutoRelockTime [int32u]",
            "    0x0024 |         | server |           RAM |           |         |                    0 |          1 |                     |               65534 |                     => SoundVolume [int8u]",
            "    0x0025 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => OperatingMode [OperatingModeEnum]",
            "    0x0026 |         | server |           RAM |           |         |                65526 |          1 |                     |               65534 |                     => SupportedOperatingModes [DlSupportedOperatingModes]",
            "    0x0027 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => DefaultConfigurationRegister [DlDefaultConfigurationRegister]",
            "    0x0028 |         | server |           RAM |           |         |                    1 |          1 |                     |               65534 |                     => EnableLocalProgramming [boolean]",
            "    0x0029 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => EnableOneTouchLocking [boolean]",
            "    0x002a |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => EnableInsideStatusLED [boolean]",
            "    0x002b |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => EnablePrivacyModeButton [boolean]",
            "    0x002c |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => LocalProgrammingFeatures [DlLocalProgrammingFeatures]",
            "    0x0030 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => WrongCodeEntryLimit [int8u]",
            "    0x0031 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => UserCodeTemporaryDisableTime [int8u]",
            "    0x0032 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => SendPINOverTheAir [boolean]",
            "    0x0033 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => RequirePINforRemoteOperation [boolean]",
            "    0x0035 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ExpiringUserTimeout [int16u]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Type [Type]",
            "    0x0003 |         | server |           NVM |           |         |               0x7FFF |          1 |                     |               65344 |                     => CurrentPositionLift [int16u]",
            "    0x0004 |         | server |           NVM |           |         |               0x7FFF |          1 |                     |               65344 |                     => CurrentPositionTilt [int16u]",
            "    0x0007 |         | server |           NVM |           |         |                 0x03 |          1 |                     |               65344 |                     => ConfigStatus [ConfigStatus]",
            "    0x0008 |         | server |           NVM |           |         |                   50 |          1 |                     |                 100 |                     => CurrentPositionLiftPercentage [Percent]",
            "    0x0009 |         | server |           NVM |           |         |                   50 |          1 |                     |                 100 |                     => CurrentPositionTiltPercentage [Percent]",
            "    0x000a |         | server |           RAM |           |         |                 0x00 |          1 |                     |                 127 |                     => OperationalStatus [OperationalStatus]",
            "    0x000b |         | server |           RAM |           |         |                  500 |          1 |                     |               10000 |                     => TargetPositionLiftPercent100ths [Percent100ths]",
            "    0x000c |         | server |           RAM |           |         |                  500 |          1 |                     |               10000 |                     => TargetPositionTiltPercent100ths [Percent100ths]",
            "    0x000d |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => EndProductType [EndProductType]",
            "    0x000e |         | server |           NVM |           |         |                  500 |          1 |                     |               10000 |                     => CurrentPositionLiftPercent100ths [Percent100ths]",
            "    0x000f |         | server |           NVM |           |         |                  500 |          1 |                     |               10000 |                     => CurrentPositionTiltPercent100ths [Percent100ths]",
            "    0x0010 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65344 |                     => InstalledOpenLimitLift [int16u]",
            "    0x0011 |         | server |           NVM |           |         |               0xFFFF |          1 |                     |               65344 |                     => InstalledClosedLimitLift [int16u]",
            "    0x0012 |         | server |           NVM |           |         |               0x0000 |          1 |                     |               65344 |                     => InstalledOpenLimitTilt [int16u]",
            "    0x0013 |         | server |           NVM |           |         |               0xFFFF |          1 |                     |               65344 |                     => InstalledClosedLimitTilt [int16u]",
            "    0x0017 |         | server |           NVM |           |         |                 0x00 |          1 |                     |                  15 |                     => Mode [Mode]",
            "    0x001a |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => SafetyStatus [SafetyStatus]",
            "    0xfffc |         | server |           RAM |           |         |                 0x17 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OutdoorTemperature [int16s]",
            "    0x0002 |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => Occupancy [bitmap8]",
            "    0x0003 |         | server |           RAM |           |         |               0x02BC |          1 |                     |               65344 |                     => AbsMinHeatSetpointLimit [int16s]",
            "    0x0004 |         | server |           Ext |           |         |               0x0BB8 |          1 |                     |               65344 |                     => AbsMaxHeatSetpointLimit [int16s]",
            "    0x0005 |         | server |           RAM |           |         |               0x0640 |          1 |                     |               65344 |                     => AbsMinCoolSetpointLimit [int16s]",
            "    0x0006 |         | server |           RAM |           |         |               0x0C80 |          1 |                     |               65344 |                     => AbsMaxCoolSetpointLimit [int16s]",
            "    0x0007 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => PICoolingDemand [int8u]",
            "    0x0008 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => PIHeatingDemand [int8u]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => HVACSystemTypeConfiguration [bitmap8]",
            "    0x0010 |         | server |           Ext |           |         |                    0 |          1 |                     |               65344 |                     => LocalTemperatureCalibration [int8s]",
            "    0x0011 |         | server |           RAM |           |         |               0x0A28 |          1 |                     |               65344 |                     => OccupiedCoolingSetpoint [int16s]",
            "    0x0012 |         | server |           RAM |           |         |               0x07D0 |          1 |                     |               65344 |                     => OccupiedHeatingSetpoint [int16s]",
            "    0x0013 |         | server |           Ext |           |         |               0x0A28 |          1 |                     |               65344 |                     => UnoccupiedCoolingSetpoint [int16s]",
            "    0x0014 |         | server |           Ext |           |         |               0x07D0 |          1 |                     |               65344 |                     => UnoccupiedHeatingSetpoint [int16s]",
            "    0x0015 |         | server |           RAM |           |         |               0x02BC |          1 |                     |               65344 |                     => MinHeatSetpointLimit [int16s]",
            "    0x0016 |         | server |           RAM |           |         |               0x0BB8 |          1 |                     |               65344 |                     => MaxHeatSetpointLimit [int16s]",
            "    0x0017 |         | server |           RAM |           |         |               0x0640 |          1 |                     |               65344 |                     => MinCoolSetpointLimit [int16s]",
            "    0x0018 |         | server |           RAM |           |         |               0x0C80 |          1 |                     |               65344 |                     => MaxCoolSetpointLimit [int16s]",
            "    0x0019 |         | server |           RAM |           |         |                 0x19 |          1 |                   1 |               65534 |                     => MinSetpointDeadBand [int8s]",
            "    0x001a |         | server |           Ext |           |         |                  0x0 |          1 |                   1 |               65534 |                     => RemoteSensing [bitmap8]",
            "    0x001b |         | server |           RAM |           |         |                 0x04 |          1 |                     |               65344 |                     => ControlSequenceOfOperation [ThermostatControlSequence]",
            "    0x001c |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => SystemMode [enum8]",
            "    0x001e |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ThermostatRunningMode [enum8]",
            "    0x0020 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => StartOfWeek [enum8]",
            "    0x0021 |         | server |           RAM |           |         |                    7 |          1 |                     |               65344 |                     => NumberOfWeeklyTransitions [int8u]",
            "    0x0022 |         | server |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => NumberOfDailyTransitions [int8u]",
            "    0x0023 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => TemperatureSetpointHold [enum8]",
            "    0x0024 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => TemperatureSetpointHoldDuration [int16u]",
            "    0x0025 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ThermostatProgrammingOperationMode [bitmap8]",
            "    0x0029 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ThermostatRunningState [bitmap16]",
            "    0x0030 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => SetpointChangeSource [enum8]",
            "    0x0031 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SetpointChangeAmount [int16s]",
            "    0x0032 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => SetpointChangeSourceTimestamp [epoch_s]",
            "    0x0034 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OccupiedSetback [int8u]",
            "    0x0035 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OccupiedSetbackMin [int8u]",
            "    0x0036 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OccupiedSetbackMax [int8u]",
            "    0x0037 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => UnoccupiedSetback [int8u]",
            "    0x0038 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => UnoccupiedSetbackMin [int8u]",
            "    0x0039 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => UnoccupiedSetbackMax [int8u]",
            "    0x003a |         | server |           Ext |           |         |              0x009F6 |          1 |                     |               65344 |                     => EmergencyHeatDelta [int8u]",
            "    0x0040 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ACType [enum8]",
            "    0x0041 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ACCapacity [int16u]",
            "    0x0042 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ACRefrigerantType [enum8]",
            "    0x0043 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ACCompressorType [enum8]",
            "    0x0045 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ACLouverPosition [enum8]",
            "    0x0046 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ACCoilTemperature [int16s]",
            "    0x0047 |         | server |           Ext |           |         |                 0x00 |          1 |                     |               65344 |                     => ACCapacityformat [enum8]",
            "    0xfffc |         | server |           RAM |           |         |               0x000b |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thermostat User Interface Configuration",
          "code": "0x0204",
          "define": "THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => TemperatureDisplayMode [enum8]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => KeypadLockout [enum8]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ScheduleProgrammingVisibility [enum8]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Tolerance [int16u]",
            "    0x0010 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => ScaledValue [int16s]",
            "    0x0011 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => MinScaledValue [int16s]",
            "    0x0012 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => MaxScaledValue [int16s]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ScaledTolerance [int16u]",
            "    0x0014 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => Scale [int8s]",
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
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |               0x2710 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [int16u]",
            "    0x0003 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Tolerance [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => ChangeChannel",
            "0x0002 |         | client |        1 |       0 => ChangeChannelByNumber",
            "0x0003 |         | client |        1 |       0 => SkipChannel"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65344 |                     => ChannelList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Lineup [LineupInfoStruct]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CurrentChannel [ChannelInfoStruct]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0001 |         | server |        1 |       0 => NavigateTargetResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TargetList [array]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => CurrentTarget [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       1 => Play",
            "0x0001 |         | client |        1 |       1 => Pause",
            "0x0002 |         | client |        1 |       1 => Stop",
            "0x0003 |         | client |        1 |       1 => StartOver",
            "0x0004 |         | client |        1 |       1 => Previous",
            "0x0005 |         | client |        1 |       1 => Next",
            "0x0006 |         | client |        1 |       1 => Rewind",
            "0x0007 |         | client |        1 |       1 => FastForward",
            "0x0008 |         | client |        1 |       1 => SkipForward",
            "0x0009 |         | client |        1 |       1 => SkipBackward",
            "0x000b |         | client |        1 |       1 => Seek"
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
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x000a |         | server |        1 |       1 => PlaybackResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => CurrentState [PlaybackStateEnum]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => StartTime [epoch_us]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Duration [int64u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SampledPosition [PlaybackPositionStruct]",
            "    0x0004 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => PlaybackSpeed [single]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => SeekRangeEnd [int64u]",
            "    0x0006 |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => SeekRangeStart [int64u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => SelectInput",
            "0x0001 |         | client |        1 |       0 => ShowInputStatus",
            "0x0002 |         | client |        1 |       0 => HideInputStatus",
            "0x0003 |         | client |        1 |       0 => RenameInput"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => InputList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65534 |                     => CurrentInput [int8u]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => Sleep"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0001 |         | server |        1 |       0 => SendKeyResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => LaunchContent",
            "0x0001 |         | client |        1 |       1 => LaunchURL"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Content Launcher",
          "code": "0x050a",
          "define": "CONTENT_LAUNCHER_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0002 |         | server |        1 |       1 => LauncherResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptHeader [array]",
            "    0x0001 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => SupportedStreamingProtocols [bitmap32]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => SelectOutput",
            "0x0001 |         | client |        1 |       0 => RenameOutput"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65344 |                     => OutputList [array]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                   1 |               65344 |                     => CurrentOutput [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Application Launcher",
          "code": "0x050c",
          "define": "APPLICATION_LAUNCHER_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0003 |         | server |        1 |       0 => LauncherResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CatalogList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => CurrentApp [ApplicationEPStruct]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => VendorName [char_string]",
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => VendorID [vendor_id]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ApplicationName [char_string]",
            "    0x0003 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => ProductID [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Application [ApplicationStruct]",
            "    0x0005 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => Status [ApplicationStatusEnum]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ApplicationVersion [char_string]",
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AllowedVendorList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | client |        1 |       0 => GetSetupPIN",
            "0x0002 |         | client |        1 |       0 => Login",
            "0x0003 |         | client |        1 |       0 => Logout"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Account Login",
          "code": "0x050e",
          "define": "ACCOUNT_LOGIN_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        0 |       1 => GetSetupPINResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 2,
      "name": "Anonymous Endpoint Type",
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
          "side": "client",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       0 => Identify",
            "0x0040 |         | client |        1 |       0 => TriggerEffect"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Identify",
          "code": "0x0003",
          "define": "IDENTIFY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => NameSupport [NameSupportBitmap]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                 0x00 |          1 |                   1 |               65534 |                     => SceneCount [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentScene [int8u]",
            "    0x0002 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => CurrentGroup [group_id]",
            "    0x0003 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => SceneValid [boolean]",
            "    0x0004 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => NameSupport [bitmap8]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0040 |         | client |        1 |       1 => OffWithEffect",
            "0x0041 |         | client |        1 |       1 => OnWithRecallGlobalScene",
            "0x0042 |         | client |        1 |       1 => OnWithTimedOff"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                     |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65534 |                     => OnOff [boolean]",
            "    0x4000 |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65534 |                     => GlobalSceneControl [boolean]",
            "    0x4001 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65534 |                     => OnTime [int16u]",
            "    0x4002 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65534 |                     => OffWaitTime [int16u]",
            "    0x4003 |         | server |           RAM |           |         |                 0xFF |          1 |                     |               65534 |                     => StartUpOnOff [OnOffStartUpOnOff]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | client |           RAM |           |         |                    5 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentLevel [int8u]",
            "    0x0001 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => RemainingTime [int16u]",
            "    0x0002 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => MinLevel [int8u]",
            "    0x0003 |         | server |           RAM |           |         |                 0xFE |          1 |                   1 |               65534 |                     => MaxLevel [int8u]",
            "    0x0004 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => CurrentFrequency [int16u]",
            "    0x0005 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => MinFrequency [int16u]",
            "    0x0006 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => MaxFrequency [int16u]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => Options [LevelControlOptions]",
            "    0x0010 |         | server |           RAM |           |         |               0x0000 |          1 |                   1 |               65534 |                     => OnOffTransitionTime [int16u]",
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OnLevel [int8u]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OnTransitionTime [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => OffTransitionTime [int16u]",
            "    0x0014 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => DefaultMoveRate [int8u]",
            "    0x4000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => StartUpCurrentLevel [int8u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0011 |         | server |           RAM | singleton |         |                    1 |          1 |                     |               65344 |                     => Reachable [boolean]",
            "    0x0012 |         | server |           Ext | singleton |         |                      |          1 |                     |               65344 |                     => UniqueID [char_string]",
            "    0x0013 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => CapabilityMinima [CapabilityMinimaStruct]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM | singleton |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Localization Configuration",
          "code": "0x002b",
          "define": "LOCALIZATION_CONFIGURATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | client |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ActiveLocale [char_string]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SupportedLocales [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Unit Localization",
          "code": "0x002d",
          "define": "UNIT_LOCALIZATION_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM | singleton |         |                      |          1 |                     |               65534 |                     => TemperatureUnit [TempUnitEnum]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
          "name": "Fixed Label",
          "code": "0x0040",
          "define": "FIXED_LABEL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => LabelList [array]",
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
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "0x0040 |         | client |        1 |       1 => EnhancedMoveToHue",
            "0x0041 |         | client |        1 |       1 => EnhancedMoveHue",
            "0x0042 |         | client |        1 |       1 => EnhancedStepHue",
            "0x0043 |         | client |        1 |       1 => EnhancedMoveToHueAndSaturation",
            "0x0044 |         | client |        1 |       1 => ColorLoopSet",
            "0x0047 |         | client |        1 |       1 => StopMoveStep",
            "0x004b |         | client |        1 |       1 => MoveColorTemperature",
            "0x004c |         | client |        1 |       1 => StepColorTemperature"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffd |         | client |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentHue [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentSaturation [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => RemainingTime [int16u]",
            "    0x0003 |         | server |           RAM |           |         |               0x616B |          1 |                   1 |               65534 |                     => CurrentX [int16u]",
            "    0x0004 |         | server |           RAM |           |         |               0x607D |          1 |                   1 |               65534 |                     => CurrentY [int16u]",
            "    0x0005 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => DriftCompensation [enum8]",
            "    0x0006 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => CompensationText [char_string]",
            "    0x0007 |         | server |           RAM |           |         |               0x00FA |          1 |                   1 |               65534 |                     => ColorTemperatureMireds [int16u]",
            "    0x0008 |         | server |           RAM | singleton |         |                    1 |          1 |                   1 |               65534 |                     => ColorMode [enum8]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => Options [bitmap8]",
            "    0x0010 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => NumberOfPrimaries [int8u]",
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary1X [int16u]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary1Y [int16u]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary1Intensity [int8u]",
            "    0x0015 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary2X [int16u]",
            "    0x0016 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary2Y [int16u]",
            "    0x0017 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary2Intensity [int8u]",
            "    0x0019 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary3X [int16u]",
            "    0x001a |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary3Y [int16u]",
            "    0x001b |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary3Intensity [int8u]",
            "    0x0020 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary4X [int16u]",
            "    0x0021 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary4Y [int16u]",
            "    0x0022 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary4Intensity [int8u]",
            "    0x0024 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary5X [int16u]",
            "    0x0025 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary5Y [int16u]",
            "    0x0026 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary5Intensity [int8u]",
            "    0x0028 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary6X [int16u]",
            "    0x0029 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary6Y [int16u]",
            "    0x002a |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => Primary6Intensity [int8u]",
            "    0x0030 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => WhitePointX [int16u]",
            "    0x0031 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => WhitePointY [int16u]",
            "    0x0032 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointRX [int16u]",
            "    0x0033 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointRY [int16u]",
            "    0x0034 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointRIntensity [int8u]",
            "    0x0036 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointGX [int16u]",
            "    0x0037 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointGY [int16u]",
            "    0x0038 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointGIntensity [int8u]",
            "    0x003a |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointBX [int16u]",
            "    0x003b |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointBY [int16u]",
            "    0x003c |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => ColorPointBIntensity [int8u]",
            "    0x4000 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65534 |                     => EnhancedCurrentHue [int16u]",
            "    0x4001 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => EnhancedColorMode [enum8]",
            "    0x4002 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65534 |                     => ColorLoopActive [int8u]",
            "    0x4003 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65534 |                     => ColorLoopDirection [int8u]",
            "    0x4004 |         | server |           RAM |           |         |               0x0019 |          1 |                     |               65534 |                     => ColorLoopTime [int16u]",
            "    0x4005 |         | server |           RAM |           |         |               0x2300 |          1 |                     |               65534 |                     => ColorLoopStartEnhancedHue [int16u]",
            "    0x4006 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65534 |                     => ColorLoopStoredEnhancedHue [int16u]",
            "    0x400a |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => ColorCapabilities [bitmap16]",
            "    0x400b |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => ColorTempPhysicalMinMireds [int16u]",
            "    0x400c |         | server |           RAM |           |         |               0xfeff |          1 |                   1 |               65534 |                     => ColorTempPhysicalMaxMireds [int16u]",
            "    0x400d |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => CoupleColorTempToLevelMinMireds [int16u]",
            "    0x4010 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => StartUpColorTemperatureMireds [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0010 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => PIROccupiedToUnoccupiedDelay [int16u]",
            "    0x0011 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => PIRUnoccupiedToOccupiedDelay [int16u]",
            "    0x0012 |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => PIRUnoccupiedToOccupiedThreshold [int8u]",
            "    0x0020 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => UltrasonicOccupiedToUnoccupiedDelay [int16u]",
            "    0x0021 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => UltrasonicUnoccupiedToOccupiedDelay [int16u]",
            "    0x0022 |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => UltrasonicUnoccupiedToOccupiedThreshold [int8u]",
            "    0x0030 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => PhysicalContactOccupiedToUnoccupiedDelay [int16u]",
            "    0x0031 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => PhysicalContactUnoccupiedToOccupiedDelay [int16u]",
            "    0x0032 |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => PhysicalContactUnoccupiedToOccupiedThreshold [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    3 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 1,
      "profileId": 259,
      "endpointId": 1,
      "networkId": 0
    }
  ]
}