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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | client |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        0 |       1 => QueryImageResponse",
            "0x0003 |         | server |        1 |       1 => ApplyUpdateResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0001 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => SupportedLocales [array]",
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
            "0x0003 |         | server => BootReason"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => WindowStatus [CommissioningWindowStatusEnum]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AdminFabricIndex [fabric_idx]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AdminVendorId [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => GroupKeyMap [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => GroupTable [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupsPerFabric [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxGroupKeysPerFabric [int16u]",
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
    }
  ]
}