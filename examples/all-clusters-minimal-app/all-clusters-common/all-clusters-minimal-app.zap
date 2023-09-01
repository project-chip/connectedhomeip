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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "0x0000 |         | server => AccessControlEntryChanged"
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
            "0x0000 |         | server => StartUp"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => Status [PowerSourceStatusEnum]",
            "    0x0001 |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => Order [int8u]",
            "    0x0002 |         | server |           RAM |           |         |                   B1 |          1 |                   1 |               65534 |                     => Description [char_string]",
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0000 |         | client |        1 |       0 => TestEventTrigger"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => NetworkInterfaces [array]",
            "    0x0001 |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => RebootCount [int16u]",
            "    0x0008 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TestEventTriggersEnabled [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0007 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => NeighborTable [array]",
            "    0x0008 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => RouteTable [array]",
            "    0x0009 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartitionId [int32u]",
            "    0x000a |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => Weighting [int8u]",
            "    0x000b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DataVersion [int8u]",
            "    0x000c |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => StableDataVersion [int8u]",
            "    0x000d |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => LeaderRouterId [int8u]",
            "    0x003b |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => SecurityPolicy [SecurityPolicy]",
            "    0x003c |         | server |           Ext |           |         |               0x0000 |          1 |                     |               65344 |                     => ChannelPage0Mask [octet_string]",
            "    0x003d |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => OperationalDatasetComponents [OperationalDatasetComponents]",
            "    0x003e |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ActiveNetworkFaultsList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           NVM |           |         |                 0xFE |          1 |                     |               65344 |                     => CurrentLevel [int8u]",
            "    0x0001 |         | server |           RAM |           |         |               0x0000 |          1 |                     |               65344 |                     => RemainingTime [int16u]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Options [LevelControlOptions]",
            "    0x0011 |         | server |           RAM |           |         |                 0xFF |          1 |                   1 |               65534 |                     => OnLevel [int8u]",
            "    0x4000 |         | server |           NVM |           |         |                  255 |          1 |                     |               65344 |                     => StartUpCurrentLevel [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartsList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
          "name": "Switch",
          "code": "0x003b",
          "define": "SWITCH_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => NumberOfPositions [int8u]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => CurrentPosition [int8u]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
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
          "name": "Door Lock",
          "code": "0x0101",
          "define": "DOOR_LOCK_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            " code  | mfgCode | source | incoming | outgoing"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                    2 |          1 |                     |               65344 |                     => LockState [DlLockState]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => LockType [DlLockType]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ActuatorEnabled [boolean]",
            "    0x0023 |         | server |           RAM |           |         |                   60 |          1 |                     |               65344 |                     => AutoRelockTime [int32u]",
            "    0x0025 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => OperatingMode [OperatingModeEnum]",
            "    0x0026 |         | server |           RAM |           |         |               0xFFF6 |          1 |                   1 |               65534 |                     => SupportedOperatingModes [DlSupportedOperatingModes]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    6 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
          ],
          "events": [
            " code  | mfgCode | side",
            "0x0000 |         | server => DoorLockAlarm",
            "0x0002 |         | server => LockOperation",
            "0x0003 |         | server => LockOperationError"
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
            "    0x0007 |         | server |           NVM |           |         |                 0x03 |          1 |                     |               65344 |                     => ConfigStatus [ConfigStatus]",
            "    0x000a |         | server |           RAM |           |         |                 0x00 |          1 |                     |                 127 |                     => OperationalStatus [OperationalStatus]",
            "    0x000d |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => EndProductType [EndProductType]",
            "    0x0017 |         | server |           NVM |           |         |                 0x00 |          1 |                     |                  15 |                     => Mode [Mode]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
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
            "    0x0011 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => EffectiveOperationMode [OperationModeEnum]",
            "    0x0012 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => EffectiveControlMode [ControlModeEnum]",
            "    0x0013 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => Capacity [int16s]",
            "    0x0020 |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => OperationMode [OperationModeEnum]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0012 |         | server |           RAM |           |         |               0x07D0 |          1 |                     |               65344 |                     => OccupiedHeatingSetpoint [int16s]",
            "    0x001b |         | server |           RAM |           |         |                 0x04 |          1 |                     |               65344 |                     => ControlSequenceOfOperation [ThermostatControlSequence]",
            "    0x001c |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => SystemMode [enum8]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0008 |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => ColorMode [enum8]",
            "    0x000f |         | server |           RAM |           |         |                 0x00 |          1 |                     |               65344 |                     => Options [bitmap8]",
            "    0x0010 |         | server |           RAM |           |         |                    0 |          1 |                     |               65344 |                     => NumberOfPrimaries [int8u]",
            "    0x4001 |         | server |           RAM |           |         |                 0x01 |          1 |                     |               65344 |                     => EnhancedColorMode [enum8]",
            "    0x400a |         | server |           RAM |           |         |                 0x1F |          1 |                     |               65344 |                     => ColorCapabilities [bitmap16]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0010 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => MinLevel [int8u]",
            "    0x0011 |         | server |           RAM |           |         |                 0xFE |          1 |                   1 |               65534 |                     => MaxLevel [int8u]",
            "    0x0020 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LampQuantity [int8u]",
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
          "name": "Wake on LAN",
          "code": "0x0503",
          "define": "WAKE_ON_LAN_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
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
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x000a |         | server |        0 |       1 => PlaybackResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => CurrentState [PlaybackStateEnum]",
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
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0003 |         | server |        0 |       1 => LauncherResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
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
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                     |               65344 |                     => ApplicationName [char_string]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => Application [ApplicationStruct]",
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
          "commands": [
            " code  | mfgCode | source | incoming | outgoing",
            "0x0001 |         | server |        0 |       1 => GetSetupPINResponse"
          ],
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x001f |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EndpointList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                     |               65344 |                     => PartsList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           Ext |           |         |                    1 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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