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
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => SubjectsPerAccessControlEntry [int16u]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => TargetsPerAccessControlEntry [int16u]",
            "    0x0004 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AccessControlEntriesPerFabric [int16u]",
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
            "    0x0013 |         | server |           Ext | singleton |         |                      |          1 |                   1 |               65534 |                     => CapabilityMinima [CapabilityMinimaStruct]",
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
            "    0xfffd |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => WindowStatus [CommissioningWindowStatusEnum]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AdminFabricIndex [fabric_idx]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AdminVendorId [int16u]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | server |           RAM |           |         |               0x0001 |          1 |                     |               65344 |                     => ClusterRevision [int16u]"
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
        }
      ]
    },
    {
      "id": 2,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 45,
        "profileId": 259,
        "label": "MA-air-purifier",
        "name": "MA-air-purifier"
      },
      "deviceTypes": [
        {
          "code": 45,
          "profileId": 259,
          "label": "MA-air-purifier",
          "name": "MA-air-purifier"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        45
      ],
      "deviceTypeName": "MA-air-purifier",
      "deviceTypeCode": 45,
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
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => DeviceTypeList [array]",
            "    0x0001 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ServerList [array]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ClientList [array]",
            "    0x0003 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => PartsList [array]",
            "    0xfff8 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                  100 |          1 |                   1 |               65534 |                     => Condition [percent]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => DegradationDirection [DegradationDirectionEnum]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ChangeIndication [ChangeIndicationEnum]",
            "    0x0003 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => InPlaceIndicator [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                  100 |          1 |                   1 |               65534 |                     => Condition [percent]",
            "    0x0001 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => DegradationDirection [DegradationDirectionEnum]",
            "    0x0002 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => ChangeIndication [ChangeIndicationEnum]",
            "    0x0003 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => InPlaceIndicator [boolean]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FanMode [FanModeEnum]",
            "    0x0001 |         | server |           RAM |           |         |                    2 |          1 |                   1 |               65534 |                     => FanModeSequence [FanModeSequenceEnum]",
            "    0x0002 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => PercentSetting [Percent]",
            "    0x0003 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => PercentCurrent [Percent]",
            "    0x0004 |         | server |           RAM |           |         |                   10 |          1 |                   1 |               65534 |                     => SpeedMax [int8u]",
            "    0x0005 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => SpeedSetting [int8u]",
            "    0x0006 |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => SpeedCurrent [int8u]",
            "    0x0007 |         | server |           RAM |           |         |                 0x01 |          1 |                   1 |               65534 |                     => RockSupport [RockBitmap]",
            "    0x0008 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => RockSetting [RockBitmap]",
            "    0x0009 |         | server |           RAM |           |         |                 0x03 |          1 |                   1 |               65534 |                     => WindSupport [WindBitmap]",
            "    0x000a |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => WindSetting [WindBitmap]",
            "    0x000b |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => AirflowDirection [AirflowDirectionEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 3,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 44,
        "profileId": 259,
        "label": "MA-air-quality-sensor",
        "name": "MA-air-quality-sensor"
      },
      "deviceTypes": [
        {
          "code": 44,
          "profileId": 259,
          "label": "MA-air-quality-sensor",
          "name": "MA-air-quality-sensor"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        44
      ],
      "deviceTypeName": "MA-air-quality-sensor",
      "deviceTypeCode": 44,
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
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
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
            "    0x0000 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [single]",
            "    0x0001 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [single]",
            "    0x0002 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [single]",
            "    0x0003 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => PeakMeasuredValue [single]",
            "    0x0004 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => PeakMeasuredValueWindow [elapsed_s]",
            "    0x0005 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AverageMeasuredValue [single]",
            "    0x0006 |         | server |           Ext |           |         |                    1 |          1 |                   1 |               65534 |                     => AverageMeasuredValueWindow [elapsed_s]",
            "    0x0008 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementUnit [MeasurementUnitEnum]",
            "    0x0009 |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => MeasurementMedium [MeasurementMediumEnum]",
            "    0x000a |         | server |           Ext |           |         |                    0 |          1 |                   1 |               65534 |                     => LevelValue [LevelValueEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 4,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 770,
        "profileId": 259,
        "label": "MA-tempsensor",
        "name": "MA-tempsensor"
      },
      "deviceTypes": [
        {
          "code": 770,
          "profileId": 259,
          "label": "MA-tempsensor",
          "name": "MA-tempsensor"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        770
      ],
      "deviceTypeName": "MA-tempsensor",
      "deviceTypeCode": 770,
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
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [int16s]",
            "    0x0001 |         | server |           RAM |           |         |               0x8000 |          1 |                   1 |               65534 |                     => MinMeasuredValue [int16s]",
            "    0x0002 |         | server |           RAM |           |         |               0x8000 |          1 |                   1 |               65534 |                     => MaxMeasuredValue [int16s]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 5,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 775,
        "profileId": 259,
        "label": "MA-humiditysensor",
        "name": "MA-humiditysensor"
      },
      "deviceTypes": [
        {
          "code": 775,
          "profileId": 259,
          "label": "MA-humiditysensor",
          "name": "MA-humiditysensor"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        775
      ],
      "deviceTypeName": "MA-humiditysensor",
      "deviceTypeCode": 775,
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
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MeasuredValue [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MinMeasuredValue [int16u]",
            "    0x0002 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => MaxMeasuredValue [int16u]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    3 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        }
      ]
    },
    {
      "id": 6,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 769,
        "profileId": 259,
        "label": "MA-thermostat",
        "name": "MA-thermostat"
      },
      "deviceTypes": [
        {
          "code": 769,
          "profileId": 259,
          "label": "MA-thermostat",
          "name": "MA-thermostat"
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        769
      ],
      "deviceTypeName": "MA-thermostat",
      "deviceTypeCode": 769,
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
            "    0x0000 |         | server |           RAM |           |         |                  0x0 |          1 |                   1 |               65534 |                     => IdentifyTime [int16u]",
            "    0x0001 |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => IdentifyType [IdentifyTypeEnum]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    0 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    4 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
            "    0xfffd |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
          ]
        },
        {
          "name": "Thermostat",
          "code": "0x0201",
          "define": "THERMOSTAT_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            "   code    | mfgCode |  side  | storageOption | singleton | bounded |     defaultValue     | reportable |     minInterval     |     maxInterval     |   reportableChange",
            "    0x0000 |         | server |           RAM |           |         |                      |          1 |                   1 |               65534 |                     => LocalTemperature [int16s]",
            "    0x0012 |         | server |           RAM |           |         |                 2000 |          1 |                   1 |               65534 |                     => OccupiedHeatingSetpoint [int16s]",
            "    0x0015 |         | server |           RAM |           |         |                 1700 |          1 |                   1 |               65534 |                     => MinHeatSetpointLimit [int16s]",
            "    0x0016 |         | server |           RAM |           |         |                 3000 |          1 |                   1 |               65534 |                     => MaxHeatSetpointLimit [int16s]",
            "    0x001b |         | server |           RAM |           |         |                 0x02 |          1 |                   1 |               65534 |                     => ControlSequenceOfOperation [ThermostatControlSequence]",
            "    0x001c |         | server |           RAM |           |         |                 0x00 |          1 |                   1 |               65534 |                     => SystemMode [enum8]",
            "    0xfff8 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => GeneratedCommandList [array]",
            "    0xfff9 |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AcceptedCommandList [array]",
            "    0xfffa |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => EventList [array]",
            "    0xfffb |         | server |           Ext |           |         |                      |          1 |                   1 |               65534 |                     => AttributeList [array]",
            "    0xfffc |         | server |           RAM |           |         |                    1 |          1 |                   1 |               65534 |                     => FeatureMap [bitmap32]",
            "    0xfffd |         | server |           RAM |           |         |                    5 |          1 |                   1 |               65534 |                     => ClusterRevision [int16u]"
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
    },
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 2,
      "profileId": 259,
      "endpointId": 2,
      "networkId": 0
    },
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 3,
      "profileId": 259,
      "endpointId": 3,
      "networkId": 0
    },
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 4,
      "profileId": 259,
      "endpointId": 4,
      "networkId": 0
    },
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 5,
      "profileId": 259,
      "endpointId": 5,
      "networkId": 0
    }
  ]
}