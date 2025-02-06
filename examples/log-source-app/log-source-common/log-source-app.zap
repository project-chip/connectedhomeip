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
      "category": "matter",
      "version": "chip-v1"
    }
  ],
  "endpointTypes": [
    {
      "id": 1,
      "name": "Anonymous Endpoint Type",
      "deviceTypeRef": {
        "code": 4293984259,
        "profileId": 259,
        "label": "MA-all-clusters-app",
        "name": "MA-all-clusters-app",
        "deviceTypeOrder": 0
      },
      "deviceTypes": [
        {
          "code": 4293984259,
          "profileId": 259,
          "label": "MA-all-clusters-app",
          "name": "MA-all-clusters-app",
          "deviceTypeOrder": 0
        }
      ],
      "deviceVersions": [
        1
      ],
      "deviceIdentifiers": [
        4293984259
      ],
      "deviceTypeName": "MA-all-clusters-app",
      "deviceTypeCode": 4293984259,
      "deviceTypeProfileId": 259,
      "clusters": [
        {
          "name": "Access Control",
          "code": 31,
          "mfgCode": null,
          "define": "ACCESS_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "ACL",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "Extension",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "SubjectsPerAccessControlEntry",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "TargetsPerAccessControlEntry",
              "code": 3,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AccessControlEntriesPerFabric",
              "code": 4,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AttributeList",
              "code": 65531,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ],
          "events": [
            {
              "name": "AccessControlEntryChanged",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1
            },
            {
              "name": "AccessControlExtensionChanged",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "included": 1
            }
          ]
        },
        {
          "name": "General Commissioning",
          "code": 48,
          "mfgCode": null,
          "define": "GENERAL_COMMISSIONING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "ArmFailSafe",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ArmFailSafeResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "SetRegulatoryConfig",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "SetRegulatoryConfigResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CommissioningComplete",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CommissioningCompleteResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "Breadcrumb",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "int64u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0000000000000000",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "BasicCommissioningInfo",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "BasicCommissioningInfo",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "SupportsConcurrentConnection",
              "code": 4,
              "mfgCode": null,
              "side": "server",
              "type": "boolean",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "1",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Network Commissioning",
          "code": 49,
          "mfgCode": null,
          "define": "NETWORK_COMMISSIONING_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "ScanNetworks",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ScanNetworksResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddOrUpdateWiFiNetwork",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddOrUpdateThreadNetwork",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveNetwork",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "NetworkConfigResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ConnectNetwork",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ConnectNetworkResponse",
              "code": 7,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ReorderNetwork",
              "code": 8,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "1",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Diagnostic Logs",
          "code": 50,
          "mfgCode": null,
          "define": "DIAGNOSTIC_LOGS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "RetrieveLogsRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RetrieveLogsResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "1",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Operational Credentials",
          "code": 62,
          "mfgCode": null,
          "define": "OPERATIONAL_CREDENTIALS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "AttestationRequest",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AttestationResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CertificateChainRequest",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CertificateChainResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "CSRRequest",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "CSRResponse",
              "code": 5,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddNOC",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "NOCResponse",
              "code": 8,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "UpdateFabricLabel",
              "code": 9,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveFabric",
              "code": 10,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddTrustedRootCertificate",
              "code": 11,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "Fabrics",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "SupportedFabrics",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "CommissionedFabrics",
              "code": 3,
              "mfgCode": null,
              "side": "server",
              "type": "int8u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "TrustedRootCertificates",
              "code": 4,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "1",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        }
      ]
    }
  ],
  "endpoints": [
    {
      "endpointTypeName": "Anonymous Endpoint Type",
      "endpointTypeIndex": 0,
      "profileId": 259,
      "endpointId": 0,
      "networkId": 0,
      "parentEndpointIdentifier": null
    }
  ]
}