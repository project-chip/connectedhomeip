{
  "writeTime": "Tue Mar 16 2021 11:47:51 GMT+0100 (Central European Standard Time)",
  "featureLevel": 11,
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
      "path": "..\\..\\..\\..\\third_party\\zap\\repo\\zcl-builtin\\silabs\\zcl.json",
      "version": "ZCL Test Data",
      "type": "zcl-properties"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "..\\..\\..\\..\\third_party\\zap\\repo\\test\\gen-template\\chip\\gen-templates.json",
      "version": "test-v1",
      "type": "gen-templates-json"
    }
  ],
  "endpointTypes": [
    {
      "name": "Anonymous Endpoint Type",
      "deviceTypeName": "HA-pump",
      "deviceTypeCode": 771,
      "deviceTypeProfileId": 260,
      "clusters": [
        {
          "name": "Basic",
          "code": 0,
          "mfgCode": null,
          "define": "BASIC_CLUSTER",
          "side": "client",
          "enabled": 0,
          "commands": [],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Basic",
          "code": 0,
          "mfgCode": null,
          "define": "BASIC_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "ZCL version",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x08",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "power source",
              "code": 7,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 1,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Identify",
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "client",
          "enabled": 1,
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ],
          "commands": [
            {
              "name": "Identify",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 1
            },
            {
              "name": "IdentifyQuery",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 1
            }
          ]
        },
        {
          "name": "Identify",
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "identify time",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0000",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ],
          "commands": [
            {
              "name": "IdentifyQueryResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "incoming": 1,
              "outgoing": 1
            }
          ]
        },
        {
          "name": "Groups",
          "code": 4,
          "mfgCode": null,
          "define": "GROUPS_CLUSTER",
          "side": "client",
          "enabled": 0,
          "commands": [
            {
              "name": "AddGroup",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "ViewGroup",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "GetGroupMembership",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "RemoveGroup",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "RemoveAllGroups",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "AddGroupIfIdentifying",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Groups",
          "code": 4,
          "mfgCode": null,
          "define": "GROUPS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "AddGroupResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "ViewGroupResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "GetGroupMembershipResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "RemoveGroupResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "name support",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Scenes",
          "code": 5,
          "mfgCode": null,
          "define": "SCENES_CLUSTER",
          "side": "client",
          "enabled": 0,
          "commands": [
            {
              "name": "AddScene",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "ViewScene",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "RemoveScene",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "RemoveAllScenes",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "StoreScene",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "RecallScene",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "GetSceneMembership",
              "code": 6,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Scenes",
          "code": 5,
          "mfgCode": null,
          "define": "SCENES_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "AddSceneResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "ViewSceneResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "RemoveSceneResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "RemoveAllScenesResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "StoreSceneResponse",
              "code": 4,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            },
            {
              "name": "GetSceneMembershipResponse",
              "code": 6,
              "mfgCode": null,
              "source": "server",
              "incoming": 0,
              "outgoing": 1
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "3",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "scene count",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "current scene",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "current group",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0000",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "scene valid",
              "code": 3,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "name support",
              "code": 4,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "On/off",
          "code": 6,
          "mfgCode": null,
          "define": "ON_OFF_CLUSTER",
          "side": "client",
          "enabled": 0,
          "commands": [
            {
              "name": "Off",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "On",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            },
            {
              "name": "Toggle",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "incoming": 1,
              "outgoing": 0
            }
          ],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "On/off",
          "code": 6,
          "mfgCode": null,
          "define": "ON_OFF_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "2",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "on/off",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Pump Configuration and Control",
          "code": 512,
          "mfgCode": null,
          "define": "PUMP_CONFIG_CONTROL_CLUSTER",
          "side": "client",
          "enabled": 0,
          "commands": [],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "client",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0001",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Pump Configuration and Control",
          "code": 512,
          "mfgCode": null,
          "define": "PUMP_CONFIG_CONTROL_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [],
          "attributes": [
            {
              "name": "cluster revision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0001",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "max pressure",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "max speed",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "max flow",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "pump status",
              "code": 16,
              "mfgCode": null,
              "side": "server",
              "included": 0,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "effective operation mode",
              "code": 17,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "effective control mode",
              "code": 18,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 0,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "capacity",
              "code": 19,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 1,
              "minInterval": 0,
              "maxInterval": 65344,
              "reportableChange": 0
            },
            {
              "name": "operation mode",
              "code": 32,
              "mfgCode": null,
              "side": "server",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 0,
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
      "profileId": 260,
      "endpointId": 1,
      "networkId": 0
    }
  ]
}